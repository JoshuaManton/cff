#include <string.h> // note(josh): for memset

#ifndef DEFAULT_ALIGNMENT
#define DEFAULT_ALIGNMENT sizeof(void *) * 2
#endif

bool is_power_of_two(uintptr_t n) {
    return n > 0 && (n & (n-1)) == 0;
}

uintptr_t align_forward(uintptr_t p, uintptr_t align) {
    assert(is_power_of_two(align));
    p = (p + (align - 1)) & (~(align - 1));
    return p;
}

uintptr_t align_backward(uintptr_t p, uintptr_t align) {
    return align_forward(p - align + 1, align);
}

void zero_memory(void *memory_void, int length) {
    char *memory = (char *)memory_void;
    uintptr_t start = (uintptr_t)memory;
    uintptr_t start_aligned = align_forward(start, alignof(uintptr_t));
    uintptr_t end = start + (uintptr_t)length;
    uintptr_t end_aligned = align_backward(end, alignof(uintptr_t));

    for (uintptr_t i = start; i < start_aligned; i++) {
        memory[i-start] = 0;
    }

    for (uintptr_t i = start_aligned; i < end_aligned; i += sizeof(uintptr_t)) {
        *((uintptr_t *)&memory[i-start]) = 0;
    }

    for (uintptr_t i = end_aligned; i < end; i++) {
        memory[i-start] = 0;
    }
}



struct Allocator {
    void *data;
    void *(*alloc_proc)(int);
    void (*free_proc)(void *);
};



void *default_allocator_alloc(int size) {
    return malloc(size);
}

void default_allocator_free(void *ptr) {
    free(ptr);
}

Allocator default_allocator() {
    Allocator a = {};
    a.alloc_proc = default_allocator_alloc;
    a.free_proc = default_allocator_free;
    return a;
}



char *buffer_allocate(char *buffer, int buffer_len, int *offset, int size, int alignment, bool panic_on_oom = true) {
    // Don't allow allocations of zero size. This would likely return a
    // pointer to a different allocation, causing many problems.
    if (size == 0) {
        return nullptr;
    }

    // todo(josh): The `align_forward()` call and the `start + size` below
    // that could overflow if the `size` or `align` parameters are super huge

    int start = align_forward(*offset, alignment);

    // Don't allow allocations that would extend past the end of the buffer.
    if ((start + size) > buffer_len) {
        if (panic_on_oom) {
            assert(0 && "buffer_allocate ran out of memory");
        }
        return nullptr;
    }

    *offset = start + size;
    char *ptr = &buffer[start];
    zero_memory(ptr, size);
    return ptr;
}



struct Arena {
    char *memory;
    int memory_size;
    int cur_offset;
};

void init_arena(Arena *arena, char *backing, int backing_size) {
    arena->memory = backing;
    arena->memory_size = backing_size;
    arena->cur_offset = 0;
}

char *arena_alloc(Arena *arena, int size, int align = DEFAULT_ALIGNMENT) {
    return buffer_allocate(arena->memory, arena->memory_size, &arena->cur_offset, size, align);
}

#define arena_new(arena, type) ((type *)arena_alloc(arena, sizeof(type)))




void *alloc(int size, Allocator allocator) {
    assert(allocator.alloc_proc != nullptr && "Alloc proc was nullptr for allocator");
    void *ptr = allocator.alloc_proc(size);
    return memset(ptr, 0, size);
}

void free(void *ptr, Allocator allocator) {
    assert(allocator.free_proc != nullptr && "Free proc was nullptr for allocator");
    allocator.free_proc(ptr);
}