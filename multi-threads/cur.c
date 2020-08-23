#include "threads.h"

void f() {
	  static int x = 0;
	    printf("Hello from thread #%d\n", x++);
//		  while (1); // make sure we're not just sequentially calling f()
}

void over() {
     printf("Over!\n");
}

int main() {
	  for (int i = 0; i < 1000; i++) {
		      create(f);
      }
	  join(over);
	  
}

