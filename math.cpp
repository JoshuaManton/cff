#include "math.h"

#include <math.h>

f32 to_radians    (f32 degrees) { return degrees * RAD_PER_DEG; }
f64 to_radians_f64(f64 degrees) { return degrees * RAD_PER_DEG; }
f32 to_degrees    (f32 radians) { return radians * DEG_PER_RAD; }
f64 to_degrees_f64(f64 radians) { return radians * DEG_PER_RAD; }

Vector2 v2(float x, float y) {
    Vector2 v;
    v.x = x;
    v.y = y;
    return v;
}

float dot(Vector2 a, Vector2 b) {
    return (a.x*b.x) + (a.y*b.y);
}
float length(Vector2 v) {
    return sqrt(dot(v, v));
}
float sqr_length(Vector2 v) {
    return dot(v, v);
}
Vector2 normalize(Vector2 v) {
    return v / length(v);
}
float cross(Vector2 a, Vector2 b) {
    return a.x*b.y - b.x*a.y;
}

Vector2 operator +(Vector2 a, Vector2 b) {
    return v2(a.x+b.x, a.y+b.y);
}
Vector2 operator +=(Vector2 &a, Vector2 b) {
    return (a = a + b);
}

Vector2 operator -(Vector2 a, Vector2 b) {
    return v2(a.x-b.x, a.y-b.y);
}
Vector2 operator -(Vector2 a) {
    return v2(-a.x, -a.y);
}
Vector2 operator -=(Vector2 &a, Vector2 b) {
    return (a = a - b);
}

Vector2 operator *(Vector2 a, float f) {
    return v2(a.x*f, a.y*f);
}
Vector2 operator *=(Vector2 &a, float f) {
    return (a = a * f);
}

Vector2 operator *(Vector2 a, Vector2 b) {
    return v2(a.x*b.x, a.y*b.y);
}
Vector2 operator *=(Vector2 &a, Vector2 b) {
    return (a = a * b);
}

Vector2 operator /(Vector2 a, float f) {
    return v2(a.x/f, a.y/f);
}
Vector2 operator /=(Vector2 &a, float f) {
    return (a = a / f);
}

Vector2 operator /(Vector2 a, Vector2 b) {
    return v2(a.x/b.x, a.y/b.y);
}
Vector2 operator /=(Vector2 &a, Vector2 b) {
    return (a = a / b);
}



