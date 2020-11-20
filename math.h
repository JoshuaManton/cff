#pragma once

#include "basic.h"

#define TAU         6.28318530717958647692528676655900576
#define PI          3.14159265358979323846264338327950288
#define E           2.71828182845904523536
#define RAD_PER_DEG (TAU/360.0)
#define DEG_PER_RAD (360.0/TAU)

f32 to_radians    (f32 degrees);
f64 to_radians_f64(f64 degrees);
f32 to_degrees    (f32 radians);
f64 to_degrees_f64(f64 radians);

struct Vector2;
struct Vector3;
struct Vector4;



struct Vector2 {
    union {
        float elements[2];
        struct {
            float x;
            float y;
        };
    };
    inline float &operator[](int index) {
        BOUNDS_CHECK(index, 0, ARRAYSIZE(elements));
        return elements[index];
    }
};

Vector2 v2         (float x, float y);
Vector2 v2         (Vector3 v);
Vector2 v2         (Vector4 v);
float   dot        (Vector2 a, Vector2 b);
float   length     (Vector2 v);
float   sqr_length (Vector2 v);
Vector2 normalize  (Vector2 v);
float   cross      (Vector2 a, Vector2 b);
Vector2 operator + (Vector2 a, Vector2 b);
Vector2 operator +=(Vector2 &a, Vector2 b);
Vector2 operator - (Vector2 a);
Vector2 operator - (Vector2 a, Vector2 b);
Vector2 operator -=(Vector2 &a, Vector2 b);
Vector2 operator * (Vector2 a, float f);
Vector2 operator *=(Vector2 &a, float f);
Vector2 operator * (Vector2 a, Vector2 b);
Vector2 operator *=(Vector2 &a, Vector2 b);
Vector2 operator / (Vector2 a, float f);
Vector2 operator /=(Vector2 &a, float f);
Vector2 operator / (Vector2 a, Vector2 b);
Vector2 operator /=(Vector2 &a, Vector2 b);



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
        BOUNDS_CHECK(index, 0, ARRAYSIZE(elements));
        return elements[index];
    }
};

Vector3 v3         (float x, float y, float z);
Vector3 v3         (Vector2 v);
Vector3 v3         (Vector4 v);
float   dot        (Vector3 a, Vector3 b);
float   length     (Vector3 v);
float   sqr_length (Vector3 v);
Vector3 normalize  (Vector3 v);
Vector3 cross      (Vector3 a, Vector3 b);
Vector3 operator + (Vector3 a, Vector3 b);
Vector3 operator +=(Vector3 &a, Vector3 b);
Vector3 operator - (Vector3 a, Vector3 b);
Vector3 operator - (Vector3 a);
Vector3 operator -=(Vector3 &a, Vector3 b);
Vector3 operator * (Vector3 a, float f);
Vector3 operator *=(Vector3 &a, float f);
Vector3 operator * (Vector3 a, Vector3 b);
Vector3 operator *=(Vector3 &a, Vector3 b);
Vector3 operator / (Vector3 a, float f);
Vector3 operator /=(Vector3 &a, float f);
Vector3 operator / (Vector3 a, Vector3 b);
Vector3 operator /=(Vector3 &a, Vector3 b);



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
        BOUNDS_CHECK(index, 0, ARRAYSIZE(elements));
        return elements[index];
    }
};

Vector4 v4         (float x, float y, float z, float w);
Vector4 v4         (Vector2 v);
Vector4 v4         (Vector3 v);
float   dot        (Vector4 a, Vector4 b);
float   length     (Vector4 v);
float   sqr_length (Vector4 v);
Vector4 normalize  (Vector4 v);
Vector4 operator + (Vector4 a, Vector4 b);
Vector4 operator +=(Vector4 &a, Vector4 b);
Vector4 operator - (Vector4 a, Vector4 b);
Vector4 operator - (Vector4 a);
Vector4 operator -=(Vector4 &a, Vector4 b);
Vector4 operator * (Vector4 a, float f);
Vector4 operator *=(Vector4 &a, float f);
Vector4 operator * (Vector4 a, Vector4 b);
Vector4 operator *=(Vector4 &a, Vector4 b);
Vector4 operator / (Vector4 a, float f);
Vector4 operator /=(Vector4 &a, float f);
Vector4 operator / (Vector4 a, Vector4 b);
Vector4 operator /=(Vector4 &a, Vector4 b);



