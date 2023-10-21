#pragma once
#include <math.h>
#include <intrin.h>
#include <corecrt_math.h>
#include <xmmintrin.h>
#include <pmmintrin.h>

#define RAD2DEG(radian) (float)radian * (180.f / (float)M_PI)
#define DEG2RAD(degree) (float)degree * ((float)M_PI / 180.f)

static const float invtwopi = 0.1591549f;
static const float twopi = 6.283185f;
static const float threehalfpi = 4.7123889f;
static const float pi = 3.141593f;
static const float pi2 = 1.570796f;
static const __m128 signmask = _mm_castsi128_ps(_mm_set1_epi32(0x80000000));

static const __declspec(align(16)) float null[4] = { 0.f, 0.f, 0.f, 0.f };
static const __declspec(align(16)) float _pi2[4] = { 1.5707963267948966192f, 1.5707963267948966192f, 1.5707963267948966192f, 1.5707963267948966192f };
static const __declspec(align(16)) float _pi[4] = { 3.141592653589793238f, 3.141592653589793238f, 3.141592653589793238f, 3.141592653589793238f };

typedef __declspec(align(16)) union
{
    float f[4];
    __m128 v;
} m128;

__forceinline __m128 sqrt_ps(const __m128 squared)
{
    return _mm_sqrt_ps(squared);
}

__forceinline __m128 cos_52s_ps(const __m128 x)
{
    const auto c1 = _mm_set1_ps(0.9999932946f);
    const auto c2 = _mm_set1_ps(-0.4999124376f);
    const auto c3 = _mm_set1_ps(0.0414877472f);
    const auto c4 = _mm_set1_ps(-0.0012712095f);
    const auto x2 = _mm_mul_ps(x, x);
    return _mm_add_ps(c1, _mm_mul_ps(x2, _mm_add_ps(c2, _mm_mul_ps(x2, _mm_add_ps(c3, _mm_mul_ps(c4, x2))))));
}

__forceinline __m128 cos_ps(__m128 angle)
{
    angle = _mm_andnot_ps(signmask, angle);
    angle = _mm_sub_ps(angle, _mm_mul_ps(_mm_cvtepi32_ps(_mm_cvttps_epi32(_mm_mul_ps(angle, _mm_set1_ps(invtwopi)))), _mm_set1_ps(twopi)));

    auto cosangle = angle;
    cosangle = _mm_xor_ps(cosangle, _mm_and_ps(_mm_cmpge_ps(angle, _mm_set1_ps(pi2)), _mm_xor_ps(cosangle, _mm_sub_ps(_mm_set1_ps(pi), angle))));
    cosangle = _mm_xor_ps(cosangle, _mm_and_ps(_mm_cmpge_ps(angle, _mm_set1_ps(pi)), signmask));
    cosangle = _mm_xor_ps(cosangle, _mm_and_ps(_mm_cmpge_ps(angle, _mm_set1_ps(threehalfpi)), _mm_xor_ps(cosangle, _mm_sub_ps(_mm_set1_ps(twopi), angle))));

    auto result = cos_52s_ps(cosangle);
    result = _mm_xor_ps(result, _mm_and_ps(_mm_and_ps(_mm_cmpge_ps(angle, _mm_set1_ps(pi2)), _mm_cmplt_ps(angle, _mm_set1_ps(threehalfpi))), signmask));
    return result;
}

__forceinline __m128 sin_ps(const __m128 angle)
{
    return cos_ps(_mm_sub_ps(_mm_set1_ps(pi2), angle));
}

