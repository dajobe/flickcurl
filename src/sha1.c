/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * sha1.c - SHA1 Message Digest Algorithm and HMAC-SHA1
 *
 * Copyright (C) 2011, David Beckett http://www.dajobe.org/
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
 * Based on the public domain version with the credits below.
 * 
 */


/*
SHA-1 in C
By Steve Reid <sreid@sea-to-sky.net>
100% Public Domain

-----------------
Modified 7/98 
By James H. Brown <jbrown@burgoyne.com>
Still 100% Public Domain

Corrected a problem which generated improper hash values on 16 bit machines
Routine SHA1Update changed from
  void SHA1Update(SHA1_CTX* context, unsigned char* data, unsigned int len)
to
  void SHA1Update(SHA1_CTX* context, unsigned char* data, unsigned long len)

The 'len' parameter was declared an int which works fine on 32 bit
machines.  However, on 16 bit machines an int is too small for the
shifts being done against it.  This caused the hash function to
generate incorrect values if len was greater than 8191 (8K - 1) due
to the 'len << 3' on line 3 of SHA1Update().

Since the file IO in main() reads 16K at a time, any file 8K or
larger would be guaranteed to generate the wrong hash (e.g. Test
Vector #3, a million "a"s).

I also changed the declaration of variables i & j in SHA1Update to
unsigned long from unsigned int for the same reason.

These changes should make no difference to any 32 bit implementations
since an int and a long are the same size in those environments.

--
I also corrected a few compiler warnings generated by Borland C.
1. Added #include <process.h> for exit() prototype
2. Removed unused variable 'j' in SHA1Final
3. Changed exit(0) to return(0) at end of main.

ALL changes I made can be located by searching for comments containing 'JHB'
-----------------
Modified 8/98
By Steve Reid <sreid@sea-to-sky.net>
Still 100% public domain

1- Removed #include <process.h> and used return() instead of exit()
2- Fixed overwriting of finalcount in SHA1Final() (discovered by Chris Hall)
3- Changed email address from steve@edmweb.com to sreid@sea-to-sky.net
*/

/*
Test Vectors (from FIPS PUB 180-1)
"abc"
  A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D
"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
  84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1
A million repetitions of "a"
  34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WIN32
#include <win32_flickcurl_config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>

#include <flickcurl.h>
#include <flickcurl_internal.h>


#define u32 uint32_t


/* #define SHA1HANDSOFF * Copies data before messing with it. */

/* Using return() instead of exit() - SWR */

typedef struct {
  u32 state[5];
  u32 count[2];
  unsigned char buffer[64];
  unsigned char digest[SHA1_DIGEST_LENGTH];
} SHA1Context;


static void SHA1Transform(u32 state[5], const unsigned char buffer[64]);
static void SHA1Init(SHA1Context* context);
static void SHA1Update(SHA1Context* context, const unsigned char* data, size_t len);	/* JHB */
static void SHA1Final(SHA1Context* context);


#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* blk0() and blk() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */
#ifdef WORDS_BIGENDIAN
#define blk0(i) block->l[i]
#else
#define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00) \
    |(rol(block->l[i],8)&0x00FF00FF))
#endif
#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
    ^block->l[(i+2)&15]^block->l[i&15],1))


/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);


/* Hash a single 512-bit block. This is the core of the algorithm. */

static void
SHA1Transform(u32 state[5], const unsigned char buffer[64])
{
  u32 a, b, c, d, e;
  typedef union {
    unsigned char c[64];
    u32 l[16];
  } CHAR64LONG16;
  CHAR64LONG16* block;
#ifdef SHA1HANDSOFF
  static unsigned char workspace[64];
  block = (CHAR64LONG16*)workspace;
  memcpy(block, buffer, 64);
#else
  block = (CHAR64LONG16*)buffer;
#endif

  /* Copy context->state[] to working vars */
  a = state[0];
  b = state[1];
  c = state[2];
  d = state[3];
  e = state[4];

  /* 4 rounds of 20 operations each. Loop unrolled. */
  R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
  R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
  R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
  R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
  R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
  R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
  R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
  R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
  R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
  R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
  R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
  R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
  R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
  R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
  R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
  R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
  R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
  R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
  R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
  R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);
  
  /* Add the working vars back into context.state[] */
  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
  state[4] += e;
  /* Wipe variables */
  /* a = b = c = d = e = 0; */ /* CLANG: useless */
}


/* SHA1Init - Initialize new context */
static void
SHA1Init(SHA1Context* context)
{
  /* SHA1 initialization constants */
  context->state[0] = 0x67452301;
  context->state[1] = 0xEFCDAB89;
  context->state[2] = 0x98BADCFE;
  context->state[3] = 0x10325476;
  context->state[4] = 0xC3D2E1F0;
  context->count[0] = context->count[1] = 0;
}


/* Run your data through this. */

