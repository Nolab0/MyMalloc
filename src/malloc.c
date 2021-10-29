#define _GNU_SOURCE

#include "malloc.h"

#include <err.h>
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "utils.h"

struct page *init = NULL;

// Create a page with mmap
static struct page *create_page(size_t size)
{
    size += sizeof(struct page);
    size += (sizeof(struct header) * 2);
    size_t alloc_size = near_size(size, 4096);
    void *alloc = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (alloc == MAP_FAILED)
    {
        return NULL;
    }
    struct page *new_page = alloc;
    new_page->page_size = alloc_size;
    new_page->next = NULL;
    // initialisation of the first header
    void *header_ptr = new_page + 1;
    struct header *first_header = header_ptr;
    first_header->size = alloc_size - sizeof(struct page);
    first_header->free = 1;
    first_header->next = NULL;
    first_header->prev = NULL;
    new_page->meta = first_header;
    return new_page;
}

// Return a page of size multiple of 4096 on the first call and the first page
// otherwise
static struct page *get_first(size_t size)
{
    if (init == NULL)
        init = create_page(size);
    return init;
}

// Find a free block in a page, return it if find else return NULL
static struct header *find_block(size_t size)
{
    struct page *p = get_first(0);
    size_t data_size = near_size(size, 16);
    while (p != NULL)
    {
        struct header *header = p->meta;
        while (header != NULL)
        {
            if (header->free && header->size >= size)
            {
                char *exit =
                    (char *)header + 2 * sizeof(struct header) + data_size;
                char *endPage = (char *)p + p->page_size; // End of the page
                if (exit < endPage)
                {
                    return header;
                }
            }
            header = header->next;
        }
        p = p->next;
    }
    return NULL; // Not found, need to expand page
}

// Split a block with a free part and return it
static struct header *split_block(struct header *new, size_t size)
{
    size_t data_size = near_size(size, 16);
    size_t save_size = new->size;
    new->size = data_size;
    new->free = 0;
    // Split with the used and free part
    char *new_adr = (char *)new;
    char *next_ptr = new_adr + sizeof(struct header) + data_size;
    struct header *next_header = (struct header *)next_ptr;
    next_header->free = 1;
    next_header->size = save_size - sizeof(struct header) - data_size;
    next_header->next = new->next;
    new->next = next_header;
    next_header->prev = new;
    return new;
}

__attribute__((visibility("default"))) void *malloc(size_t size)
{
    if (size == 0)
        return NULL;
    size_t alloc_size = near_size(size, 16);
    struct page *init = get_first(alloc_size);
    if (init == NULL)
        return NULL;
    struct header *new = find_block(alloc_size);
    if (new
        != NULL &&new->size
            > alloc_size + sizeof(struct header)) // Block larger than necessary
    {
        new = split_block(new, size);
        return get_data(new);
    }
    else if (new != NULL) // Find a convinient block
    {
        new->free = 0;
        return get_data(new);
    }
    else
    {
        struct page *new_page = create_page(alloc_size);
        struct page *save_init = init;
        while (save_init->next != NULL)
            save_init = save_init->next;
        save_init->next = new_page; // Link the new page
        new = new_page->meta;
        new = split_block(new, size);
        return get_data(new);
    }
    return NULL;
}

// Return wether a page is empty or not
static int is_empty(struct page *page)
{
    struct header *header = page->meta;
    while (header != NULL)
    {
        if (!header->free)
            return 0;
        header = header->next;
    }
    return 1;
}

// Remove empty page in the linked list
static void remove_empty_pages(void)
{
    struct page *current = get_first(0);
    if (current == NULL)
        return;
    current = current->next;
    struct page *prev = get_first(0);
    while (current != NULL)
    {
        current = prev->next;
        if (current != NULL && is_empty(current)) // Empty page
        {
            struct page *save_next = current->next;
            int error = munmap(current, current->page_size);
            if (error == -1)
                errx(1, "free: failed to unmap page");
            prev->next = save_next;
        }
        else
            prev = prev->next;
    }
    prev = get_first(0);
    if (is_empty(prev))
    {
        struct page *save = prev->next;
        int error = munmap(prev, prev->page_size);
        if (error == -1)
            errx(1, "free: failed to unmap page");
        init = save;
    }
}

__attribute__((visibility("default"))) void free(void *ptr)
{
    if (ptr == NULL)
        return;
    struct header *header = ptr;
    header = header - 1;
    header->free = 1;
    if (header->prev && header->prev->free == 1)
    {
        header->prev->next = header->next;
        header->prev->size += header->size;
        header = header->prev;
        if (header->next)
            header->next->prev = header;
    }
    if (header->next && header->next->free == 1)
    {
        header->size += header->next->size;
        header->next = header->next->next;
    }
    remove_empty_pages();
}

__attribute__((visibility("default"))) void *realloc(void *ptr, size_t size)
{
    if (ptr == NULL)
        return malloc(size);
    if (size == 0)
    {
        free(ptr);
        return NULL;
    }
    struct header *header = ptr;
    header = header - 1;
    if (size <= header->size) // Remain space in current header
    {
        return get_data(header);
    }
    else if (header->next && header->next->free == 1)
    {
        header->size += header->next->size;
        header->next = header->next->next;
        return get_data(header);
    }
    else
    {
        void *new_alloc = malloc(size);
        void *data_zone = get_data(header);
        mempcpy(new_alloc, data_zone, header->size);
        header->free = 1;
        return new_alloc;
    }
    return NULL;
}

__attribute__((visibility("default"))) void *calloc(size_t nmemb, size_t size)
{
    if (nmemb == 0 || size == 0)
        return NULL;
    size_t res = 0;
    if (__builtin_umull_overflow(nmemb, size, &res))
    {
        return NULL;
    }
    else
    {
        void *alloc = malloc(res);
        char *set = alloc;
        for (size_t i = 0; i < res; i++)
            set[i] = 0;
        return alloc;
    }
}
