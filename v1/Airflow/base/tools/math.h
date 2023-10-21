#pragma once
#include <cmath>
#include <corecrt_math.h>
#include <xmmintrin.h>
#include <pmmintrin.h>

#define PI 3.14159265358979323846
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

class vector3d;
class vector2d;

struct matrix3x4_t;

namespace easings
{
	float east_out_cubic(float x);
	float ease_out_quint(float x);
	float ease_out_expo(float x);
	float ease_out_quad(float x);
}

namespace math
{
	extern float rad_to_deg(float radian);
	extern float deg_to_rad(float degree);

	float get_fov(const vector3d& view_angle, const vector3d& aim_angle);

	vector3d get_vector_transform(vector3d& in1, const matrix3x4_t& in2);
	void vector_transform(vector3d in1, const matrix3x4_t& in2, vector3d& out);

	void vector_to_angles(vector3d forward, vector3d& angles);
	void vector_to_angles(vector3d& forward, vector3d& up, vector3d& angles);
	void angle_to_vectors(vector3d angles, vector3d& forward);
	void angle_to_vectors(vector3d angles, vector3d& forward, vector3d& right, vector3d& up);

	bool is_near_equal(float v1, float v2, float toler);
	vector3d angle_from_vectors(vector3d a, vector3d b);

	float angle_diff(float destAngle, float srcAngle);
	float normalize(float ang);
	vector3d normalize(vector3d ang, bool fix_pitch);

	float remap_val(float val, float A, float B, float C, float D);
	float remap_val_clamped(float val, float A, float B, float C, float D);

	float interpolate_inversed(float percent, const float& A, const float& B);
	float interpolate(const float& from, const float& to, const float& percent);
	vector3d interpolate(const vector3d& from, const vector3d& to, const float& percent);

	extern int time_to_ticks(float time);
	extern float ticks_to_time(int ticks);

	vector3d vector_rotate(const vector3d& in1, const vector3d& in2);
	void vector_i_transform(const vector3d& in1, const matrix3x4_t& in2, vector3d& out);
	void vector_i_rotate(const vector3d& in1, const matrix3x4_t& in2, vector3d& out);
	bool intersect_line_with_bb(vector3d& start, vector3d& end, vector3d& min, vector3d& max);

	float segment_to_segment(const vector3d& s1, const vector3d& s2, vector3d& k1, vector3d& k2);

	void contact_transforms(const matrix3x4_t& in1, const matrix3x4_t& in2, matrix3x4_t& out);

	void random_seed(uint32_t seed);
	float random_float(float min, float max);
	int random_int(int min, int max);

	void vector_multiply(const vector3d& start, float scale, const vector3d& direction, vector3d& dest);

	void rotate_triangle_points(vector2d points[3], float rotation);
	void rotate_matrix(matrix3x4_t in[128], matrix3x4_t out[128], float delta, vector3d render_origin);
	void change_matrix_position(matrix3x4_t* bones, size_t msize, vector3d current_position, vector3d new_position);

	float reval_map_clamped(float val, float A, float B, float C, float D);
	float simple_spline_reval_map_clamped(float val, float A, float B, float C, float D);

	float smoothstep_bounds(float edge0, float edge1, float x);
	float approach(float target, float value, float speed);
	vector3d approach(vector3d target, vector3d value, float speed);
	float clamp_cycle(float cycle_in);
	float bias(float x, float bias_amt);
	float angle_diff(float dest, float src);

	template < typename T >
	T hermite_spline(T p1, T p2, T d1, T d2, float t)
	{
		float tSqr = t * t;
		float tCube = t * tSqr;

		float b1 = 2.f * tCube - 3.f * tSqr + 1.f;
		float b2 = 1.f - b1;
		float b3 = tCube - 2 * tSqr + t;
		float b4 = tCube - tSqr;

		T output;
		output = p1 * b1;
		output += p2 * b2;
		output += d1 * b3;
		output += d2 * b4;

		return output;
	}

