#pragma once

struct Vector2 {
    float x;
    float y;
};

struct Vector3 {
    float x;
    float y;
    float z;
};

struct Vector4 {
    float x;
    float y;
    float z;
    float w;
};

Vector2 v2(float x, float y);
Vector2 operator +(Vector2 a, Vector2 b);
Vector2 operator -(Vector2 a, Vector2 b);
Vector2 operator *(Vector2 a, float v);
Vector2 operator /(Vector2 a, float v);

Vector3 v3(float x, float y, float z);
Vector3 operator +(Vector3 a, Vector3 b);
Vector3 operator -(Vector3 a, Vector3 b);
Vector3 operator *(Vector3 a, float v);
Vector3 operator /(Vector3 a, float v);

Vector4 v4(float x, float y, float z, float w);
Vector4 operator +(Vector4 a, Vector4 b);
Vector4 operator -(Vector4 a, Vector4 b);
Vector4 operator *(Vector4 a, float v);
Vector4 operator /(Vector4 a, float v);
