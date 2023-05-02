/* Arbitrary precision integer functions. */

#ifndef _BIGNUM_H_
#define _BIGNUM_H_

#ifndef __KERNEL__
#include <stddef.h>
#else
#include <linux/types.h>
#endif

#ifndef DIGIT_SIZE
#if defined(__LP64__) || defined(__x86_64__) || defined(__amd64__) || \
    defined(__aarch64__)
#define DIGIT_SIZE 8
#else
#define DIGIT_SIZE 4
#endif
#endif

#if DIGIT_SIZE == 4
typedef unsigned int digit_t;
typedef long long c_digit_t;
typedef unsigned long long uc_digit_t;
#define DIGIT_BITS 32
#define DIGIT_HSHIFT 16U
#define DIGIT_MAX 0xFFFFFFFFU

#elif DIGIT_SIZE == 8
typedef unsigned long long digit_t;
typedef __int128_t c_digit_t;
typedef __uint128_t uc_digit_t;
#define DIGIT_BITS 64
#define DIGIT_HSHIFT 32U
#define DIGIT_MAX 0xFFFFFFFFFFFFFFFFU

#else
#error "DIGIT_SIZE must be 4 or 8"
#endif


typedef struct {
    digit_t *number;   /* Digits of number. */
    unsigned int size; /* Length of number. */
    unsigned sign;     /* Sign bit. */
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
void bn_mult(const bn *a, const bn *b, bn *c);

void bn_sqr(const bn *a, bn *c);

void bn_karatsuba(const bn *a, const bn *b, bn *c);

void bn_cpy(bn *dest, const bn *src);

void bn_resize(bn *num, int n);

char *bn_to_string(bn *src);


#endif /* !_BIGNUM_H_ */