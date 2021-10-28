#ifndef MALLOC_H
#define MALLOC_H

#include <stddef.h>

struct page
{
    struct header *meta; // Ptr to the first header of the page

    size_t page_size; // Size of page

    struct page *next;
};

struct header
{
    struct header *next; // Next header

    struct header *prev; // Previous header

    size_t size; // Size of data zone

    size_t used_size; // Size asked by the user

    size_t free; // 1 if free 0 otherwise

    void *data; // Ptr to the data
};

void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t number, size_t size);
void *realloc(void *ptr, size_t size);

void print_adr(struct page *page);

extern struct page *init;

#endif