	template < typename T >
	T hermite_spline(T p0, T p1, T p2, float t)
	{
		return hermite_spline(p1, p2, p1 - p0, p2 - p1, t);
	}
}

class quaternion
{
public:
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
	float w = 0.f;
};

class rect2d
{
public:
	float w = 0.f, h = 0.f;

	rect2d()
	{
	}
	rect2d(float w, float h) : w(w), h(h)
	{
	}

	__forceinline void reset()
	{
		w = 0.f;
		h = 0.f;
	}

	__forceinline bool invalid()
	{
		return w != 0.f && h != 0.f;
	}
};

class vector2d
{
public:
	float x = 0.f, y = 0.f;

	vector2d()
	{
	}
	vector2d(float x, float y) : x(x), y(y)
	{
	}

	bool operator==(const vector2d& v)
	{
		return this->x == v.x && this->y == v.y;
	}

	vector2d operator+(const vector2d& v)
	{
		return vector2d(this->x + v.x, this->y + v.y);
	}

	vector2d operator-(const vector2d& v)
	{
		return vector2d(this->x - v.x, this->y - v.y);
	}

	vector2d operator/(const vector2d& v)
	{
		return vector2d(this->x / v.x, this->y / v.y);
	}

	vector2d operator*(const vector2d& v)
	{
		return vector2d(this->x * v.x, this->y * v.y);
	}

	vector2d operator+=(const vector2d& v)
	{
		this->x += v.x;
		this->y += v.y;
		return *this;
	}

	vector2d operator-=(const vector2d& v)
	{
		this->x -= v.x;
		this->y -= v.y;
		return *this;
	}

	vector2d operator/=(const vector2d& v)
	{
		this->x /= v.x;
		this->y /= v.y;
		return *this;
	}

	vector2d operator*=(const vector2d& v)
	{
		this->x *= v.x;
		this->y *= v.y;
		return *this;
	}

	__forceinline float scalar_product(const vector2d& v)
	{
		return this->x * v.x + this->y * v.y;
	}

	__forceinline float dist_to(const vector2d& p)
	{
		return std::sqrt(std::pow(p.x - this->x, 2) + std::pow(p.y - this->y, 2));
	}

	__forceinline float length()
	{
		return std::sqrt(std::pow(this->x, 2) + std::pow(this->y, 2));
	}

	__forceinline void reset()
	{
		x = y = 0.f;
	}
};

class vector3d
{
public:
	float x = 0.f, y = 0.f, z = 0.f;

	vector3d()
	{
	}
	vector3d(float x, float y, float z) : x(x), y(y), z(z)
	{
	}

	bool operator==(const vector3d& v)
	{
		return this->x == v.x && this->y == v.y && this->z == v.z;
	}

	bool operator!=(const vector3d& v)
	{
		return this->x != v.x && this->y != v.y && this->z != v.z;
	}

	vector3d operator+(const vector3d& v) const
	{
		return vector3d(this->x + v.x, this->y + v.y, this->z + v.z);
	}

	vector3d operator-() const
	{
		return vector3d(-this->x, -this->y, -this->z);
	}

	vector3d operator-(const vector3d& v) const
	{
		return vector3d(this->x - v.x, this->y - v.y, this->z - v.z);
	}

	vector3d operator/(const vector3d& v) const
	{
		return vector3d(this->x / v.x, this->y / v.y, this->z / v.z);
	}

	vector3d operator*(const vector3d& v) const
	{
		return vector3d(this->x * v.x, this->y * v.y, this->z * v.z);
	}

	vector3d operator*(const float& v) const
	{
		return vector3d(this->x * v, this->y * v, this->z * v);
	}

	vector3d operator+=(const vector3d& v)
	{
		this->x += v.x;
		this->y += v.y;
		this->z += v.z;
		return *this;
	}

	vector3d operator-=(const vector3d& v)
	{
		this->x -= v.x;
		this->y -= v.y;
		this->z -= v.z;
		return *this;
	}