Vector3 v3(float x, float y, float z) {
    Vector3 v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

float dot(Vector3 a, Vector3 b) {
    return (a.x*b.x) + (a.y*b.y) + (a.z*b.z);
}
float length(Vector3 v) {
    return sqrt(dot(v, v));
}
float sqr_length(Vector3 v) {
    return dot(v, v);
}
Vector3 normalize(Vector3 v) {
    return v / length(v);
}
Vector3 cross(Vector3 a, Vector3 b) {
    Vector3 result;
    result.x = a.y*b.z - b.y*a.z;
    result.y = a.z*b.x - b.z*a.x;
    result.z = a.x*b.y - b.x*a.y;
    return result;
}

Vector3 operator +(Vector3 a, Vector3 b) {
    return v3(a.x+b.x, a.y+b.y, a.z+b.z);
}
Vector3 operator +=(Vector3 &a, Vector3 b) {
    return (a = a + b);
}

Vector3 operator -(Vector3 a, Vector3 b) {
    return v3(a.x-b.x, a.y-b.y, a.z-b.z);
}
Vector3 operator -(Vector3 a) {
    return v3(-a.x, -a.y, -a.z);
}
Vector3 operator -=(Vector3 &a, Vector3 b) {
    return (a = a - b);
}

Vector3 operator *(Vector3 a, float f) {
    return v3(a.x*f, a.y*f, a.z*f);
}
Vector3 operator *=(Vector3 &a, float f) {
    return (a = a * f);
}

Vector3 operator *(Vector3 a, Vector3 b) {
    return v3(a.x*b.x, a.y*b.y, a.z*b.z);
}
Vector3 operator *=(Vector3 &a, Vector3 b) {
    return (a = a * b);
}

Vector3 operator /(Vector3 a, float f) {
    return v3(a.x/f, a.y/f, a.z/f);
}
Vector3 operator /=(Vector3 &a, float f) {
    return (a = a / f);
}

Vector3 operator /(Vector3 a, Vector3 b) {
    return v3(a.x/b.x, a.y/b.y, a.z/b.z);
}
Vector3 operator /=(Vector3 &a, Vector3 b) {
    return (a = a / b);
}



Vector4 v4(float x, float y, float z, float w) {
    Vector4 v;
    v.x = x;
    v.y = y;
    v.z = z;
    v.w = w;
    return v;
}

float dot(Vector4 a, Vector4 b) {
    return (a.x*b.x) + (a.y*b.y) + (a.z*b.z) + (a.w*b.w);
}
float length(Vector4 v) {
    return sqrt(dot(v, v));
}
float sqr_length(Vector4 v) {
    return dot(v, v);
}
Vector4 normalize(Vector4 v) {
    return v / length(v);
}

Vector4 operator +(Vector4 a, Vector4 b) {
    return v4(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);
}
Vector4 operator +=(Vector4 &a, Vector4 b) {
    return (a = a + b);
}

Vector4 operator -(Vector4 a, Vector4 b) {
    return v4(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w);
}
Vector4 operator -(Vector4 a) {
    return v4(-a.x, -a.y, -a.z, -a.w);
}
Vector4 operator -=(Vector4 &a, Vector4 b) {
    return (a = a - b);
}

Vector4 operator *(Vector4 a, float f) {
    return v4(a.x*f, a.y*f, a.z*f, a.w*f);
}
Vector4 operator *=(Vector4 &a, float f) {
    return (a = a * f);
}

Vector4 operator *(Vector4 a, Vector4 b) {
    return v4(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w);
}
Vector4 operator *=(Vector4 &a, Vector4 b) {
    return (a = a * b);
}

Vector4 operator /(Vector4 a, float f) {
    return v4(a.x/f, a.y/f, a.z/f, a.w/f);
}
Vector4 operator /=(Vector4 &a, float f) {
    return (a = a / f);
}

Vector4 operator /(Vector4 a, Vector4 b) {
    return v4(a.x/b.x, a.y/b.y, a.z/b.z, a.w/b.w);
}
Vector4 operator /=(Vector4 &a, Vector4 b) {
    return (a = a / b);
}



Quaternion quaternion_identity() {
    Quaternion result;
    result.x = 0.0f;
    result.y = 0.0f;
    result.z = 0.0f;
    result.w = 1.0f;
    return result;
}

Quaternion quaternion(float x, float y, float z, float w) {
    Quaternion result;
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;
    return result;
}

Quaternion operator +(Quaternion a, Quaternion b) {
    Quaternion result;
    result.x = a.x+b.x;
    result.y = a.y+b.y;
    result.z = a.z+b.z;
    result.w = a.w+b.w;
    return result;
}
Quaternion operator +=(Quaternion &a, Quaternion b) {
    return (a = a + b);
}

Quaternion operator -(Quaternion a, Quaternion b) {
    Quaternion result;
    result.x = a.x-b.x;
    result.y = a.y-b.y;
    result.z = a.z-b.z;
    result.w = a.w-b.w;
    return result;
}
Quaternion operator -=(Quaternion &a, Quaternion b) {
    return (a = a - b);
}

Quaternion operator *(Quaternion a, Quaternion b) {
    Quaternion result;
    result.x = ( a.x*b.w) + (a.y*b.z) - (a.z*b.y) + (a.w*b.x);
    result.y = (-a.x*b.z) + (a.y*b.w) + (a.z*b.x) + (a.w*b.y);
    result.z = ( a.x*b.y) - (a.y*b.x) + (a.z*b.w) + (a.w*b.z);
    result.w = (-a.x*b.x) - (a.y*b.y) - (a.z*b.z) + (a.w*b.w);
    return result;
}
Quaternion operator *=(Quaternion &a, Quaternion b) {
    return (a = a * b);
}

Vector3 operator *(Quaternion q, Vector3 v) {
    Vector3 qxyz = v3(q.x, q.y, q.z);
    Vector3 t = cross(qxyz * 2.0f, v);
    return v + t*q.w + cross(qxyz, t);
}

Quaternion operator *(Quaternion a, float f) {
    Quaternion result;
    result.x = a.x*f;
    result.y = a.y*f;
    result.z = a.z*f;
    result.w = a.w*f;
    return result;
}
Quaternion operator *=(Quaternion &a, float f) {
    return (a = a * f);
}

Quaternion operator /(Quaternion a, float f) {
    Quaternion result;
    result.x = a.x/f;
    result.y = a.y/f;
    result.z = a.z/f;
    result.w = a.w/f;
    return result;
}
Quaternion operator /=(Quaternion &a, float f) {
    return (a = a / f);
}

float dot(Quaternion a, Quaternion b) {
    return (a.x*b.x) + (a.y*b.y) + (a.z*b.z) + (a.w*b.w);
}

float length(Quaternion q) {
    return sqrt(dot(q, q));
}

float sqr_length(Quaternion q) {
    return dot(q, q);
}

Quaternion normalize(Quaternion q) {
    return q / length(q);
}

Quaternion inverse(Quaternion q) {
    Quaternion conjugate;
    conjugate.x = -q.x;
    conjugate.y = -q.y;
    conjugate.z = -q.z;
    conjugate.w = q.w;

    float len_sqr = dot(q, q);
    Quaternion result;
    result = conjugate / len_sqr;
    return result;
}

Quaternion axis_angle(Vector3 axis, float angle_radians) {
    Vector3 norm = normalize(axis);
    float s = sin(angle_radians / 2.0f);

    Vector3 vec = norm * s;

    Quaternion result;
    result.x = vec.x;
    result.y = vec.y;
    result.z = vec.z;
    result.w = cos(angle_radians / 2.0f);
    return result;
}

Vector3 quaternion_right  (Quaternion q) { return q * v3(1, 0, 0); }
Vector3 quaternion_up     (Quaternion q) { return q * v3(0, 1, 0); }
Vector3 quaternion_forward(Quaternion q) { return q * v3(0, 0, 1); }
Vector3 quaternion_left   (Quaternion q) { return -quaternion_right(q);   }
Vector3 quaternion_down   (Quaternion q) { return -quaternion_up(q);      }
Vector3 quaternion_back   (Quaternion q) { return -quaternion_forward(q); }



Matrix4 m4_identity() {
    Matrix4 m = {};
    m.elements[0][0] = 1.0f;
    m.elements[1][1] = 1.0f;
    m.elements[2][2] = 1.0f;
    m.elements[3][3] = 1.0f;
    return m;
}

Matrix4 m4(Vector4 columns[4]) {
    Matrix4 m;
    m[0] = columns[0];
    m[1] = columns[1];
    m[2] = columns[2];
    m[3] = columns[3];
    return m;
}

Matrix4 operator *(Matrix4 a, Matrix4 b) {
    Matrix4 result;
    for(int column = 0; column < 4; column++) {
        for(int row = 0; row < 4; row++) {
            float sum = 0;
            for(int vector = 0; vector < 4; vector++) {
                sum += a[vector][row] * b[column][vector];
            }
            result[column][row] = sum;
        }
    }
    return result;
}

Vector4 operator *(Matrix4 a, Vector4 v) {
    Vector4 result;
    for(int row = 0; row < 4; row++) {
        float sum = 0;
        for(int column = 0; column < 4; column++) {
            sum += a[column][row] * v[column];
        }
        result[row] = sum;
    }
    return result;
}

Matrix4 operator *(Matrix4 a, float f) {
    Matrix4 result;
    for(int column = 0; column < 4; column++) {
        for(int row = 0; row < 4; row++) {
            result[column][row] = a[column][row] * f;
        }
    }
    return result;
}

Matrix4 translate(Vector3 v) {
    Matrix4 result = m4_identity();
    result[3][0] = v[0];
    result[3][1] = v[1];
    result[3][2] = v[2];
    return result;
}

Matrix4 rotate(float angle_radians, Vector3 v) {
    float c = cos(angle_radians);
    float s = sin(angle_radians);

    Vector3 a = normalize(v);
    Vector3 t = a * (1.0f-c);

    Matrix4 result = m4_identity();

    result[0][0] = c + t[0]*a[0];
    result[0][1] = 0 + t[0]*a[1] + s*a[2];
    result[0][2] = 0 + t[0]*a[2] - s*a[1];
    result[0][3] = 0;

    result[1][0] = 0 + t[1]*a[0] - s*a[2];
    result[1][1] = c + t[1]*a[1];
    result[1][2] = 0 + t[1]*a[2] + s*a[0];
    result[1][3] = 0;

    result[2][0] = 0 + t[2]*a[0] + s*a[1];
    result[2][1] = 0 + t[2]*a[1] - s*a[0];
    result[2][2] = c + t[2]*a[2];
    result[2][3] = 0;

    return result;
}

Matrix4 mat4_scale(Vector3 v) {
    Matrix4 result = {};
    result[0][0] = v[0];
    result[1][1] = v[1];
    result[2][2] = v[2];
    result[3][3] = 1;
    return result;
}

Matrix4 transpose(Matrix4 a) {
    Matrix4 result;
    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 4; i++) {
            result[j][i] = a[i][j];
        }
    }
    return result;
}

