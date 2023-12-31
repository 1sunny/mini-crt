//
// Created by will on 2023/10/28.
//

#include "minicrt.h"

typedef struct _heap_header {
    enum {
        HEAP_BLOCK_FREE = 0xABABABAB, // magic number of free block
        HEAP_BLOCK_USED = 0xCDCDCDCD, // magic number of used block
    } type; // block type FREE/USED
    unsigned size; // block size including header
    struct _heap_header *next;
    struct _heap_header *prev;
} heap_header;

#define ADDR_ADD(a, o) (((char*)(a)) + o)
#define HEADER_SIZE (sizeof(heap_header))
static heap_header *list_head = NULL;

void free(void *ptr) {
  heap_header *header = (heap_header *) ADDR_ADD(ptr, -HEADER_SIZE);
  if (header->type != HEAP_BLOCK_USED)
    return;
  header->type = HEAP_BLOCK_FREE;
  if (header->prev != NULL && header->prev->type == HEAP_BLOCK_FREE) {
// merge
    header->prev->next = header->next;
    if (header->next != NULL)
      header->next->prev = header->prev;
    header->prev->size += header->size;
    header = header->prev;
  }
  if (header->next != NULL && header->next->type == HEAP_BLOCK_FREE) {
// merge
    header->size += header->next->size;
    header->next = header->next->next;
  }
}

void *malloc(unsigned size) {
  heap_header *header;
  if (size == 0)
    return NULL;
  header = list_head;
  while (header != 0) {
    if (header->type == HEAP_BLOCK_USED) {
      header = header->next;
      continue;
    }
    if (header->size > size + HEADER_SIZE &&
        header->size <= size + HEADER_SIZE * 2) {
      header->type = HEAP_BLOCK_USED;
    }
    if (header->size > size + HEADER_SIZE * 2) {
// split
      heap_header *next = (heap_header *) ADDR_ADD(header, size +
                                                           HEADER_SIZE);
      next->prev = header;
      next->next = header->next;
      next->type = HEAP_BLOCK_FREE;
      next->size = header->size - (size - HEADER_SIZE);
      header->next = next;
      header->size = size + HEADER_SIZE;
      header->type = HEAP_BLOCK_USED;
      return ADDR_ADD(header, HEADER_SIZE);
    }
    header = header->next;
  }
  return NULL;
}

// Linux brk system call
// C library/kernel differences
//   The return value described above for brk() is the behavior
//   provided by the glibc wrapper function for the Linux brk() system
//   call.  (On most other implementations, the return value from
//   brk() is the same; this return value was also specified in
//   SUSv2.)  However, the actual Linux system call returns the new
//   program break on success.  On failure, the system call returns
//   the current break.
static long brk(void *end_data_segment) {
  long ret = 0;

  asm("movq $12, %%rax \n\t"
      "movq %1, %%rdi \n\t"
      "syscall \n\t"
      "movq %%rax, %0 \n\t"
          : "=r"(ret)
          : "m"(end_data_segment)
          : "%rax");

  return ret;
}

int mini_crt_heap_init() {
  void *base = NULL;
  heap_header *header = NULL;
// 32 MB heap size
  unsigned heap_size = 1024 * 1024 * 32;

  base = (void *) brk(0);
  void *end = ADDR_ADD(base, heap_size);
  end = (void *) brk(end);
  if (!end)
    return 0;

  header = (heap_header *) base;
  header->size = heap_size;
  header->type = HEAP_BLOCK_FREE;
  header->next = NULL;
  header->prev = NULL;
  list_head = header;
  return 1;
}