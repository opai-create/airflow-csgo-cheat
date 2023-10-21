#pragma once

constexpr auto KNEEMAX_EPSILON = 0.9998f;

class c_ik_solver {
public:
	static float findD(float a, float b, float c) {
		return (c + (a * a - b * b) / c) / 2;
	}
	static float findE(float a, float d) { return sqrt(a * a - d * d); }

	float Mfwd[3][3];
	float Minv[3][3];

	bool solve(float A, float B, float const P[], float const D[], float Q[]) {
		float R[3];
		defineM(P, D);
		rot(Minv, P, R);
		float r = length(R);
		float d = findD(A, B, r);
		float e = findE(A, d);
		float S[3] = { d,e,0 };
		rot(Mfwd, S, Q);
		return d > (r - B) && d < A;
	}

	void defineM(float const P[], float const D[]) {
		float* X = Minv[0], * Y = Minv[1], * Z = Minv[2];

		int i;
		for (i = 0; i < 3; i++)
			X[i] = P[i];
		normalize(X);

		float dDOTx = dot(D, X);
		for (i = 0; i < 3; i++)
			Y[i] = D[i] - dDOTx * X[i];
		normalize(Y);

		cross(X, Y, Z);
		for (i = 0; i < 3; i++) {
			Mfwd[i][0] = Minv[0][i];
			Mfwd[i][1] = Minv[1][i];
			Mfwd[i][2] = Minv[2][i];
		}
	}

	static float dot(float const a[], float const b[]) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; }

	static float length(float const v[]) { return sqrt(dot(v, v)); }

	static void normalize(float v[]) {
		float norm = length(v);
		for (int i = 0; i < 3; i++)
			v[i] /= norm;
	}

	static void cross(float const a[], float const b[], float c[]) {
		c[0] = a[1] * b[2] - a[2] * b[1];
		c[1] = a[2] * b[0] - a[0] * b[2];
		c[2] = a[0] * b[1] - a[1] * b[0];
	}

	static void rot(float const M[3][3], float const src[], float dst[]) {
		for (int i = 0; i < 3; i++)
			dst[i] = dot(M[i], src);
	}
};

void studio_align_ik_matrix(matrix3x4_t& mMat, const vec3_t& vAlignTo) {
	vec3_t tmp1, tmp2, tmp3;

	tmp1 = vAlignTo;
	tmp1 = tmp1.normalized();

	mMat.mat[0][0] = tmp1.x;
	mMat.mat[1][0] = tmp1.y;
	mMat.mat[2][0] = tmp1.z;

	tmp3.x = mMat.mat[0][2];
	tmp3.y = mMat.mat[1][2];
	tmp3.z = mMat.mat[2][2];

	tmp2 = tmp3.cross(tmp1);
	tmp2 = tmp2.normalized();

	mMat.mat[0][1] = tmp2.x;
	mMat.mat[1][1] = tmp2.y;
	mMat.mat[2][1] = tmp2.z;

	tmp3 = tmp1.cross(tmp2);

	mMat.mat[0][2] = tmp3.x;
	mMat.mat[1][2] = tmp3.y;
	mMat.mat[2][2] = tmp3.z;
}

bool studio_solve_ik(int thigh, int knee, int foot, vec3_t& target_foot, vec3_t& target_knee_pos, vec3_t& target_knee_dir, matrix3x4_t* bone_to_world) {
	auto world_foot = bone_to_world[foot].get_origin();
	auto world_knee = bone_to_world[knee].get_origin();
	auto world_thigh = bone_to_world[thigh].get_origin();

	auto ik_foot = target_foot - world_thigh;
	auto ik_knee = target_knee_pos - world_thigh;

	float l1 = (world_knee - world_thigh).length();
	float l2 = (world_foot - world_knee).length();

	float d = (target_foot - world_thigh).length() - std::min<float>(l1, l2);
	d = std::max<float>(l1 + l2, d);

	d = d * 100;

	auto ik_traget_knee = ik_knee + target_knee_dir * d;
	int color[3] = { 0, 255, 0 };

	if (ik_foot.length() > (l1 + l2) * KNEEMAX_EPSILON) {
		ik_foot = ik_foot.normalized();
		ik_foot *= (l1 + l2) * KNEEMAX_EPSILON;

		color[0] = 255; color[1] = 0; color[2] = 0;
	}

	float min_distance = std::max<float>(std::fabs(l1 - l2) * 1.15f, std::min<float>(l1, l2) * 0.15f);
	if (ik_foot.length() < min_distance) {
		ik_foot = (world_foot - world_thigh).normalized();
		ik_foot *= min_distance;
	}

	c_ik_solver ik;
	if (ik.solve(l1, l2, ik_foot.base(), ik_traget_knee.base(), ik_knee.base()))
	{
		matrix3x4_t& matrix_world_thigh = bone_to_world[thigh];
		matrix3x4_t& matrix_world_knee = bone_to_world[knee];
		matrix3x4_t& matrix_world_foot = bone_to_world[foot];

		studio_align_ik_matrix(matrix_world_thigh, ik_knee);
		studio_align_ik_matrix(matrix_world_knee, ik_foot - ik_knee);

		matrix_world_knee.mat[0][3] = ik_knee.x + world_thigh.x;
		matrix_world_knee.mat[1][3] = ik_knee.y + world_thigh.y;
		matrix_world_knee.mat[2][3] = ik_knee.z + world_thigh.z;

		matrix_world_foot.mat[0][3] = ik_foot.x + world_thigh.x;
		matrix_world_foot.mat[1][3] = ik_foot.y + world_thigh.y;
		matrix_world_foot.mat[2][3] = ik_foot.z + world_thigh.z;

		return true;
	}
	else
		return false;
}

bool studio_solve_ik(int thigh, int knee, int foot, vec3_t& target_foot, matrix3x4_t* bone_to_world) {
	auto world_foot = bone_to_world[foot].get_origin();
	auto world_knee = bone_to_world[knee].get_origin();
	auto world_thigh = bone_to_world[thigh].get_origin();

	auto ik_foot = target_foot - world_thigh;
	auto ik_knee = world_knee - world_thigh;

	float l1 = (world_knee - world_thigh).length();
	float l2 = (world_foot - world_knee).length();
	float l3 = (world_foot - world_thigh).length();

	if (l3 > (l1 + l2) * KNEEMAX_EPSILON) {
		return false;
	}

	if (l1 <= 0 || l2 <= 0 || l3 <= 0) {
		return false;
	}

	auto ik_half = (world_foot - world_thigh) * (l1 / l3);

	auto ik_knee_dir = (ik_knee - ik_half).normalized();
	return studio_solve_ik(thigh, knee, foot, target_foot, world_knee, ik_knee_dir, bone_to_world);
}