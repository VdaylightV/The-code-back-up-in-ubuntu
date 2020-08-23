#include "stdio.h"
#include "stdlib.h"
#include "string.h"

 #define LENGTH(arr) (sizeof(arr)/sizeof(arr[0])) 


struct state {
    char buf[5];
};

typedef struct state state;


state states[] = {
    { "[(\0\0\0" },
	{ "[\0\0\0\0" },
};

int main() {
    char buf[5] = "\0\0\0\0\0";
	char ch = '[';
    char* tmp = (char*)malloc(5*sizeof(char));
    char* bit = (char*)malloc(2*sizeof(char));
    memset(tmp, '\0', 5);
	memset(bit, '\0', 2);
	bit[0] = ch;
	strcpy(tmp, buf);
	strcat(tmp, bit);
	int i = strcmp(tmp, states[1].buf);
	printf("i = %d\n", i);
	return 0;

}
