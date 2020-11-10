#pragma once

#include <assert.h>
#include <stdio.h>
#include <math.h>

typedef unsigned char      byte; static_assert(sizeof(byte) == 1, "byte size was not 1");
typedef unsigned char      u8;   static_assert(sizeof(u8)   == 1, "u8 size was not 1");
typedef unsigned short     u16;  static_assert(sizeof(u16)  == 2, "u16 size was not 2");
typedef unsigned int       u32;  static_assert(sizeof(u32)  == 4, "u32 size was not 4");
typedef unsigned long long u64;  static_assert(sizeof(u64)  == 8, "u64 size was not 8");

typedef u32 uint; static_assert(sizeof(uint)  == 4, "uint size was not 4");

typedef char      i8;   static_assert(sizeof(i8)   == 1, "i8 size was not 1");
typedef short     i16;  static_assert(sizeof(i16)  == 2, "i16 size was not 2");
typedef int       i32;  static_assert(sizeof(i32)  == 4, "i32 size was not 4");
typedef long long i64;  static_assert(sizeof(i64)  == 8, "i64 size was not 8");

static_assert(sizeof(int)  == 4, "int size was not 4");

typedef float  f32; static_assert(sizeof(f32) == 4, "f32 size was not 4");
typedef double f64; static_assert(sizeof(f64) == 8, "f64 size was not 8");

static_assert(sizeof(float) == 4, "float size was not 4");

static_assert(sizeof(void *) == 8, "void * size was not 8");

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) \
    ((sizeof(a) / sizeof(*(a))) / \
    static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))
#endif

static void bounds__check(int index, int min, int max_plus_one, char *file, int line) {
    if ((index < min) || (index >= max_plus_one)) {
        printf("<%s:%d> Index %d is out of range %d..<%d\n", file, line, index, min, max_plus_one);
        assert(false);
    }
}

#define BOUNDS_CHECK(index, min, max_plus_one) bounds__check(index, min, max_plus_one, __FILE__, __LINE__)

#ifndef DEFAULT_ALIGNMENT
#define DEFAULT_ALIGNMENT sizeof(void *) * 2
#endif

struct Allocator {
    void *data;
    void *(*alloc_proc)(void *allocator, int size, int alignment);
    void (*free_proc)(void *allocator, void *ptr);
};

struct Arena {
    char *memory;
    int memory_size;
    int cur_offset;
};

void *alloc(Allocator allocator, int size, int alignment = DEFAULT_ALIGNMENT);
void free(Allocator allocator, void *ptr);
#define NEW(allocator, type) ((type *)alloc(allocator, sizeof(type), alignof(type)))
#define MAKE(allocator, type, count) ((type *)alloc(allocator, sizeof(type) * count, alignof(type)))

void *default_allocator_alloc(void *allocator, int size, int alignment);
void default_allocator_free(void *allocator, void *ptr);
Allocator default_allocator();

char *buffer_allocate(char *buffer, int buffer_len, int *offset, int size, int alignment, bool panic_on_oom = true);

void init_arena(Arena *arena, char *backing, int backing_size);
void *arena_alloc(void *allocator, int size, int align = DEFAULT_ALIGNMENT);
void arena_free(void *allocator, void *ptr);
Allocator arena_allocator();

// todo(josh): read_entire_file should be in a different file I think
char *read_entire_file(char *filename, int *len);

// note(josh): defer implementation stolen from gb.h
#if !defined(GB_NO_DEFER) && defined(__cplusplus) && ((defined(_MSC_VER) && _MSC_VER >= 1400) || (__cplusplus >= 201103L))
extern "C++" {
    // NOTE(bill): Stupid fucking templates
    template <typename T> struct gbRemoveReference       { typedef T Type; };
    template <typename T> struct gbRemoveReference<T &>  { typedef T Type; };
    template <typename T> struct gbRemoveReference<T &&> { typedef T Type; };

    /// NOTE(bill): "Move" semantics - invented because the C++ committee are idiots (as a collective not as indiviuals (well a least some aren't))
    template <typename T> inline T &&gb_forward(typename gbRemoveReference<T>::Type &t)  { return static_cast<T &&>(t); }
    template <typename T> inline T &&gb_forward(typename gbRemoveReference<T>::Type &&t) { return static_cast<T &&>(t); }
    template <typename T> inline T &&gb_move   (T &&t)                                   { return static_cast<typename gbRemoveReference<T>::Type &&>(t); }
    template <typename F>
    struct gbprivDefer {
        F f;
        gbprivDefer(F &&f) : f(gb_forward<F>(f)) {}
        ~gbprivDefer() { f(); }
    };
    template <typename F> gbprivDefer<F> gb__defer_func(F &&f) { return gbprivDefer<F>(gb_forward<F>(f)); }

    #define GB_DEFER_1(x, y) x##y
    #define GB_DEFER_2(x, y) GB_DEFER_1(x, y)
    #define GB_DEFER_3(x)    GB_DEFER_2(x, __COUNTER__)
    #define defer(code)      auto GB_DEFER_3(_defer_) = gb__defer_func([&]()->void{code;})
}
#endif




