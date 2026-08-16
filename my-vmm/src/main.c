#include <stdio.h>

int main(int argc, char *argv[]) {
  printf("Hello, World!\n");
  printf("Im the root task kids, you dont see systemd return anything");

  /* Root tasks must not return; there is no process to exit to. */
  while (1) {
  }
}
