#pragma once

#include "basic.h"

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



#ifdef DEVELOPER
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