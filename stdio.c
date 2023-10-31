//
// Created by will on 2023/10/28.
//

#include "minicrt.h"

int mini_crt_init_io() {
  return 1;
}

static int open(const char *pathname, int flags, int mode) {
  int fd = 0;
  asm("movq $2,%%rax \n\t"
      "movq %1,%%rdi \n\t"
      "movslq %2,%%rsi \n\t"
      "movslq %3,%%rdx \n\t"
      "syscall \n\t"
      "movl %%eax,%0 \n\t"
          : "=m"(fd)
          : "m"(pathname), "r"(flags), "r"(mode)
          : "%rax");
  return fd;
}

static int read(int fd, void *buffer, unsigned size) {
  int ret = 0;
  asm("movq $0,%%rax \n\t"
      "movslq %1,%%rdi \n\t"
      "movq %2,%%rsi \n\t"
      "movslq %3,%%rdx \n\t"
      "syscall \n\t"
      "movl %%eax,%0 \n\t"
          : "=m"(ret)
          : "r"(fd), "m"(buffer), "r"(size)
          : "%rax");
  return ret;
}

static int write(int fd, const void *buffer, unsigned size) {
  int ret = 0;
  asm("movq $1,%%rax \n\t"
      "movslq %1,%%rdi \n\t"
      "movq %2,%%rsi \n\t"
      "movslq %3,%%rdx \n\t"
      "syscall \n\t"
      "movl %%eax,%0 \n\t"
          : "=m"(ret)
          : "r"(fd), "m"(buffer), "r"(size)
          : "%rax");
  return ret;
}

static int close(int fd) {
  int ret = 0;
  asm("movq $3,%%rax \n\t"
      "movslq %1,%%rdi \n\t"
      "syscall \n\t"
      "movl %%eax,%0 \n\t"
          : "=m"(ret)
          : "r"(fd)
          : "%rax");
  return ret;
}

static int seek(int fd, int offset, int mode) {
  int ret = 0;
  asm("movq $8,%%rax \n\t"
      "movslq %1,%%rdi \n\t"
      "movslq %2,%%rsi \n\t"
      "movslq %3,%%rdx \n\t"
      "syscall \n\t"
      "movl %%eax,%0 \n\t"
          : "=m"(ret)
          : "r"(fd), "r"(offset), "r"(mode)
          : "%rax");
  return ret;
}

FILE *fopen(const char *filename, const char *mode) {
  int fd;
  int flags = 0;
  int access = 00700;
// 创建文件的权限
// 来自于/usr/include/fcntl.h
//   -> /usr/include/x86_64-linux-gnu/bits/fcntl.h
//   -> /usr/include/x86_64-linux-gnu/bits/fcntl-linux.h
// 注意：以0开始的数字是八进制的
#define O_RDONLY 00
#define O_WRONLY 01
#define O_RDWR 02
#define O_CREAT 0100
#define O_TRUNC 01000
#define O_APPEND 02000
  if (strcmp(mode, "w") == 0)
    flags |= O_WRONLY | O_CREAT | O_TRUNC;
  if (strcmp(mode, "w+") == 0)
    flags |= O_RDWR | O_CREAT | O_TRUNC;
  if (strcmp(mode, "r") == 0)
    flags |= O_RDONLY;
  if (strcmp(mode, "r+") == 0)
    flags |= O_RDWR | O_CREAT;
  fd = open(filename, flags, access);
  return (FILE *) fd;
}

int fread(void *buffer, int size, int count, FILE *stream) {
  return read((int) stream, buffer, size * count);
}

int fwrite(const void *buffer, int size, int count, FILE *stream) {
  return write((int) stream, buffer, size * count);
}

int fclose(FILE *fp) {
  return close((int) fp);
}

int fseek(FILE *fp, int offset, int set) {
  return seek((int) fp, offset, set);
}