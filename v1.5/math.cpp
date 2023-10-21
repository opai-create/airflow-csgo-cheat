#include "globals.hpp"
#include <DirectXMath.h>

namespace math
{
	INLINE void vector_angles(const vec3_t& forward, const vec3_t& pseudoup, vec3_t& angles)
	{
		auto left = pseudoup.cross(forward);
		left = left.normalized();

		float forward_dist = forward.length_2d();
		if (forward_dist > 0.001f)
		{
			angles.x = (std::atan2f(-forward.z, forward_dist) * 180.f / (float)M_PI);
			angles.y = (std::atan2f(forward.y, forward.x) * 180.f / (float)M_PI);
			angles.z = (std::atan2f(left.z, (left.y * forward.x) - (left.x * forward.y)) * 180.f / (float)M_PI);
		}
		else
		{
			angles.x = (std::atan2f(-forward.z, forward_dist) * 180.f / (float)M_PI);
			angles.y = (std::atan2f(-left.x, left.y) * 180.f / (float)M_PI);
			angles.z = 0.f;
		}
	}

	INLINE void vector_angles(const vec3_t& forward, vec3_t& angles)
	{
		float yaw, pitch;

		if (forward.y == 0.f && forward.x == 0.f)
		{
			yaw = 0.f;
			if (forward.z > 0)
				pitch = 270.f;
			else
				pitch = 90.f;
		}
		else
		{
			yaw = (float)(std::atan2f(forward.y, forward.x) * 180.f / (float)M_PI);
			if (yaw < 0.f)
				yaw += 360.f;

			auto tmp = std::sqrtf(forward.x * forward.x + forward.y * forward.y);
			pitch = (float)(std::atan2f(-forward[2], tmp) * 180.f / (float)M_PI);
			if (pitch < 0.f)
				pitch += 360.f;
		}

		angles.x = pitch;
		angles.y = yaw;
		angles.z = 0.f;
	}

	INLINE void angle_vectors(const vec3_t& angles, vec3_t& forward)
	{
		float sx{}, sy{}, cx{}, cy{};

		DirectX::XMScalarSinCos(&sy, &cy, DEG2RAD(angles.y));
		DirectX::XMScalarSinCos(&sx, &cx, DEG2RAD(angles.x));

		forward.x = cx * cy;
		forward.y = cx * sy;
		forward.z = -sx;
	}

	INLINE void angle_vectors(const vec3_t& angles, vec3_t* forward, vec3_t* right, vec3_t* up)
	{
		float sr, sp, sy, cr, cp, cy;

		DirectX::XMScalarSinCos(&sy, &cy, DEG2RAD(angles.y));
		DirectX::XMScalarSinCos(&sp, &cp, DEG2RAD(angles.x));
		DirectX::XMScalarSinCos(&sr, &cr, DEG2RAD(angles.z));

		if (forward)
		{
			forward->x = (cp * cy);
			forward->y = (cp * sy);
			forward->z = (-sp);
		}

		if (right)
		{
			right->x = (-1.f * sr * sp * cy + -1.f * cr * -sy);
			right->y = (-1.f * sr * sp * sy + -1.f * cr * cy);
			right->z = (-1.f * sr * cp);
		}

		if (up)
		{
			up->x = (cr * sp * cy + -sr * -sy);
			up->y = (cr * sp * sy + -sr * cy);
			up->z = (cr * cp);
		}
	}

	INLINE float normalize_yaw(float yaw)
	{
		while (yaw < -180.f)
			yaw += 360.f;
		while (yaw > 180.f)
			yaw -= 360.f;

		return yaw;
	}

	INLINE void vector_transform(const vec3_t& in1, const matrix3x4_t& in2, vec3_t& out)
	{
		out =
		{
			in1.dot({ in2.mat[0][0], in2.mat[0][1], in2.mat[0][2] }) + in2.mat[0][3],
			in1.dot({ in2.mat[1][0], in2.mat[1][1], in2.mat[1][2] }) + in2.mat[1][3],
			in1.dot({ in2.mat[2][0], in2.mat[2][1], in2.mat[2][2] }) + in2.mat[2][3]
		};
	}

