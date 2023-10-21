#pragma once

struct RnMaterial_t
{
	RnMaterial_t()
	{

	}

	// Default material	(density of water in kg/inch^3)
	RnMaterial_t(float flFriction, float flRestitution)
		: m_flDensity(0.015625f)
		, m_flFriction(flFriction)
		, m_flRestitution(flRestitution)
	{

	}

	float m_flDensity;
	float m_flFriction;
	float m_flRestitution;
	unsigned int m_pUserData;
};

struct CShapeCastResult
{
	float m_flHitTime;
	vec3_t m_vHitPoint;
	vec3_t m_vHitNormal;
	const RnMaterial_t* m_pMaterial;
	bool m_bStartInSolid;

	CShapeCastResult(void)
	{
		m_flHitTime = 1.0f;
		m_vHitPoint = vec3_t(0, 0, 0);
		m_vHitNormal = vec3_t(0, 0, 0);
		m_bStartInSolid = false;
		m_pMaterial = NULL;
	}

	bool DidHit(void)
	{
		return m_flHitTime < 1.0f;
	}
};

inline float DotProduct(const vec3_t& a, const vec3_t& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline float DotProduct(const float* a, const float* b)
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

inline float DotProduct(const vec3_t& a, const float* b)
{
	return a.x * b[0] + a.y * b[1] + a.z * b[2];
}

inline void CrossProduct(const vec3_t& a, const vec3_t& b, vec3_t& result)
{
	result.x = a.y * b.z - a.z * b.y;
	result.y = a.z * b.x - a.x * b.z;
	result.z = a.x * b.y - a.y * b.x;
}

// assuming the matrix is orthonormal, transform in1 by the transpose (also the inverse in this case) of in2.
void VectorITransform(const vec3_t& in1, const matrix3x4_t& in2, vec3_t& out)
{
	float in1t[3];

	in1t[0] = in1[0] - in2.mat[0][3];
	in1t[1] = in1[1] - in2.mat[1][3];
	in1t[2] = in1[2] - in2.mat[2][3];

	float x = in1t[0] * in2.mat[0][0] + in1t[1] * in2.mat[1][0] + in1t[2] * in2.mat[2][0];
	float y = in1t[0] * in2.mat[0][1] + in1t[1] * in2.mat[1][1] + in1t[2] * in2.mat[2][1];
	float z = in1t[0] * in2.mat[0][2] + in1t[1] * in2.mat[1][2] + in1t[2] * in2.mat[2][2];

	out[0] = x;
	out[1] = y;
	out[2] = z;
}

INLINE void VectorMultiply(const vec3_t& a, float b, vec3_t& c)
{
	c.x = a.x * b;
	c.y = a.y * b;
	c.z = a.z * b;
}

INLINE void VectorMultiply(const vec3_t& a, const vec3_t& b, vec3_t& c)
{
	c.x = a.x * b.x;
	c.y = a.y * b.y;
	c.z = a.z * b.z;
}

// for backwards compatability
inline void VectorScale(const vec3_t& in, float scale, vec3_t& result)
{
	VectorMultiply(in, scale, result);
}

inline float Sqr(float v)
{
	return v * v;
}

#define fsel(c,x,y) ( (c) >= 0 ? (x) : (y) )

inline float RemapVal(float val, float A, float B, float C, float D)
{
	if (A == B)
		return fsel(val - B, D, C);
	return C + (D - C) * (val - A) / (B - A);
}

void VectorPerpendicularToVector(vec3_t const& in, vec3_t* pvecOut)
{
	float flY = in.y * in.y;
	pvecOut->x = RemapVal(flY, 0, 1, in.z, 1);
	pvecOut->y = 0;
	pvecOut->z = -in.x;
	*pvecOut = pvecOut->normalized();
	float flDot = DotProduct(*pvecOut, in);
	*pvecOut -= vec3_t(flDot, flDot, flDot) * in;
	*pvecOut = pvecOut->normalized();
}

inline const vec3_t VectorPerpendicularToVector(const vec3_t& in)
{
	vec3_t out;
	VectorPerpendicularToVector(in, &out);
	return out;
}

struct CapsuleCast2D_t
{
	float m_flCapsule, m_flRay;
};

//--------------------------------------------------------------------------------------------------
static void CastCapsuleRay2DCoaxialInternal(CapsuleCast2D_t& out, float mx, float dx, float h, float e)
{
	float mxProj = mx + e; // m.x - (-e)
	if (mxProj < 0)
	{
		// ray starts before the capsule cap
		out.m_flCapsule = 0;
		if (dx >= -mxProj) // otherwise, ending before capsule starts: FLT_MAX
		{
			out.m_flRay = -mxProj / dx;
		}
	}
	else if (mx < h + e) // otherwise, starting after capsule ends : FLT_MAX
	{
		out.m_flCapsule = std::clamp(mx, 0.0f, h);
		out.m_flRay = 0;
	}
	else
	{
		// ray starts after the capsule cap
		out.m_flCapsule = h;
		float mxEnd = mx - (h + e);
		if (-dx >= mxEnd)
		{
			out.m_flRay = mxEnd / -dx;
		}
	}
}


//--------------------------------------------------------------------------------------------------
static void CastCapsuleRay2DParallelInternal(CapsuleCast2D_t& out, const vec2_t& m, float dx, float h, float rr)
{
	float e2 = rr - Sqr(m.y);
	if (e2 > 0) // otherwise, going parallel and outside : FLT_MAX
	{
		// going parallel and inside the infinite slab at level m.y, left to right
		float e = sqrtf(e2); // -e..h+e is the extent 
		CastCapsuleRay2DCoaxialInternal(out, m.x, dx, h, e);
	}
}

INLINE double fpmax(double a, double b)
{
	return a >= b ? a : b;
}

//--------------------------------------------------------------------------------------------------
// Intersect 2D ray with 2D capsule; capsule has radius r, length h, it starts at (0,0) and ends at (h,0)
// ray goes from m, delta d
// return: time of hit
static void CastCapsuleRay2DInternal(CapsuleCast2D_t& out, const vec2_t& m, const vec2_t& d, float h, float rr)
{
	float my2 = Sqr(m.y);
	out.m_flCapsule = std::clamp(m.x, 0.0f, h);

	// Easy case we'll have to check a few times if we delay: are we starting in solid?
	// same idea as with box-box distance: cut out x=0..h, capsule becomes a circle, find distance to circle
	// I'm sure there's more elegant way to handle it
	if (Sqr(m.x - out.m_flCapsule) + my2 < rr)
	{
		out.m_flRay = 0; // start-in-solid
		return;
	}
	// well, we don't start inside the capsule. Good to know
	float r = sqrtf(rr), dd = Sqr(d.x) + Sqr(d.y), ddInv = 1.0f / dd, dymy = d.y * m.y;

	// first, intersect the ray with the rectangle

	float t = (-r - m.y) / d.y, t0 = fpmax(0, t), s0 = m.x + d.x * t0;

	// solutions: -b0�sqrt(b0^2-c0) , -b1�sqrt(b1^2-c1) with	� controlled by d.x sign
	// since we know we go left-bottom to right-top, we can just choose the circle we wanna hit
	// since we know we don't start-in-solid, we know the first root (if any) will be t>=0
	float mxh;
	if (s0 < 0)
	{
		// we're entering through the left cap
		// if we hit, we hit left circle
		out.m_flCapsule = 0;
		mxh = m.x;
	}
	else if (s0 < h)
	{
		// we're entering through the side of the capsule
		out.m_flCapsule = s0;
		if (t >= 0) // only if we didn't enter before ray started; otherwise, since we didn't start-in-solid, we don't hit capsule at all
		{
			out.m_flRay = t; // the caller will sort out if it's >1 or not
		}
		return;
	}
	else
	{
		out.m_flCapsule = h;
		mxh = m.x - h;
	}

	float b = (d.x * mxh + dymy) * ddInv, c = (mxh * mxh + my2 - rr) * ddInv, D = b * b - c;
	if (D >= 0)
	{
		float tc = -b - sqrtf(D);
		//Assert(tc - t >= -1e-4f);	// the ray should really enter the circle after it entered the stripe of halfspaces 
		// if tc < 0, we entered capsule before ray began; since we didn't start-in-solid, it means we don't hit the capsule at all
		if (tc >= 0)
		{
			out.m_flRay = tc;
		}
	}
}

static void CastCapsuleShortRay(CShapeCastResult& out, const vec3_t& sUnit, float sLen, const vec3_t& m, const vec3_t& vRayStart, const vec3_t vCenter[], float flRadius)
{
	// the ray is too short, just compute the distance to the capsule and compare with radius
	// if we really need both high precision and stability, we need to compute distance to capsule from both ends of the ray: the capsule curvature is very low in the vicinity of the ray and is o(d^2) effect
	float flProjOnCapsule = DotProduct(sUnit, m);
	vec3_t vDistance;
	if (flProjOnCapsule < 0)
	{
		vDistance = m;
	}
	else if (flProjOnCapsule > sLen)
	{
		vDistance = vRayStart - vCenter[1];

	}
	else
	{
		vDistance = m - vCenter[0] * flProjOnCapsule;
	}

	float flDistFromCapsuleSqr = vDistance.length_sqr();

	if (flDistFromCapsuleSqr > flRadius)
	{
		// the ray is outside of the capsule
		out.m_bStartInSolid = false;
		out.m_flHitTime = 1.0f;
	}
	else
	{
		out.m_bStartInSolid = true;
		out.m_flHitTime = 0;
		out.m_vHitNormal = flDistFromCapsuleSqr > 1e-8f ? vDistance / sqrtf(flDistFromCapsuleSqr) : VectorPerpendicularToVector(sUnit);
		out.m_vHitPoint = vRayStart;
	}
}

inline vec3_t CrossProduct(const vec3_t& a, const vec3_t& b)
{
	return vec3_t(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

static void CastStationaryHit(CShapeCastResult& out, float c, const vec3_t& p, const vec3_t& m, float mm)
{
	// return a sphere hit for zero-length ray at point p, with
	// m = p - m_vCenter
	// mm = DotProduct( m, m )
	// c = mm - Sqr( m_flRadius )

	if (c <= 0)
	{
		out.m_flHitTime = 0;
		out.m_vHitPoint = p;
		if (mm > FLT_EPSILON)
		{
			out.m_vHitNormal = m / vec3_t(sqrtf(mm), sqrtf(mm), sqrtf(mm));
		}
		else
		{
			out.m_vHitNormal = vec3_t(0, 0, 1);
		}
	}
	else
	{
		// we didn't hit - we're outside and we don't move
		out.m_flHitTime = FLT_MAX;
	}
}


//--------------------------------------------------------------------------------------------------
void CastSphereRay(CShapeCastResult& out, const vec3_t& m, const vec3_t& p, const vec3_t& d, float flRadius)
{
	float a = DotProduct(d, d), mm = DotProduct(m, m), c = mm - Sqr(flRadius);
	if (a < FLT_EPSILON * FLT_EPSILON)
	{
		// we barely move; just detect if we're in the sphere or not
		CastStationaryHit(out, c, p, m, mm);
		return;
	}

	float b = DotProduct(m, d); // solve: at^2+2bt+c=0; t = (-b�sqrt(b^2-ac))/a = -b/a � sqrt((b/a)^2-c/a))
	float D = Sqr(b) - a * c;
	if (D < 0)
	{
		// no intersection at all
		out.m_flHitTime = FLT_MAX;
		return;
	}
	float sqrtD = sqrtf(D);
	float t = (-b - sqrtD) / a;
	if (t < 0)
	{
		// this was the first hit in the past - determine if we're still inside the sphere at time t=0
		// we could do that by checking if float t1 = ( b + sqrtD ) / a; is > 0 or not, but it's easier to:
		// we barely move; just detect if we're in the sphere or not
		CastStationaryHit(out, c, p, m, mm);
	}
	else
	{
		out.m_flHitTime = t;
		vec3_t dt = d * t;
		out.m_vHitPoint = p + dt;
		out.m_vHitNormal = (m + dt) / flRadius; // Should I normalize this here or is this sufficient precision?
	}
}


void CastCapsuleRay(CShapeCastResult& out, const vec3_t& vRayStart, const vec3_t& vRayDelta, const vec3_t vCenter[], float flRadius)
{
	vec3_t m = vRayStart - vCenter[0], s = vCenter[1] - vCenter[0];
	float sLen = s.length();

	if (flRadius < 1e-5f)
	{
		return;
	}

	if (sLen < 1e-3f) // note: we should filter out 0-length capsules somewhere outside of this function
	{
		CastSphereRay(out, m, vRayStart, vRayDelta, flRadius);
		return;
	}
	vec3_t sUnit = s / sLen;
	float dLen = vRayDelta.length();
	if (dLen > 1e-4f)
	{
		vec3_t dUnit = vRayDelta / vec3_t(dLen, dLen, dLen);
		vec3_t z = CrossProduct(sUnit, dUnit);
		float zLenSqr = z.length_sqr();
		float dsUnit = DotProduct(vRayDelta, sUnit);

		CapsuleCast2D_t cast;
		cast.m_flRay = FLT_MAX;

		if (zLenSqr > 256 * 256 * FLT_EPSILON * FLT_EPSILON) // the tolerance here is found experimentally, with the target of achieving minimal orthogonality of 1e-3 between z^s and z^d
		{
			float zLen = sqrtf(zLenSqr);
			vec3_t zUnit = z / zLen;

			float mzUnit = DotProduct(m, zUnit), rr = Sqr(flRadius) - Sqr(mzUnit);
			if (rr <= 0)
			{
				out.m_flHitTime = FLT_MAX;
				return;
			}
			else
			{
				vec3_t yUnit = CrossProduct(zUnit, sUnit);
				vec2_t mProj(DotProduct(m, sUnit), DotProduct(m, yUnit));
				float dyUnit = DotProduct(vRayDelta, yUnit);
				CastCapsuleRay2DInternal(cast, mProj, vec2_t(dsUnit, dyUnit), sLen, rr);
			}
		}
		else
		{
			// they're parallel..
			float msUnit = DotProduct(m, sUnit);
			vec3_t zAlt = m - sUnit * msUnit;
			float zAltLenSqr = zAlt.length_sqr();
			if (zAltLenSqr < FLT_EPSILON * FLT_EPSILON)
			{
				// ray and capsule are coaxial...
				CastCapsuleRay2DCoaxialInternal(cast, msUnit, dsUnit, sLen, flRadius); // note: we're passing radius!
			}
			else
			{
				// ray and capsule are parallel
				vec3_t zUnit = zAlt / sqrtf(zAltLenSqr), yUnit = CrossProduct(zUnit, sUnit);
				CastCapsuleRay2DParallelInternal(cast, vec2_t(DotProduct(m, sUnit), DotProduct(m, yUnit)), dsUnit, sLen, Sqr(flRadius) - zAltLenSqr); // r^2 may be negative here - it'll just return no hit
			}
		}

		out.m_flHitTime = cast.m_flRay;
		out.m_vHitPoint = vRayStart + vRayDelta * cast.m_flRay;
		out.m_vHitNormal = (out.m_vHitPoint - (vCenter[0] + sUnit * cast.m_flCapsule)).normalized();
	}
	else
	{
		CastCapsuleShortRay(out, sUnit, sLen, m, vRayStart, vCenter, flRadius);
	}
}


static int ClipRayToCapsule(const ray_t& ray, mstudiobbox_t* pbox, matrix3x4_t& matrix, c_game_trace& tr)
{
	vec3_t vecCapsuleCenters[2];
	math::vector_transform(pbox->min, matrix, vecCapsuleCenters[0]);
	math::vector_transform(pbox->max, matrix, vecCapsuleCenters[1]);

	CShapeCastResult cast;
	CastCapsuleRay(cast, ray.start, ray.delta * tr.fraction, vecCapsuleCenters, pbox->radius);
	if (cast.DidHit())
	{
		tr.fraction *= cast.m_flHitTime;
		if (cast.m_bStartInSolid)
		{
			tr.start_solid = true;
			// tr.allsolid - not computed yet
		}

		// tr.contents, dispFlags - not computed yet
		tr.end = cast.m_vHitPoint;
		tr.plane.normal = cast.m_vHitNormal;

		//extern IVDebugOverlay *debugoverlay;
		//debugoverlay->AddCapsuleOverlay( vecCapsuleCenters[ 0 ], vecCapsuleCenters[ 1 ], pbox->flCapsuleRadius, 0, 255, 0, 255, 10 );
		//debugoverlay->AddLineOverlay( ray.m_Start /*+offset?*/, cast.m_vHitPoint, 0, 0, 255, 200, 0.25f, 10 );
		//debugoverlay->AddLineOverlay( cast.m_vHitPoint, cast.m_vHitPoint + 4 * cast.m_vHitNormal, 0, 255, 0, 200, 0.25f, 10 );

		// plane.dist and others are not computed yet
		return 0; // hitside is not computed (yet?)
	}
	return -1;
}

struct BoxTraceInfo_t
{
	float t1;
	float t2;
	int	hitside;
	bool startsolid;
};


bool IntersectRayWithBox(const vec3_t& vecRayStart, const vec3_t& vecRayDelta,
	const vec3_t& boxMins, const vec3_t& boxMaxs, float flTolerance, BoxTraceInfo_t* pTrace)
{
	int			i;
	float		d1, d2;
	float		f;

	pTrace->t1 = -1.0f;
	pTrace->t2 = 1.0f;
	pTrace->hitside = -1;

	// UNDONE: This makes this code a little messy
	pTrace->startsolid = true;

	for (i = 0; i < 6; ++i)
	{
		if (i >= 3)
		{
			d1 = vecRayStart[i - 3] - boxMaxs[i - 3];
			d2 = d1 + vecRayDelta[i - 3];
		}
		else
		{
			d1 = -vecRayStart[i] + boxMins[i];
			d2 = d1 - vecRayDelta[i];
		}

		// if completely in front of face, no intersection
		if (d1 > 0 && d2 > 0)
		{
			// UNDONE: Have to revert this in case it's still set
			// UNDONE: Refactor to have only 2 return points (true/false) from this function
			pTrace->startsolid = false;
			return false;
		}

		// completely inside, check next face
		if (d1 <= 0 && d2 <= 0)
			continue;

		if (d1 > 0)
		{
			pTrace->startsolid = false;
		}

		// crosses face
		if (d1 > d2)
		{
			f = d1 - flTolerance;
			if (f < 0)
			{
				f = 0;
			}
			f = f / (d1 - d2);
			if (f > pTrace->t1)
			{
				pTrace->t1 = f;
				pTrace->hitside = i;
			}
		}
		else
		{
			// leave
			f = (d1 + flTolerance) / (d1 - d2);
			if (f < pTrace->t2)
			{
				pTrace->t2 = f;
			}
		}
	}

	return pTrace->startsolid || (pTrace->t1 < pTrace->t2 && pTrace->t1 >= 0.0f);
}

static void Collision_ClearTrace(const vec3_t& vecRayStart, const vec3_t& vecRayDelta, c_game_trace* pTrace)
{
	pTrace->start = vecRayStart;
	pTrace->end = vecRayStart;
	pTrace->end += vecRayDelta;
	pTrace->start_solid = false;
	pTrace->all_solid = false;
	pTrace->fraction = 1.0f;
	pTrace->contents = 0;
}


//-----------------------------------------------------------------------------
// Intersects a ray against a box
//-----------------------------------------------------------------------------
bool IntersectRayWithBox(const vec3_t& vecRayStart, const vec3_t& vecRayDelta,
	const vec3_t& boxMins, const vec3_t& boxMaxs, float flTolerance, c_game_trace* pTrace, float* pFractionLeftSolid = nullptr)
{
	Collision_ClearTrace(vecRayStart, vecRayDelta, pTrace);

	BoxTraceInfo_t trace;

	if (IntersectRayWithBox(vecRayStart, vecRayDelta, boxMins, boxMaxs, flTolerance, &trace))
	{
		pTrace->start_solid = trace.startsolid;
		if (trace.t1 < trace.t2 && trace.t1 >= 0.0f)
		{
			pTrace->fraction = trace.t1;
			math::vector_multiply(pTrace->start, trace.t1, vecRayDelta, pTrace->end);
			pTrace->contents = CONTENTS_SOLID;
			pTrace->plane.normal = vec3_t{0, 0, 0};
			if (trace.hitside >= 3)
			{
				trace.hitside -= 3;
				pTrace->plane.dist = boxMaxs[trace.hitside];
				pTrace->plane.normal[trace.hitside] = 1.0f;
				pTrace->plane.type = trace.hitside;
			}
			else
			{
				pTrace->plane.dist = -boxMins[trace.hitside];
				pTrace->plane.normal[trace.hitside] = -1.0f;
				pTrace->plane.type = trace.hitside;
			}
			return true;
		}

		if (pTrace->start_solid)
		{
			pTrace->all_solid = (trace.t2 <= 0.0f) || (trace.t2 >= 1.0f);
			pTrace->fraction = 0;
			if (pFractionLeftSolid)
			{
				*pFractionLeftSolid = trace.t2;
			}
			pTrace->end = pTrace->start;
			pTrace->contents = CONTENTS_SOLID;
			pTrace->plane.dist = pTrace->start[0];
			pTrace->plane.normal = vec3_t(1.0f, 0.0f, 0.0f);
			pTrace->plane.type = 0;
			pTrace->start = vecRayStart + (vec3_t(trace.t2, trace.t2, trace.t2) * vecRayDelta);
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------
// Intersects a ray against a box
//-----------------------------------------------------------------------------
bool IntersectRayWithBox(const ray_t& ray, const vec3_t& boxMins, const vec3_t& boxMaxs,
	float flTolerance, c_game_trace* pTrace, float* pFractionLeftSolid = nullptr)
{
	if (!ray.is_ray)
	{
		vec3_t vecExpandedMins = boxMins - ray.extents;
		vec3_t vecExpandedMaxs = boxMaxs + ray.extents;
		bool bIntersects = IntersectRayWithBox(ray.start, ray.delta, vecExpandedMins, vecExpandedMaxs, flTolerance, pTrace, pFractionLeftSolid);
		pTrace->start += ray.start_offset;
		pTrace->end += ray.start_offset;
		return bIntersects;
	}
	return IntersectRayWithBox(ray.start, ray.delta, boxMins, boxMaxs, flTolerance, pTrace, pFractionLeftSolid);
}

static int ClipRayToHitbox(const ray_t& ray, mstudiobbox_t* pbox, matrix3x4_t& matrix, c_game_trace& tr)
{
	const float flProjEpsilon = 0.01f;
	if (pbox->radius > 0)
	{
		return ClipRayToCapsule(ray, pbox, matrix, tr);
	}

	// scale by current t so hits shorten the ray and increase the likelihood of early outs
	vec3_t delta2;
	VectorScale(ray.delta, (0.5f * tr.fraction), delta2);

	// OPTIMIZE: Store this in the box instead of computing it here
	// compute center in local space
	vec3_t boxextents;
	boxextents.x = (pbox->min.x + pbox->max.x) * 0.5;
	boxextents.y = (pbox->min.y + pbox->max.y) * 0.5;
	boxextents.z = (pbox->min.z + pbox->max.z) * 0.5;

	// transform to world space
	vec3_t boxCenter;
	math::vector_transform(boxextents, matrix, boxCenter);

	// calc extents from local center
	boxextents.x = pbox->max.x - boxextents.x;
	boxextents.y = pbox->max.y - boxextents.y;
	boxextents.z = pbox->max.z - boxextents.z;

	// OPTIMIZE: This is optimized for world space.  If the transform is fast enough, it may make more
	// sense to just xform and call UTIL_ClipToBox() instead.  MEASURE THIS.

	// save the extents of the ray along 
	vec3_t extent, uextent;
	vec3_t segmentCenter;
	segmentCenter.x = ray.start.x + delta2.x - boxCenter.x;
	segmentCenter.y = ray.start.y + delta2.y - boxCenter.y;
	segmentCenter.z = ray.start.z + delta2.z - boxCenter.z;

	extent.reset();

	// check box axes for separation
	for (int j = 0; j < 3; j++)
	{
		extent[j] = delta2.x * matrix.mat[0][j] + delta2.y * matrix.mat[1][j] + delta2.z * matrix.mat[2][j];
		uextent[j] = fabsf(extent[j]);
		float coord = segmentCenter.x * matrix.mat[0][j] + segmentCenter.y * matrix.mat[1][j] + segmentCenter.z * matrix.mat[2][j];
		coord = fabsf(coord);

		if (coord > (boxextents[j] + uextent[j]))
			return -1;
	}

	// now check cross axes for separation
	float tmp, cextent;
	vec3_t cross;
	CrossProduct(delta2, segmentCenter, cross);
	cextent = cross.x * matrix.mat[0][0] + cross.y * matrix.mat[1][0] + cross.z * matrix.mat[2][0];
	cextent = fabsf(cextent);
	tmp = boxextents[1] * uextent[2] + boxextents[2] * uextent[1];
	tmp = std::max(tmp, flProjEpsilon);
	if (cextent > tmp)
		return -1;

	cextent = cross.x * matrix.mat[0][1] + cross.y * matrix.mat[1][1] + cross.z * matrix.mat[2][1];
	cextent = fabsf(cextent);
	tmp = boxextents[0] * uextent[2] + boxextents[2] * uextent[0];
	tmp = std::max(tmp, flProjEpsilon);
	if (cextent > tmp)
		return -1;

	cextent = cross.x * matrix.mat[0][2] + cross.y * matrix.mat[1][2] + cross.z * matrix.mat[2][2];
	cextent = fabsf(cextent);
	tmp = boxextents[0] * uextent[1] + boxextents[1] * uextent[0];
	tmp = std::max(tmp, flProjEpsilon);
	if (cextent > tmp)
		return -1;

	vec3_t start;

	// Compute ray start in bone space
	VectorITransform(ray.start, matrix, start);
	// extent is delta2 in bone space, recompute delta in bone space
	VectorScale(extent, 2, extent);

	// delta was prescaled by the current t, so no need to see if this intersection
	// is closer
	c_game_trace boxTrace;
	if (!IntersectRayWithBox(start, extent, pbox->min, pbox->max, 0.0f, &boxTrace))
		return -1;

	tr.fraction *= boxTrace.fraction;
	tr.start_solid = boxTrace.start_solid;
	int hitside = boxTrace.plane.type;
	if (boxTrace.plane.normal[hitside] >= 0)
	{
		hitside += 3;
	}
	return hitside;
}