__forceinline void sincos_ps(__m128 angle, __m128* sin, __m128* cos)
{
    const auto anglesign = _mm_or_ps(_mm_set1_ps(1.f), _mm_and_ps(signmask, angle));
    angle = _mm_andnot_ps(signmask, angle);
    angle = _mm_sub_ps(angle, _mm_mul_ps(_mm_cvtepi32_ps(_mm_cvttps_epi32(_mm_mul_ps(angle, _mm_set1_ps(invtwopi)))), _mm_set1_ps(twopi)));

    auto cosangle = angle;
    cosangle = _mm_xor_ps(cosangle, _mm_and_ps(_mm_cmpge_ps(angle, _mm_set1_ps(pi2)), _mm_xor_ps(cosangle, _mm_sub_ps(_mm_set1_ps(pi), angle))));
    cosangle = _mm_xor_ps(cosangle, _mm_and_ps(_mm_cmpge_ps(angle, _mm_set1_ps(pi)), signmask));
    cosangle = _mm_xor_ps(cosangle, _mm_and_ps(_mm_cmpge_ps(angle, _mm_set1_ps(threehalfpi)), _mm_xor_ps(cosangle, _mm_sub_ps(_mm_set1_ps(twopi), angle))));

    auto result = cos_52s_ps(cosangle);
    result = _mm_xor_ps(result, _mm_and_ps(_mm_and_ps(_mm_cmpge_ps(angle, _mm_set1_ps(pi2)), _mm_cmplt_ps(angle, _mm_set1_ps(threehalfpi))), signmask));
    *cos = result;

    const auto sinmultiplier = _mm_mul_ps(anglesign, _mm_or_ps(_mm_set1_ps(1.f), _mm_and_ps(_mm_cmpgt_ps(angle, _mm_set1_ps(pi)), signmask)));
    *sin = _mm_mul_ps(sinmultiplier, sqrt_ps(_mm_sub_ps(_mm_set1_ps(1.f), _mm_mul_ps(result, result))));
}

struct quaternion_t
{
    float x, y, z, w;
};

struct rect2_t
{
    float w, h;

    INLINE rect2_t() : w(0.f), h(0.f) {}
    INLINE rect2_t(const rect2_t& other) : w(other.w), h(other.h) {}
    INLINE rect2_t(const float& w, const float& h) : w(w), h(h) {}

    INLINE void reset()
    {
        w = 0.f;
        h = 0.f;
    }

    INLINE bool invalid()
    {
        return w != 0.f && h != 0.f;
    }
};

struct vec2_t
{
    float x, y;

    INLINE vec2_t() : x(0.f), y(0.f) {}
    INLINE vec2_t(const vec2_t& other) : x(other.x), y(other.y) {}
    INLINE vec2_t(const float& x, const float& y) : x(x), y(y) {}

    INLINE bool operator==(const vec2_t& other)
    {
        return this->x == other.x && this->y == other.y;
    }

    INLINE vec2_t operator+(const vec2_t& other)
    {
        return vec2_t(this->x + other.x, this->y + other.y);
    }

    INLINE vec2_t operator-(const vec2_t& other)
    {
        return vec2_t(this->x - other.x, this->y - other.y);
    }

    INLINE vec2_t operator/(const vec2_t& other)
    {
        return vec2_t(this->x / other.x, this->y / other.y);
    }

    INLINE vec2_t operator*(const vec2_t& other)
    {
        return vec2_t(this->x * other.x, this->y * other.y);
    }

    INLINE vec2_t operator+=(const vec2_t& other)
    {
        this->x += other.x;
        this->y += other.y;
        return *this;
    }

    INLINE vec2_t operator-=(const vec2_t& other)
    {
        this->x -= other.x;
        this->y -= other.y;
        return *this;
    }

    INLINE vec2_t operator/=(const vec2_t& other)
    {
        this->x /= other.x;
        this->y /= other.y;
        return *this;
    }

    INLINE vec2_t operator*=(const vec2_t& other)
    {
        this->x *= other.x;
        this->y *= other.y;
        return *this;
    }

    INLINE float dot(const vec2_t& other) const
    {
        return this->x * other.x + this->y * other.y;
    }

    INLINE float dist_to(const vec2_t& other) const
    {
        return std::sqrtf(std::powf(other.x - this->x, 2) + std::powf(other.y - this->y, 2));
    }

