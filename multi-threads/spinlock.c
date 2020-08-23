#include "threads.h"
#include <assert.h>

volatile char compare_and_swap(int *ptr, int old, int new) {
	    unsigned char ret;
	    // Note that sete sets a ’byte’ not the word
	    __asm__ __volatile__ (
			" lock\n"
			" cmpxchgl %2,%1\n"
			" sete %0\n"
			: "=q" (ret), "=m" (*ptr)
			: "r" (new), "m" (*ptr), "a" (old)
			: "memory");
	    return ret;
}


typedef struct spinlock {
  int locked;
} spinlock_t;

void spin_init(spinlock_t *lk) {
  lk->locked = 0;
}

void spin_lock(spinlock_t *lk) {
  //while (atomic_xchg(&lk->locked, 1)) ;
  while(compare_and_swap(&lk->locked, 0, 1));
  //assert(lk->locked == 1);
}

void spin_unlock(spinlock_t *lk) {
  //atomic_xchg(&lk->locked, 0);
  compare_and_swap(&lk->locked, 1, 0);
  //assert(lk->locked == 0);
}

spinlock_t lk1, lk2;

long volatile sum = 0;

void do_sum1() {
  for (int i = 0; i < 1000000; i++) {
    spin_lock(&lk1);
    sum++;
    spin_unlock(&lk1);
  }
}

void do_sum2() {
  for (int i = 0; i < 1000000; i++) {
    spin_lock(&lk2);
    sum++;
    spin_unlock(&lk2);
  }
}

void print() {
  printf("sum = %ld\n", sum);
}

int main() {
  spin_init(&lk1);
  for (int i = 0; i < 4; i++) {
	  if(i % 2 == 0 || i %2 == 1) {
	      create(do_sum1);
	  }
	  else {
	      create(do_sum2);
	  }
  }
  join(print);
}
