#include <math.h>

struct Vector2 {
    float x;
    float y;
};

Vector2 v2(float x, float y) {
    Vector2 v;
    v.x = x;
    v.y = y;
    return v;
}

Vector2 operator +(Vector2 a, Vector2 b) {
    return v2(a.x+b.x, a.y+b.y);
}

Vector2 operator -(Vector2 a, Vector2 b) {
    return v2(a.x-b.x, a.y-b.y);
}

Vector2 operator *(Vector2 a, float v) {
    return v2(a.x*v, a.y*v);
}

Vector2 operator /(Vector2 a, float v) {
    return v2(a.x/v, a.y/v);
}

struct Vector3 {
    float x;
    float y;
    float z;
};

Vector3 v3(float x, float y, float z) {
    Vector3 v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

Vector3 operator +(Vector3 a, Vector3 b) {
    return v3(a.x+b.x, a.y+b.y, a.z+b.z);
}

Vector3 operator -(Vector3 a, Vector3 b) {
    return v3(a.x-b.x, a.y-b.y, a.z-b.z);
}

Vector3 operator *(Vector3 a, float v) {
    return v3(a.x*v, a.y*v, a.z*v);
}

Vector3 operator /(Vector3 a, float v) {
    return v3(a.x/v, a.y/v, a.z/v);
}



struct Vector4 {
    float x;
    float y;
    float z;
    float w;
};

Vector4 v4(float x, float y, float z, float w) {
    Vector4 v;
    v.x = x;
    v.y = y;
    v.z = z;
    v.w = w;
    return v;
}

Vector4 operator +(Vector4 a, Vector4 b) {
    return v4(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);
}

Vector4 operator -(Vector4 a, Vector4 b) {
    return v4(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w);
}

Vector4 operator *(Vector4 a, float v) {
    return v4(a.x*v, a.y*v, a.z*v, a.w*v);
}

Vector4 operator /(Vector4 a, float v) {
    return v4(a.x/v, a.y/v, a.z/v, a.w/v);
}

float sin(float t) {
    return (float)sin((double)t);
}