#include "beware_overflow.h"

void *beware_overflow(void *ptr, size_t nmemb, size_t size)
{
    size_t res = 0;
    int over = __builtin_umull_overflow(nmemb, size, &res);
    if (over)
        return NULL;
    else
    {
        char *tmp = ptr;
        tmp += res;
        void *res_ptr = tmp;
        return res_ptr;
    }
}