    INLINE float length_sqr() const
    {
        return this->x * this->x + this->y * this->y;
    }

    INLINE float length() const
    {
        return std::sqrtf(length_sqr());
    }

    INLINE void reset()
    {
        x = y = 0.f;
    }
};

struct vec3_t
{
    float x, y, z;

    INLINE vec3_t() : x(0.f), y(0.f), z(0.f) {}
    INLINE vec3_t(const vec3_t& other) : x(other.x), y(other.y), z(other.z) {}
    INLINE vec3_t(float x, float y, float z) : x(x), y(y), z(z) {}

    INLINE float* base()
    {
        return (float*)this;
    }

    INLINE bool operator==(const vec3_t& other)
    {
        return this->x == other.x && this->y == other.y && this->z == other.z;
    }

    INLINE bool operator!=(const vec3_t& other)
    {
        return this->x != other.x && this->y != other.y && this->z != other.z;
    }

    INLINE vec3_t operator+(const vec3_t& other) const
    {
        return vec3_t{ this->x + other.x, this->y + other.y, this->z + other.z };
    }

    INLINE vec3_t operator-() const
    {
        return vec3_t{ -this->x, -this->y, -this->z };
    }

    INLINE vec3_t operator-(const vec3_t& other) const
    {
        return vec3_t{ this->x - other.x, this->y - other.y, this->z - other.z };
    }

    INLINE vec3_t operator/(const vec3_t& other) const
    {
        return vec3_t{ this->x / other.x, this->y / other.y, this->z / other.z };
    }

    INLINE vec3_t operator*(const vec3_t& other) const
    {
        return vec3_t{ this->x * other.x, this->y * other.y, this->z * other.z };
    }

    INLINE vec3_t operator*(const float& v) const
    {
        return vec3_t{ this->x * v, this->y * v, this->z * v };
    }

    INLINE vec3_t operator+=(const vec3_t& other)
    {
        this->x += other.x;
        this->y += other.y;
        this->z += other.z;
        return *this;
    }

    INLINE vec3_t operator-=(const vec3_t& other)
    {
        this->x -= other.x;
        this->y -= other.y;
        this->z -= other.z;
        return *this;
    }

    INLINE vec3_t operator/=(const vec3_t& other)
    {
        this->x /= other.x;
        this->y /= other.y;
        this->z /= other.z;
        return *this;
    }

    INLINE vec3_t operator*=(const vec3_t& other)
    {
        this->x *= other.x;
        this->y *= other.y;
        this->z *= other.z;
        return *this;
    }

    INLINE vec3_t operator*=(const float& fl)
    {
        this->x *= fl;
        this->y *= fl;
        this->z *= fl;
        return *this;
    }

    INLINE vec3_t operator/(const float& fl)
    {
        return vec3_t{ this->x / fl, this->y / fl, this->z / fl };
    }

    INLINE vec3_t operator/=(const float& fl)
    {
        this->x *= fl;
        this->y *= fl;
        this->z *= fl;
        return *this;
    }

    INLINE float& operator[](int i)
    {
        return ((float*)this)[i];
    }

    INLINE float operator[](int i) const
    {
        return ((float*)this)[i];
    }

    INLINE void reset()
    {
        x = y = z = 0.f;
    }

    INLINE void reset_invalid()
    {
        if (std::isnan(this->x) || std::isnan(this->y) || std::isnan(this->z))
            reset();

        if (std::isinf(this->x) || std::isinf(this->y) || std::isinf(this->z))
            reset();
    }

    INLINE bool valid()
    {
        return length() > 0.f;
    }

    INLINE float dot(float* other) const
    {
        return this->x * other[0] + this->y * other[1] + this->z * other[2];
    }

    INLINE float dot(const vec3_t& other) const
    {
        return this->x * other.x + this->y * other.y + this->z * other.z;
    }

    INLINE vec3_t cross(const vec3_t& other) const
    {
        return vec3_t{ (this->y * other.z) - (this->z * other.y), (this->z * other.x) - (this->x * other.z), (this->x * other.y) - (this->y * other.x) };
    }