// note(josh): left-handed
Matrix4 perspective(float fovy_radians, float aspect, float near, float far) {
    float tan_half_fovy = tan(0.5f * fovy_radians);
    Matrix4 result = {};
    result[0][0] = 1.0f / (aspect*tan_half_fovy);
    result[1][1] = 1.0f / (tan_half_fovy);
    result[2][2] = +(far + near) / (far - near);
    result[2][3] = 1.0f;
    result[3][2] = -2.0f*far*near / (far - near);

    // note(josh): uncomment to support right-handed coords. right-handed would be flip_z_axis=true
    // if flip_z_axis {
    //     m[2] = -m[2];
    // }

    return result;
}

Matrix4 orthographic(float left, float right, float bottom, float top, float near, float far) {
    Matrix4 result = {};
    result[0][0] = 2.0f / (right - left);
    result[1][1] = 2.0f / (top - bottom);
    result[2][2] = 2.0f / (far - near);
    result[3][0] = (right + left)   / (left - right);
    result[3][1] = (top   + bottom) / (bottom - top);
    result[3][2] = (far + near) / (near - far);
    result[3][3] = 1.0f;

    // note(josh): uncomment to support right-handed coords. right-handed would be flip_z_axis=true
    // if flip_z_axis {
    //     m[2] = -m[2];
    // }

    return result;
}

