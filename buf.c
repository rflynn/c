/* ex: set ff=dos ts=2 et: */
/* $Id$ */
/*
 * Copyright 2008 Ryan Flynn
 * All rights reserved.
 */
/*
 * Conceptual Model of a buf:
 *
 *        data
 *         |..............................................|
 *            ^                    ^                      ^
 *            |                    |                      |
 *          start                 len                   buflen
 *   (some offset where   (how much data we      (how many total bytes are
 *   our data begins)     contain after start)   available for storage)
 *
 * a buf manages a fixed-size buffer within which a single, contiguous set
 * of bytes may be stored.
 *
 * basic operations:
 *   append  moves 'len'   forward
 *   clear   moves 'len'   backward to 0
 *   consume moves 'start' forward
 *   shift   moves 'start' backward to 0
 *
 * some functions allow a buf to be managed circularly; that is, for
 * start + len to be > buflen; bytes are stored up until 'buflen' and then
 * wrapped around to the beginning; this can be done for performance reasons
 * to avoid gratuitous 'shift'ing of 'start' as bytes are consumed; but obviously
 * care must be taken to use the correct operations
 *
 * NOTE: don't modify the internals of 'buf' unless you really know what you are
 * doing, it was hard to get right!
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buf.h"
#include "util.h"

void buf_init(buf *b, u8 *buf, size_t buflen)
{
  ASSERT(buf);
  ASSERT(buflen);
  b->data = buf;
  b->buflen = buflen;
  b->start = 0,
  b->len = 0;
  memset(buf, 0xFF, buflen); /* NOTE: helps us find errors in buf's implementation;
                              * performance hit is minimal as bufs are initialized
                              * once upon startup and only rarely thereafter */
}

/**
 * return the number of bytes between 'ptr' and the end of the buffer (contiguously)
 * @return number of bytes of data after ptr if ptr is between start and len, 0 otherwise
 */
size_t buf_after(const buf *b, const u8 *ptr)
{
  size_t after = 0;
  ptrdiff_t diff = ptr - (b->data + b->start);
  ASSERT(diff >= 0);
  ASSERT((size_t)diff <= b->len); /* try to catch programming errors where we run off the end */
  after = b->len - diff;
  return after;
}

/**
 * for appending know data to a buf, i.e. for sending
 */
size_t buf_append(buf *b, const u8 *s, size_t len)
{
  if (len > buf_space(b)) {
    if (b->start) { /* try to make more room */
      buf_shift(b);
    }
    assert(len <= buf_space(b));
  }
  memcpy(buf_end(b), s, len);
  b->len += len;
  ASSERT(b->len < b->buflen);
  return len;
}

/**
 * treat buffer as circular buffer, wrap around at the end, as long as
 * we don't pass start again.
 */
size_t buf_append_circ(buf *b, const u8 *s, size_t len)
{
  size_t contig,
         endpos;
  if (buf_len(b) + len > buf_buflen(b)) {
    LOGF(__FILE__, __LINE__, NULL, "BUFFER FULL, TRUNCATING (buf_len=%lu + len=%lu > buf_buflen=%lu)\n",
      (unsigned long)buf_len(b), (unsigned long)len, (unsigned long)buf_buflen(b));
    buf_clr(b);
  }
#if 0 /* dammit, this happens too often and I'm not sure if it's my fault or the CA's... */
  ASSERT("buf too small!" && buf_len(b) + len <= buf_buflen(b));
#endif
  contig = buf_space_contig(b);
  endpos = b->start + b->len;
  printf("%s:%u: len=%u contig=%u endpos=%u b->start=%u b->len=%u b->buflen=%u buf=\"",
    __func__, __LINE__, len, contig, endpos, b->start, b->len, b->buflen);
  dump_chars((char*)b->data, b->buflen, stdout);
  printf("\" s=\"");
  dump_chars((char*)s, len, stdout);
  printf("\"\n");
  if (len <= contig) { /* straight-forward */
    memcpy(b->data + endpos, s, len);
  } else { /* is circular */
    memcpy(b->data + endpos, s, contig); /* to the end... */
    memcpy(b->data, s + contig, len - contig); /* rest at the beginning */
    endpos = len - contig;
  }
  b->len += len;
  ASSERT(b->len <= b->buflen);
  ASSERT(b->start < b->buflen);
#if 0
  printf("buf_append_circ() len=%lu\n", (unsigned long)b->len);
#endif
  return len;
}

/**
 * how many contiguous bytes lie after b->start
 */
size_t buf_data_contig(buf *b)
{
  size_t contig = b->len;
  if (b->start + b->len > b->buflen)
    contig = b->buflen - b->start;
  return contig;
}

