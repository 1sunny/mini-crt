//
// Created by will on 2023/10/28.
//

#include "minicrt.h"

extern int main(int argc, char *argv[]);

void exit(int);

static void crt_fatal_error(const char *msg) {
  printf("fatal error: %s", msg);
  exit(1);
}

void mini_crt_entry(void) {
  int ret;

  int argc;
  char **argv;
  char *rbp_reg = 0;

  // rbp_reg = %rbp
  asm("movq %%rbp,%0 \n":"=r"(rbp_reg));

  argc = *(int *) (rbp_reg + 8);
  argv = (char **) (rbp_reg + 16);

  printf("argc: %d\n", argc);

  if (!mini_crt_heap_init())
    crt_fatal_error("heap initialize failed");
  if (!mini_crt_init_io())
    crt_fatal_error("IO initialize failed");

  ret = main(argc, argv);
  exit(ret);
}

void exit(int exitCode) {
//mini_crt_call_exit_routine();

  asm("movslq %0,%%rdi \n\t"
      "movq $60,%%rax \n\t"
      "syscall \n\t"
      "hlt \n\t"::"r"(exitCode));
}