    INLINE float length_sqr_2d() const
    {
        return this->x * this->x + this->y * this->y;
    }

    INLINE float length_sqr() const
    {
        return this->x * this->x + this->y * this->y + this->z * this->z;
    }

    INLINE float length_2d() const
    {
        return std::sqrtf(length_sqr_2d());
    }

    INLINE float length() const
    {
        return std::sqrtf(length_sqr());
    }

    INLINE float dist_to(const vec3_t& other) const
    {
        return std::sqrtf(std::powf(other.x - this->x, 2) + std::powf(other.y - this->y, 2) + std::powf(other.z - this->z, 2));
    }

    INLINE void multiply_angle(const vec3_t& start, float scale, const vec3_t& dir)
    {
        x = start.x + dir.x * scale;
        y = start.y + dir.y * scale;
        z = start.z + dir.z * scale;
    }

    INLINE vec3_t normalized() const
    {
        vec3_t out = *this;
        out.normalized_float();
        return out;
    }

    INLINE float normalized_float()
    {
        const float vec_length = length();
        const float radius = 1.0f / (vec_length + FLT_EPSILON);

        this->x *= radius;
        this->y *= radius;
        this->z *= radius;

        return vec_length;
    }

    INLINE float normalize_angle(float angle)
    {
        angle = fmodf(angle, 360.0f);
        if (angle > 180)
        {
            angle -= 360;
        }
        if (angle < -180)
        {
            angle += 360;
        }
        return angle;
    }

    INLINE vec3_t normalized_angle()
    {
        auto vec = *this;
        vec.x = normalize_angle(vec.x);
        vec.y = normalize_angle(vec.y);
        vec.z = normalize_angle(vec.z);
        return vec;
    }
};

struct vec4_t
{
    float x, y, w, h;

    INLINE vec4_t() : x(0.f), y(0.f), w(0.f), h(0.f) {}
    INLINE vec4_t(const vec4_t& other) : x(other.x), y(other.y), w(other.w), h(other.h) {}
    INLINE vec4_t(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}

    INLINE vec4_t operator*=(const vec4_t& other)
    {
        this->x *= other.x;
        this->y *= other.y;
        this->w *= other.w;
        this->h *= other.h;
        return *this;
    }

    INLINE vec3_t as_vec3_t()
    {
        return *(vec3_t*)this;
    }

    INLINE float* base()
    {
        return (float*)this;
    }
};

struct vertex_t
{
    vec2_t position;
    vec2_t coords;

    INLINE vertex_t() {}
    INLINE vertex_t(const vec2_t& position, const vec2_t& coords = vec2_t(0, 0)) : position(position), coords(coords) {}
};

struct __declspec(align(16)) matrix3x4_t
{
    float mat[3][4];

    matrix3x4_t() = default;

    INLINE vec3_t at(int i) const
    {
        return vec3_t{ mat[0][i], mat[1][i], mat[2][i] };
    }

    INLINE float* base()
    {
        return &mat[0][0];
    }

    INLINE const float* base() const
    {
        return &mat[0][0];
    }

    INLINE matrix3x4_t(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23)
    {
        mat[0][0] = m00;
        mat[0][1] = m01;
        mat[0][2] = m02;
        mat[0][3] = m03;
        mat[1][0] = m10;
        mat[1][1] = m11;
        mat[1][2] = m12;
        mat[1][3] = m13;
        mat[2][0] = m20;
        mat[2][1] = m21;
        mat[2][2] = m22;
        mat[2][3] = m23;
    }

    INLINE void set_origin(const vec3_t& p)
    {
        mat[0][3] = p.x;
        mat[1][3] = p.y;
        mat[2][3] = p.z;
    }

    INLINE void scale(int i, float scale)
    {
        mat[i][0] *= scale;
        mat[i][1] *= scale;
        mat[i][2] *= scale;
    }