	vector3d operator/=(const vector3d& v)
	{
		this->x /= v.x;
		this->y /= v.y;
		this->z /= v.z;
		return *this;
	}

	vector3d operator*=(const vector3d& v)
	{
		this->x *= v.x;
		this->y *= v.y;
		this->z *= v.z;
		return *this;
	}

	vector3d operator*=(const float& fl)
	{
		this->x *= fl;
		this->y *= fl;
		this->z *= fl;
		return *this;
	}

	vector3d operator/(const float& fl)
	{
		return vector3d(this->x / fl, this->y / fl, this->z / fl);
	}

	vector3d operator/=(const float& fl)
	{
		this->x *= fl;
		this->y *= fl;
		this->z *= fl;
		return *this;
	}

	__forceinline void reset()
	{
		x = y = z = 0.f;
	}

	__forceinline bool valid()
	{
		return *this != vector3d(0.f, 0.f, 0.f);
	}

	inline void reset_invalid()
	{
		if (std::isnan(this->x) || std::isnan(this->y) || std::isnan(this->z))
			this->reset();

		if (std::isinf(this->x) || std::isinf(this->y) || std::isinf(this->z))
			this->reset();
	}

	void init(float x, float y, float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	__forceinline float& operator[](int i)
	{
		return ((float*)this)[i];
	}

	__forceinline float operator[](int i) const
	{
		return ((float*)this)[i];
	}

	__forceinline float dot(float* v)
	{
		return this->x * v[0] + this->y * v[1] + this->z * v[2];
	}

	__forceinline vector3d cross(vector3d& v) const
	{
		return vector3d((this->y * v.z) - (this->z * v.y), (this->z * v.x) - (this->x * v.z), (this->x * v.y) - (this->y * v.x));
	}

	__forceinline float dot(const vector3d& v)
	{
		return this->x * v.x + this->y * v.y + this->z * v.z;
	}

	__forceinline float length(bool ignore_z) const
	{
		if (ignore_z)
			return std::sqrt(this->x * this->x + this->y * this->y);

		return std::sqrt(this->x * this->x + this->y * this->y + this->z * this->z);
	}

	__forceinline float dist_to(const vector3d& p)
	{
		return std::sqrt(std::pow(p.x - this->x, 2) + std::pow(p.y - this->y, 2) + std::pow(p.z - this->z, 2));
	}

	__forceinline float length_sqr()
	{
		return this->x * this->x + this->y * this->y + this->z * this->z;
	}

	__forceinline vector3d normalized_angle()
	{
		while (this->y > 180)
		{
			this->y -= 360;
		}
		while (this->y < -180)
		{
			this->y += 360;
		}

		while (this->x > 89)
		{
			this->x -= 180;
		}

		while (this->x < -89)
		{
			this->x += 180;
		}
		return *this;
	}

	__forceinline vector3d normalized() const
	{
		vector3d out = *this;
		out.normalized_float();
		return out;
	}

	__forceinline float normalized_float()
	{
		const float vec_length = length(false);
		const float radius = 1.0f / (vec_length + FLT_EPSILON);

		this->x *= radius;
		this->y *= radius;
		this->z *= radius;

		return vec_length;
	}

	void ma(const vector3d& start, float scale, const vector3d& dir)
	{
		x = start.x + dir.x * scale;
		y = start.y + dir.y * scale;
		z = start.z + dir.z * scale;
	}

	float* base()
	{
		return (float*)this;
	}
};

class vector4d
{
public:
	float x = 0.f, y = 0.f, w = 0.f, h = 0.f;

	vector4d()
	{
	}
	vector4d(float x, float y, float w, float h) : x(x), y(y), w(w), h(h)
	{
	}

	vector4d operator*=(const vector4d& v)
	{
		this->x *= v.x;
		this->y *= v.y;
		this->w *= v.w;
		this->h *= v.h;
		return *this;
	}

	vector3d as_vector3d()
	{
		return *(vector3d*)this;
	}

	float* base()
	{
		return (float*)this;
	}
};

class vertex
{
public:
	vector2d position;
	vector2d tex_position;

