#include "malloc.h"

int main()
{
    char *p1 = malloc(15);
    char *p2 = malloc(15);
    char *p3 = malloc(15);
    p3[0] = 'b';
    p1[0] = 'u';
    p2[0] = 'w';
    free(p1);
    free(p2);
    free(p3);
}