struct Quaternion {
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
        BOUNDS_CHECK(index, 0, ARRAYSIZE(elements));
        return elements[index];
    }
};

Quaternion quaternion_identity();
Quaternion quaternion         (float x, float y, float z, float w);
Quaternion operator +         (Quaternion a, Quaternion b);
Quaternion operator +=        (Quaternion &a, Quaternion b);
Quaternion operator -         (Quaternion a);
Quaternion operator -         (Quaternion a, Quaternion b);
Quaternion operator -=        (Quaternion &a, Quaternion b);
Quaternion operator *         (Quaternion a, Quaternion b);
Quaternion operator *=        (Quaternion &a, Quaternion b);
Quaternion operator *         (Quaternion a, float f);
Quaternion operator *=        (Quaternion &a, float f);
Vector3    operator *         (Quaternion q, Vector3 v);
Quaternion operator /         (Quaternion a, float f);
Quaternion operator /=        (Quaternion &a, float f);
float      dot                (Quaternion a, Quaternion b);
float      length             (Quaternion q);
float      sqr_length         (Quaternion q);
Quaternion normalize          (Quaternion q);
Quaternion inverse            (Quaternion q);
Quaternion axis_angle         (Vector3 axis, float angle_radians);
Quaternion slerp              (Quaternion a, Quaternion b, float t);
Quaternion quaternion_difference(Quaternion a, Quaternion b);
Vector3    quaternion_right   (Quaternion q);
Vector3    quaternion_up      (Quaternion q);
Vector3    quaternion_forward (Quaternion q);
Vector3    quaternion_left    (Quaternion q);
Vector3    quaternion_down    (Quaternion q);
Vector3    quaternion_back    (Quaternion q);



struct Matrix3 {
    union {
        float elements[3][3];
        Vector3 columns[3];
    };
    inline Vector3 &operator[](int index) {
        BOUNDS_CHECK(index, 0, ARRAYSIZE(columns));
        return columns[index];
    }
};

Matrix3 m3_identity();
Matrix3 m3(Vector3 columns[3]);
Matrix3 operator *(Matrix3 a, Matrix3 b);
Vector3 operator *(Matrix3 a, Vector3 v);
Matrix3 operator *(Matrix3 a, float f);
float matrix3_determinant(Matrix3 m);

struct Matrix4 {
    union {
        float elements[4][4];
        Vector4 columns[4];
    };
    inline Vector4 &operator[](int index) {
        BOUNDS_CHECK(index, 0, ARRAYSIZE(columns));
        return columns[index];
    }
};

Matrix4 m4_identity();
Matrix4 m4(Vector4 columns[4]);
Matrix4 operator *(Matrix4 a, Matrix4 b);
Vector4 operator *(Matrix4 a, Vector4 v);
Matrix4 operator *(Matrix4 a, float f);
Matrix4 transpose(Matrix4 a);
Matrix4 look_at(Vector3 eye, Vector3 center, Vector3 up);
Matrix4 construct_perspective_matrix (float fovy_radians, float aspect, float near, float far);
Matrix4 construct_orthographic_matrix(float left, float right, float bottom, float top, float near, float far);
Matrix4 construct_translation_matrix (Vector3 v);
Matrix4 construct_rotation_matrix    (float angle_radians, Vector3 v);
Matrix4 construct_scale_matrix       (Vector3 v);
Matrix4 inverse(Matrix4 m);
float matrix4_minor(Matrix4 m, int c, int y);
float matrix4_cofactor(Matrix4 m, int c, int r);
Matrix4 matrix4_adjoint(Matrix4 m);
float matrix4_determinant(Matrix4 m);
Matrix4 matrix4_inverse_transpose(Matrix4 m);

Matrix4    quaternion_to_matrix4(Quaternion q);
Quaternion matrix4_to_quaternion(Matrix4 m);
Quaternion quaternion_look_at   (Vector3 eye, Vector3 center, Vector3 up);

Matrix4 construct_view_matrix (Vector3 position, Quaternion orientation);
Matrix4 construct_model_matrix(Vector3 position, Vector3 scale, Quaternion orientation);
Matrix4 construct_trs_matrix(Vector3 t, Quaternion r, Vector3 s);