static void
SHA1Update(SHA1Context* context, const unsigned char* data, size_t len)	/* JHB */
{
  u32 i, j;	/* JHB */

  j = (context->count[0] >> 3) & 63;
  if ((context->count[0] += len << 3) < (len << 3))
    context->count[1]++;

  context->count[1] += (len >> 29);
  if ((j + len) > 63) {
    memcpy(&context->buffer[j], data, (i = 64-j));
    SHA1Transform(context->state, context->buffer);
    for ( ; i + 63 < len; i += 64) {
      SHA1Transform(context->state, &data[i]);
    }
    j = 0;
  }
  else
    i = 0;
  memcpy(&context->buffer[j], &data[i], len - i);
}


/* Add padding and return the message digest. */

static void
SHA1Final(SHA1Context* context)
{
  u32 i;	/* JHB */
  unsigned char finalcount[8];

  for (i = 0; i < 8; i++) {
    finalcount[i] = (unsigned char)((context->count[(i >= 4 ? 0 : 1)]
                                     >> ((3-(i & 3)) * 8) ) & 255);  /* Endian independent */
  }
  SHA1Update(context, (const unsigned char*)"\200", 1);
  while ((context->count[0] & 504) != 448) {
    SHA1Update(context, (const unsigned char*)"\0", 1);
  }
  SHA1Update(context, finalcount, 8);  /* Should cause a SHA1Transform() */
  for (i = 0; i < SHA1_DIGEST_LENGTH; i++) {
    context->digest[i] = (unsigned char)
      ((context->state[i>>2] >> ((3-(i & 3)) * 8) ) & 255);
  }

  /* Wipe variables */
  /* i = 0; */	/* JHB */  /* CLANG: useless */
  memset(context->buffer, 0, 64);
  memset(context->state, 0, SHA1_DIGEST_LENGTH);
  memset(context->count, 0, 8);
  memset(finalcount, 0, 8);	/* SWR */
#ifdef SHA1HANDSOFF  /* make SHA1Transform overwrite it's own static vars */
  SHA1Transform(context->state, context->buffer);
#endif
}


/* DJB code from here */

#define IPAD_CHAR 0x36
#define OPAD_CHAR 0x5c

#define HMAC_SHA1_BLOCKSIZE 64

/*
 * flickcurl_hmac_sha1:
 * @data: data
 * @data_size: size of data in bytes
 * @key: key
 * @key_len: size of key in bytes
 *
 * INTERNAL - Calculate the HMAC-SHA1 digest of key and data
 *
 * Based on specification at http://tools.ietf.org/html/rfc2104
 * Section 2. "Definition of HMAC" where B=64 H=SHA1 L=SHA1_DIGEST_LENGTH (20)
 *
 * Return value: buffer of size SHA1_DIGEST_LENGTH or NULL on failure 
 */
unsigned char*
flickcurl_hmac_sha1(const void *data, size_t data_len,
                    const void *key, size_t key_len)
{
  unsigned int i;
  SHA1Context inner;
  SHA1Context outer;
  SHA1Context key_hash;
  unsigned char kpad[HMAC_SHA1_BLOCKSIZE];
  unsigned char* result;
  
  if(!key || !data)
    return NULL;
        
  result = (unsigned char*)malloc(SHA1_DIGEST_LENGTH);
  if(!result)
    return NULL;
  
  if(key_len > HMAC_SHA1_BLOCKSIZE) {
    /* When key (K) is > blocksize, key := sha1-hash(key) */
    SHA1Init(&key_hash);
    SHA1Update(&key_hash, (const unsigned char*)key, key_len);
    SHA1Final(&key_hash);
    
    key = key_hash.digest;
    key_len = SHA1_DIGEST_LENGTH;
  }

  memset(kpad, '\0', sizeof(kpad));
  memcpy(kpad, key, key_len);
  for(i = 0; i < HMAC_SHA1_BLOCKSIZE; i++)
    kpad[i] ^= IPAD_CHAR;
  
  /* inner := sha1-hash(ipad // message) */
  SHA1Init(&inner);
  SHA1Update(&inner, kpad, HMAC_SHA1_BLOCKSIZE);
  SHA1Update(&inner, (const unsigned char*)data, data_len);
  SHA1Final(&inner);

  memset(kpad, '\0', sizeof(kpad));
  memcpy(kpad, key, key_len);
  for(i = 0; i < HMAC_SHA1_BLOCKSIZE; i++)
    kpad[i] ^= OPAD_CHAR;
  
  /* final outer := sha1-hash(opad // inner) */
  SHA1Init(&outer);
  SHA1Update(&outer, kpad, HMAC_SHA1_BLOCKSIZE);
  /* Result of inner hash is in inner.digest of size SHA1_DIGEST_LENGTH */
  SHA1Update(&outer, inner.digest, SHA1_DIGEST_LENGTH);
  SHA1Final(&outer);

  /* copy final digest into result buffer */
  memcpy(result, outer.digest, SHA1_DIGEST_LENGTH);

  return result;
}
