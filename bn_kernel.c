#include <linux/slab.h>
#include "bn.h"


bn *bn_alloc(size_t size)
{
    bn *num = kmalloc(sizeof(bn), GFP_KERNEL);
    num->number = kmalloc(sizeof(u64) * size, GFP_KERNEL);
    memset(num->number, 0, sizeof(u64) * size);
    num->size = size;
    num->sign = 0;
    return num;
}


void bn_resize(bn *num, int n)
{
    if (n == num->size)
        return;
    num->number = krealloc(num->number, sizeof(u64) * n, GFP_KERNEL);
    if (n > num->size)
        memset(num->number + num->size, 0, sizeof(u64) * (n - num->size));
    num->size = n;
}

void bn_free(bn *p)
{
    if (p == NULL)
        return;
    kfree(p->number);
    kfree(p);
}

char *bn_to_string(bn *src)
{
    // log10(x) = log2(x) / log2(10) ~= log2(x) / 3.322
    size_t len = (8 * sizeof(u64) * src->size) / 3 + 2 + src->sign;
    char *s = kmalloc(len, GFP_KERNEL);
    char *p = s;

    memset(s, '0', len - 1);
    s[len - 1] = '\0';
    for (int i = src->size - 1; i >= 0; i--) {
        for (u64 d = 1UL << 63; d; d >>= 1) {
            /* binary -> decimal string */
            int carry = ((d & src->number[i]) != 0);
            for (int j = len - 2; j >= 0; j--) {
                s[j] += s[j] - '0' + carry;  // double it
                carry = (s[j] > '9');
                if (carry)
                    s[j] -= 10;
            }
        }
    }
    // skip leading zero
    while (p[0] == '0' && p[1] != '\0') {
        p++;
    }
    if (src->sign)
        *(--p) = '-';
    memmove(s, p, strlen(p) + 1);
    return s;
}
/* copy b to a */
void bn_cpy(bn *dest, const bn *src)
{
    bn_resize(dest, src->size);
    memcpy(dest->number, src->number, sizeof(u64) * src->size);
    dest->sign = src->sign;
}
