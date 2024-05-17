/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * md5.c - MD5 Message Digest Algorithm
 *
 * Copyright (C) 2007-2008, David Beckett http://www.dajobe.org/
 * 
 * This file is licensed under the following three licenses as alternatives:
 *   1. GNU Lesser General Public License (LGPL) V2.1 or any newer version
 *   2. GNU General Public License (GPL) V2 or any newer version
 *   3. Apache License, V2.0 or any newer version
 * 
 * You may not use this file except in compliance with at least one of
 * the above three licenses.
 * 
 * See LICENSE.html or LICENSE.txt at the top of this package for the
 * complete terms and further detail along with the license texts for
 * the licenses in COPYING.LIB, COPYING and LICENSE-2.0.txt respectively.
 * 
 * 
 */


/* Original code notes: */

/* An independent implementation of MD5 by Colin Plumb. */

/* Original copyright message: */

/*
 * This code implements the MD5 message-digest algorithm.  To compute
 * the message digest of a chunk of bytes, declare an MD5Context
 * structure, pass it to MD5Init, call MD5Update as needed on
 * buffers full of bytes, and then call MD5Final, which will fill a
 * supplied 16-byte array with the digest.
 *
 * Equivalent code is available from RSA Data Security, Inc.  This code
 * has been tested against that, and is equivalent, except that you
 * don't need to include two pages of legalese with every copy.
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WIN32
#include <win32_flickcurl_config.h>
#endif

#include <flickcurl.h>
#include <flickcurl_internal.h>


#define MD5_DIGEST_LEN 16


/* original code from header - function names have changed */

/* use unions to make compiler align uint32_t* and unsigned char buffers */
struct MD5Context {
  union 
  {
    unsigned char c[128];
    uint32_t w[32];
  } buf;
  uint32_t bits[2];
  union {
    unsigned char c[64];
    uint32_t w[16];
  } in;
  unsigned char digest[MD5_DIGEST_LEN];
};

static void MD5Init(struct MD5Context *context);
static void MD5Update(struct MD5Context *context, 
                      const unsigned char *buf,
                      size_t len);
static void MD5Final(struct MD5Context *context);
static void MD5Transform(uint32_t buf[4], uint32_t const in[MD5_DIGEST_LEN]);


/* original code from C file - GNU configurised */

#ifndef WORDS_BIGENDIAN
#define byteReverse(buf, len)	/* Nothing */
#else
static void byteReverse(unsigned char *buf, unsigned longs);

/* ASM ifdef removed */

/*
 * Note: this code is harmless on little-endian machines.
 */
static void byteReverse(unsigned char *buf, unsigned longs)
{
  uint32_t t;
  do {
    t = (uint32_t) ((unsigned) buf[3] << 8 | buf[2]) << 16 |
      ((unsigned) buf[1] << 8 | buf[0]);
    *(uint32_t *) buf = t;
    buf += 4;
  } while (--longs);
}
#endif /* WORDS_BIGENDIAN */


