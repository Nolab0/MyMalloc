#ifndef UTILS_H
#define UTILS_H

/* Utilitaries for memory allocation */
#include "malloc.h"

// Return the first multiple of size
int near_size(int nb, int size);

// Get the data zone starting with a header
void *get_data(struct header *header);

#endif /* UTILS_H  */
