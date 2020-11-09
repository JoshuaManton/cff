#pragma once

struct Vector2 {
    union {
        float elements[2];
        struct {
            float x;
            float y;
        };
    };
    inline float &operator[](int index) {
        return elements[index];
    }
};

struct Vector3 {
    union {
        float elements[3];
        struct {
            float x;
            float y;
            float z;
        };
    };
    inline float &operator[](int index) {
        return elements[index];
    }
};

struct Vector4 {
    union {
        float elements[4];
        struct {
            float x;
            float y;
            float z;
            float w;
        };
    };
    inline float &operator[](int index) {
        return elements[index];
    }
};

Vector2 v2(float x, float y);
float dot(Vector2 a, Vector2 b);
float length(Vector2 v);
float sqr_length(Vector2 v);
Vector2 normalize(Vector2 v);
float cross(Vector2 a, Vector2 b);
Vector2 operator +(Vector2 a, Vector2 b);
Vector2 operator +=(Vector2 &a, Vector2 b);
Vector2 operator -(Vector2 a, Vector2 b);
Vector2 operator -=(Vector2 &a, Vector2 b);
Vector2 operator *(Vector2 a, float v);
Vector2 operator *=(Vector2 &a, float v);
Vector2 operator /(Vector2 a, float v);
Vector2 operator /=(Vector2 &a, float v);

Vector3 v3(float x, float y, float z);
float dot(Vector3 a, Vector3 b);
float length(Vector3 v);
float sqr_length(Vector3 v);
Vector3 normalize(Vector3 v);
Vector3 cross(Vector3 a, Vector3 b);
Vector3 operator +(Vector3 a, Vector3 b);
Vector3 operator +=(Vector3 &a, Vector3 b);
Vector3 operator -(Vector3 a, Vector3 b);
Vector3 operator -=(Vector3 &a, Vector3 b);
Vector3 operator *(Vector3 a, float v);
Vector3 operator *=(Vector3 &a, float v);
Vector3 operator /(Vector3 a, float v);
Vector3 operator /=(Vector3 &a, float v);

Vector4 v4(float x, float y, float z, float w);
float dot(Vector4 a, Vector4 b);
float length(Vector4 v);
float sqr_length(Vector4 v);
Vector4 normalize(Vector4 v);
Vector4 operator +(Vector4 a, Vector4 b);
Vector4 operator +=(Vector4 &a, Vector4 b);
Vector4 operator -(Vector4 a, Vector4 b);
Vector4 operator -=(Vector4 &a, Vector4 b);
Vector4 operator *(Vector4 a, float v);
Vector4 operator *=(Vector4 &a, float v);
Vector4 operator /(Vector4 a, float v);
Vector4 operator /=(Vector4 &a, float v);

struct Matrix4 {
    union {
        float elements[4][4];
        Vector4 columns[4];
    };
    inline Vector4 &operator[](int index) {
        return columns[index];
    }
};

Matrix4 m4_identity();
Matrix4 m4(Vector4 columns[4]);
Matrix4 transpose(Matrix4 a);
Matrix4 perspective(float fovy_radians, float aspect, float near, float far);
Matrix4 orthographic(float left, float right, float bottom, float top, float near, float far);
Matrix4 look_at(Vector3 eye, Vector3 centre, Vector3 up);
Matrix4 translate(Vector3 v);
Matrix4 rotate(float angle_radians, Vector3 v);
Matrix4 scale(Vector3 v);
Matrix4 operator *(Matrix4 a, Matrix4 b);
Vector4 operator *(Matrix4 a, Vector4 v);
Matrix4 operator *(Matrix4 a, float f);

