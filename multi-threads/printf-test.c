#include "threads.h"
#include <stdio.h>

void thread_entry(int tid) {
	  for (int i = 1; i <= 5; i++) {
		      printf("T%d's %d-th operation\n", tid, i);
			    }
}

int main() {
	  for (int i = 0; i < 3; i++)
		      create(thread_entry);
}
