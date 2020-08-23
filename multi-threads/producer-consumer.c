#include "threads.h"

#define N 1

sem_t fill, empty;

void producer() {
//  while (1) {
    P(&empty);
    printf("(");
	fflush(stdout);
    V(&fill);
//	while(1);
//  }
}

void consumer() {
//  while (1) {
    P(&fill);
    printf(")");
	fflush(stdout);
    V(&empty);
//	while(1);
//  }
}

void over() {
    printf("\nGame Over!\n");
}

int main() {
  SEM_INIT(fill, 0);
  SEM_INIT(empty, N);
  for (int i = 0; i < 3; i++) create(producer);
  for (int i = 0; i < 1; i++) create(consumer);
  join(over);
}