    INLINE vec3_t get_origin()
    {
        return { mat[0][3], mat[1][3], mat[2][3] };
    }

    INLINE void quaternion_matrix(const quaternion_t& q)
    {
        mat[0][0] = 1.f - 2.f * q.y * q.y - 2.f * q.z * q.z;
        mat[1][0] = 2.f * q.x * q.y + 2.f * q.w * q.z;
        mat[2][0] = 2.f * q.x * q.z - 2.f * q.w * q.y;

        mat[0][1] = 2.f * q.x * q.y - 2.f * q.w * q.z;
        mat[1][1] = 1.f - 2.f * q.x * q.x - 2.f * q.z * q.z;
        mat[2][1] = 2.f * q.y * q.z + 2.f * q.w * q.x;

        mat[0][2] = 2.f * q.x * q.z + 2.f * q.w * q.y;
        mat[1][2] = 2.f * q.y * q.z - 2.f * q.w * q.x;
        mat[2][2] = 1.f - 2.f * q.x * q.x - 2.f * q.y * q.y;

        mat[0][3] = 0.f;
        mat[1][3] = 0.f;
        mat[2][3] = 0.f;
    }

    INLINE void quaternion_matrix(const quaternion_t& q, const vec3_t& pos)
    {
        quaternion_matrix(q);

        mat[0][3] = pos.x;
        mat[1][3] = pos.y;
        mat[2][3] = pos.z;
    }

    INLINE matrix3x4_t contact_transforms(matrix3x4_t in) const
    {
        matrix3x4_t out = { };

        out.mat[0][0] = mat[0][0] * in.mat[0][0] + mat[0][1] * in.mat[1][0] + mat[0][2] * in.mat[2][0];
        out.mat[0][1] = mat[0][0] * in.mat[0][1] + mat[0][1] * in.mat[1][1] + mat[0][2] * in.mat[2][1];
        out.mat[0][2] = mat[0][0] * in.mat[0][2] + mat[0][1] * in.mat[1][2] + mat[0][2] * in.mat[2][2];
        out.mat[0][3] = mat[0][0] * in.mat[0][3] + mat[0][1] * in.mat[1][3] + mat[0][2] * in.mat[2][3] + mat[0][3];
        out.mat[1][0] = mat[1][0] * in.mat[0][0] + mat[1][1] * in.mat[1][0] + mat[1][2] * in.mat[2][0];
        out.mat[1][1] = mat[1][0] * in.mat[0][1] + mat[1][1] * in.mat[1][1] + mat[1][2] * in.mat[2][1];
        out.mat[1][2] = mat[1][0] * in.mat[0][2] + mat[1][1] * in.mat[1][2] + mat[1][2] * in.mat[2][2];
        out.mat[1][3] = mat[1][0] * in.mat[0][3] + mat[1][1] * in.mat[1][3] + mat[1][2] * in.mat[2][3] + mat[1][3];
        out.mat[2][0] = mat[2][0] * in.mat[0][0] + mat[2][1] * in.mat[1][0] + mat[2][2] * in.mat[2][0];
        out.mat[2][1] = mat[2][0] * in.mat[0][1] + mat[2][1] * in.mat[1][1] + mat[2][2] * in.mat[2][1];
        out.mat[2][2] = mat[2][0] * in.mat[0][2] + mat[2][1] * in.mat[1][2] + mat[2][2] * in.mat[2][2];
        out.mat[2][3] = mat[2][0] * in.mat[0][3] + mat[2][1] * in.mat[1][3] + mat[2][2] * in.mat[2][3] + mat[2][3];

        return out;
    }

