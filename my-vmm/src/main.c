#include "../include/init.h"
#include <stdio.h>
int main(int argc, char *argv[]) {
  int err;
  printf("Hello, World!\n");
  printf("constucting allocators\n");
  err = initialize_allocators();
  printf("finished allocator init with result: ");
  if (err) {
    printf("we done fucked up with initialize_allocators\n");
  }
  printf("allocators ready\n");
  /* Root tasks must not return; there is no process to exit to. */
  printf("Im the root task kids, you dont see systemd return anything :)\n");
  while (1) {
  }
}
