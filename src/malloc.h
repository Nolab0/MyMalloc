#ifndef MALLOC_H
#define MALLOC_H

#include <stddef.h>

struct page
{
    struct header *meta; // Ptr to the first header of the page

    size_t page_size; // Size of page

    struct page *next;

    size_t align; // To align on 16 bytes
};

// Need to reduce the field to optimize space
struct header
{
    struct header *next; // Next header

    struct header *prev; // Previous header

    size_t size; // Size of data zone

    int free; // 1 if free 0 otherwise
};

void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t number, size_t size);
void *realloc(void *ptr, size_t size);

extern struct page *init;

#endif