	INLINE vec3_t get_vector_transform(vec3_t& in1, const matrix3x4_t& in2)
	{
		return 
		{
			in1.dot(vec3_t(in2.mat[0][0], in2.mat[0][1], in2.mat[0][2])) + in2.mat[0][3],
			in1.dot(vec3_t(in2.mat[1][0], in2.mat[1][1], in2.mat[1][2])) + in2.mat[1][3],
			in1.dot(vec3_t(in2.mat[2][0], in2.mat[2][1], in2.mat[2][2])) + in2.mat[2][3]
		};
	}

	INLINE void change_bones_position(matrix3x4_t* bones, size_t msize, vec3_t current_position, vec3_t new_position)
	{
		for (size_t i = 0; i < msize; ++i)
		{
			bones[i].mat[0][3] += new_position.x - current_position.x;
			bones[i].mat[1][3] += new_position.y - current_position.y;
			bones[i].mat[2][3] += new_position.z - current_position.z;
		}
	}

	INLINE vec3_t calc_angle(const vec3_t& a, const vec3_t& b)
	{
		vec3_t angles{};

		vec3_t delta = a - b;
		float hyp = delta.length_2d();

		// 57.295f - pi in degrees
		angles.y = std::atan(delta.y / delta.x) * 57.2957795131f;
		angles.x = std::atan(-delta.z / hyp) * -57.2957795131f;
		angles.z = 0.0f;

		if (delta.x >= 0.0f)
			angles.y += 180.0f;

		return angles;
	}

	// thanks to https://www.unknowncheats.me/forum/counterstrike-global-offensive/282111-offscreen-esp.html
	INLINE void rotate_triangle_points(vec2_t points[3], float rotation)
	{
		const auto points_center = (points[0] + points[1] + points[2]) / vec2_t(3.f, 3.f);
		for (int i = 0; i < 3; i++)
		{
			vec2_t& point = points[i];

			point -= points_center;

			const auto temp_x = point.x;
			const auto temp_y = point.y;

			const auto theta = rotation;
			const auto c = std::cos(theta);
			const auto s = std::sin(theta);

			point.x = temp_x * c - temp_y * s;
			point.y = temp_x * s + temp_y * c;

			point += points_center;
		}
	}

	INLINE void random_seed(std::uint32_t seed)
	{
		static random_seed_fn random_seed = NULL;

		if (!random_seed)
			random_seed = (random_seed_fn)WINCALL(GetProcAddress)(HACKS->modules.vstdlib, CXOR("RandomSeed"));

		random_seed(seed);
	}

	INLINE float random_float(float min, float max)
	{
		static random_float_fn random_float = NULL;

		if (!random_float)
			random_float = (random_float_fn)WINCALL(GetProcAddress)(HACKS->modules.vstdlib, CXOR("RandomFloat"));

		return random_float(min, max);
	}

	INLINE int random_int(int min, int max)
	{
		static random_int_fn random_int = NULL;

		if (!random_int)
			random_int = (random_int_fn)WINCALL(GetProcAddress)(HACKS->modules.vstdlib, CXOR("RandomInt"));

		return random_int(min, max);
	}