template<typename T>
struct Array {
    T *data;
    int count;
    int capacity;
    Allocator allocator;

    T *append(T element);
    T *insert(int index, T element);
    void reserve(int capacity);
    T pop();
    T ordered_remove(int index);
    T unordered_remove(int index);
    void clear();
    void destroy();

    inline T &operator[](int index) {
        BOUNDS_CHECK(index, 0, count);
        return data[index];
    }
};

template<typename T>
T *Array<T>::append(T element) {
    assert(allocator.alloc_proc != nullptr);
    if ((count+1) >= capacity) {
        reserve(8 + (capacity * 2));
    }
    data[count] = element;
    count += 1;
    return &data[count-1];
}

template<typename T>
T *Array<T>::insert(int index, T element) {
    BOUNDS_CHECK(index, 0, count);
    assert(allocator.alloc_proc != nullptr);
    if ((count+1) >= capacity) {
        reserve(8 + (capacity * 2));
    }
    for (int i = count; i >= index; i--) {
        data[i] = data[i-1];
    }
    data[index] = element;
    count += 1;
    return &data[index];
}

template<typename T>
void Array<T>::reserve(int capacity) {
    assert(allocator.alloc_proc != nullptr);
    if (this->capacity >= capacity) {
        return;
    }

    void *new_data = alloc(allocator, sizeof(T) * capacity);
    if (data != nullptr) {
        memcpy(new_data, data, sizeof(T) * count);
        free(allocator, data);
    }

    data = (T *)new_data;
    this->capacity = capacity;
}

template<typename T>
void Array<T>::destroy() {
    if (data) {
        free(allocator, data);
    }
}

template<typename T>
void Array<T>:: clear() {
    count = 0;
}

template<typename T>
T Array<T>::pop() {
    BOUNDS_CHECK(count-1, 0, count);
    T t = data[count-1];
    count -= 1;
    return t;
}

template<typename T>
T Array<T>::ordered_remove(int index) {
    BOUNDS_CHECK(index, 0, count);
    T t = data[index];
    if (index != (count-1)) {
        for (int i = index+1; i < count; i++) {
            data[i-1] = data[i];
        }
    }
    count -= 1;
    return t;
}

template<typename T>
T Array<T>::unordered_remove(int index) {
    BOUNDS_CHECK(index, 0, count);
    T t = data[index];
    if (index != (count-1)) {
        data[index] = data[count-1];
    }
    count -= 1;
    return t;
}

#define Foreach(var, array) for (auto *var = array.data; (uintptr_t)var < (uintptr_t)(&array.data[array.count]); var++)



#if 0
struct Array_Test_Struct {
    Vector3 position;
    Vector3 tex_coord;
    Vector4 color;
};

void run_array_tests() {
    // todo(josh): add asserts and whatnot to this

    Array<Array_Test_Struct> array = {};
    array.allocator = default_allocator();
    defer(array.destroy());
    Array_Test_Struct a = {v3(1, 2, 3), v3(0.1f, 0.1f, 0), v4(1, 0, 1, 1)};
    Array_Test_Struct b = {v3(4, 5, 6), v3(0.2f, 0.2f, 0), v4(0, 1, 1, 1)};
    Array_Test_Struct c = {v3(7, 8, 9), v3(0.4f, 0.4f, 0), v4(1, 1, 0, 1)};
    array.append(a);
    array.append(b);
    array.append(c);
    array.unordered_remove(0);
    array.insert(1, a);

    printf("--------------\n");
    Foreach(v, array) {
        printf("%f %f %f\n", v->position.x, v->position.y, v->position.z);
    }

    Array_Test_Struct last_vertex = array.pop();
    printf("--------------\n");
    printf("%f %f %f\n", last_vertex.position.x, last_vertex.position.y, last_vertex.position.z);

    printf("--------------\n");
    Foreach(v, array) {
        printf("%f %f %f\n", v->position.x, v->position.y, v->position.z);
    }

    Array_Test_Struct crazy_vertex = {v3(9, 9, 9), v3(1, 1, 1), v4(1, 2, 3, 4)};
    array[1] = crazy_vertex;
    printf("--------------\n");
    Foreach(v, array) {
        printf("%f %f %f\n", v->position.x, v->position.y, v->position.z);
    }
}
#endif





struct String_Builder {
    Array<char> buf;

    void print(char *str);
    void clear();
    char *string();
};

String_Builder make_string_builder(Allocator allocator);
void destroy_string_builder(String_Builder sb);