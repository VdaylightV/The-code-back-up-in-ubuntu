#include "threads.h"
#define nworkers 4

int done;

void workload() {
	  // workload
	  asm volatile("lock addq $1, %0" :
			                 "=m"(done)); // sync(): 顺序、原子、可见
}

int main() {
	  for (int i = 0; i < nworkers; i++) 
		      create(workload);
	    while (done != nworkers); // sync(): 等到所有 workers 都完成
	    printf("All done\n");
}

