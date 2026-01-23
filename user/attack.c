#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

int
main(int argc, char *argv[])
{
  // Your code here.
  char *addr = sbrk(1);
  printf("%s\n", addr+32);

  exit(0);
}
