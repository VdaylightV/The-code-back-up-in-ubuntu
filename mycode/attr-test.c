#include <stdio.h>

__attribute__((constructor)) void SayHi() {
    printf("Hello World!\n");
}

__attribute__((destructor)) void SayBye() {
    printf("Bye Program!\n");
}

int 
main() {
    return 0;
}
