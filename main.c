#include "io.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
  uint64_t off;
  const uint64_t cap;
  uint8_t *buf;
  bool comp;
} bytes_ctx;

qbs_io_respose_t bytes_read(void *ctx, uint8_t *b, uint64_t sz) {
  bytes_ctx *btx = (bytes_ctx *)ctx;
  if (btx->comp)
    return (qbs_io_respose_t){
        .err = qbs_io_err_no_progress,
        .n = 0,
    };

  if (btx->cap == btx->off) {
    btx->comp = true;
    return (qbs_io_respose_t){
        .err = qbs_io_err_eof,
        .n = 0,
    };
  }

  sz = qbs_io_min(sz, btx->cap - btx->off);
  for (size_t i = 0; i < sz; i++)
    b[i] = btx->buf[btx->off++];

  return (qbs_io_respose_t){
      .err = qbs_io_err_null,
      .n = sz,
  };
}

qbs_io_respose_t bytes_write(void *ctx, uint8_t *b, uint64_t sz) {
  bytes_ctx *btx = (bytes_ctx *)ctx;
  assert(sz <= btx->cap - btx->off);

  for (size_t i = 0; i < sz; i++)
    btx->buf[btx->off++] = b[i];

  return (qbs_io_respose_t){
      .err = qbs_io_err_null,
      .n = sz,
  };
}

#define TEST "this is a test bytes. blah, blah ... blah\n"
#define TEST2 TEST TEST
#define TEST3 TEST2 TEST2
#define TEST4 TEST3 TEST3
#define TEST5 TEST4 TEST4
#define TEST6 TEST5 TEST5

void test_copy_n() {
  unsigned char in[] = {TEST6};
  uint8_t out[2024] = {0};

  bytes_ctx rtx = {
      .cap = sizeof(in),
      .off = 0,
      .buf = in,
      .comp = false,
  };
  bytes_ctx wtx = {
      .cap = 2024,
      .off = 0,
      .buf = out,
      .comp = false,
  };

  qbs_io_t r = {
      .ctx = &rtx,
      .read = bytes_read,
      .write = 0,
  };
  qbs_io_t w = {
      .ctx = &wtx,
      .read = 0,
      .write = bytes_write,
  };

  uint64_t n = sizeof(TEST) - 1;
  qbs_io_respose_t rio = qbs_io_copy_n(&r, &w, n);

  printf("the size of n %ld expected %ld\n", rio.n, n);
  assert(rio.err == qbs_io_err_null);
  assert(rio.n == n);
  printf("%s", out);
}

void test_copy() {
  unsigned char in[] = {TEST6};
  uint8_t out[2024] = {0};

  bytes_ctx rtx = {
      .cap = sizeof(in),
      .off = 0,
      .buf = in,
      .comp = false,
  };
  bytes_ctx wtx = {
      .cap = 2024,
      .off = 0,
      .buf = out,
      .comp = false,
  };

  qbs_io_t r = {
      .ctx = &rtx,
      .read = bytes_read,
      .write = 0,
  };
  qbs_io_t w = {
      .ctx = &wtx,
      .read = 0,
      .write = bytes_write,
  };

  qbs_io_respose_t rio = qbs_io_copy(&r, &w);
  assert(rio.err == qbs_io_err_null);
  printf("the size of n %ld expected %ld\n", rio.n, sizeof(in));
  assert(rio.n == sizeof(in));
  printf("%s", out);
}

void test_read_full() {
  unsigned char in[] = {TEST6};
  unsigned char out[1111] = {0};
  bytes_ctx rtx = {
      .cap = sizeof(in),
      .off = 0,
      .buf = in,
      .comp = false,
  };

  qbs_io_t r = {
      .ctx = &rtx,
      .read = bytes_read,
      .write = 0,
  };

  qbs_io_respose_t rio = qbs_io_read_full(&r, out, sizeof(out));
  assert(rio.err == qbs_io_err_null);
  printf("the size of n %ld expected %ld\n", rio.n, sizeof(out));
  assert(rio.n == sizeof(out));
  printf("%s\n", out);
}

int main(void) {
  test_copy();
  test_copy_n();
  test_read_full();
  return 0;
}
