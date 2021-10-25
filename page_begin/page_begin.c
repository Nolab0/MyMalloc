#include "page_begin.h"

void *page_begin(void *ptr, size_t page_size)
{
    unsigned long tmp = (unsigned long) ptr;
    size_t dec = 0;
    while (page_size != 1)
    {
        dec++;
        page_size >>= 1;
    }
    size_t wise = 1;
    wise <<= dec;
    wise -= 1;
    unsigned long res = tmp - (tmp & wise);
    return (void *) res;
}