/**
 * how much contiguous space is at the "end"
 */
size_t buf_space_contig(buf *b)
{
  size_t contig = b->start + b->len;
  if (contig <= b->buflen) {
    contig = b->buflen - contig;
  } else { /* is circular */
    contig = b->start - (contig - b->buflen);
  }
  return contig;
}

/**
 * record next 'len' bytes as consumed
 */
void buf_consume(buf *b, size_t len)
{
  ASSERT("buf_consume() can't consume more than we have!" && len <= buf_len(b));
  b->start += len;
  b->start %= b->buflen; /* wrap around */
  b->len -= len;
  ASSERT(b->start < b->buflen);
}

/**
 * if one wants to directly write to 'data' (i.e. use it as a buffer via recv(2))
 * one can update length here
 */
void buf_lengthen(buf *b, size_t len)
{
  if (b->len + len > b->buflen) {
    LOGF(__FILE__, __LINE__, NULL, "buf_lengthen: b->len=%u + len=%u > b->buflen=%u!!\n",
      (unsigned)b->len, (unsigned)len, (unsigned)b->buflen);
    abort();
  }
  b->len += len;
}

void buf_shift(buf *b)
{
  ASSERT(b->len <= b->buflen);
  ASSERT(b->start < b->buflen);
#if 0 /* i think this is the right thing to do, but not certain... */
  ASSERT(b->start + b->len <= b->buflen); /* shouldn't be called on a circular buffer... right? */
#endif
  /* skip empty, full or circular */
  if (b->len > 0 && b->start > 0)
    memmove(b->data, b->data + b->start, b->len);
  b->start = 0;
}

void buf_clr(buf *b)
{
  b->start = b->len = 0;
}

#ifdef TEST

static void test_append_circ(void)
{
  u8 data[3] = "\xFF\xFF\xFF"; /* remember that currently buf_init() overwrites with 0xFF */
  buf b;
  buf_init(&b, data, sizeof data);
  printf("test_append_circ: around and around we go... ");
  assert(data == buf_start(&b));
  assert(0 == buf_len(&b));
  buf_append_circ(&b, (u8 *)"A", 1);
  assert(0 == memcmp(data, (u8 *)"A\xFF\xFF", 3));
  buf_append_circ(&b, (u8 *)"B", 1);
  assert(0 == memcmp(data, "AB\xFF", 3));
  buf_append_circ(&b, (u8 *)"C", 1);
  assert(0 == memcmp(data, (u8 *)"ABC", 3));
  buf_consume(&b, 3);
  buf_append_circ(&b, (u8 *)"XYZ", 3);
  assert(0 == memcmp(data, (u8 *)"XYZ", 3));
  buf_consume(&b, 1);
  buf_append_circ(&b, (u8 *)"A", 1);
  assert(0 == memcmp(data, (u8 *)"AYZ", 3));
  buf_consume(&b, 3);
  buf_append_circ(&b, (u8 *)"BCD", 3);
  assert(0 == memcmp(data, (u8 *)"DBC", 3));
  buf_append_circ(&b, (u8 *)"", 0);
  printf("OK.\n");
}

static void test_contig(void)
{
  u8 data[3] = "\xFF\xFF\xFF"; /* remember that currently buf_init() overwrites with 0xFF */
  buf b;
  buf_init(&b, data, sizeof data);
  printf("test_contig... ");

  assert(0 == buf_len(&b));
  assert(0 == buf_data_contig(&b));
  assert(3 == buf_space_contig(&b));

  buf_append_circ(&b, (u8 *)"A", 1);
  assert(1 == buf_len(&b));
  assert(1 == buf_data_contig(&b));
  assert(2 == buf_space_contig(&b));

  buf_append_circ(&b, (u8 *)"B", 1);
  assert(2 == buf_len(&b));
  assert(2 == buf_data_contig(&b));
  assert(1 == buf_space_contig(&b));

  buf_consume(&b, 2);
  assert(0 == buf_len(&b));
  assert(0 == buf_data_contig(&b));
  assert(1 == buf_space_contig(&b));

  buf_append_circ(&b, (u8 *)"CD", 2);
  assert(2 == buf_len(&b));
  assert(1 == buf_data_contig(&b));
  assert(1 == buf_space_contig(&b));

  buf_consume(&b, 2);
  assert(0 == buf_len(&b));
  assert(0 == buf_data_contig(&b));
  assert(2 == buf_space_contig(&b));

  printf("OK.\n");
}

int main(void)
{
  test_append_circ();
  test_contig();
  return 0;
}

#endif

