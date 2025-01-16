
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

// Flags
int verbose_flag = 0;
int help_flag = 0;
int chunk_count = 128;
int chunk_size = 16;
int chunk_free_skip = 2;


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

#define TCACHE 0x55555555a010
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

void print_depth(int depth){
    if(verbose_flag == 1){
        for (int i = 0; i < depth; i++){
            print("  ");
        }
    }
}

void print_bin_chain(struct malloc_chunk* chunk, int depth){
    if (chunk < (struct malloc_chunk*)0x7ffff0000000){
        print_depth(depth);
        if(verbose_flag == 1) println(itoa_hex((uint64_t)chunk));
        print_bin_chain(chunk->fd, depth + 1);
    } else {
        print("END COUNT: ");
        println(itoa(depth));
    }
}

#define PROTECT_PTR(pos, ptr) \
  ((__typeof (ptr)) ((((size_t) pos) >> 12) ^ ((size_t) ptr)))
#define REVEAL_PTR(ptr)  PROTECT_PTR (&ptr, ptr)

void print_protected_bin_chain(struct malloc_chunk* chunk, int depth){
    print_depth(depth);
    if(verbose_flag == 1) println(itoa_hex((uint64_t)chunk));
    if (REVEAL_PTR(chunk->fd) != NULL){
        print_protected_bin_chain(REVEAL_PTR(chunk->fd), depth + 1);
    } else {
        // Give a count
        print("END COUNT: ");
        println(itoa(depth + 1));
    }
}

void print_protected_tcache_chain(void* ptr, int depth){
    if(ptr == NULL) return;
    print_depth(depth);
    struct malloc_chunk* chunk = recover_chunk(ptr);
    if(verbose_flag == 1) println(itoa_hex((uint64_t)chunk));
    if (REVEAL_PTR(chunk->fd) != NULL){
        // print("PROC: ");
        // println(itoa_hex((uint64_t)chunk->fd));
        print_protected_tcache_chain(REVEAL_PTR(chunk->fd), depth + 1);
    }
}

void walk_main_arena(struct malloc_state* state){
    for (int i = 0; i < BIN_N; i += 2){
        if (state->bins[i] < (struct malloc_chunk*)0x7ffff0000000){
            print("E: ");
            println(itoa(i));
            print_bin_chain(state->bins[i],0);
        }
    }
}

void walk_fast_bins(struct malloc_state* state){
    for (int i = 0; i < FASTBIN_N; i++){
        if(state->fastbinsY[i] != NULL){
            print("E: ");
            println(itoa(i));
            print_protected_bin_chain(state->fastbinsY[i],0);
        }
    }
}

void walk_tcache(struct tcache_perthread_struct* tcache){
    for (int i = 0 ; i < TCACHE_ENTRY_N; i++){
        if (tcache->counts[i] > 0){
            print("E: ");
            print(itoa(i));
            print(" COUNT: ");
            println(itoa(tcache->counts[i]));
            print_protected_tcache_chain(tcache->entries[i],0);
        }
    }
}

int parse_flags(int argc, char* argv[]){
    for (int i = 1; i < argc; i++) { // Start at 1 because argv[0] is the program name
        if (strcmp(argv[i], "-v") == 0) {
            verbose_flag = 1;
        } else if (strcmp(argv[i], "--help") == 0) {
            help_flag = 1;
        } if (strcmp(argv[i], "-n") == 0) {
            // Ensure the next argument is available for the required number option
            if (i + 1 < argc) {
                chunk_count = atoi(argv[i + 1]);  // Convert string to integer
                if (chunk_count == 0 && strcmp(argv[i + 1], "0") != 0) {
                    println("Error: -n option requires a valid integer value.\n");
                    exit(EXIT_FAILURE);
                }
                i++; // Skip the next argument since we just processed it
            } else {
                println("Error: -n option requires an integer value.\n");
                exit(EXIT_FAILURE);
            }
        } if (strcmp(argv[i], "-s") == 0) {
            // Ensure the next argument is available for the required number option
            if (i + 1 < argc) {
                chunk_size = atoi(argv[i + 1]);  // Convert string to integer
                if (chunk_size == 0 && strcmp(argv[i + 1], "0") != 0) {
                    println("Error: -s option requires a valid integer value.\n");
                    exit(EXIT_FAILURE);
                }
                i++; // Skip the next argument since we just processed it
            } else {
                println("Error: -s option requires an integer value.\n");
                exit(EXIT_FAILURE);
            }
        } if (strcmp(argv[i], "-f") == 0) {
            // Ensure the next argument is available for the required number option
            if (i + 1 < argc) {
                chunk_free_skip = atoi(argv[i + 1]);  // Convert string to integer
                if (chunk_free_skip == 0 && strcmp(argv[i + 1], "0") != 0) {
                    println("Error: -f option requires a valid integer value.\n");
                    exit(EXIT_FAILURE);
                }
                i++; // Skip the next argument since we just processed it
            } else {
                println("Error: -f option requires an integer value.\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    if (verbose_flag) {
        println("Verbose mode is enabled.\n");
    }

    if (help_flag) {
        println("Help flag is enabled.\n");
        println("Usage: ./program [options]\n");
        println("  -v         Enable verbose mode\n");
        println("  --help     Show help message\n");
        println("  -n [int]   Set the number of chunks to malloc default is 128\n");
        println("  -s [int]   Byte size of chunks default is 16\n");
        println("  -f [int]   How many chunks to jump between frees default is 2 meaning skip freeing every other chunk\n");
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[]) {

    printf("%d, %d, %d\n", chunk_count, chunk_size, chunk_free_skip);

    if(parse_flags(argc, argv)) return 0;

    struct malloc_state* my_state = (struct malloc_state*)(MAIN_ARENA);
    struct tcache_perthread_struct* my_tcache = (struct tcache_perthread_struct*)(TCACHE);

    struct malloc_chunk* ptrs[chunk_count];

    for(int i = 0; i < chunk_count; i++){
        ptrs[i] = malloc_chunk(chunk_size);
    }
    for(int i = 0; i < chunk_count; i += chunk_free_skip){
        free_chunk(ptrs[i]);
    }

    println("---------------------- MAIN ARENA ----------------------------");
    walk_main_arena(my_state);
    println("---------------------- FAST BINS ----------------------------");
    walk_fast_bins(my_state);
    println("---------------------- TCACHE ----------------------------");
    walk_tcache(my_tcache);
}


