

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

// setarch -R


// Location of main_arena: 0x7ffff7f8fac0

#define MAIN_ARENA 0x7ffff7f8fac0

#define FASTBIN_N 10
#define BIN_N 254

struct malloc_state {
    int mutex;
    int flags;
    int have_fastchunks;
    void* fastbinsY[FASTBIN_N];
    void* top;
    void* last_remainder;
    void* bins[BIN_N];
    unsigned int binmap[4];
    void* next;
    void* next_free;
    size_t attached_threads;
    size_t system_mem;
    size_t max_system_mem;
};


void chunk_loc(struct malloc_chunk* chunk){
    printf("Chunk Located in: FAST BINS\n");
    printf("Chunk Located in: BINS\n");
    printf("Chunk Located in: TCACHE\n");
}

void fast_bin(struct malloc_chunk* chunk){
    // For each bin
    for(int i = 0; i < FASTBIN_N; i++){
        // Traverse the list
        
    }
}

void print_bin_chain(struct malloc_chunk* chunk, int depth){
    for (int i = 0; i < depth; i++){
        printf("  ");
    }
    printf("%p\n",chunk);
    if (chunk < (struct malloc_chunk*)0x7ffff0000000){
        print_bin_chain(chunk->fd, depth + 1);
    }
}

void walk_main_arena(struct malloc_state* state){
    for (int i = 0; i < BIN_N; i += 2){
        print_bin_chain(state->bins[i],0);
    }
}

void walk_fast_bins(struct malloc_state* state){
    for (int i = 0; i < FASTBIN_N; i++){
        if(state->fastbinsY[i] != NULL){
            print_bin_chain(state->fastbinsY[i],0);
        } else {
            printf("NULL\n");
        }
    }
}


#define COUNT 64
#define CHUNK_SIZE 16

int main() {

    struct malloc_state* my_state = (struct malloc_state*)(MAIN_ARENA);

    struct malloc_chunk* ptrs[COUNT];

    for(int i = 0; i < COUNT; i++){
        ptrs[i] = malloc_chunk(CHUNK_SIZE);
    }
    for(int i = 0; i < COUNT / 2; i++){
        free_chunk(ptrs[i]);
    }

    walk_main_arena(my_state);
    walk_fast_bins(my_state);

    // chunk_loc(ptrs[COUNT]);

}


