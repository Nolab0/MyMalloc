#define _GNU_SOURCE

#include "malloc.h"

#include <stddef.h>
#include <sys/mman.h>
#include <string.h>
#include <err.h>

struct page *init = NULL;

// Return the first multiple of size
static int near_size(int nb, int size)
{
    while (nb % size != 0)
        nb++;
    return nb;
}

struct page *create_page(size_t size)
{
    size += sizeof(struct page);
    size += (sizeof(struct header) * 2);
    size_t alloc_size = near_size(size, 4096);
    void *alloc = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (alloc == MAP_FAILED)
        return NULL;
    struct page *new_page = alloc;
    new_page->page_size = alloc_size;
    new_page->next = NULL;
    // initialisation of the first header
    void *header_ptr = new_page + 1;
    struct header *first_header = header_ptr;
    first_header->size = alloc_size - sizeof(struct page);
    first_header->used_size = 0;
    first_header->free = 1;
    first_header->data = first_header + 1; // get the data
    first_header->next = NULL;
    new_page->meta = first_header;
    return new_page;
}

// Return a page of size multiple of 4096 on the first call and the first page otherwise
struct page *get_first(size_t size)
{
    if (init == NULL)
        init = create_page(size);
    return init;
}

// Find a free block in a page, return it if find else return NULL
static struct header *find_block(size_t size)
{
    struct page *p = get_first(near_size(size, 16));
    while (p != NULL)
    {
        struct header *header = p->meta;
        while (header != NULL)
        {
            if (header->free && header->size >= size)
                return header;
            header = header->next;
        }
        p = p->next;
    }
    return NULL; // Not found, need to expand page
}

// Split a block with a free part and return it
struct header *split_block(struct header *new, size_t size)
{
    size_t data_size = near_size(size, 16);
    size_t save_size = new->size;
    new->size = data_size;
    new->used_size = size;
    new->free = 0;
    new->data = new + 1;
    // Split with the used and free part
    char *new_adr = (char *)new;
    char *next_ptr = new_adr + sizeof(struct header) + data_size;
    struct header *next_header = (struct header *)next_ptr;
    next_header->free = 1;
    next_header->size = save_size - sizeof(struct header) - data_size;
    next_header->used_size = 0;
    next_header->next = new->next;
    new->next = next_header;
    char *data_ptr = next_ptr + sizeof(struct header);
    next_header->data = data_ptr;
    return new;
}

__attribute__((visibility("default")))
void *malloc(size_t size)
{
    if (size == 0)
        return NULL;
    size_t alloc_size = near_size(size, 16);
    struct page *init = get_first(alloc_size);
    if (init == NULL)
        return NULL;
    struct header *new = find_block(alloc_size);
    if (new != NULL && new->size > alloc_size + sizeof(struct header)) // Block larger than necessary
    {
        new = split_block(new, size);
        //print_adr(init);
        return new->data;
    }
    else if (new != NULL) // Find a convinient block
    {
        new->used_size = size;
        new->free = 0;
        //print_adr(init);
        return new->data;
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
        //print_adr(init);
        return new->data;
    }
    return NULL;
}

__attribute__((visibility("default")))
void free(void *ptr)
{
    if (ptr == NULL)
        return;
    struct header *header = ptr;
    header = header - 1;
    header->free = 1;
    header->used_size = 0;
}
/*
   __attribute__((visibility("default")))
   void *realloc(void *ptr, size_t size)
   {
   return NULL;
   }
   */

__attribute__((visibility("default")))
void *calloc(size_t nmemb, size_t size)
{
    if (nmemb == 0 || size == 0)
        return NULL;
    size_t res = 0;
    int over = __builtin_umull_overflow(nmemb, size, &res);
    if (over)
        errx(1, "calloc: overflow detected");
    else
    {
        void *alloc = malloc(res);
        char *set = alloc;
        for (size_t i = 0; i < res; i++)
            set[i] = 0;
        return alloc;
    }
}