Matrix4 look_at(Vector3 eye, Vector3 center, Vector3 up) {
    Vector3 f = normalize(center - eye);
    Vector3 s = normalize(cross(f, up));
    Vector3 u = cross(s, f);

    float fe = dot(f, eye);

    Vector4 columns[4] = {
        {+s.x, +u.x, -f.x, 0},
        {+s.y, +u.y, -f.y, 0},
        {+s.z, +u.z, -f.z, 0},
        {-dot(s, eye), -dot(u, eye), -fe, 1},
    };

    // note(josh): uncomment to support right-handed coords. right-handed would be flip_z_axis=true
    // if flip_z_axis {
    //     columns[3][2] = -columns[3][2];
    // }

    Matrix4 result = m4(columns);
    return result;
}



Matrix4 quaternion_to_matrix4(Quaternion q) {
    Quaternion norm = normalize(q);

    float xx = norm.x * norm.x;
    float yy = norm.y * norm.y;
    float zz = norm.z * norm.z;
    float xy = norm.x * norm.y;
    float xz = norm.x * norm.z;
    float yz = norm.y * norm.z;
    float wx = norm.w * norm.x;
    float wy = norm.w * norm.y;
    float wz = norm.w * norm.z;

    Matrix4 result;

    result[0][0] = 1.0f - 2.0f * (yy + zz);
    result[0][1] = 2.0f * (xy + wz);
    result[0][2] = 2.0f * (xz - wy);
    result[0][3] = 0.0f;

    result[1][0] = 2.0f * (xy - wz);
    result[1][1] = 1.0f - 2.0f * (xx + zz);
    result[1][2] = 2.0f * (yz + wx);
    result[1][3] = 0.0f;

    result[2][0] = 2.0f * (xz + wy);
    result[2][1] = 2.0f * (yz - wx);
    result[2][2] = 1.0f - 2.0f * (xx + yy);
    result[2][3] = 0.0f;

    result[3][0] = 0.0f;
    result[3][1] = 0.0f;
    result[3][2] = 0.0f;
    result[3][3] = 1.0f;

    return result;
}

Quaternion matrix4_to_quaternion(Matrix4 m) {
    float t;
    Quaternion result;
    if (m[2][2] < 0.0f) {
        if (m[0][0] > m[1][1]) {
            t = 1 + m[0][0] - m[1][1] - m[2][2];
            result = quaternion(
                t,
                m[0][1] + m[1][0],
                m[2][0] + m[0][2],
                m[1][2] - m[2][1]
            );
        }
        else {
            t = 1 - m[0][0] + m[1][1] - m[2][2];
            result = quaternion(
                m[0][1] + m[1][0],
                t,
                m[1][2] + m[2][1],
                m[2][0] - m[0][2]
            );
        }
    }
    else {
        if (m[0][0] < -m[1][1]) {
            t = 1 - m[0][0] - m[1][1] + m[2][2];
            result = quaternion(
                m[2][0] + m[0][2],
                m[1][2] + m[2][1],
                t,
                m[0][1] - m[1][0]
            );
        }
        else {
            t = 1 + m[0][0] + m[1][1] + m[2][2];
            result = quaternion(
                m[1][2] - m[2][1],
                m[2][0] - m[0][2],
                m[0][1] - m[1][0],
                t
            );
        }
    }
    result *= 0.5f / sqrt(t);
    return result;
}

Quaternion quaternion_look_at(Vector3 eye, Vector3 center, Vector3 up) {
    // todo(josh): figure out why we have to swap center & eye, and why we need the inverse()
    Matrix4 m = look_at(center, eye, up);
    return inverse(matrix4_to_quaternion(m));
}

Matrix4 view_matrix(Vector3 position, Quaternion orientation) {
    Matrix4 t = translate(-position);
    Matrix4 r = quaternion_to_matrix4(inverse(orientation));
    Matrix4 result = r * t;
    return result;
}

Matrix4 model_matrix(Vector3 position, Vector3 scale, Quaternion orientation) {
    Matrix4 t = translate(position);
    Matrix4 s = mat4_scale(scale);
    Matrix4 r = quaternion_to_matrix4(orientation);
    Matrix4 model_matrix = (t * r) * s;
    return model_matrix;
}
