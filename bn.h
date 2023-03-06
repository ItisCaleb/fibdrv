/* Arbitrary precision integer functions. */

#ifndef _BIGNUM_H_
#define _BIGNUM_H_

#include <linux/types.h>

typedef struct {
    unsigned int *number; /* Digits of number. */
    unsigned int size;    /* Length of number. */
    unsigned sign;        /* Sign bit. */
} bn;

bn *bn_alloc(size_t size);

void bn_free(bn *p);

#define bn_is_zero(n) ((n)->size == 0)
void bn_swap(bn *a, bn *b);

void bn_lshift(const bn *p, size_t bits, bn *q);

/* S = A + B */
void bn_add(bn *a, bn *b, bn *s);

void bn_sub(bn *a, const bn *b, bn *c);

/* P = A * B */
void bn_mult(const bn *a, const bn *b, bn *p);

void bn_cpy(bn *dest, const bn *src);

char *bn_to_string(bn *src);


#endif /* !_BIGNUM_H_ */