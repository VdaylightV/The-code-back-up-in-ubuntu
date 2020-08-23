#include "threads.h"

void f() {
  static int x = 0;
  printf("Hello from thread #%d\n", x++);
  //while (1); // to make sure we're not just sequentially calling f()
  for(volatile int i=0; i < 1000000; i ++);
}

void Done() {
    printf("Done!\n");
}

int main() {
  for (int i = 0; i < 1000; i++) {
    create(f);
  }
  join(Done);
}
