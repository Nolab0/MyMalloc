#include <stdio.h>

#include "malloc.h"

void print_adr(struct page *page)
{
    printf("%d\n", sizeof(struct page));
    printf("%d\n", sizeof(struct header));
    int i = 1;
    if (page == NULL)
        printf("Empty page\n");
    while (page != NULL)
    {
        printf("---Page number: %d----\n", i);
        printf("Begining of the page: %p\n", page);
        printf("Size of the page: %lu\n", page->page_size);
        printf("End of the page: %p\n", page + page->page_size);
        struct header *header = page->meta;
        int j = 1;
        while (header != NULL)
        {
            printf("Header number: %d\n", j);
            printf("Previous header: %p\n", header->prev);
            printf("Free: %d\n", header->free);
            printf("Start of the header: %p\n", header);
            printf("Size of the header: %d\n", header->size);
            printf("Start of Data zone: %p\n", header + 1);
            printf("End of Data zone: %p\n\n", header + 1 + header->size);
            header = header->next;
            j++;
        }
        page = page->next;
        i++;
    }
}

int near_size(int nb, int size)
{
    while (nb % size != 0)
        nb++;
    return nb;
}

void *get_data(struct header *header)
{
    return header + 1;
}