    INLINE void angle_matrix(const vec3_t& angles)
    {
        float sr, sp, sy, cr, cp, cy;

        sy = std::sin(DEG2RAD(angles.y));
        cy = std::cos(DEG2RAD(angles.y));

        sp = std::sin(DEG2RAD(angles.x));
        cp = std::cos(DEG2RAD(angles.x));

        sr = std::sin(DEG2RAD(angles.z));
        cr = std::cos(DEG2RAD(angles.z));

        mat[0][0] = cp * cy;
        mat[1][0] = cp * sy;
        mat[2][0] = -sp;

        float crcy = cr * cy;
        float crsy = cr * sy;
        float srcy = sr * cy;
        float srsy = sr * sy;
        mat[0][1] = sp * srcy - crsy;
        mat[1][1] = sp * srsy + crcy;
        mat[2][1] = sr * cp;

        mat[0][2] = (sp * crcy + srsy);
        mat[1][2] = (sp * crsy - srcy);
        mat[2][2] = cr * cp;

        mat[0][3] = 0.0f;
        mat[1][3] = 0.0f;
        mat[2][3] = 0.0f;
    }

    INLINE void multiply(const matrix3x4_t& other)
    {
        mat[0][0] = mat[0][0] * other.mat[0][0] + mat[0][1] * other.mat[1][0] + mat[0][2] * other.mat[2][0];
        mat[0][1] = mat[0][0] * other.mat[0][1] + mat[0][1] * other.mat[1][1] + mat[0][2] * other.mat[2][1];
        mat[0][2] = mat[0][0] * other.mat[0][2] + mat[0][1] * other.mat[1][2] + mat[0][2] * other.mat[2][2];
        mat[0][3] = mat[0][0] * other.mat[0][3] + mat[0][1] * other.mat[1][3] + mat[0][2] * other.mat[2][3] + mat[0][3];
        mat[1][0] = mat[1][0] * other.mat[0][0] + mat[1][1] * other.mat[1][0] + mat[1][2] * other.mat[2][0];
        mat[1][1] = mat[1][0] * other.mat[0][1] + mat[1][1] * other.mat[1][1] + mat[1][2] * other.mat[2][1];
        mat[1][2] = mat[1][0] * other.mat[0][2] + mat[1][1] * other.mat[1][2] + mat[1][2] * other.mat[2][2];
        mat[1][3] = mat[1][0] * other.mat[0][3] + mat[1][1] * other.mat[1][3] + mat[1][2] * other.mat[2][3] + mat[1][3];
        mat[2][0] = mat[2][0] * other.mat[0][0] + mat[2][1] * other.mat[1][0] + mat[2][2] * other.mat[2][0];
        mat[2][1] = mat[2][0] * other.mat[0][1] + mat[2][1] * other.mat[1][1] + mat[2][2] * other.mat[2][1];
        mat[2][2] = mat[2][0] * other.mat[0][2] + mat[2][1] * other.mat[1][2] + mat[2][2] * other.mat[2][2];
        mat[2][3] = mat[2][0] * other.mat[0][3] + mat[2][1] * other.mat[1][3] + mat[2][2] * other.mat[2][3] + mat[2][3];
    }

    INLINE void angle_matrix(const vec3_t& angles, const vec3_t& position)
    {
        this->angle_matrix(angles);
        this->set_origin(position);
    }
};

class __declspec(align(16)) vec3_aligned_t : public vec3_t
{
public:
    vec3_aligned_t() {}

    INLINE vec3_aligned_t(float x, float y, float z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    INLINE explicit vec3_aligned_t(const vec3_t& other)
    {
        this->x = other.x;
        this->y = other.y;
        this->z = other.z;
    }

    INLINE vec3_aligned_t& operator=(const vec3_t& other)
    {
        this->x = other.x;
        this->y = other.y;
        this->z = other.z;
        return *this;
    }

    INLINE vec3_aligned_t& operator=(const vec3_aligned_t& other)
    {
        this->x = other.x;
        this->y = other.y;
        this->z = other.z;
        return *this;
    }

    float w;
};

namespace math
{
    extern void vector_angles(const vec3_t& forward, const vec3_t& pseudoup, vec3_t& angles);
    extern void vector_angles(const vec3_t& forward, vec3_t& angles);
    extern void angle_vectors(const vec3_t& angles, vec3_t& forward);
    extern void angle_vectors(const vec3_t& angles, vec3_t* forward, vec3_t* right, vec3_t* up);
    extern float normalize_yaw(float yaw);
    extern void vector_transform(const vec3_t& in1, const matrix3x4_t& in2, vec3_t& out);
    extern vec3_t get_vector_transform(vec3_t& in1, const matrix3x4_t& in2);
    extern void change_bones_position(matrix3x4_t* bones, size_t msize, vec3_t current_position, vec3_t new_position);
    extern vec3_t calc_angle(const vec3_t& a, const vec3_t& b);
    extern void rotate_triangle_points(vec2_t points[3], float rotation);

