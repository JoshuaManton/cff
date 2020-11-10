#include "basic.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



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



void *alloc(Allocator allocator, int size, int alignment) {
    assert(allocator.alloc_proc != nullptr && "Alloc proc was nullptr for allocator");
    void *ptr = allocator.alloc_proc(allocator.data, size, alignment);
    return memset(ptr, 0, size);
}

void free(Allocator allocator, void *ptr) {
    assert(allocator.free_proc != nullptr && "Free proc was nullptr for allocator");
    allocator.free_proc(allocator.data, ptr);
}



void *default_allocator_alloc(void *allocator, int size, int alignment) {
    return malloc(size);
}

void default_allocator_free(void *allocator, void *ptr) {
    free(ptr);
}

Allocator default_allocator() {
    Allocator a = {};
    a.alloc_proc = default_allocator_alloc;
    a.free_proc = default_allocator_free;
    return a;
}



char *buffer_allocate(char *buffer, int buffer_len, int *offset, int size, int alignment, bool panic_on_oom) {
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



void init_arena(Arena *arena, char *backing, int backing_size) {
    arena->memory = backing;
    arena->memory_size = backing_size;
    arena->cur_offset = 0;
}

void *arena_alloc(void *allocator, int size, int align) {
    Arena *arena = (Arena *)allocator;
    return buffer_allocate(arena->memory, arena->memory_size, &arena->cur_offset, size, align);
}

void arena_free(void *allocator, void *ptr) {
    // note(josh): freeing from arenas does nothing.
}

Allocator arena_allocator() {
    Allocator a = {};
    a.alloc_proc = arena_alloc;
    a.free_proc = arena_free;
    return a;
}



// todo(josh): custom allocator
char *read_entire_file(char *filename, int *len) {
    auto file = fopen(filename, "rb");
    if (file == nullptr) {
        printf("read_entire_file couldn't find file: %s\n", filename);
        assert(false);
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *str = (char *)malloc(length + 1);
    fread(str, 1, length, file);
    fclose(file);

    str[length] = 0;
    *len = length+1;
    return str;
}



String_Builder make_string_builder(Allocator allocator) {
    String_Builder sb = {};
    sb.buf.allocator = allocator;
    return sb;
}

void destroy_string_builder(String_Builder sb) {
    sb.buf.destroy();
}

void String_Builder::print(char *str) {
    for (char *s = str; *s != '\0'; s++) {
        buf.append(*s);
    }
    BOUNDS_CHECK(buf.count, 0, buf.capacity);
    buf.data[buf.count] = 0;
}

void String_Builder::clear() {
    buf.clear();
    BOUNDS_CHECK(buf.count, 0, buf.capacity);
    buf.data[buf.count] = 0;
}

char *String_Builder::string() {
    return buf.data;
}