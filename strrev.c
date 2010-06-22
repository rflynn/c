/* ex: set ts=2 et: */
/*
 * Copyright 2008 Ryan Flynn
 *
 * www.parseerror.com
 *
 * Mon Nov 24 15:49:08 EST 2008
 *
 * How Fast Can We Reverse a String?
 *
 * NOTE: run like so and send me all the results:
 *  $ cat /proc/cpuinfo # or equivalent
 *  $ cc -std=c99 -W -Wall -pedantic -O3 -o strrev strrev.c
 *  $ ./strrev
 *
 * I stumbled across this exercise in Palindromic Numbers from
 * <URL:http://mathworld.wolfram.com/196-Algorithm.html> by way of MathWorld's
 * list of Unsolved Problems
 * <URL:http://mathworld.wolfram.com/UnsolvedProblems.html>
 * 
 * The algorithm involves taking the base-10 digits of a number 123 -> [1,2,3]
 * reversing them Rev([1,2,3]) -> [3,2,1]
 * and then adding them to the original number [1,2,3]+[3,2,1] -> [4,4,4]
 * until one gets a palindromic number.
 *
 * Most numbers yield palindromic numbers quickly using this method, but a
 * few do not.
 *
 * I built a program to search for a palindromic result for the number
 * 196. I am now trying to make it run faster, and as the function spends
 * 99% of its time reversing and adding arrays of integers together I thought
 * I would try to find a faster way to reverse the contents of a string.
 *
 * So the goal is to develop a string reversal function SR for which
 *
 *  SR("", 0) -> ""
 *  SR("a", 1) -> "a"
 *  SR("ab", 2) -> "ba"
 *  SR("abc", 3) -> "cba"
 *
 * and so on.
 *
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#if LONG_MAX > 2147483647L /* what's a better way to detect 64-bitness? */
# define CAN_DO 128
#else
# define CAN_DO 64
#endif

#ifdef WIN32 /* assume Visual Studio */
# define WIN32_LEAN_AND_MEAN
# include <winsock2.h>
# include "stdint.h"
# define __builtin_prefetch(ptr) 0
# define __builtin_expect(foo, yes) 0

# ifdef _DEBUG
#   error "this inline asm won't work with Debug stacks!"
# endif

__inline uint32_t bswap32(uint32_t x)
{
  __asm {
    mov   edx, [ebp+8]
    bswap edx
  }
}

__inline uint64_t __bswap_64(uint64_t x)
{
  __asm {
    mov   edx, [ebp+8]
    mov   eax, [ebp+12]
    bswap edx
    bswap eax
  }
}

#else /* assume GCC on Linux */
# include <stdint.h>
# define bswap32 ntohl /* NOTE: works on little-endian only, but x86 is assumed... */
# define bswap64 __bswap_64 /* GCC built-in */
#endif

/**
 * this is the first thing that came to my mind.
 * use only one index and a halfway marker.
 * it works perfectly well and is easy to understand.
 */
