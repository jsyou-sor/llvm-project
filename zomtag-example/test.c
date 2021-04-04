#include <stdio.h>
#include <stdlib.h>

int main()
{
  printf("Starting TEST program...\n");
  void *p = malloc(256);
  void *q = NULL;

  q = p + 100;
  printf("p: %p\nq: %p\n", p, q);

  printf("TEST done...\n");

  return 0;
}
