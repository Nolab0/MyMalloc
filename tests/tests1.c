#include "malloc.h"

int main()
{
    char *c2 = malloc(500);
    char *c3 = malloc(5000);
    free(c2);
    free(c3);
}
