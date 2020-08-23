#include "stdio.h"
#include "string.h"

int main() {
    char a[10] = "hs8j4a";
    char* str = strchr(a, '0');
    
    char* key = strchr(a, 'h');

    char* res = str || key; 
    
    printf("Hello World! %s\n", res);
    return 0;
}
