
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

static uint64_t const CHUNK_HDR_SZ = 2 * sizeof(void *);

// BENS PRINT FUNCS

static char *itoa_hex(uint64_t n) {
    static char const UINT64_MAX_STR[] = "0xffffffffffffffff";
    static char result_data[sizeof(UINT64_MAX_STR)];
    char *result = result_data;
    memset(result, 0, sizeof(result));

    static char PREFIX[] = "0x";
    strcpy(result, PREFIX);
    for (size_t i = 0; i < strlen(PREFIX); i++) {
        result++;
    }

    if (n == 0) {
        result[0] = '0';
        return result;
    }

    char const HEXDIGS[] = "0123456789abcdef";

    for (int i = 0; n > 0; i++) {
        result[i] = *(HEXDIGS + n % 16);
        n /= 16;
    }

    for (uint64_t i = 0; i < strlen(result) / 2; i++) {
        char tmp = result[i];
        result[i] = result[strlen(result) - i - 1];
        result[strlen(result) - i - 1] = tmp;
    }

    return result - strlen(PREFIX);
}

static char const UINT64_MAX_STR[] = "18446744073709551615";
static char *itoa(uint64_t n) {
    static char result[sizeof(UINT64_MAX_STR)];
    memset(result, 0, sizeof(result));

    if (n == 0) {
        result[0] = '0';
        return result;
    }

    for (int i = 0; n > 0; i++) {
        result[i] = '0' + n % 10;
        n /= 10;
    }

    for (uint64_t i = 0; i < strlen(result) / 2; i++) {
        char tmp = result[i];
        result[i] = result[strlen(result) - i - 1];
        result[strlen(result) - i - 1] = tmp;
    }

    return result;
}

void print(char const * const s) {
    write(STDOUT_FILENO, s, strlen(s));
}

void println(char const * const s) {
    static char const NL[] = "\n";
    print(s);
    print(NL);
}

// MY STUFF

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

struct malloc_chunk* recover_chunk(void* ptr){
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

#define TCACHE 0x555555559010
#define TCACHE_ENTRY_N 64

struct tcache_perthread_struct {
    uint16_t counts[TCACHE_ENTRY_N];
    void* entries[TCACHE_ENTRY_N];
};


void chunk_loc(struct malloc_chunk* chunk){
    print("Chunk Located in: FAST BINS\n");
    print("Chunk Located in: BINS\n");
    print("Chunk Located in: TCACHE\n");
}

void print_bin_chain(struct malloc_chunk* chunk, int depth){
    for (int i = 0; i < depth; i++){
        print("  ");
    }
    println(itoa_hex((uint64_t)chunk));
    if (chunk < (struct malloc_chunk*)0x7ffff0000000){
        print_bin_chain(chunk->fd, depth + 1);
    }
}

#define PROTECT_PTR(pos, ptr) \
  ((__typeof (ptr)) ((((size_t) pos) >> 12) ^ ((size_t) ptr)))
#define REVEAL_PTR(ptr)  PROTECT_PTR (&ptr, ptr)

void print_protected_bin_chain(struct malloc_chunk* chunk, int depth){
    for (int i = 0; i < depth; i++){
        print("  ");
    }
    println(itoa_hex((uint64_t)chunk));
    if (REVEAL_PTR(chunk->fd) != NULL){
        print_protected_bin_chain(REVEAL_PTR(chunk->fd), depth + 1);
    }
}

void print_protected_tcache_chain(void* ptr, int depth){
    if(ptr == NULL) return;
    for (int i = 0; i < depth; i++){
        print("  ");
    }
    struct malloc_chunk* chunk = recover_chunk(ptr);
    println(itoa_hex((uint64_t)chunk));
    if (REVEAL_PTR(chunk->fd) != NULL){
        // print("PROC: ");
        // println(itoa_hex((uint64_t)chunk->fd));
        print_protected_tcache_chain(REVEAL_PTR(chunk->fd), depth + 1);
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
            print_protected_bin_chain(state->fastbinsY[i],0);
        } else {
            print("NULL\n");
        }
    }
}

void walk_tcache(struct tcache_perthread_struct* tcache){
    for (int i = 0 ; i < TCACHE_ENTRY_N; i++){
        print("COUNT: ");
        println(itoa(tcache->counts[i]));
        print_protected_tcache_chain(tcache->entries[i],0);
    }
}

#define COUNT 64
#define CHUNK_SIZE 16

int main() {

    struct malloc_state* my_state = (struct malloc_state*)(MAIN_ARENA);
    struct tcache_perthread_struct* my_tcache = (struct tcache_perthread_struct*)(TCACHE);

    struct malloc_chunk* ptrs[COUNT];

    for(int i = 0; i < COUNT; i++){
        ptrs[i] = malloc_chunk(CHUNK_SIZE);
    }
    for(int i = 0; i < COUNT / 2; i++){
        free_chunk(ptrs[i]);
    }

    println("---------------------- MAIN ARENA ----------------------------");
    walk_main_arena(my_state);
    println("---------------------- FAST BINS ----------------------------");
    walk_fast_bins(my_state);
    println("---------------------- TCACHE ----------------------------");
    walk_tcache(my_tcache);


    // chunk_loc(ptrs[COUNT]);

}


