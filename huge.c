#include <stdio.h>
#include <unistd.h>


int main() {
    foo();
    printf("pid = %d\n", getpid());
	while(1) sleep(1);
}
