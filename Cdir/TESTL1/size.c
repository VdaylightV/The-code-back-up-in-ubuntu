#include <stdio.h>
#include <stdint.h>

struct header_t {
    union {
	    uintptr_t ptr;
	    uintptr_t brk;
	};

	size_t size;

	struct header* next;
};

int main() {
	
	printf("size_t:%ld\n",sizeof(size_t));
	printf("uintptr_t:%ld\n",sizeof(uintptr_t));
	printf("void*:%ld\n",sizeof(void*));
	printf("struct header_t:%ld\n",sizeof(struct header_t));
	return 0;

}
