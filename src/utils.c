#include <stdio.h>

#include "malloc.h"

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