	vertex()
	{
	}
	vertex(const vector2d& pos, const vector2d& coord = vector2d(0, 0)) : position(pos), tex_position(coord)
	{
	}
};

struct matrix3x4_t
{
	matrix3x4_t() = default;

	vector3d at(int i) const
	{
		return vector3d{ mat[0][i], mat[1][i], mat[2][i] };
	}
	float* operator[](int i)
	{
		return mat[i];
	}
	const float* operator[](int i) const
	{
		return mat[i];
	}
	float* base()
	{
		return &mat[0][0];
	}
	const float* base() const
	{
		return &mat[0][0];
	}

	matrix3x4_t(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23)
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

	__forceinline void set_origin(const vector3d& p)
	{
		mat[0][3] = p.x;
		mat[1][3] = p.y;
		mat[2][3] = p.z;
	}

	void scale(int i, float scale)
	{
		mat[i][0] *= scale;
		mat[i][1] *= scale;
		mat[i][2] *= scale;
	}

	__forceinline vector3d get_origin()
	{
		vector3d ret(mat[0][3], mat[1][3], mat[2][3]);
		return ret;
	}

	void quaternion_matrix(const quaternion& q)
	{
		mat[0][0] = 1.0 - 2.0 * q.y * q.y - 2.0 * q.z * q.z;
		mat[1][0] = 2.0 * q.x * q.y + 2.0 * q.w * q.z;
		mat[2][0] = 2.0 * q.x * q.z - 2.0 * q.w * q.y;

		mat[0][1] = 2.0f * q.x * q.y - 2.0f * q.w * q.z;
		mat[1][1] = 1.0f - 2.0f * q.x * q.x - 2.0f * q.z * q.z;
		mat[2][1] = 2.0f * q.y * q.z + 2.0f * q.w * q.x;

		mat[0][2] = 2.0f * q.x * q.z + 2.0f * q.w * q.y;
		mat[1][2] = 2.0f * q.y * q.z - 2.0f * q.w * q.x;
		mat[2][2] = 1.0f - 2.0f * q.x * q.x - 2.0f * q.y * q.y;

		mat[0][3] = 0.0f;
		mat[1][3] = 0.0f;
		mat[2][3] = 0.0f;
	}

	void quaternion_matrix(const quaternion& q, const vector3d& pos)
	{
		quaternion_matrix(q);

		mat[0][3] = pos.x;
		mat[1][3] = pos.y;
		mat[2][3] = pos.z;
	}

	matrix3x4_t contact_transforms(matrix3x4_t in) const
	{
		matrix3x4_t out = {};

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

	void angle_matrix(const vector3d& angles)
	{
		float sr, sp, sy, cr, cp, cy;

		sy = std::sin(math::deg_to_rad(angles.y));
		cy = std::cos(math::deg_to_rad(angles.y));

		sp = std::sin(math::deg_to_rad(angles.x));
		cp = std::cos(math::deg_to_rad(angles.x));

		sr = std::sin(math::deg_to_rad(angles.z));
		cr = std::cos(math::deg_to_rad(angles.z));

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

	void multiply(const matrix3x4_t& other)
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

	void angle_matrix(const vector3d& angles, const vector3d& position)
	{
		this->angle_matrix(angles);
		this->set_origin(position);
	}

	float mat[3][4] = {};
};

class __declspec(align(16)) matrix3x4a_t : public matrix3x4_t
{

};

class __declspec(align(16)) vector_aligned : public vector3d
{
public:
	vector_aligned()
	{
	}

	vector_aligned(float x, float y, float z)
	{
		init(x, y, z);
	}

	explicit vector_aligned(const vector3d& v)
	{
		init(v.x, v.y, v.z);
	}

	vector_aligned& operator=(const vector3d& v)
	{
		init(v.x, v.y, v.z);
		return *this;
	}

	vector_aligned& operator=(const vector_aligned& v)
	{
		init(v.x, v.y, v.z);
		return *this;
	}

	float w = 0.f;
};