    template <class T>
    INLINE T lerp(float percent, T const& A, T const& B)
    {
        return A + (B - A) * percent;
    }

    template <class T>
    INLINE T reversed_lerp(float percent, T const& A, T const& B)
    {
        return B * percent + A * (1.f - percent);
    }

    INLINE void memcpy_sse(void* dest, const void* src, std::size_t count) 
    {
        __movsb(static_cast<BYTE*>(dest), static_cast<const BYTE*>(src), count);
    }

    INLINE float simple_spline(float value)
    {
        float val_squared = value * value;

        // Nice little ease-in, ease-out spline-like curve
        return (3 * val_squared - 2 * val_squared * value);
    }

    INLINE float reval_map_clamped(float val, float A, float B, float C, float D)
    {
        if (A == B)
            return val >= B ? D : C;
        float cVal = (val - A) / (B - A);
        cVal = std::clamp(cVal, 0.0f, 1.0f);
        return C + (D - C) * cVal;
    }

    INLINE float simple_spline_reval_map_clamped(float val, float A, float B, float C, float D)
    {
        if (A == B)
            return val >= B ? D : C;
        float cVal = (val - A) / (B - A);
        cVal = std::clamp(cVal, 0.0f, 1.0f);
        return C + (D - C) * simple_spline(cVal);
    }

    INLINE void matrix_copy(const matrix3x4_t& source, matrix3x4_t& target)
    {
        memcpy_sse(&target, &source, sizeof(matrix3x4_t));
    }

    extern void contact_transforms(const matrix3x4_t& in1, const matrix3x4_t& in2, matrix3x4_t& out);
    extern float get_fov(const vec3_t& view_angle, const vec3_t& aim_angle);
    extern void vector_i_transform(const vec3_t& in1, const matrix3x4_t& in2, vec3_t& out);
    extern void vector_i_rotate(const vec3_t& in1, const matrix3x4_t& in2, vec3_t& out);
    extern bool intersect_line_with_bb(vec3_t& start, vec3_t& end, vec3_t& min, vec3_t& max);
    extern float segment_to_segment(const vec3_t& s1, const vec3_t& s2, vec3_t& k1, vec3_t& k2);

    using random_seed_fn = void(*)(UINT);
    using random_int_fn = int(*)(int, int);
    using random_float_fn = float(*)(float, float);

    extern void random_seed(std::uint32_t seed);
    extern float random_float(float min, float max);
    extern int random_int(int min, int max);

    extern void vector_multiply(const vec3_t& start, float scale, const vec3_t& direction, vec3_t& dest);
    extern float approach(float target, float value, float speed);
    extern vec3_t approach(vec3_t target, vec3_t value, float speed);
    extern float approach_angle(float target, float value, float speed);
    extern float angle_diff(float dst, float src);

    INLINE float smoothstep_bounds(float edge0, float edge1, float x) 
    {
        x = std::clamp<float>((x - edge0) / (edge1 - edge0), 0, 1);
        return x * x * (3 - 2 * x);
    }

    INLINE float clamp_cycle(float cycle_in) 
    {
        cycle_in -= static_cast<int>(cycle_in);

        if (cycle_in < 0) 
        {
            cycle_in += 1;
        }
        else if (cycle_in > 1) 
        {
            cycle_in -= 1;
        }

        return cycle_in;
    }
}