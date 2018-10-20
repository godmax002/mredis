# include <stdio.h>
# include <string.h>
# include "zmalloc.h"
int main(int argc, char *argv[]) {
    char* ptr;
    size_t size_2 = 2;
    
    ptr = zmalloc(size_2);
    strcpy(ptr, "abaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

    printf("%lu, %s", zsize(ptr), ptr);
}