/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
static void MD5Init(struct MD5Context *ctx)
{
  ctx->buf.w[0] = 0x67452301;
  ctx->buf.w[1] = 0xefcdab89;
  ctx->buf.w[2] = 0x98badcfe;
  ctx->buf.w[3] = 0x10325476;

  ctx->bits[0] = 0;
  ctx->bits[1] = 0;
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
static void MD5Update(struct MD5Context *ctx,
                      const unsigned char* buf,
                      size_t len)
{
  uint32_t t;
  uint32_t ulen = (uint32_t)len;

  /* Update bitcount */
  
  t = ctx->bits[0];
  if ((ctx->bits[0] = t + (ulen << 3)) < t)
    ctx->bits[1]++;		/* Carry from low to high */
  ctx->bits[1] += ulen >> 29;
  
  t = (t >> 3) & 0x3f;	/* Bytes already in shsInfo->data */
  
  /* Handle any leading odd-sized chunks */
  
  if (t) {
    unsigned char *p = ctx->in.c + t;
    
    t = 64 - t;
    if (len < t) {
      memcpy(p, buf, len);
      return;
    }
    memcpy(p, buf, t);
    byteReverse(ctx->in.c, 16);
    MD5Transform(ctx->buf.w, ctx->in.w);
    buf += t;
    len -= t;
  }

  /* Process data in 64-byte chunks */

  while (len >= 64) {
    memcpy(ctx->in.c, buf, 64);
    byteReverse(ctx->in.c, 16);
    MD5Transform(ctx->buf.w, ctx->in.w);
    buf += 64;
    len -= 64;
  }

  /* Handle any remaining bytes of data. */
  
  memcpy(ctx->in.c, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern 
 * 1 0* (64-bit count of bits processed, MSB-first)
 */

/* Interface altered by DJB to write digest into pre-allocated context */
static void MD5Final(struct MD5Context *ctx)
{
  unsigned count;
  unsigned char *p;

  /* Compute number of bytes mod 64 */
  count = (ctx->bits[0] >> 3) & 0x3F;

  /* Set the first char of padding to 0x80.  This is safe since there is
     always at least one byte free */
  p = ctx->in.c + count;
  *p++ = 0x80;
  
  /* Bytes of padding needed to make 64 bytes */
  count = 64 - 1 - count;
  
  /* Pad out to 56 mod 64 */
  if (count < 8) {
    /* Two lots of padding:  Pad the first block to 64 bytes */
    memset(p, 0, count);
    byteReverse(ctx->in.c, 16);
    MD5Transform(ctx->buf.w, ctx->in.w);
    
    /* Now fill the next block with 56 bytes */
    memset(ctx->in.c, 0, 56);
  } else {
    /* Pad block to 56 bytes */
    memset(p, 0, count - 8);
  }
  byteReverse(ctx->in.c, 14);
  
  /* Append length in bits and transform */
  (ctx->in.w)[14] = ctx->bits[0];
  (ctx->in.w)[15] = ctx->bits[1];

  MD5Transform(ctx->buf.w, ctx->in.w);
  byteReverse(ctx->buf.c, 4);
  memcpy(ctx->digest, ctx->buf.c, MD5_DIGEST_LEN);
}


/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define MD5STEP(f, w, x, y, z, data, s) \
	( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
static void MD5Transform(uint32_t buf[4], uint32_t const in[MD5_DIGEST_LEN])
{
  register uint32_t a, b, c, d;
  
  a = buf[0];
  b = buf[1];
  c = buf[2];
  d = buf[3];
  
  MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
  MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
  MD5STEP(F1, c, d, a, b, in[2] + 0x242070db, 17);
  MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
  MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
  MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
  MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613, 17);
  MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501, 22);
  MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8, 7);
  MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
  MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
  MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
  MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122, 7);
  MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
  MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
  MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);
  
  MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
  MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340, 9);
  MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
  MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
  MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
  MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
  MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
  MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
  MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
  MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
  MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
  MD5STEP(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
  MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
  MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
  MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
  MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

  MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
  MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681, 11);
  MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
  MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
  MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
  MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
  MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
  MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
  MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
  MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
  MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
  MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05, 23);
  MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
  MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
  MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
  MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);
  
  MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244, 6);
  MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97, 10);
  MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
  MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
  MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
  MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
  MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
  MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
  MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
  MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
  MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314, 15);
  MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
  MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
  MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
  MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
  MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391, 21);
  
  buf[0] += a;
  buf[1] += b;
  buf[2] += c;
  buf[3] += d;
}


/* my code from here */

char* MD5_string(char *string);


char*
MD5_string(char *string)
{
  struct MD5Context md5;
  size_t len = strlen(string);
  char* b = NULL;
  
  int i;

  MD5Init(&md5);
  MD5Update(&md5, (const unsigned char*)string, len);
  MD5Final(&md5);
  
  b = (char*)malloc(1 + (MD5_DIGEST_LEN << 1));
  if(!b)
    return NULL;
  
  for(i = 0; i < MD5_DIGEST_LEN; i++)
    sprintf(b+(i<<1), "%02x", (unsigned int)md5.digest[i]);
  b[i<<1] = '\0';
  
  return b;
}
