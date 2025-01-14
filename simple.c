

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>


#define COUNT 1024

int main() {

    void* ptrs[COUNT];

    for(int i = 0; i < COUNT; i++){
        ptrs[i] = malloc(16 + i * 16);
    }
    for(int i = 0; i < COUNT; i++){
        free(ptrs[i]);
    }
}


