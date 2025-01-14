

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

struct malloc_chunk {
    size_t prev_size;   /* Size of previous chunk (if free). */
    size_t size;        /* Size of this chunk, including metadata. */

    struct malloc_chunk* fd;         /* Forward pointer (for free list). */
    struct malloc_chunk* bk;         /* Backward pointer (for free list). */

    struct malloc_chunk* fd_nextsize; /* Forward pointer for small bins. */
    struct malloc_chunk* bk_nextsize; /* Backward pointer for small bins. */
};

void print_basic_info(struct malloc_chunk* chunk){
    size_t chunk_size = chunk->size & ~0x7; // Mask out the lower 3 bits (flags)
    int prev_inuse = chunk->size & 0x1;
    int is_mmapped = chunk->size & 0x2;
    int non_main_arena = chunk->size & 0x4;

    printf("Chunk address: %p\n", chunk);
    printf("  Prev size (if free): %zu\n", chunk->prev_size);
    printf("  Size: %zu\n", chunk_size);
    printf("  Flags:\n");
    printf("    Previous in use: %s\n", prev_inuse ? "Yes" : "No");
    printf("    Is mmapped: %s\n", is_mmapped ? "Yes" : "No");
    printf("    Non-main arena: %s\n", non_main_arena ? "Yes" : "No");
}

void print_freed_info(struct malloc_chunk* chunk) {
    printf("  Forward pointer (fd): %p\n", chunk->fd);
    printf("  Backward pointer (bk): %p\n", chunk->bk);
    printf("  fd_nextsize: %p\n", chunk->fd_nextsize);
    printf("  bk_nextsize: %p\n", chunk->bk_nextsize);
}

struct malloc_chunk* malloc_chunk(int size){
    void* ptr = malloc(size);
    return (struct malloc_chunk *)(ptr - sizeof(void*) * 2);
}
void free_chunk(struct malloc_chunk* chunk){
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wfree-nonheap-object"
    free(((void*)chunk) + sizeof(void*) * 2);
    #pragma GCC diagnostic pop
}

int main() {
    struct malloc_chunk* chunk1 = malloc_chunk(100);
    struct malloc_chunk* chunk2 = malloc_chunk(100);
    struct malloc_chunk* chunk3 = malloc_chunk(100);
    printf("\nMALLOCED INFORMATION\n");
    printf("----------------------------------\n");
    print_basic_info(chunk2);

    // printf("\nOVERALL INFORMATION\n");
    // printf("----------------------------------\n");


    // malloc_info(0, stdout);
    free_chunk(chunk1);
    free_chunk(chunk2);
    free_chunk(chunk3);


    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wuse-after-free"
    printf("\nFREED INFORMATION 1\n");
    printf("----------------------------------\n");
    print_basic_info(chunk1);
    print_freed_info(chunk1);
    printf("\nFREED INFORMATION 2\n");
    printf("----------------------------------\n");
    print_basic_info(chunk2);
    print_freed_info(chunk2);
    printf("\nFREED INFORMATION 3\n");
    printf("----------------------------------\n");
    print_basic_info(chunk3);
    print_freed_info(chunk3);
    #pragma GCC diagnostic pop


    return 0;
}