	INLINE void contact_transforms(const matrix3x4_t& in1, const matrix3x4_t& in2, matrix3x4_t& out)
	{
		if (&in1 == &out)
		{
			matrix3x4_t in1b;
			matrix_copy(in1, in1b);
			contact_transforms(in1b, in2, out);
			return;
		}

		if (&in2 == &out)
		{
			matrix3x4_t in2b;
			matrix_copy(in2, in2b);
			contact_transforms(in1, in2b, out);
			return;
		}

		out.mat[0][0] = in1.mat[0][0] * in2.mat[0][0] + in1.mat[0][1] * in2.mat[1][0] + in1.mat[0][2] * in2.mat[2][0];
		out.mat[0][1] = in1.mat[0][0] * in2.mat[0][1] + in1.mat[0][1] * in2.mat[1][1] + in1.mat[0][2] * in2.mat[2][1];
		out.mat[0][2] = in1.mat[0][0] * in2.mat[0][2] + in1.mat[0][1] * in2.mat[1][2] + in1.mat[0][2] * in2.mat[2][2];
		out.mat[0][3] = in1.mat[0][0] * in2.mat[0][3] + in1.mat[0][1] * in2.mat[1][3] + in1.mat[0][2] * in2.mat[2][3] + in1.mat[0][3];

		out.mat[1][0] = in1.mat[1][0] * in2.mat[0][0] + in1.mat[1][1] * in2.mat[1][0] + in1.mat[1][2] * in2.mat[2][0];
		out.mat[1][1] = in1.mat[1][0] * in2.mat[0][1] + in1.mat[1][1] * in2.mat[1][1] + in1.mat[1][2] * in2.mat[2][1];
		out.mat[1][2] = in1.mat[1][0] * in2.mat[0][2] + in1.mat[1][1] * in2.mat[1][2] + in1.mat[1][2] * in2.mat[2][2];
		out.mat[1][3] = in1.mat[1][0] * in2.mat[0][3] + in1.mat[1][1] * in2.mat[1][3] + in1.mat[1][2] * in2.mat[2][3] + in1.mat[1][3];

		out.mat[2][0] = in1.mat[2][0] * in2.mat[0][0] + in1.mat[2][1] * in2.mat[1][0] + in1.mat[2][2] * in2.mat[2][0];
		out.mat[2][1] = in1.mat[2][0] * in2.mat[0][1] + in1.mat[2][1] * in2.mat[1][1] + in1.mat[2][2] * in2.mat[2][1];
		out.mat[2][2] = in1.mat[2][0] * in2.mat[0][2] + in1.mat[2][1] * in2.mat[1][2] + in1.mat[2][2] * in2.mat[2][2];
		out.mat[2][3] = in1.mat[2][0] * in2.mat[0][3] + in1.mat[2][1] * in2.mat[1][3] + in1.mat[2][2] * in2.mat[2][3] + in1.mat[2][3];
	}

	INLINE float get_fov(const vec3_t& view_angle, const vec3_t& aim_angle)
	{
		vec3_t delta = aim_angle - view_angle;
		delta = delta.normalized_angle();
		return std::min(std::sqrtf(std::powf(delta.x, 2.0f) + std::powf(delta.y, 2.0f)), 180.0f);
	}

	INLINE void vector_i_transform(const vec3_t& in1, const matrix3x4_t& in2, vec3_t& out)
	{
		out.x = (in1.x - in2.mat[0][3]) * in2.mat[0][0] + (in1.y - in2.mat[1][3]) * in2.mat[1][0] + (in1.z - in2.mat[2][3]) * in2.mat[2][0];
		out.y = (in1.x - in2.mat[0][3]) * in2.mat[0][1] + (in1.y - in2.mat[1][3]) * in2.mat[1][1] + (in1.z - in2.mat[2][3]) * in2.mat[2][1];
		out.z = (in1.x - in2.mat[0][3]) * in2.mat[0][2] + (in1.y - in2.mat[1][3]) * in2.mat[1][2] + (in1.z - in2.mat[2][3]) * in2.mat[2][2];
	}

	INLINE void vector_i_rotate(const vec3_t& in1, const matrix3x4_t& in2, vec3_t& out)
	{
		out.x = in1.x * in2.mat[0][0] + in1.y * in2.mat[1][0] + in1.z * in2.mat[2][0];
		out.y = in1.x * in2.mat[0][1] + in1.y * in2.mat[1][1] + in1.z * in2.mat[2][1];
		out.z = in1.x * in2.mat[0][2] + in1.y * in2.mat[1][2] + in1.z * in2.mat[2][2];
	}

