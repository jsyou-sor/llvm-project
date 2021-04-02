#include <stdio.h>
#include <stdlib.h>

int main()
{
  int *arr = malloc(10 * sizeof(int));
  *(arr + 5) = 2;

  return 0;
}
