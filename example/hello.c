#include <stdio.h>

void func_say(char *);
void func_do(void (*)(char *));

void (*g_func)(char *) = func_say;
char *g_string;

__attribute_noinline__
void func_say(char *string) {
  printf("%s", string);
}

__attribute_noinline__
void func_do(void (*f)(char *)) {
  f(g_string);
}

int main(void) {
  g_string = "Hello World\n";
  func_do(g_func);
  return 0;
}
