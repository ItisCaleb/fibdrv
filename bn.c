#include "bn.h"

#define DIV_ROUNDUP(x, len) (((x) + (len) -1) / (len))
#define MAX(a, b) ((a) > (b)) ? (a) : (b)
#ifndef SWAP
#define SWAP(x, y)           \
    do {                     \
        typeof(x) __tmp = x; \
        x = y;               \
        y = __tmp;           \
    } while (0)
#endif


static int bn_clz(const bn *src)
{
    int cnt = 0;
    for (int i = src->size - 1; i >= 0; i--) {
        if (src->number[i]) {
// prevent undefined behavior when src = 0
#if DIGIT_SIZE == 4
            cnt += __builtin_clz(src->number[i]);
#elif DIGIT_SIZE == 8
            cnt += __builtin_clzl(src->number[i]);
#endif
            return cnt;
        } else {
            cnt += DIGIT_BITS;
        }
    }
    return cnt;
}

/* count the digits of most significant bit */
static int bn_msb(const bn *src)
{
    return src->size * DIGIT_BITS - bn_clz(src);
}


static void bn_do_add(const bn *a, const bn *b, bn *c)
{
    // max digits = max(sizeof(a) + sizeof(b)) + 1
    int d = MAX(bn_msb(a), bn_msb(b)) + 1;
    d = DIV_ROUNDUP(d, DIGIT_BITS) + !d;
    bn_resize(c, d);  // round up, min size = 1

    uc_digit_t carry = 0;
    for (int i = 0; i < c->size; i++) {
        digit_t tmp1 = (i < a->size) ? a->number[i] : 0;
        digit_t tmp2 = (i < b->size) ? b->number[i] : 0;
        carry += (uc_digit_t) tmp1 + tmp2;
        c->number[i] = carry;
        carry >>= DIGIT_BITS;
    }

    if (!c->number[c->size - 1] && c->size > 1)
        bn_resize(c, c->size - 1);
}

static void bn_do_sub(const bn *a, const bn *b, bn *c)
{
    // max digits = max(sizeof(a) + sizeof(b))
    int d = MAX(a->size, b->size);
    bn_resize(c, d);
    c_digit_t carry = 0;
    for (int i = 0; i < c->size; i++) {
        digit_t tmp1 = (i < a->size) ? a->number[i] : 0;
        digit_t tmp2 = (i < b->size) ? b->number[i] : 0;
        carry = (c_digit_t) tmp1 - tmp2 - carry;
        if (carry < 0) {
            c_digit_t t = 1;
            c->number[i] = carry + (t << DIGIT_BITS);
            carry = 1;
        } else {
            c->number[i] = carry;
            carry = 0;
        }
    }

    d = bn_clz(c) / DIGIT_BITS;
    if (d == c->size)
        --d;
    bn_resize(c, c->size - d);
}

static int bn_cmp(const bn *a, const bn *b)
{
    if (a->size > b->size)
        return 1;
    else if (a->size < b->size)
        return -1;
    else {
        for (int i = a->size - 1; i >= 0; i--) {
            if (a->number[i] > b->number[i])
                return 1;
            else if (a->number[i] < b->number[i])
                return -1;
        }
        return 0;
    }
}

/* c = a + b
 * Note: work for c == a or c == b
 */
void bn_add(bn *a, bn *b, bn *c)
{
    if (a->sign == b->sign) {  // both positive or negative
        bn_do_add(a, b, c);
        c->sign = a->sign;
    } else {          // different sign
        if (a->sign)  // let a > 0, b < 0
            SWAP(a, b);
        int cmp = bn_cmp(a, b);
        if (cmp > 0) {
            /* |a| > |b| and b < 0, hence c = a - |b| */
            bn_do_sub(a, b, c);
            c->sign = 0;
        } else if (cmp < 0) {
            /* |a| < |b| and b < 0, hence c = -(|b| - |a|) */
            bn_do_sub(b, a, c);
            c->sign = 1;
        } else {
            /* |a| == |b| */
            bn_resize(c, 1);
            c->number[0] = 0;
            c->sign = 0;
        }
    }
}

/* c = a - b
 * Note: work for c == a or c == b
 */
void bn_sub(bn *a, const bn *b, bn *c)
{
    /* xor the sign bit of b and let bn_add handle it */
    bn tmp = *b;
    tmp.sign ^= 1;  // a - b = a + (-b)
    bn_add(a, &tmp, c);
}
/* c += x, starting from offset */
static void bn_mult_add(bn *c, int offset, uc_digit_t x)
{
    uc_digit_t carry = 0;
    for (int i = offset; i < c->size; i++) {
        carry += c->number[i] + (x & DIGIT_MAX);
        c->number[i] = carry;
        carry >>= DIGIT_BITS;
        x >>= DIGIT_BITS;
        if (!x && !carry)  // done
            return;
    }
}

void bn_mult(const bn *a, const bn *b, bn *c)
{
    // max digits = sizeof(a) + sizeof(b))
    int d = bn_msb(a) + bn_msb(b);
    d = DIV_ROUNDUP(d, DIGIT_BITS) + !d;  // round up, min size = 1
    bn *tmp;
    /* make it work properly when c == a or c == b */
    if (c == a || c == b) {
        tmp = c;  // save c
        c = bn_alloc(d);
    } else {
        tmp = NULL;
        for (int i = 0; i < c->size; i++)
            c->number[i] = 0;
        bn_resize(c, d);
    }

    for (int i = 0; i < a->size; i++) {
        for (int j = 0; j < b->size; j++) {
            uc_digit_t carry = 0;
            carry = (uc_digit_t) a->number[i] * b->number[j];
            bn_mult_add(c, i + j, carry);
        }
    }
    c->sign = a->sign ^ b->sign;

    if (tmp) {
        bn_swap(tmp, c);  // restore c
        bn_free(c);
    }
}

void bn_lshift(const bn *src, size_t shift, bn *dest)
{
    size_t z = bn_clz(src);
    shift &= (DIGIT_BITS - 1);  // only handle shift within 64 bits atm
    if (!shift)
        return;

    if (shift > z) {
        bn_resize(dest, src->size + 1);
    } else {
        bn_resize(dest, src->size);
    }
    /* bit shift */
    for (int i = src->size - 1; i > 0; i--)
        dest->number[i] = src->number[i] << shift |
                          src->number[i - 1] >> (DIGIT_BITS - shift);
    dest->number[0] = src->number[0] << shift;
}

void bn_swap(bn *a, bn *b)
{
    bn tmp = *a;
    *a = *b;
    *b = tmp;
}
