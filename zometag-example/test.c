#include <stdio.h>
#include <stdlib.h>

struct TestStruct {
  int a;
  int b;
  int *ptr;
};

int main()
{
  printf("Starting TEST program...\n");
  void *p = malloc(256);
  struct TestStruct *q = malloc(sizeof(struct TestStruct));
  void *r;
  void *s;

  r = q->ptr;
  s = p + 100;

  printf("p: %p\nq: %p\nr: %p\n", p, q, r);

  printf("TEST done...\n");

  return 0;
}