void obvious(char *dst, const char *src, unsigned len)
{
  if (len) {
    long i = 0,
         halfway = len / 2;
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

/**
 * work one ascending index forwards, one at the end working frontwards,
 * copy their bytes until they meet
 *
 * this function fits everything we care about into x86 registers,
 * and yet it is not faster than 'obvious'... why?
 *
 * the inner loop consists of 4 movs, 1 inc and 1 dec. the one-byte
 * reads must be saturating our memory bus
 */
void obvious_twoindex(char *dst, const char *src, unsigned len)
{
  if (len) {
  long i = 0,
       j = len - 1;
  while (i <= j) {
    dst[i] = src[j];
    dst[j] = src[i];
    i++, j--;
  }
  }
}

/**
 * use only pointers, no indexes
 */
void obvious_pointer(char *dst, const char *src, unsigned len)
{
  if (len) {
          char *d = dst + (len-1);
    const char *s = src + (len-1);
    while (dst <= d) {
      *dst++ = *s--;
      *d-- = *src++;
    }
  }
}

/**
 * check if the bytes are different before swapping.
 * surprisingly, this produces a savings from 'obvious' even given
 * a string with low likelihood of the swap chars matching.
 * significant savings if the string is a palindrome; but still
 * not faster than the unrolled ones.
 */
void obvious_check(char *dst, const char *src, unsigned len)
{
  if (len) {
    long i = 0,
        j = len - 1;
    while (i < j) {
      if (dst[i] != src[j])
        dst[i] = src[j];
      if (dst[j] != src[i])
        dst[j] = src[i];
      i++, j--;
    }
    if (dst[i] != src[j]) {
      dst[i] = src[j];
      dst[j] = src[i];
    }
  }
}

static void recurse_(char *dst, const char *src, unsigned i, unsigned j)
{
  if (i < j) {
    dst[i] = src[j];
    dst[j] = src[i];
    recurse_(dst, src, ++i, --j);
  }
}

/**
 * work one ascending index forwards, one at the end working frontwards,
 * copy their bytes until they meet
 * NOTE: not acceptable for long strings.
 */
void obvious_recurse(char *dst, const char *src, unsigned len)
{
  if (len) {
    recurse_(dst, src, 0, len-1);
    if (len & 1)
      dst[len/2] = src[len/2];
  }
}

/**
 * work from the middle outwards
 * maybe locality of reference will make a difference in medium-sized strings?
 * this function is nice and simple
 */
void insideout(char *dst, const char *src, unsigned len)
{
  if (len) {
    long i = (len - 1) / 2,
         j = len / 2;
    while (j < (long)len) {
      dst[i] = src[j];
      dst[j++] = src[i--];
    }
  }
}

/**
 * use GCC's builtin CPU-hinting prefetch to
 * populate the cache.
 * use only one index and a halfway marker.
 * it works perfectly, is easy to understand.
 */
void obvious_prefetch(char *dst, const char *src, unsigned len)
{
  if (len) {
    long i = 0,
         halfway = len / 2;
    __builtin_prefetch(dst+i);
    __builtin_prefetch(src+len-1);
    __builtin_prefetch(src+i);
    __builtin_prefetch(dst+len-1);
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
      __builtin_prefetch(dst+i);
      __builtin_prefetch(src+i);
      __builtin_prefetch(src+i-i);
      __builtin_prefetch(dst+len-1-i);
    }
  }
}

/**
 * copy two-byte pieces at a time, when possible.
 * should be faster.
 */
void byte2_unroll(char *dst, const char *src, unsigned len)
{
  if (len) {
    long i = 0,
         halfway = len / 2;
    union {
      uint16_t i;
      uint8_t c[2];
    } tmp;
    while (i <= halfway - (long)sizeof tmp.c) {
      tmp.i = *(uint16_t *)(src+i);
      dst[len-1-i] = tmp.c[0];
      dst[len-2-i] = tmp.c[1];
      tmp.i = *(uint16_t *)(src+len-i-sizeof tmp.c);
      dst[i++] = tmp.c[1];
      dst[i++] = tmp.c[0];
    }
    /* shore up any single bytes */
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

/**
 * copy four-byte pieces at a time, when possible.
 * should be faster.
 */
void byte4_unroll(char *dst, const char *src, unsigned len)
{
  if (len) {
    unsigned i = 0,
             halfway = len / 2;
    union {
      uint32_t i;
      uint8_t c[4];
    } tmp;
    while (i+4 <= halfway) {
      tmp.i = *(uint32_t *)(src+i);
      dst[len-1-i] = tmp.c[0];
      dst[len-2-i] = tmp.c[1];
      dst[len-3-i] = tmp.c[2];
      dst[len-4-i] = tmp.c[3];
      tmp.i = *(uint32_t *)(src+len-4-i);
      dst[i++] = tmp.c[3];
      dst[i++] = tmp.c[2];
      dst[i++] = tmp.c[1];
      dst[i++] = tmp.c[0];
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

/**
 * copy four-byte pieces at a time, when possible.
 * should be faster.
 */
void byte4_unroll_prefetch(char *dst, const char *src, unsigned len)
{
  if (len) {
    unsigned i = 0,
             halfway = len / 2;
    union {
      uint32_t i;
      uint8_t c[4];
    } tmp;
    __builtin_prefetch(src);
    __builtin_prefetch(src+len-4);
    while (i+4 <= halfway) {
      tmp.i = *(uint32_t *)(src+i);
      dst[len-1-i] = tmp.c[0];
      dst[len-2-i] = tmp.c[1];
      dst[len-3-i] = tmp.c[2];
      dst[len-4-i] = tmp.c[3];
      tmp.i = *(uint32_t *)(src+len-4-i);
      dst[i++] = tmp.c[3];
      dst[i++] = tmp.c[2];
      dst[i++] = tmp.c[1];
      dst[i++] = tmp.c[0];
      __builtin_prefetch(src+i);
      __builtin_prefetch(src+len-4-i);
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

/**
 * copy four-byte pieces at a time, when possible.
 * should be faster.
 */
void byte4_unroll2(char *dst, const char *src, unsigned len)
{
  if (len) {
    union {
      uint32_t i;
      uint8_t c[sizeof(uint32_t)];
    } tmp;
    const long halfway = len / 2;
    register long i = 0;
    while (i <= halfway-4) {
      tmp.i = *(uint32_t *)(src+i);
      dst[len-1-i] = tmp.c[0];
      dst[len-2-i] = tmp.c[1];
      dst[len-3-i] = tmp.c[2];
      dst[len-4-i] = tmp.c[3];
      tmp.i = *(uint32_t *)(src+len-i-sizeof tmp.c);
      dst[i++] = tmp.c[3];
      dst[i++] = tmp.c[2];
      dst[i++] = tmp.c[1];
      dst[i++] = tmp.c[0];
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

/**
 * copy four-byte pieces at a time, when possible.
 * should be faster.
 */
void byte4_unroll2_expect(char *dst, const char *src, unsigned len)
{
  if (len) {
    union {
      uint32_t i;
      uint8_t c[sizeof(uint32_t)];
    } tmp;
    const long halfway = len / 2;
    register long i = 0;
    while (__builtin_expect(i <= halfway-4,1)) {
      tmp.i = *(uint32_t *)(src+i);
      dst[len-1-i] = tmp.c[0];
      dst[len-2-i] = tmp.c[1];
      dst[len-3-i] = tmp.c[2];
      dst[len-4-i] = tmp.c[3];
      tmp.i = *(uint32_t *)(src+len-i-sizeof tmp.c);
      dst[i++] = tmp.c[3];
      dst[i++] = tmp.c[2];
      dst[i++] = tmp.c[1];
      dst[i++] = tmp.c[0];
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

/**
 * copy four-byte pieces at a time, when possible.
 * should be faster.
 */
void byte4_unroll3(char *dst, const char *src, unsigned len)
{
  if (len) {
    const long halfway = len / 2,
               halfway2 = halfway - 4;
    uint32_t j;
    register long i = 0;
    while (i <= halfway2) {
      j = *(uint32_t *)(src+i);
      dst[len-1-i] = (uint8_t)(j);
      dst[len-2-i] = (uint8_t)(j >>  8);
      dst[len-3-i] = (uint8_t)(j >> 16);
      dst[len-4-i] = (uint8_t)(j >> 24);
      j = *(uint32_t *)(src+len-i-sizeof j);
      dst[i++] = (uint8_t)(j >> 24);
      dst[i++] = (uint8_t)(j >> 16);
      dst[i++] = (uint8_t)(j >>  8);
      dst[i++] = (uint8_t)(j);
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

/**
 * copy four-byte pieces at a time, when possible.
 * should be faster.
 */
void byte4_loop(char *dst, const char *src, unsigned len)
{
  if (len) {
    unsigned i = 0,
             halfway = len / 2;
    union {
      uint32_t i;
      uint8_t c[4];
    } tmp;
    while (i+4 <= halfway) {
      int d = len-1-i;
      tmp.i = *(uint32_t *)(src+i);
      dst[d--] = tmp.c[0];
      dst[d--] = tmp.c[1];
      dst[d--] = tmp.c[2];
      dst[d]   = tmp.c[3];
      tmp.i = *(uint32_t *)(src+len-4-i);
      dst[i++] = tmp.c[3];
      dst[i++] = tmp.c[2];
      dst[i++] = tmp.c[1];
      dst[i++] = tmp.c[0];
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

/**
 * copy four-byte pieces at a time, when possible.
 * should be faster.
 */
void byte4_wb(char *dst, const char *src, unsigned len)
{
  if (len) {
    unsigned i = 0,
             halfway = len / 2;
    union {
      uint32_t i;
      char c[4];
    } tmp;
    while (i+4 <= halfway) {
      tmp.i = *(uint32_t *)(src+i);
      tmp.i = (tmp.c[0] << 24) |
              (tmp.c[1] << 16) |
              (tmp.c[2] <<  8) |
              tmp.c[3];
      *(uint32_t *)(dst+len-4-i) = tmp.i;
      tmp.i = *(uint32_t *)(src+len-4-i);
      tmp.i = (tmp.c[0] << 24) |
              (tmp.c[1] << 16) |
              (tmp.c[2] <<  8) |
              tmp.c[3];
      *(uint32_t *)(dst+i) = tmp.i;
      i += 4;
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

#ifdef WIN32
# include <winsock2.h>
#else
# include <arpa/inet.h>
#endif

/**
 * read four bytes, perform bitwise operations on that quantity to order it,
 * then write the whole thing out in one piece
 */
void byte4_w32(char *dst, const char *src, unsigned len)
{
  if (len) {
    long i = 0,
         halfway = len / 2;
    while (i <= halfway-4) {
      *(uint32_t *)(dst+len-4-i) = bswap32(*(uint32_t *)(src+i));
      *(uint32_t *)(dst+i) = bswap32(*(uint32_t *)(src+len-4-i));
      i += 4;
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

/**
 * read four bytes, perform bitwise operations on that quantity to order it,
 * then write the whole thing out in one piece
 * FIXME: this abuse of bswap32() will only work on little-endian machines
 */
void byte4_wc(char *dst, const char *src, unsigned len)
{
  if (len) {
    long i = 0,
         halfway = len / 2;
    __builtin_prefetch(dst+len-4-i);
    __builtin_prefetch(src+i);
    __builtin_prefetch(dst+i);
    __builtin_prefetch(src+len-4-i);
    while (i <= halfway-4) {
      *(uint32_t *)(dst+len-4-i) = bswap32(*(uint32_t *)(src+i));
      *(uint32_t *)(dst+i) = bswap32(*(uint32_t *)(src+len-4-i));
      i += 4;
      __builtin_prefetch(dst+len-4-i);
      __builtin_prefetch(src+i);
      __builtin_prefetch(dst+i);
      __builtin_prefetch(src+len-4-i);
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

/**
 * copy four-byte pieces at a time, when possible.
 * should be faster.
 */
void byte8_unroll(char *dst, const char *src, unsigned len)
{
  if (len) {
    unsigned i = 0,
            halfway = len / 2;
    union {
      uint64_t i;
      char c[8];
    } tmp;
    while (i+8 <= halfway) {
      tmp.i = *(uint64_t *)(src+i);
      dst[len-1-i] = tmp.c[0];
      dst[len-2-i] = tmp.c[1];
      dst[len-3-i] = tmp.c[2];
      dst[len-4-i] = tmp.c[3];
      dst[len-5-i] = tmp.c[4];
      dst[len-6-i] = tmp.c[5];
      dst[len-7-i] = tmp.c[6];
      dst[len-8-i] = tmp.c[7];
      tmp.i = *(uint64_t *)(src+len-8-i);
      dst[i++] = tmp.c[7];
      dst[i++] = tmp.c[6];
      dst[i++] = tmp.c[5];
      dst[i++] = tmp.c[4];
      dst[i++] = tmp.c[3];
      dst[i++] = tmp.c[2];
      dst[i++] = tmp.c[1];
      dst[i++] = tmp.c[0];
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

/**
 * copy four-byte pieces at a time, when possible.
 * should be faster.
 */
void byte8_subloop(char *dst, const char *src, unsigned len)
{
  if (len) {
    unsigned i = 0,
             halfway = len / 2;
    union {
      uint64_t i;
      char c[8];
    } tmp;
    while (i+8 <= halfway) {
      int j = 0;
      tmp.i = *(uint64_t *)(src+i);
      while (j < 8) {
        dst[len-j-1-i] = tmp.c[j];
        j++;
      }
      tmp.i = *(uint64_t *)(src+len-8-i);
      while (j--)
        dst[i++] = tmp.c[j];
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

/**
 * 64-bit version of byte4_w
 */
void byte8_w32(char *dst, const char *src, unsigned len)
{
  if (len) {
    long i = 0,
         halfway = len / 2;
    while (i <= halfway-8) {
      *(uint32_t *)(dst+len-8-i) = bswap32(*(uint32_t *)(src+i+4));
      *(uint32_t *)(dst+len-4-i) = bswap32(*(uint32_t *)(src+i));
      *(uint32_t *)(dst+i)       = bswap32(*(uint32_t *)(src+len-4-i));
      *(uint32_t *)(dst+i+4)     = bswap32(*(uint32_t *)(src+len-8-i));
      i += 8;
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

/**
 * 64-bit version of byte4_w
 */
void byte8_w64(char *dst, const char *src, unsigned len)
{
  if (len) {
    long i = 0,
         halfway = len / 2;
    while (i <= halfway-8) {
      *(uint64_t *)(dst+len-8-i) = bswap64(*(uint64_t *)(src+i));
      *(uint64_t *)(dst+i)       = bswap64(*(uint64_t *)(src+len-8-i));
      i += 8;
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

#ifdef MMX

//---------------------------------------------------
// A simple example using MMX to copy 8 bytes of data 
// From source s2 to destination s1
//---------------------------------------------------
void __fastcall CopyMemory8(char *s1, const char *s2)
{
   __asm
   {
    push edx
    mov ecx, s2
    mov edx, s1
    movq   mm0, [ecx   ]
    movq   [edx   ], mm0
    pop edx
    emms
   }
}

#endif

/**
 * 64-bit version of byte4_w
 */
void byte8_w64_mmx(char *dst, const char *src, unsigned len)
{
  if (len) {
    long i = 0,
         halfway = len / 2;
    while (i <= halfway-8) {
      *(uint64_t *)(dst+len-8-i) = bswap64(*(uint64_t *)(src+i));
      *(uint64_t *)(dst+i)       = bswap64(*(uint64_t *)(src+len-8-i));
      i += 8;
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

/**
 * 128-bit version of byte4_w
 */
void byte16_w32(char *dst, const char *src, unsigned len)
{
  if (len) {
    long i = 0,
         halfway = len / 2;
    while (i <= halfway-16) {
      *(uint32_t *)(dst+len-16-i) = bswap32(*(uint32_t *)(src+i+12));
      *(uint32_t *)(dst+len-12-i) = bswap32(*(uint32_t *)(src+i+8));
      *(uint32_t *)(dst+len-8-i)  = bswap32(*(uint32_t *)(src+i+4));
      *(uint32_t *)(dst+len-4-i)  = bswap32(*(uint32_t *)(src+i));
      *(uint32_t *)(dst+i)        = bswap32(*(uint32_t *)(src+len-4-i));
      *(uint32_t *)(dst+i+4)      = bswap32(*(uint32_t *)(src+len-8-i));
      *(uint32_t *)(dst+i+8)      = bswap32(*(uint32_t *)(src+len-12-i));
      *(uint32_t *)(dst+i+12)     = bswap32(*(uint32_t *)(src+len-16-i));
      i += 16;
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

/**
 * 128-bit version of byte4_write32
 */
void byte16_w64(char *dst, const char *src, unsigned len)
{
  if (len) {
    long i = 0,
         halfway = len / 2;
    while (i <= halfway-16) {
      *(uint64_t *)(dst+len-16-i) = bswap64(*(uint64_t *)(src+i+8));
      *(uint64_t *)(dst+len-8-i)  = bswap64(*(uint64_t *)(src+i));
      *(uint64_t *)(dst+i)        = bswap64(*(uint64_t *)(src+len-8-i));
      *(uint64_t *)(dst+i+8)      = bswap64(*(uint64_t *)(src+len-16-i));
      i += 16;
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

/**
 * 256-bit version of byte4_write32
 */
void byte32_w32(char *dst, const char *src, unsigned len)
{
  if (len) {
    long i = 0,
         halfway = len / 2;
    while (i <= halfway-32) {
      *(uint32_t *)(dst+len-32-i) = bswap32(*(uint32_t *)(src+i+28));
      *(uint32_t *)(dst+len-28-i) = bswap32(*(uint32_t *)(src+i+24));
      *(uint32_t *)(dst+len-24-i) = bswap32(*(uint32_t *)(src+i+20));
      *(uint32_t *)(dst+len-20-i) = bswap32(*(uint32_t *)(src+i+16));
      *(uint32_t *)(dst+len-16-i) = bswap32(*(uint32_t *)(src+i+12));
      *(uint32_t *)(dst+len-12-i) = bswap32(*(uint32_t *)(src+i+8));
      *(uint32_t *)(dst+len-8-i)  = bswap32(*(uint32_t *)(src+i+4));
      *(uint32_t *)(dst+len-4-i)  = bswap32(*(uint32_t *)(src+i));
      *(uint32_t *)(dst+i)        = bswap32(*(uint32_t *)(src+len-4-i));
      *(uint32_t *)(dst+i+4)      = bswap32(*(uint32_t *)(src+len-8-i));
      *(uint32_t *)(dst+i+8)      = bswap32(*(uint32_t *)(src+len-12-i));
      *(uint32_t *)(dst+i+12)     = bswap32(*(uint32_t *)(src+len-16-i));
      *(uint32_t *)(dst+i+16)     = bswap32(*(uint32_t *)(src+len-20-i));
      *(uint32_t *)(dst+i+20)     = bswap32(*(uint32_t *)(src+len-24-i));
      *(uint32_t *)(dst+i+24)     = bswap32(*(uint32_t *)(src+len-28-i));
      *(uint32_t *)(dst+i+28)     = bswap32(*(uint32_t *)(src+len-32-i));
      i += 32;
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

/**
 * 256-bit version of byte4_write32
 */
void byte32_w_prefetch(char *dst, const char *src, unsigned len)
{
  if (len) {
    long i = 0,
         halfway = len / 2;
    while (i <= halfway-32) {
      __builtin_prefetch(dst+i);
      __builtin_prefetch(src+i);
      *(uint32_t *)(dst+len-32-i) = bswap32(*(uint32_t *)(src+i+28));
      *(uint32_t *)(dst+len-28-i) = bswap32(*(uint32_t *)(src+i+24));
      *(uint32_t *)(dst+len-24-i) = bswap32(*(uint32_t *)(src+i+20));
      *(uint32_t *)(dst+len-20-i) = bswap32(*(uint32_t *)(src+i+16));
      *(uint32_t *)(dst+len-16-i) = bswap32(*(uint32_t *)(src+i+12));
      *(uint32_t *)(dst+len-12-i) = bswap32(*(uint32_t *)(src+i+8));
      *(uint32_t *)(dst+len-8-i)  = bswap32(*(uint32_t *)(src+i+4));
      *(uint32_t *)(dst+len-4-i)  = bswap32(*(uint32_t *)(src+i));
      *(uint32_t *)(dst+i)        = bswap32(*(uint32_t *)(src+len-4-i));
      *(uint32_t *)(dst+i+4)      = bswap32(*(uint32_t *)(src+len-8-i));
      *(uint32_t *)(dst+i+8)      = bswap32(*(uint32_t *)(src+len-12-i));
      *(uint32_t *)(dst+i+12)     = bswap32(*(uint32_t *)(src+len-16-i));
      *(uint32_t *)(dst+i+16)     = bswap32(*(uint32_t *)(src+len-20-i));
      *(uint32_t *)(dst+i+20)     = bswap32(*(uint32_t *)(src+len-24-i));
      *(uint32_t *)(dst+i+24)     = bswap32(*(uint32_t *)(src+len-28-i));
      *(uint32_t *)(dst+i+28)     = bswap32(*(uint32_t *)(src+len-32-i));
      i += 32;
    }
    __builtin_prefetch(dst+i);
    __builtin_prefetch(src+i);
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

/**
 * 256-bit version of byte4_write32
 */
void byte32_w64(char *dst, const char *src, unsigned len)
{
  if (len) {
    long i = 0,
         halfway = len / 2;
    while (i <= halfway-32) {
      *(uint64_t *)(dst+len-32-i) = bswap64(*(uint64_t *)(src+i+24));
      *(uint64_t *)(dst+len-24-i) = bswap64(*(uint64_t *)(src+i+16));
      *(uint64_t *)(dst+len-16-i) = bswap64(*(uint64_t *)(src+i+8));
      *(uint64_t *)(dst+len- 8-i) = bswap64(*(uint64_t *)(src+i));
      *(uint64_t *)(dst+i)        = bswap64(*(uint64_t *)(src+len-8-i));
      *(uint64_t *)(dst+i+8)      = bswap64(*(uint64_t *)(src+len-16-i));
      *(uint64_t *)(dst+i+16)     = bswap64(*(uint64_t *)(src+len-24-i));
      *(uint64_t *)(dst+i+24)     = bswap64(*(uint64_t *)(src+len-32-i));
      i += 32;
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

/**
 * 512-bit version of byte4_write32
 */
void byte64_w32(char *dst, const char *src, unsigned len)
{
  if (len) {
    long i = 0,
         halfway = len / 2;
    while (i <= halfway-64) {
      *(uint32_t *)(dst+len-64-i) = bswap32(*(uint32_t *)(src+i+60));
      *(uint32_t *)(dst+len-60-i) = bswap32(*(uint32_t *)(src+i+56));
      *(uint32_t *)(dst+len-56-i) = bswap32(*(uint32_t *)(src+i+52));
      *(uint32_t *)(dst+len-52-i) = bswap32(*(uint32_t *)(src+i+48));
      *(uint32_t *)(dst+len-48-i) = bswap32(*(uint32_t *)(src+i+44));
      *(uint32_t *)(dst+len-44-i) = bswap32(*(uint32_t *)(src+i+40));
      *(uint32_t *)(dst+len-40-i) = bswap32(*(uint32_t *)(src+i+36));
      *(uint32_t *)(dst+len-36-i) = bswap32(*(uint32_t *)(src+i+32));
      *(uint32_t *)(dst+len-32-i) = bswap32(*(uint32_t *)(src+i+28));
      *(uint32_t *)(dst+len-28-i) = bswap32(*(uint32_t *)(src+i+24));
      *(uint32_t *)(dst+len-24-i) = bswap32(*(uint32_t *)(src+i+20));
      *(uint32_t *)(dst+len-20-i) = bswap32(*(uint32_t *)(src+i+16));
      *(uint32_t *)(dst+len-16-i) = bswap32(*(uint32_t *)(src+i+12));
      *(uint32_t *)(dst+len-12-i) = bswap32(*(uint32_t *)(src+i+8));
      *(uint32_t *)(dst+len-8-i)  = bswap32(*(uint32_t *)(src+i+4));
      *(uint32_t *)(dst+len-4-i)  = bswap32(*(uint32_t *)(src+i));
      *(uint32_t *)(dst+i)        = bswap32(*(uint32_t *)(src+len-4-i));
      *(uint32_t *)(dst+i+4)      = bswap32(*(uint32_t *)(src+len-8-i));
      *(uint32_t *)(dst+i+8)      = bswap32(*(uint32_t *)(src+len-12-i));
      *(uint32_t *)(dst+i+12)     = bswap32(*(uint32_t *)(src+len-16-i));
      *(uint32_t *)(dst+i+16)     = bswap32(*(uint32_t *)(src+len-20-i));
      *(uint32_t *)(dst+i+20)     = bswap32(*(uint32_t *)(src+len-24-i));
      *(uint32_t *)(dst+i+24)     = bswap32(*(uint32_t *)(src+len-28-i));
      *(uint32_t *)(dst+i+28)     = bswap32(*(uint32_t *)(src+len-32-i));
      *(uint32_t *)(dst+i+32)     = bswap32(*(uint32_t *)(src+len-36-i));
      *(uint32_t *)(dst+i+36)     = bswap32(*(uint32_t *)(src+len-40-i));
      *(uint32_t *)(dst+i+40)     = bswap32(*(uint32_t *)(src+len-44-i));
      *(uint32_t *)(dst+i+44)     = bswap32(*(uint32_t *)(src+len-48-i));
      *(uint32_t *)(dst+i+48)     = bswap32(*(uint32_t *)(src+len-52-i));
      *(uint32_t *)(dst+i+52)     = bswap32(*(uint32_t *)(src+len-56-i));
      *(uint32_t *)(dst+i+56)     = bswap32(*(uint32_t *)(src+len-60-i));
      *(uint32_t *)(dst+i+60)     = bswap32(*(uint32_t *)(src+len-64-i));
      i += 64;
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

/**
 * 512-bit version of byte4_write32
 */
void byte64_w64(char *dst, const char *src, unsigned len)
{
  if (len) {
    long i = 0,
         halfway = len / 2;
    while (i <= halfway-64) {
      *(uint64_t *)(dst+len-56-i) = bswap64(*(uint64_t *)(src+i+48));
      *(uint64_t *)(dst+len-48-i) = bswap64(*(uint64_t *)(src+i+40));
      *(uint64_t *)(dst+len-40-i) = bswap64(*(uint64_t *)(src+i+32));
      *(uint64_t *)(dst+len-32-i) = bswap64(*(uint64_t *)(src+i+24));
      *(uint64_t *)(dst+len-24-i) = bswap64(*(uint64_t *)(src+i+16));
      *(uint64_t *)(dst+len-16-i) = bswap64(*(uint64_t *)(src+i+8));
      *(uint64_t *)(dst+len- 8-i) = bswap64(*(uint64_t *)(src+i));
      *(uint64_t *)(dst+i)        = bswap64(*(uint64_t *)(src+len-8-i));
      *(uint64_t *)(dst+i+8)      = bswap64(*(uint64_t *)(src+len-16-i));
      *(uint64_t *)(dst+i+16)     = bswap64(*(uint64_t *)(src+len-24-i));
      *(uint64_t *)(dst+i+24)     = bswap64(*(uint64_t *)(src+len-32-i));
      *(uint64_t *)(dst+i+32)     = bswap64(*(uint64_t *)(src+len-40-i));
      *(uint64_t *)(dst+i+40)     = bswap64(*(uint64_t *)(src+len-48-i));
      *(uint64_t *)(dst+i+48)     = bswap64(*(uint64_t *)(src+len-56-i));
      *(uint64_t *)(dst+i+56)     = bswap64(*(uint64_t *)(src+len-64-i));
      i += 32;
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

/**
 * 1024-bit version of byte4_write32
 */
void byte128_w32(char *dst, const char *src, unsigned len)
{
  if (len) {
    long i = 0,
         halfway = len / 2;
    while (i <= halfway-128) {
      *(uint32_t *)(dst+len-128-i) = bswap32(*(uint32_t *)(src+i+124));
      *(uint32_t *)(dst+len-124-i) = bswap32(*(uint32_t *)(src+i+120));
      *(uint32_t *)(dst+len-120-i) = bswap32(*(uint32_t *)(src+i+116));
      *(uint32_t *)(dst+len-116-i) = bswap32(*(uint32_t *)(src+i+112));
      *(uint32_t *)(dst+len-112-i) = bswap32(*(uint32_t *)(src+i+108));
      *(uint32_t *)(dst+len-108-i) = bswap32(*(uint32_t *)(src+i+104));
      *(uint32_t *)(dst+len-104-i) = bswap32(*(uint32_t *)(src+i+100));
      *(uint32_t *)(dst+len-100-i) = bswap32(*(uint32_t *)(src+i+96));
      *(uint32_t *)(dst+len-96-i)  = bswap32(*(uint32_t *)(src+i+92));
      *(uint32_t *)(dst+len-92-i)  = bswap32(*(uint32_t *)(src+i+88));
      *(uint32_t *)(dst+len-88-i)  = bswap32(*(uint32_t *)(src+i+84));
      *(uint32_t *)(dst+len-84-i)  = bswap32(*(uint32_t *)(src+i+80));
      *(uint32_t *)(dst+len-80-i)  = bswap32(*(uint32_t *)(src+i+76));
      *(uint32_t *)(dst+len-76-i)  = bswap32(*(uint32_t *)(src+i+72));
      *(uint32_t *)(dst+len-72-i)  = bswap32(*(uint32_t *)(src+i+68));
      *(uint32_t *)(dst+len-68-i)  = bswap32(*(uint32_t *)(src+i+64));
      *(uint32_t *)(dst+len-64-i)  = bswap32(*(uint32_t *)(src+i+60));
      *(uint32_t *)(dst+len-60-i)  = bswap32(*(uint32_t *)(src+i+56));
      *(uint32_t *)(dst+len-56-i)  = bswap32(*(uint32_t *)(src+i+52));
      *(uint32_t *)(dst+len-52-i)  = bswap32(*(uint32_t *)(src+i+48));
      *(uint32_t *)(dst+len-48-i)  = bswap32(*(uint32_t *)(src+i+44));
      *(uint32_t *)(dst+len-44-i)  = bswap32(*(uint32_t *)(src+i+40));
      *(uint32_t *)(dst+len-40-i)  = bswap32(*(uint32_t *)(src+i+36));
      *(uint32_t *)(dst+len-36-i)  = bswap32(*(uint32_t *)(src+i+32));
      *(uint32_t *)(dst+len-32-i)  = bswap32(*(uint32_t *)(src+i+28));
      *(uint32_t *)(dst+len-28-i)  = bswap32(*(uint32_t *)(src+i+24));
      *(uint32_t *)(dst+len-24-i)  = bswap32(*(uint32_t *)(src+i+20));
      *(uint32_t *)(dst+len-20-i)  = bswap32(*(uint32_t *)(src+i+16));
      *(uint32_t *)(dst+len-16-i)  = bswap32(*(uint32_t *)(src+i+12));
      *(uint32_t *)(dst+len-12-i)  = bswap32(*(uint32_t *)(src+i+8));
      *(uint32_t *)(dst+len-8-i)   = bswap32(*(uint32_t *)(src+i+4));
      *(uint32_t *)(dst+len-4-i)   = bswap32(*(uint32_t *)(src+i));
      *(uint32_t *)(dst+i)         = bswap32(*(uint32_t *)(src+len-4-i));
      *(uint32_t *)(dst+i+4)       = bswap32(*(uint32_t *)(src+len-8-i));
      *(uint32_t *)(dst+i+8)       = bswap32(*(uint32_t *)(src+len-12-i));
      *(uint32_t *)(dst+i+12)      = bswap32(*(uint32_t *)(src+len-16-i));
      *(uint32_t *)(dst+i+16)      = bswap32(*(uint32_t *)(src+len-20-i));
      *(uint32_t *)(dst+i+20)      = bswap32(*(uint32_t *)(src+len-24-i));
      *(uint32_t *)(dst+i+24)      = bswap32(*(uint32_t *)(src+len-28-i));
      *(uint32_t *)(dst+i+28)      = bswap32(*(uint32_t *)(src+len-32-i));
      *(uint32_t *)(dst+i+32)      = bswap32(*(uint32_t *)(src+len-36-i));
      *(uint32_t *)(dst+i+36)      = bswap32(*(uint32_t *)(src+len-40-i));
      *(uint32_t *)(dst+i+40)      = bswap32(*(uint32_t *)(src+len-44-i));
      *(uint32_t *)(dst+i+44)      = bswap32(*(uint32_t *)(src+len-48-i));
      *(uint32_t *)(dst+i+48)      = bswap32(*(uint32_t *)(src+len-52-i));
      *(uint32_t *)(dst+i+52)      = bswap32(*(uint32_t *)(src+len-56-i));
      *(uint32_t *)(dst+i+56)      = bswap32(*(uint32_t *)(src+len-60-i));
      *(uint32_t *)(dst+i+60)      = bswap32(*(uint32_t *)(src+len-64-i));
      *(uint32_t *)(dst+i+64)      = bswap32(*(uint32_t *)(src+len-68-i));
      *(uint32_t *)(dst+i+68)      = bswap32(*(uint32_t *)(src+len-72-i));
      *(uint32_t *)(dst+i+72)      = bswap32(*(uint32_t *)(src+len-76-i));
      *(uint32_t *)(dst+i+76)      = bswap32(*(uint32_t *)(src+len-80-i));
      *(uint32_t *)(dst+i+80)      = bswap32(*(uint32_t *)(src+len-84-i));
      *(uint32_t *)(dst+i+84)      = bswap32(*(uint32_t *)(src+len-88-i));
      *(uint32_t *)(dst+i+88)      = bswap32(*(uint32_t *)(src+len-92-i));
      *(uint32_t *)(dst+i+92)      = bswap32(*(uint32_t *)(src+len-96-i));
      *(uint32_t *)(dst+i+96)      = bswap32(*(uint32_t *)(src+len-100-i));
      *(uint32_t *)(dst+i+100)     = bswap32(*(uint32_t *)(src+len-104-i));
      *(uint32_t *)(dst+i+104)     = bswap32(*(uint32_t *)(src+len-108-i));
      *(uint32_t *)(dst+i+108)     = bswap32(*(uint32_t *)(src+len-112-i));
      *(uint32_t *)(dst+i+112)     = bswap32(*(uint32_t *)(src+len-116-i));
      *(uint32_t *)(dst+i+116)     = bswap32(*(uint32_t *)(src+len-120-i));
      *(uint32_t *)(dst+i+120)     = bswap32(*(uint32_t *)(src+len-124-i));
      *(uint32_t *)(dst+i+124)     = bswap32(*(uint32_t *)(src+len-128-i));
      i += 128;
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

/**
 * 1024-bit version of byte4_write32
 */
void byte128_w64(char *dst, const char *src, unsigned len)
{
  if (len) {
    long i = 0,
         halfway = len / 2;
    while (i <= halfway-128) {
      *(uint64_t *)(dst+len-128-i) = bswap64(*(uint64_t *)(src+i+120));
      *(uint64_t *)(dst+len-120-i) = bswap64(*(uint64_t *)(src+i+112));
      *(uint64_t *)(dst+len-112-i) = bswap64(*(uint64_t *)(src+i+104));
      *(uint64_t *)(dst+len-104-i) = bswap64(*(uint64_t *)(src+i+96));
      *(uint64_t *)(dst+len-96-i)  = bswap64(*(uint64_t *)(src+i+88));
      *(uint64_t *)(dst+len-88-i)  = bswap64(*(uint64_t *)(src+i+80));
      *(uint64_t *)(dst+len-80-i)  = bswap64(*(uint64_t *)(src+i+72));
      *(uint64_t *)(dst+len-72-i)  = bswap64(*(uint64_t *)(src+i+64));
      *(uint64_t *)(dst+len-64-i)  = bswap64(*(uint64_t *)(src+i+56));
      *(uint64_t *)(dst+len-56-i)  = bswap64(*(uint64_t *)(src+i+48));
      *(uint64_t *)(dst+len-48-i)  = bswap64(*(uint64_t *)(src+i+40));
      *(uint64_t *)(dst+len-40-i)  = bswap64(*(uint64_t *)(src+i+32));
      *(uint64_t *)(dst+len-32-i)  = bswap64(*(uint64_t *)(src+i+24));
      *(uint64_t *)(dst+len-24-i)  = bswap64(*(uint64_t *)(src+i+16));
      *(uint64_t *)(dst+len-16-i)  = bswap64(*(uint64_t *)(src+i+ 8));
      *(uint64_t *)(dst+len- 8-i)  = bswap64(*(uint64_t *)(src+i));
      *(uint64_t *)(dst+i)         = bswap64(*(uint64_t *)(src+len-8-i));
      *(uint64_t *)(dst+i+8)       = bswap64(*(uint64_t *)(src+len-16-i));
      *(uint64_t *)(dst+i+16)      = bswap64(*(uint64_t *)(src+len-24-i));
      *(uint64_t *)(dst+i+24)      = bswap64(*(uint64_t *)(src+len-32-i));
      *(uint64_t *)(dst+i+32)      = bswap64(*(uint64_t *)(src+len-40-i));
      *(uint64_t *)(dst+i+40)      = bswap64(*(uint64_t *)(src+len-48-i));
      *(uint64_t *)(dst+i+48)      = bswap64(*(uint64_t *)(src+len-56-i));
      *(uint64_t *)(dst+i+56)      = bswap64(*(uint64_t *)(src+len-64-i));
      *(uint64_t *)(dst+i+64)      = bswap64(*(uint64_t *)(src+len-72-i));
      *(uint64_t *)(dst+i+72)      = bswap64(*(uint64_t *)(src+len-80-i));
      *(uint64_t *)(dst+i+80)      = bswap64(*(uint64_t *)(src+len-88-i));
      *(uint64_t *)(dst+i+88)      = bswap64(*(uint64_t *)(src+len-96-i));
      *(uint64_t *)(dst+i+96)      = bswap64(*(uint64_t *)(src+len-104-i));
      *(uint64_t *)(dst+i+104)     = bswap64(*(uint64_t *)(src+len-112-i));
      *(uint64_t *)(dst+i+112)     = bswap64(*(uint64_t *)(src+len-120-i));
      *(uint64_t *)(dst+i+120)     = bswap64(*(uint64_t *)(src+len-128-i));
      i += 128;
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

/**
 * 2048-bit version of byte4_w
 */
void byte256_w32(char *dst, const char *src, unsigned len)
{
  if (len) {
    long i = 0,
         halfway = len / 2;
    while (i <= halfway-256) {
      *(uint64_t *)(dst+len-256-i) = bswap64(*(uint64_t *)(src+i+252));
      *(uint64_t *)(dst+len-252-i) = bswap64(*(uint64_t *)(src+i+248));
      *(uint64_t *)(dst+len-248-i) = bswap64(*(uint64_t *)(src+i+244));
      *(uint64_t *)(dst+len-244-i) = bswap64(*(uint64_t *)(src+i+240));
      *(uint64_t *)(dst+len-240-i) = bswap64(*(uint64_t *)(src+i+236));
      *(uint64_t *)(dst+len-236-i) = bswap64(*(uint64_t *)(src+i+232));
      *(uint64_t *)(dst+len-232-i) = bswap64(*(uint64_t *)(src+i+228));
      *(uint64_t *)(dst+len-228-i) = bswap64(*(uint64_t *)(src+i+224));
      *(uint64_t *)(dst+len-224-i) = bswap64(*(uint64_t *)(src+i+220));
      *(uint64_t *)(dst+len-220-i) = bswap64(*(uint64_t *)(src+i+216));
      *(uint64_t *)(dst+len-216-i) = bswap64(*(uint64_t *)(src+i+212));
      *(uint64_t *)(dst+len-212-i) = bswap64(*(uint64_t *)(src+i+208));
      *(uint64_t *)(dst+len-208-i) = bswap64(*(uint64_t *)(src+i+204));
      *(uint64_t *)(dst+len-204-i) = bswap64(*(uint64_t *)(src+i+200));
      *(uint64_t *)(dst+len-200-i) = bswap64(*(uint64_t *)(src+i+196));
      *(uint64_t *)(dst+len-196-i) = bswap64(*(uint64_t *)(src+i+192));
      *(uint64_t *)(dst+len-192-i) = bswap64(*(uint64_t *)(src+i+188));
      *(uint64_t *)(dst+len-188-i) = bswap64(*(uint64_t *)(src+i+184));
      *(uint64_t *)(dst+len-184-i) = bswap64(*(uint64_t *)(src+i+180));
      *(uint64_t *)(dst+len-180-i) = bswap64(*(uint64_t *)(src+i+176));
      *(uint64_t *)(dst+len-176-i) = bswap64(*(uint64_t *)(src+i+172));
      *(uint64_t *)(dst+len-172-i) = bswap64(*(uint64_t *)(src+i+168));
      *(uint64_t *)(dst+len-168-i) = bswap64(*(uint64_t *)(src+i+164));
      *(uint64_t *)(dst+len-164-i) = bswap64(*(uint64_t *)(src+i+160));
      *(uint64_t *)(dst+len-160-i) = bswap64(*(uint64_t *)(src+i+156));
      *(uint64_t *)(dst+len-156-i) = bswap64(*(uint64_t *)(src+i+152));
      *(uint64_t *)(dst+len-152-i) = bswap64(*(uint64_t *)(src+i+148));
      *(uint64_t *)(dst+len-148-i) = bswap64(*(uint64_t *)(src+i+144));
      *(uint64_t *)(dst+len-144-i) = bswap64(*(uint64_t *)(src+i+140));
      *(uint64_t *)(dst+len-140-i) = bswap64(*(uint64_t *)(src+i+136));
      *(uint64_t *)(dst+len-136-i) = bswap64(*(uint64_t *)(src+i+132));
      *(uint64_t *)(dst+len-132-i) = bswap64(*(uint64_t *)(src+i+128));
      *(uint64_t *)(dst+len-128-i) = bswap64(*(uint64_t *)(src+i+124));
      *(uint64_t *)(dst+len-124-i) = bswap64(*(uint64_t *)(src+i+120));
      *(uint64_t *)(dst+len-120-i) = bswap64(*(uint64_t *)(src+i+116));
      *(uint64_t *)(dst+len-116-i) = bswap64(*(uint64_t *)(src+i+112));
      *(uint64_t *)(dst+len-112-i) = bswap64(*(uint64_t *)(src+i+108));
      *(uint64_t *)(dst+len-108-i) = bswap64(*(uint64_t *)(src+i+104));
      *(uint64_t *)(dst+len-104-i) = bswap64(*(uint64_t *)(src+i+100));
      *(uint64_t *)(dst+len-100-i) = bswap64(*(uint64_t *)(src+i+92));
      *(uint64_t *)(dst+len-96-i)  = bswap64(*(uint64_t *)(src+i+88));
      *(uint64_t *)(dst+len-92-i)  = bswap64(*(uint64_t *)(src+i+84));
      *(uint64_t *)(dst+len-88-i)  = bswap64(*(uint64_t *)(src+i+80));
      *(uint64_t *)(dst+len-84-i)  = bswap64(*(uint64_t *)(src+i+76));
      *(uint64_t *)(dst+len-80-i)  = bswap64(*(uint64_t *)(src+i+72));
      *(uint64_t *)(dst+len-76-i)  = bswap64(*(uint64_t *)(src+i+68));
      *(uint64_t *)(dst+len-72-i)  = bswap64(*(uint64_t *)(src+i+64));
      *(uint64_t *)(dst+len-68-i)  = bswap64(*(uint64_t *)(src+i+60));
      *(uint64_t *)(dst+len-64-i)  = bswap64(*(uint64_t *)(src+i+56));
      *(uint64_t *)(dst+len-60-i)  = bswap64(*(uint64_t *)(src+i+52));
      *(uint64_t *)(dst+len-56-i)  = bswap64(*(uint64_t *)(src+i+48));
      *(uint64_t *)(dst+len-52-i)  = bswap64(*(uint64_t *)(src+i+44));
      *(uint64_t *)(dst+len-48-i)  = bswap64(*(uint64_t *)(src+i+40));
      *(uint64_t *)(dst+len-44-i)  = bswap64(*(uint64_t *)(src+i+36));
      *(uint64_t *)(dst+len-40-i)  = bswap64(*(uint64_t *)(src+i+32));
      *(uint64_t *)(dst+len-36-i)  = bswap64(*(uint64_t *)(src+i+28));
      *(uint64_t *)(dst+len-32-i)  = bswap64(*(uint64_t *)(src+i+24));
      *(uint64_t *)(dst+len-28-i)  = bswap64(*(uint64_t *)(src+i+20));
      *(uint64_t *)(dst+len-24-i)  = bswap64(*(uint64_t *)(src+i+16));
      *(uint64_t *)(dst+len-20-i)  = bswap64(*(uint64_t *)(src+i+12));
      *(uint64_t *)(dst+len-16-i)  = bswap64(*(uint64_t *)(src+i+ 8));
      *(uint64_t *)(dst+len-12-i)  = bswap64(*(uint64_t *)(src+i+ 4));
      *(uint64_t *)(dst+len- 8-i)  = bswap64(*(uint64_t *)(src+i+ 0));
      *(uint64_t *)(dst+len- 4-i)  = bswap64(*(uint64_t *)(src+i));
      *(uint64_t *)(dst+i)         = bswap64(*(uint64_t *)(src+len-8-i));
      *(uint64_t *)(dst+i+4)       = bswap64(*(uint64_t *)(src+len-12-i));
      *(uint64_t *)(dst+i+8)       = bswap64(*(uint64_t *)(src+len-16-i));
      *(uint64_t *)(dst+i+16)      = bswap64(*(uint64_t *)(src+len-24-i));
      *(uint64_t *)(dst+i+20)      = bswap64(*(uint64_t *)(src+len-28-i));
      *(uint64_t *)(dst+i+24)      = bswap64(*(uint64_t *)(src+len-32-i));
      *(uint64_t *)(dst+i+28)      = bswap64(*(uint64_t *)(src+len-36-i));
      *(uint64_t *)(dst+i+32)      = bswap64(*(uint64_t *)(src+len-40-i));
      *(uint64_t *)(dst+i+36)      = bswap64(*(uint64_t *)(src+len-44-i));
      *(uint64_t *)(dst+i+40)      = bswap64(*(uint64_t *)(src+len-48-i));
      *(uint64_t *)(dst+i+44)      = bswap64(*(uint64_t *)(src+len-52-i));
      *(uint64_t *)(dst+i+48)      = bswap64(*(uint64_t *)(src+len-56-i));
      *(uint64_t *)(dst+i+52)      = bswap64(*(uint64_t *)(src+len-60-i));
      *(uint64_t *)(dst+i+56)      = bswap64(*(uint64_t *)(src+len-64-i));
      *(uint64_t *)(dst+i+60)      = bswap64(*(uint64_t *)(src+len-68-i));
      *(uint64_t *)(dst+i+64)      = bswap64(*(uint64_t *)(src+len-72-i));
      *(uint64_t *)(dst+i+68)      = bswap64(*(uint64_t *)(src+len-76-i));
      *(uint64_t *)(dst+i+72)      = bswap64(*(uint64_t *)(src+len-80-i));
      *(uint64_t *)(dst+i+76)      = bswap64(*(uint64_t *)(src+len-84-i));
      *(uint64_t *)(dst+i+80)      = bswap64(*(uint64_t *)(src+len-88-i));
      *(uint64_t *)(dst+i+84)      = bswap64(*(uint64_t *)(src+len-92-i));
      *(uint64_t *)(dst+i+88)      = bswap64(*(uint64_t *)(src+len-96-i));
      *(uint64_t *)(dst+i+92)      = bswap64(*(uint64_t *)(src+len-100-i));
      *(uint64_t *)(dst+i+96)      = bswap64(*(uint64_t *)(src+len-104-i));
      *(uint64_t *)(dst+i+100)     = bswap64(*(uint64_t *)(src+len-108-i));
      *(uint64_t *)(dst+i+104)     = bswap64(*(uint64_t *)(src+len-112-i));
      *(uint64_t *)(dst+i+108)     = bswap64(*(uint64_t *)(src+len-116-i));
      *(uint64_t *)(dst+i+112)     = bswap64(*(uint64_t *)(src+len-120-i));
      *(uint64_t *)(dst+i+116)     = bswap64(*(uint64_t *)(src+len-124-i));
      *(uint64_t *)(dst+i+120)     = bswap64(*(uint64_t *)(src+len-128-i));
      *(uint64_t *)(dst+i+124)     = bswap64(*(uint64_t *)(src+len-132-i));
      *(uint64_t *)(dst+i+128)     = bswap64(*(uint64_t *)(src+len-136-i));
      *(uint64_t *)(dst+i+132)     = bswap64(*(uint64_t *)(src+len-140-i));
      *(uint64_t *)(dst+i+136)     = bswap64(*(uint64_t *)(src+len-144-i));
      *(uint64_t *)(dst+i+140)     = bswap64(*(uint64_t *)(src+len-148-i));
      *(uint64_t *)(dst+i+144)     = bswap64(*(uint64_t *)(src+len-152-i));
      *(uint64_t *)(dst+i+148)     = bswap64(*(uint64_t *)(src+len-156-i));
      *(uint64_t *)(dst+i+152)     = bswap64(*(uint64_t *)(src+len-160-i));
      *(uint64_t *)(dst+i+156)     = bswap64(*(uint64_t *)(src+len-164-i));
      *(uint64_t *)(dst+i+160)     = bswap64(*(uint64_t *)(src+len-168-i));
      *(uint64_t *)(dst+i+164)     = bswap64(*(uint64_t *)(src+len-172-i));
      *(uint64_t *)(dst+i+168)     = bswap64(*(uint64_t *)(src+len-176-i));
      *(uint64_t *)(dst+i+172)     = bswap64(*(uint64_t *)(src+len-180-i));
      *(uint64_t *)(dst+i+176)     = bswap64(*(uint64_t *)(src+len-184-i));
      *(uint64_t *)(dst+i+180)     = bswap64(*(uint64_t *)(src+len-188-i));
      *(uint64_t *)(dst+i+184)     = bswap64(*(uint64_t *)(src+len-192-i));
      *(uint64_t *)(dst+i+188)     = bswap64(*(uint64_t *)(src+len-196-i));
      *(uint64_t *)(dst+i+192)     = bswap64(*(uint64_t *)(src+len-200-i));
      *(uint64_t *)(dst+i+196)     = bswap64(*(uint64_t *)(src+len-204-i));
      *(uint64_t *)(dst+i+200)     = bswap64(*(uint64_t *)(src+len-208-i));
      *(uint64_t *)(dst+i+204)     = bswap64(*(uint64_t *)(src+len-212-i));
      *(uint64_t *)(dst+i+208)     = bswap64(*(uint64_t *)(src+len-216-i));
      *(uint64_t *)(dst+i+212)     = bswap64(*(uint64_t *)(src+len-220-i));
      *(uint64_t *)(dst+i+216)     = bswap64(*(uint64_t *)(src+len-224-i));
      *(uint64_t *)(dst+i+220)     = bswap64(*(uint64_t *)(src+len-228-i));
      *(uint64_t *)(dst+i+224)     = bswap64(*(uint64_t *)(src+len-232-i));
      *(uint64_t *)(dst+i+228)     = bswap64(*(uint64_t *)(src+len-236-i));
      *(uint64_t *)(dst+i+232)     = bswap64(*(uint64_t *)(src+len-240-i));
      *(uint64_t *)(dst+i+236)     = bswap64(*(uint64_t *)(src+len-244-i));
      *(uint64_t *)(dst+i+240)     = bswap64(*(uint64_t *)(src+len-248-i));
      *(uint64_t *)(dst+i+244)     = bswap64(*(uint64_t *)(src+len-252-i));
      *(uint64_t *)(dst+i+248)     = bswap64(*(uint64_t *)(src+len-256-i));
      *(uint64_t *)(dst+i+252)     = bswap64(*(uint64_t *)(src+len-260-i));
      i += 256;
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

/**
 * 2048-bit version of byte4_w64
 */
void byte256_w64(char *dst, const char *src, unsigned len)
{
  if (len) {
    long i = 0,
         halfway = len / 2;
    while (i <= halfway-256) {
      *(uint64_t *)(dst+len-256-i) = bswap64(*(uint64_t *)(src+i+248));
      *(uint64_t *)(dst+len-248-i) = bswap64(*(uint64_t *)(src+i+240));
      *(uint64_t *)(dst+len-240-i) = bswap64(*(uint64_t *)(src+i+232));
      *(uint64_t *)(dst+len-232-i) = bswap64(*(uint64_t *)(src+i+224));
      *(uint64_t *)(dst+len-224-i) = bswap64(*(uint64_t *)(src+i+216));
      *(uint64_t *)(dst+len-216-i) = bswap64(*(uint64_t *)(src+i+208));
      *(uint64_t *)(dst+len-208-i) = bswap64(*(uint64_t *)(src+i+200));
      *(uint64_t *)(dst+len-200-i) = bswap64(*(uint64_t *)(src+i+192));
      *(uint64_t *)(dst+len-192-i) = bswap64(*(uint64_t *)(src+i+184));
      *(uint64_t *)(dst+len-184-i) = bswap64(*(uint64_t *)(src+i+176));
      *(uint64_t *)(dst+len-176-i) = bswap64(*(uint64_t *)(src+i+168));
      *(uint64_t *)(dst+len-168-i) = bswap64(*(uint64_t *)(src+i+160));
      *(uint64_t *)(dst+len-160-i) = bswap64(*(uint64_t *)(src+i+152));
      *(uint64_t *)(dst+len-152-i) = bswap64(*(uint64_t *)(src+i+144));
      *(uint64_t *)(dst+len-144-i) = bswap64(*(uint64_t *)(src+i+136));
      *(uint64_t *)(dst+len-136-i) = bswap64(*(uint64_t *)(src+i+128));
      *(uint64_t *)(dst+len-128-i) = bswap64(*(uint64_t *)(src+i+120));
      *(uint64_t *)(dst+len-120-i) = bswap64(*(uint64_t *)(src+i+112));
      *(uint64_t *)(dst+len-112-i) = bswap64(*(uint64_t *)(src+i+104));
      *(uint64_t *)(dst+len-104-i) = bswap64(*(uint64_t *)(src+i+96));
      *(uint64_t *)(dst+len-96-i)  = bswap64(*(uint64_t *)(src+i+88));
      *(uint64_t *)(dst+len-88-i)  = bswap64(*(uint64_t *)(src+i+80));
      *(uint64_t *)(dst+len-80-i)  = bswap64(*(uint64_t *)(src+i+72));
      *(uint64_t *)(dst+len-72-i)  = bswap64(*(uint64_t *)(src+i+64));
      *(uint64_t *)(dst+len-64-i)  = bswap64(*(uint64_t *)(src+i+56));
      *(uint64_t *)(dst+len-56-i)  = bswap64(*(uint64_t *)(src+i+48));
      *(uint64_t *)(dst+len-48-i)  = bswap64(*(uint64_t *)(src+i+40));
      *(uint64_t *)(dst+len-40-i)  = bswap64(*(uint64_t *)(src+i+32));
      *(uint64_t *)(dst+len-32-i)  = bswap64(*(uint64_t *)(src+i+24));
      *(uint64_t *)(dst+len-24-i)  = bswap64(*(uint64_t *)(src+i+16));
      *(uint64_t *)(dst+len-16-i)  = bswap64(*(uint64_t *)(src+i+ 8));
      *(uint64_t *)(dst+len- 8-i)  = bswap64(*(uint64_t *)(src+i));
      *(uint64_t *)(dst+i)         = bswap64(*(uint64_t *)(src+len-8-i));
      *(uint64_t *)(dst+i+8)       = bswap64(*(uint64_t *)(src+len-16-i));
      *(uint64_t *)(dst+i+16)      = bswap64(*(uint64_t *)(src+len-24-i));
      *(uint64_t *)(dst+i+24)      = bswap64(*(uint64_t *)(src+len-32-i));
      *(uint64_t *)(dst+i+32)      = bswap64(*(uint64_t *)(src+len-40-i));
      *(uint64_t *)(dst+i+40)      = bswap64(*(uint64_t *)(src+len-48-i));
      *(uint64_t *)(dst+i+48)      = bswap64(*(uint64_t *)(src+len-56-i));
      *(uint64_t *)(dst+i+56)      = bswap64(*(uint64_t *)(src+len-64-i));
      *(uint64_t *)(dst+i+64)      = bswap64(*(uint64_t *)(src+len-72-i));
      *(uint64_t *)(dst+i+72)      = bswap64(*(uint64_t *)(src+len-80-i));
      *(uint64_t *)(dst+i+80)      = bswap64(*(uint64_t *)(src+len-88-i));
      *(uint64_t *)(dst+i+88)      = bswap64(*(uint64_t *)(src+len-96-i));
      *(uint64_t *)(dst+i+96)      = bswap64(*(uint64_t *)(src+len-104-i));
      *(uint64_t *)(dst+i+104)     = bswap64(*(uint64_t *)(src+len-112-i));
      *(uint64_t *)(dst+i+112)     = bswap64(*(uint64_t *)(src+len-120-i));
      *(uint64_t *)(dst+i+120)     = bswap64(*(uint64_t *)(src+len-128-i));
      *(uint64_t *)(dst+i+128)     = bswap64(*(uint64_t *)(src+len-136-i));
      *(uint64_t *)(dst+i+136)     = bswap64(*(uint64_t *)(src+len-144-i));
      *(uint64_t *)(dst+i+144)     = bswap64(*(uint64_t *)(src+len-152-i));
      *(uint64_t *)(dst+i+152)     = bswap64(*(uint64_t *)(src+len-160-i));
      *(uint64_t *)(dst+i+160)     = bswap64(*(uint64_t *)(src+len-168-i));
      *(uint64_t *)(dst+i+168)     = bswap64(*(uint64_t *)(src+len-176-i));
      *(uint64_t *)(dst+i+176)     = bswap64(*(uint64_t *)(src+len-184-i));
      *(uint64_t *)(dst+i+184)     = bswap64(*(uint64_t *)(src+len-192-i));
      *(uint64_t *)(dst+i+192)     = bswap64(*(uint64_t *)(src+len-200-i));
      *(uint64_t *)(dst+i+200)     = bswap64(*(uint64_t *)(src+len-208-i));
      *(uint64_t *)(dst+i+208)     = bswap64(*(uint64_t *)(src+len-216-i));
      *(uint64_t *)(dst+i+216)     = bswap64(*(uint64_t *)(src+len-224-i));
      *(uint64_t *)(dst+i+224)     = bswap64(*(uint64_t *)(src+len-232-i));
      *(uint64_t *)(dst+i+232)     = bswap64(*(uint64_t *)(src+len-240-i));
      *(uint64_t *)(dst+i+240)     = bswap64(*(uint64_t *)(src+len-248-i));
      *(uint64_t *)(dst+i+248)     = bswap64(*(uint64_t *)(src+len-256-i));
      i += 256;
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}


/**
 * process 4096 bits per loop, 64 bits at a time
 */
void byte512_w64(char *dst, const char *src, unsigned len)
{
  if (len) {
    long i = 0,
         halfway = len / 2;
    while (i <= halfway-512) {
      *(uint64_t *)(dst+len-512-i) = bswap64(*(uint64_t *)(src+i+504));
      *(uint64_t *)(dst+len-504-i) = bswap64(*(uint64_t *)(src+i+496));
      *(uint64_t *)(dst+len-496-i) = bswap64(*(uint64_t *)(src+i+488));
      *(uint64_t *)(dst+len-488-i) = bswap64(*(uint64_t *)(src+i+480));
      *(uint64_t *)(dst+len-480-i) = bswap64(*(uint64_t *)(src+i+472));
      *(uint64_t *)(dst+len-472-i) = bswap64(*(uint64_t *)(src+i+464));
      *(uint64_t *)(dst+len-464-i) = bswap64(*(uint64_t *)(src+i+456));
      *(uint64_t *)(dst+len-456-i) = bswap64(*(uint64_t *)(src+i+448));
      *(uint64_t *)(dst+len-448-i) = bswap64(*(uint64_t *)(src+i+440));
      *(uint64_t *)(dst+len-440-i) = bswap64(*(uint64_t *)(src+i+432));
      *(uint64_t *)(dst+len-432-i) = bswap64(*(uint64_t *)(src+i+424));
      *(uint64_t *)(dst+len-424-i) = bswap64(*(uint64_t *)(src+i+416));
      *(uint64_t *)(dst+len-416-i) = bswap64(*(uint64_t *)(src+i+408));
      *(uint64_t *)(dst+len-408-i) = bswap64(*(uint64_t *)(src+i+400));
      *(uint64_t *)(dst+len-400-i) = bswap64(*(uint64_t *)(src+i+392));
      *(uint64_t *)(dst+len-392-i) = bswap64(*(uint64_t *)(src+i+384));
      *(uint64_t *)(dst+len-384-i) = bswap64(*(uint64_t *)(src+i+376));
      *(uint64_t *)(dst+len-376-i) = bswap64(*(uint64_t *)(src+i+368));
      *(uint64_t *)(dst+len-368-i) = bswap64(*(uint64_t *)(src+i+360));
      *(uint64_t *)(dst+len-360-i) = bswap64(*(uint64_t *)(src+i+352));
      *(uint64_t *)(dst+len-352-i) = bswap64(*(uint64_t *)(src+i+344));
      *(uint64_t *)(dst+len-344-i) = bswap64(*(uint64_t *)(src+i+336));
      *(uint64_t *)(dst+len-336-i) = bswap64(*(uint64_t *)(src+i+328));
      *(uint64_t *)(dst+len-328-i) = bswap64(*(uint64_t *)(src+i+320));
      *(uint64_t *)(dst+len-320-i) = bswap64(*(uint64_t *)(src+i+312));
      *(uint64_t *)(dst+len-312-i) = bswap64(*(uint64_t *)(src+i+304));
      *(uint64_t *)(dst+len-304-i) = bswap64(*(uint64_t *)(src+i+296));
      *(uint64_t *)(dst+len-296-i) = bswap64(*(uint64_t *)(src+i+288));
      *(uint64_t *)(dst+len-288-i) = bswap64(*(uint64_t *)(src+i+280));
      *(uint64_t *)(dst+len-280-i) = bswap64(*(uint64_t *)(src+i+272));
      *(uint64_t *)(dst+len-272-i) = bswap64(*(uint64_t *)(src+i+264));
      *(uint64_t *)(dst+len-264-i) = bswap64(*(uint64_t *)(src+i+256));
      *(uint64_t *)(dst+len-256-i) = bswap64(*(uint64_t *)(src+i+248));
      *(uint64_t *)(dst+len-248-i) = bswap64(*(uint64_t *)(src+i+240));
      *(uint64_t *)(dst+len-240-i) = bswap64(*(uint64_t *)(src+i+232));
      *(uint64_t *)(dst+len-232-i) = bswap64(*(uint64_t *)(src+i+224));
      *(uint64_t *)(dst+len-224-i) = bswap64(*(uint64_t *)(src+i+216));
      *(uint64_t *)(dst+len-216-i) = bswap64(*(uint64_t *)(src+i+208));
      *(uint64_t *)(dst+len-208-i) = bswap64(*(uint64_t *)(src+i+200));
      *(uint64_t *)(dst+len-200-i) = bswap64(*(uint64_t *)(src+i+192));
      *(uint64_t *)(dst+len-192-i) = bswap64(*(uint64_t *)(src+i+184));
      *(uint64_t *)(dst+len-184-i) = bswap64(*(uint64_t *)(src+i+176));
      *(uint64_t *)(dst+len-176-i) = bswap64(*(uint64_t *)(src+i+168));
      *(uint64_t *)(dst+len-168-i) = bswap64(*(uint64_t *)(src+i+160));
      *(uint64_t *)(dst+len-160-i) = bswap64(*(uint64_t *)(src+i+152));
      *(uint64_t *)(dst+len-152-i) = bswap64(*(uint64_t *)(src+i+144));
      *(uint64_t *)(dst+len-144-i) = bswap64(*(uint64_t *)(src+i+136));
      *(uint64_t *)(dst+len-136-i) = bswap64(*(uint64_t *)(src+i+128));
      *(uint64_t *)(dst+len-128-i) = bswap64(*(uint64_t *)(src+i+120));
      *(uint64_t *)(dst+len-120-i) = bswap64(*(uint64_t *)(src+i+112));
      *(uint64_t *)(dst+len-112-i) = bswap64(*(uint64_t *)(src+i+104));
      *(uint64_t *)(dst+len-104-i) = bswap64(*(uint64_t *)(src+i+96));
      *(uint64_t *)(dst+len-96-i)  = bswap64(*(uint64_t *)(src+i+88));
      *(uint64_t *)(dst+len-88-i)  = bswap64(*(uint64_t *)(src+i+80));
      *(uint64_t *)(dst+len-80-i)  = bswap64(*(uint64_t *)(src+i+72));
      *(uint64_t *)(dst+len-72-i)  = bswap64(*(uint64_t *)(src+i+64));
      *(uint64_t *)(dst+len-64-i)  = bswap64(*(uint64_t *)(src+i+56));
      *(uint64_t *)(dst+len-56-i)  = bswap64(*(uint64_t *)(src+i+48));
      *(uint64_t *)(dst+len-48-i)  = bswap64(*(uint64_t *)(src+i+40));
      *(uint64_t *)(dst+len-40-i)  = bswap64(*(uint64_t *)(src+i+32));
      *(uint64_t *)(dst+len-32-i)  = bswap64(*(uint64_t *)(src+i+24));
      *(uint64_t *)(dst+len-24-i)  = bswap64(*(uint64_t *)(src+i+16));
      *(uint64_t *)(dst+len-16-i)  = bswap64(*(uint64_t *)(src+i+ 8));
      *(uint64_t *)(dst+len- 8-i)  = bswap64(*(uint64_t *)(src+i));
      *(uint64_t *)(dst+i)         = bswap64(*(uint64_t *)(src+len-8-i));
      *(uint64_t *)(dst+i+8)       = bswap64(*(uint64_t *)(src+len-16-i));
      *(uint64_t *)(dst+i+16)      = bswap64(*(uint64_t *)(src+len-24-i));
      *(uint64_t *)(dst+i+24)      = bswap64(*(uint64_t *)(src+len-32-i));
      *(uint64_t *)(dst+i+32)      = bswap64(*(uint64_t *)(src+len-40-i));
      *(uint64_t *)(dst+i+40)      = bswap64(*(uint64_t *)(src+len-48-i));
      *(uint64_t *)(dst+i+48)      = bswap64(*(uint64_t *)(src+len-56-i));
      *(uint64_t *)(dst+i+56)      = bswap64(*(uint64_t *)(src+len-64-i));
      *(uint64_t *)(dst+i+64)      = bswap64(*(uint64_t *)(src+len-72-i));
      *(uint64_t *)(dst+i+72)      = bswap64(*(uint64_t *)(src+len-80-i));
      *(uint64_t *)(dst+i+80)      = bswap64(*(uint64_t *)(src+len-88-i));
      *(uint64_t *)(dst+i+88)      = bswap64(*(uint64_t *)(src+len-96-i));
      *(uint64_t *)(dst+i+96)      = bswap64(*(uint64_t *)(src+len-104-i));
      *(uint64_t *)(dst+i+104)     = bswap64(*(uint64_t *)(src+len-112-i));
      *(uint64_t *)(dst+i+112)     = bswap64(*(uint64_t *)(src+len-120-i));
      *(uint64_t *)(dst+i+120)     = bswap64(*(uint64_t *)(src+len-128-i));
      *(uint64_t *)(dst+i+128)     = bswap64(*(uint64_t *)(src+len-136-i));
      *(uint64_t *)(dst+i+136)     = bswap64(*(uint64_t *)(src+len-144-i));
      *(uint64_t *)(dst+i+144)     = bswap64(*(uint64_t *)(src+len-152-i));
      *(uint64_t *)(dst+i+152)     = bswap64(*(uint64_t *)(src+len-160-i));
      *(uint64_t *)(dst+i+160)     = bswap64(*(uint64_t *)(src+len-168-i));
      *(uint64_t *)(dst+i+168)     = bswap64(*(uint64_t *)(src+len-176-i));
      *(uint64_t *)(dst+i+176)     = bswap64(*(uint64_t *)(src+len-184-i));
      *(uint64_t *)(dst+i+184)     = bswap64(*(uint64_t *)(src+len-192-i));
      *(uint64_t *)(dst+i+192)     = bswap64(*(uint64_t *)(src+len-200-i));
      *(uint64_t *)(dst+i+200)     = bswap64(*(uint64_t *)(src+len-208-i));
      *(uint64_t *)(dst+i+208)     = bswap64(*(uint64_t *)(src+len-216-i));
      *(uint64_t *)(dst+i+216)     = bswap64(*(uint64_t *)(src+len-224-i));
      *(uint64_t *)(dst+i+224)     = bswap64(*(uint64_t *)(src+len-232-i));
      *(uint64_t *)(dst+i+232)     = bswap64(*(uint64_t *)(src+len-240-i));
      *(uint64_t *)(dst+i+240)     = bswap64(*(uint64_t *)(src+len-248-i));
      *(uint64_t *)(dst+i+248)     = bswap64(*(uint64_t *)(src+len-256-i));
      *(uint64_t *)(dst+i+256)     = bswap64(*(uint64_t *)(src+len-264-i));
      *(uint64_t *)(dst+i+264)     = bswap64(*(uint64_t *)(src+len-272-i));
      *(uint64_t *)(dst+i+272)     = bswap64(*(uint64_t *)(src+len-280-i));
      *(uint64_t *)(dst+i+280)     = bswap64(*(uint64_t *)(src+len-288-i));
      *(uint64_t *)(dst+i+288)     = bswap64(*(uint64_t *)(src+len-296-i));
      *(uint64_t *)(dst+i+296)     = bswap64(*(uint64_t *)(src+len-304-i));
      *(uint64_t *)(dst+i+304)     = bswap64(*(uint64_t *)(src+len-312-i));
      *(uint64_t *)(dst+i+312)     = bswap64(*(uint64_t *)(src+len-320-i));
      *(uint64_t *)(dst+i+320)     = bswap64(*(uint64_t *)(src+len-328-i));
      *(uint64_t *)(dst+i+328)     = bswap64(*(uint64_t *)(src+len-336-i));
      *(uint64_t *)(dst+i+336)     = bswap64(*(uint64_t *)(src+len-344-i));
      *(uint64_t *)(dst+i+344)     = bswap64(*(uint64_t *)(src+len-352-i));
      *(uint64_t *)(dst+i+352)     = bswap64(*(uint64_t *)(src+len-360-i));
      *(uint64_t *)(dst+i+360)     = bswap64(*(uint64_t *)(src+len-368-i));
      *(uint64_t *)(dst+i+368)     = bswap64(*(uint64_t *)(src+len-376-i));
      *(uint64_t *)(dst+i+376)     = bswap64(*(uint64_t *)(src+len-384-i));
      *(uint64_t *)(dst+i+384)     = bswap64(*(uint64_t *)(src+len-392-i));
      *(uint64_t *)(dst+i+392)     = bswap64(*(uint64_t *)(src+len-400-i));
      *(uint64_t *)(dst+i+400)     = bswap64(*(uint64_t *)(src+len-408-i));
      *(uint64_t *)(dst+i+408)     = bswap64(*(uint64_t *)(src+len-416-i));
      *(uint64_t *)(dst+i+416)     = bswap64(*(uint64_t *)(src+len-424-i));
      *(uint64_t *)(dst+i+424)     = bswap64(*(uint64_t *)(src+len-432-i));
      *(uint64_t *)(dst+i+432)     = bswap64(*(uint64_t *)(src+len-440-i));
      *(uint64_t *)(dst+i+440)     = bswap64(*(uint64_t *)(src+len-448-i));
      *(uint64_t *)(dst+i+448)     = bswap64(*(uint64_t *)(src+len-456-i));
      *(uint64_t *)(dst+i+456)     = bswap64(*(uint64_t *)(src+len-464-i));
      *(uint64_t *)(dst+i+464)     = bswap64(*(uint64_t *)(src+len-472-i));
      *(uint64_t *)(dst+i+472)     = bswap64(*(uint64_t *)(src+len-480-i));
      *(uint64_t *)(dst+i+480)     = bswap64(*(uint64_t *)(src+len-488-i));
      *(uint64_t *)(dst+i+488)     = bswap64(*(uint64_t *)(src+len-496-i));
      *(uint64_t *)(dst+i+496)     = bswap64(*(uint64_t *)(src+len-504-i));
      *(uint64_t *)(dst+i+504)     = bswap64(*(uint64_t *)(src+len-512-i));
      i += 512;
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

#if CAN_DO == 128

#if 0 /* NOT totally clear the best way to get 128-bit int in GCC */
typedef __int128_t int128_t;
#else
typedef unsigned int int128_t __attribute__((mode(TI)));
#endif

inline int128_t bswap128(int128_t x)
{
  x = (((int128_t)bswap64((uint64_t)(x))) << 64) |
        (int128_t)bswap64((uint64_t)(x    >> 64));
  return x;
}

/**
 * 128-bit version of byte4_w
 */
void byte16_w128(char *dst, const char *src, unsigned len)
{
  if (len) {
    long i = 0,
         halfway = len / 2;
    while (i <= halfway-16) {
      *(int128_t *)(dst+len-16-i) = bswap128(*(int128_t *)(src+i));
      *(int128_t *)(dst+i) = bswap128(*(int128_t *)(src+len-16-i));
      i += 16;
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

/**
 * 256-bit version of byte4_w
 */
void byte32_w128(char *dst, const char *src, unsigned len)
{
  if (len) {
    long i = 0,
         halfway = len / 2;
    while (i <= halfway-32) {
      *(int128_t *)(dst+len-32-i) = bswap128(*(int128_t *)(src+i+16));
      *(int128_t *)(dst+len-16-i) = bswap128(*(int128_t *)(src+i));
      *(int128_t *)(dst+i)        = bswap128(*(int128_t *)(src+len-16-i));
      *(int128_t *)(dst+i+16)     = bswap128(*(int128_t *)(src+len-32-i));
      i += 32;
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

/**
 * 512-bit version of byte4_w
 */
void byte64_w128(char *dst, const char *src, unsigned len)
{
  if (len) {
    long i = 0,
         halfway = len / 2;
    while (i <= halfway-64) {
      *(int128_t *)(dst+len-64-i) = bswap128(*(int128_t *)(src+i+48));
      *(int128_t *)(dst+len-48-i) = bswap128(*(int128_t *)(src+i+32));
      *(int128_t *)(dst+len-32-i) = bswap128(*(int128_t *)(src+i+16));
      *(int128_t *)(dst+len-16-i) = bswap128(*(int128_t *)(src+i));
      *(int128_t *)(dst+i)        = bswap128(*(int128_t *)(src+len-16-i));
      *(int128_t *)(dst+i+16)     = bswap128(*(int128_t *)(src+len-32-i));
      *(int128_t *)(dst+i+32)     = bswap128(*(int128_t *)(src+len-48-i));
      *(int128_t *)(dst+i+48)     = bswap128(*(int128_t *)(src+len-64-i));
      i += 64;
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

/**
 * 1024-bit version of byte4_w
 */
void byte128_w128(char *dst, const char *src, unsigned len)
{
  if (len) {
    long i = 0,
         halfway = len / 2;
    while (i <= halfway-64) {
      *(int128_t *)(dst+len-128-i) = bswap128(*(int128_t *)(src+i+112));
      *(int128_t *)(dst+len-112-i) = bswap128(*(int128_t *)(src+i+96));
      *(int128_t *)(dst+len-96-i)  = bswap128(*(int128_t *)(src+i+80));
      *(int128_t *)(dst+len-80-i)  = bswap128(*(int128_t *)(src+i+64));
      *(int128_t *)(dst+len-64-i)  = bswap128(*(int128_t *)(src+i+48));
      *(int128_t *)(dst+len-48-i)  = bswap128(*(int128_t *)(src+i+32));
      *(int128_t *)(dst+len-32-i)  = bswap128(*(int128_t *)(src+i+16));
      *(int128_t *)(dst+len-16-i)  = bswap128(*(int128_t *)(src+i));
      *(int128_t *)(dst+i)         = bswap128(*(int128_t *)(src+len-16-i));
      *(int128_t *)(dst+i+16)      = bswap128(*(int128_t *)(src+len-32-i));
      *(int128_t *)(dst+i+32)      = bswap128(*(int128_t *)(src+len-48-i));
      *(int128_t *)(dst+i+48)      = bswap128(*(int128_t *)(src+len-64-i));
      *(int128_t *)(dst+i+64)      = bswap128(*(int128_t *)(src+len-80-i));
      *(int128_t *)(dst+i+80)      = bswap128(*(int128_t *)(src+len-96-i));
      *(int128_t *)(dst+i+96)      = bswap128(*(int128_t *)(src+len-112-i));
      *(int128_t *)(dst+i+112)     = bswap128(*(int128_t *)(src+len-128-i));
      i += 128;
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}

/**
 * 1024-bit version of byte4_w
 */
void byte256_w128(char *dst, const char *src, unsigned len)
{
  if (len) {
    long i = 0,
         halfway = len / 2;
    while (i <= halfway-256) {
      *(int128_t *)(dst+len-256-i) = bswap128(*(int128_t *)(src+i+240));
      *(int128_t *)(dst+len-240-i) = bswap128(*(int128_t *)(src+i+224));
      *(int128_t *)(dst+len-224-i) = bswap128(*(int128_t *)(src+i+208));
      *(int128_t *)(dst+len-208-i) = bswap128(*(int128_t *)(src+i+192));
      *(int128_t *)(dst+len-192-i) = bswap128(*(int128_t *)(src+i+176));
      *(int128_t *)(dst+len-176-i) = bswap128(*(int128_t *)(src+i+160));
      *(int128_t *)(dst+len-160-i) = bswap128(*(int128_t *)(src+i+144));
      *(int128_t *)(dst+len-144-i) = bswap128(*(int128_t *)(src+i+128));
      *(int128_t *)(dst+len-128-i) = bswap128(*(int128_t *)(src+i+112));
      *(int128_t *)(dst+len-112-i) = bswap128(*(int128_t *)(src+i+96));
      *(int128_t *)(dst+len-96-i)  = bswap128(*(int128_t *)(src+i+80));
      *(int128_t *)(dst+len-80-i)  = bswap128(*(int128_t *)(src+i+64));
      *(int128_t *)(dst+len-64-i)  = bswap128(*(int128_t *)(src+i+48));
      *(int128_t *)(dst+len-48-i)  = bswap128(*(int128_t *)(src+i+32));
      *(int128_t *)(dst+len-32-i)  = bswap128(*(int128_t *)(src+i+16));
      *(int128_t *)(dst+len-16-i)  = bswap128(*(int128_t *)(src+i));
      *(int128_t *)(dst+i)         = bswap128(*(int128_t *)(src+len-16-i));
      *(int128_t *)(dst+i+16)      = bswap128(*(int128_t *)(src+len-32-i));
      *(int128_t *)(dst+i+32)      = bswap128(*(int128_t *)(src+len-48-i));
      *(int128_t *)(dst+i+48)      = bswap128(*(int128_t *)(src+len-64-i));
      *(int128_t *)(dst+i+64)      = bswap128(*(int128_t *)(src+len-80-i));
      *(int128_t *)(dst+i+80)      = bswap128(*(int128_t *)(src+len-96-i));
      *(int128_t *)(dst+i+96)      = bswap128(*(int128_t *)(src+len-112-i));
      *(int128_t *)(dst+i+112)     = bswap128(*(int128_t *)(src+len-128-i));
      *(int128_t *)(dst+i+128)     = bswap128(*(int128_t *)(src+len-144-i));
      *(int128_t *)(dst+i+144)     = bswap128(*(int128_t *)(src+len-160-i));
      *(int128_t *)(dst+i+160)     = bswap128(*(int128_t *)(src+len-176-i));
      *(int128_t *)(dst+i+176)     = bswap128(*(int128_t *)(src+len-192-i));
      *(int128_t *)(dst+i+192)     = bswap128(*(int128_t *)(src+len-208-i));
      *(int128_t *)(dst+i+208)     = bswap128(*(int128_t *)(src+len-224-i));
      *(int128_t *)(dst+i+224)     = bswap128(*(int128_t *)(src+len-240-i));
      *(int128_t *)(dst+i+240)     = bswap128(*(int128_t *)(src+len-256-i));
      i += 256;
    }
    while (i <= halfway) {
      dst[i] = src[len-1-i];
      dst[len-1-i] = src[i];
      i++;
    }
  }
}


#endif


/************************* test crap ****************************/

#include <stdlib.h>
#ifdef WIN32

#include <sys/timeb.h> /* timeb */

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
  struct timeb tb;
	if(NULL == tv)
		return -1;
#if MSCVER > 1400
	_ftime_s(&tb); /* NOTE: VS2K5 warns about ftime() */
#else
  ftime(&tb);
#endif
	tv->tv_sec = (long)tb.time;
	tv->tv_usec = tb.millitm * 1000;
	return 0;
}
#else
# include <sys/time.h>
# include <time.h>
#endif

/**
 * test a function for correctness over a range of strings with
 * known correct answers.
 */
void test(const char *name, void (*f)(char *, const char *, unsigned))
{
  const struct {
    const unsigned len;
    const char * const str,
               * const rev;
  } *t, Test[] = {
    {    0, "", "" },
    {    1, "a", "a" },
    {    2, "ab", "ba" },
    {    3, "abc", "cba" },
    {    4, "abcd", "dcba" },
    {    5, "abcde", "edcba" },
    {    6, "abcdef", "fedcba" },
    {    7, "abcdefg", "gfedcba" },
    {    8, "01234567", "76543210" },
    {    9, "abcdefghi", "ihgfedcba" },
    {   10, "abcdefghij", "jihgfedcba" },
    {   11, "abcdefghijk", "kjihgfedcba" },
    {   12, "abcdefghijkl", "lkjihgfedcba" },
    {   13, "abcdefghijklm", "mlkjihgfedcba" },
    {   14, "abcdefghijklmn", "nmlkjihgfedcba" },
    {   15, "abcdefghijklmno", "onmlkjihgfedcba" },
    {   16, "abcdefghijklmnop", "ponmlkjihgfedcba" },
    {   17, "abcdefghijklmnopq", "qponmlkjihgfedcba" },
    {   18, "aaaaaaaaaaaaaaaaaa", "aaaaaaaaaaaaaaaaaa" },
    {   19, "aaaaaaaaaaaaaaaaaaa", "aaaaaaaaaaaaaaaaaaa" },
    {   64, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./",
            "/.9876543210ZYXWVUTSRQPONMLKJIHGFEDCBAzyxwvutsrqponmlkjihgfedcba" },
    {  128, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./"
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./",
            "/.9876543210ZYXWVUTSRQPONMLKJIHGFEDCBAzyxwvutsrqponmlkjihgfedcba" 
            "/.9876543210ZYXWVUTSRQPONMLKJIHGFEDCBAzyxwvutsrqponmlkjihgfedcba" },
    {  256, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./"
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./"
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./"
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./",
            "/.9876543210ZYXWVUTSRQPONMLKJIHGFEDCBAzyxwvutsrqponmlkjihgfedcba" 
            "/.9876543210ZYXWVUTSRQPONMLKJIHGFEDCBAzyxwvutsrqponmlkjihgfedcba" 
            "/.9876543210ZYXWVUTSRQPONMLKJIHGFEDCBAzyxwvutsrqponmlkjihgfedcba" 
            "/.9876543210ZYXWVUTSRQPONMLKJIHGFEDCBAzyxwvutsrqponmlkjihgfedcba" }
  };
  char pad[1] = { 0x7F }; /* detect buffer underflow */
  char dst[256];
  char pad2[1] = { 0x7F }; /* detect buffer overflow */
  unsigned i = 0,
           matches = 0,
           len;
  t = Test;
#undef VALIDATE /* #define this for verbose validation output; useful for developing new algorithms */
#ifdef VALIDATE
  printf("validating %s dst=%p...\n", name, (void *)dst);
#endif
  while (i < sizeof Test / sizeof Test[0])
  {
    int match;
#ifdef VALIDATE
    printf("%2u len=%u \"%.*s\" -> \"%.*s\" ",
      i, t->len, t->len, t->str, t->len, t->rev);
#endif
    fflush(stdout);
    len = t->len;
    assert(0x7F == pad[0]); /* check underflow */
    assert(0x7F == pad2[0]); /* check overflow */
    f(dst, t->str, t->len);
    assert(0x7F == pad[0]); /* check underflow */
    assert(0x7F == pad2[0]); /* check overflow */
    assert(t->len == len);
    match = 0 == memcmp(dst, t->rev, t->len); /* verify results */
    matches += match;
#ifdef VALIDATE
    printf("[%s] (\"%.*s\")\n",
      match ? "OK" : "!!", t->len, dst);
#endif
    assert(match);
    i++, t++;
  }
}

#define MAXLEN 128*1024 /* maximum string length to test */

/**
 * return the number of seconds required to run function 'f' against
 * input 'src' for all lengths [0..len]
 */
static double speed(char *dst, const char *src, unsigned len, void (*f)())
{
  double d[2], total;
  struct timeval tv[2];
  /* test speed */
  gettimeofday(tv, NULL);
  do
    f(dst, src, len);
  while (len--); /* run for all variations of [len..0] */
  gettimeofday(tv + 1, NULL);
  d[0] = (tv[0].tv_sec * 1000000) + tv[0].tv_usec;
  d[1] = (tv[1].tv_sec * 1000000) + tv[1].tv_usec;
  total = (d[1] - d[0]) / 1000000;
  printf("%5.2f ", total);
  fflush(stdout);
  return total;
}

/**
 * run function 'f' through a variety of lengths and print the total amount of time
 */
static void run(const char *name, void (*f)(char *, const char *, unsigned))
{
  static double FirstRun = 0;
  char *src = malloc(MAXLEN),
       *dst = malloc(MAXLEN);
  double time = 0;
  unsigned i = MAXLEN;
  assert(src);
  assert(dst);
  while (i--)
    src[i] = (char)(rand() % 10); /* ~10% chance swapped chars will be the same */
  printf("%28s ", name);
  fflush(stdout);
  /* test for correctness ... */
  test(name, f);
  time += speed(dst, src, MAXLEN, f);
  if (0 == FirstRun)
    FirstRun = time;
  printf(" %6.1f%%\n", FirstRun/time*100-100);
  if (obvious == f)
    FirstRun = time;
  free(src);
  free(dst);
}

/* Stringification */
#define S(str)  S_(str)
#define S_(str)  #str

/* handy macro so i can get the function name as a string and a symbol
 * from the same input, saves me typing and protects against human error */
#define V(f) run(S(f), f)

int main(void)
{
  printf("%28s %5s %7s\n", "function", "sec", "speedup");
  srand(time(NULL));
  V(obvious);
  V(obvious); /* run twice to get the CPU warmed up */
  V(byte4_w32);
  V(byte8_w32);
  V(byte8_w64);
  V(byte16_w32);
  V(byte16_w64);
#if CAN_DO == 128
  V(byte16_w128);
#endif
  V(byte32_w32);
  V(byte32_w_prefetch);
  V(byte32_w64);
#if CAN_DO == 128
  V(byte32_w128);
#endif
  V(byte64_w32);
  V(byte64_w64);
#if CAN_DO == 128
  V(byte64_w128);
#endif
  V(byte128_w32);
  V(byte128_w64);
#if CAN_DO == 128
  V(byte128_w128);
#endif
  V(byte256_w32);
  V(byte256_w64);
#if CAN_DO == 128
  V(byte256_w128);
#endif
  V(byte512_w64);
  V(obvious_prefetch);
  V(obvious_check);
  V(obvious_pointer);
  V(obvious_twoindex);
  V(insideout);
#if 0
  V(obvious_recurse); /* NOTE: not acceptable for long strings, sorry */
#endif
  V(byte2_unroll);
  V(byte4_unroll);
  V(byte4_unroll_prefetch);
  V(byte4_unroll2);
  V(byte4_unroll2_expect);
  V(byte4_unroll3);
  V(byte4_loop);
  V(byte4_wb);
  V(byte4_wc);
  V(byte8_unroll);
  V(byte8_subloop);
  V(obvious);
  return 0;
}