	INLINE bool intersect_line_with_bb(vec3_t& start, vec3_t& end, vec3_t& min, vec3_t& max)
	{
		float d1{}, d2{}, f{};
		auto t1 = -1.f, t2 = 1.f;

		auto start_solid = true;

		for (std::size_t i{}; i < 6u; ++i)
		{
			if (i >= 3)
			{
				const auto j = i - 3u;

				d1 = start[j] - max[j];
				d2 = d1 + end[j];
			}
			else
			{
				d1 = -start[i] + min[i];
				d2 = d1 - end[i];
			}

			if (d1 > 0.0f && d2 > 0.0f)
				return false;

			if (d1 <= 0.0f && d2 <= 0.0f)
				continue;

			if (d1 > 0.f)
				start_solid = false;

			if (d1 > d2)
			{
				f = d1;

				if (f < 0.f)
					f = 0.f;

				f /= d1 - d2;

				if (f > t1)
					t1 = f;
			}
			else
			{
				f = d1 / (d1 - d2);

				if (f < t2)
					t2 = f;
			}
		}

		return start_solid || (t1 < t2 && t1 >= 0.f);
	}

	INLINE float segment_to_segment(const vec3_t& s1, const vec3_t& s2, vec3_t& k1, vec3_t& k2)
	{
		static auto constexpr epsilon = 0.00000001;

		auto u = s2 - s1;
		auto v = k2 - k1;
		const auto w = s1 - k1;

		const auto a = u.dot(u);
		const auto b = u.dot(v);
		const auto c = v.dot(v);
		const auto d = u.dot(w);
		const auto e = v.dot(w);
		const auto D = a * c - b * b;
		float sn, sd = D;
		float tn, td = D;

		if (D < epsilon) {
			sn = 0.0;
			sd = 1.0;
			tn = e;
			td = c;
		}
		else {
			sn = b * e - c * d;
			tn = a * e - b * d;

			if (sn < 0.0) {
				sn = 0.0;
				tn = e;
				td = c;
			}
			else if (sn > sd) {
				sn = sd;
				tn = e + b;
				td = c;
			}
		}

		if (tn < 0.0) {
			tn = 0.0;

			if (-d < 0.0)
				sn = 0.0;
			else if (-d > a)
				sn = sd;
			else {
				sn = -d;
				sd = a;
			}
		}
		else if (tn > td) {
			tn = td;

			if (-d + b < 0.0)
				sn = 0;
			else if (-d + b > a)
				sn = sd;
			else {
				sn = -d + b;
				sd = a;
			}
		}

		const float sc = abs(sn) < epsilon ? 0.0f : sn / sd;
		const float tc = abs(tn) < epsilon ? 0.0f : tn / td;

		m128 n{};
		auto dp = w + u * sc - v * tc;
		n.f[0] = dp.dot(dp);
		const auto calc = _mm_sqrt_ps(n.v);
		return reinterpret_cast<const m128*>(&calc)->f[0];
	}

	INLINE void vector_multiply(const vec3_t& start, float scale, const vec3_t& direction, vec3_t& dest)
	{
		dest.x = start.x + direction.x * scale;
		dest.y = start.y + direction.y * scale;
		dest.z = start.z + direction.z * scale;
	}

	inline float approach(float target, float value, float speed) {
		float delta = target - value;

		if (delta > speed)
			value += speed;
		else if (delta < -speed)
			value -= speed;
		else
			value = target;

		return value;
	}

	inline vec3_t approach(vec3_t target, vec3_t value, float speed) {
		vec3_t diff = (target - value);
		float delta = diff.length();

		if (delta > speed)
			value += diff.normalized() * speed;
		else if (delta < -speed)
			value -= diff.normalized() * speed;
		else
			value = target;

		return value;
	}

	inline float anglemod(float a) {
		a = (360.f / 65536) * ((int)(a * (65536.f / 360.0f)) & 65535);
		return a;
	}

	inline float approach_angle(float target, float value, float speed) {
		target = anglemod(target);
		value = anglemod(value);

		float delta = target - value;

		// Speed is assumed to be positive
		if (speed < 0)
			speed = -speed;

		if (delta < -180)
			delta += 360;
		else if (delta > 180)
			delta -= 360;

		if (delta > speed)
			value += speed;
		else if (delta < -speed)
			value -= speed;
		else
			value = target;

		return value;
	}

	inline float angle_diff(float dst, float src) {
		float delta = dst - src;

		if (delta < -180)
			delta += 360;
		else if (delta > 180)
			delta -= 360;

		return delta;
	}
}