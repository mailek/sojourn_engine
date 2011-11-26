#include "stdafx.h"
#include "mathutil.h"

//////////////////////////////////////////////////////////////////////////
// Float Math Functions
//////////////////////////////////////////////////////////////////////////
float frand(void)
{
	return (float)rand()/(float)RAND_MAX;
}

float lerp(float x, float x1, float s)
{
	return x + s*(x1-x);
}

float fmax(float a, float b )
{
	return( a > b ? a : b );
}

float fmin(float a, float b )
{
	return( a < b ? a : b );
}

float fclamp(float x, float min, float max)
{
	return x < min ? min : (x > max ? max : x);
}

bool fcompare(float a, float b)
{
	return (::fabs(a-b) < FLT_EPSILON);
}

//////////////////////////////////////////////////////////////////////////
// ABB Routines
//////////////////////////////////////////////////////////////////////////

/* Divide an input ABB by the requested plane divisions 
   Note: Using this to divide an ABB ensures that the space is covered and
     is free from floating point error. 
   Return: number of ABBs in (out) */

inline int ABB_Divide( const ABB_MaxMin target, int xDivs, int yDivs, int zDivs, ABB_MaxMin* out )
{
#define MAX_DIVS (5)
#define X_PTS    (0)
#define Y_PTS    (1)
#define Z_PTS    (2)

	// note: probably not the most efficient implementation, but it should be easy to debug
	const int numXPts = 2 + xDivs;
	const int numYPts = 2 + yDivs;
	const int numZPts = 2 + zDivs;
	const int numOutAbbs = (xDivs+1) * (yDivs+1) * (zDivs+1);

	float XYZPts[3][MAX_DIVS+2];

#if defined(_DEBUG)
	::ZeroMemory(&XYZPts, sizeof(XYZPts));
	assert((xDivs < MAX_DIVS) && (yDivs < MAX_DIVS) && (zDivs < MAX_DIVS));
#endif

	/* Build the array of unique points (for each axis) */
	XYZPts[X_PTS][0] = target.min.x;
	XYZPts[Y_PTS][0] = target.min.y;
	XYZPts[Z_PTS][0] = target.min.z;

	float xDelta = (target.max.x - target.min.x) / (xDivs+1);
	float yDelta = (target.max.y - target.min.y) / (yDivs+1);
	float zDelta = (target.max.z - target.min.z) / (zDivs+1);

	for(int i = 0; i < xDivs; i++) XYZPts[X_PTS][(i+1)] = XYZPts[X_PTS][i] + xDelta;		
	for(int i = 0; i < yDivs; i++) XYZPts[Y_PTS][(i+1)] = XYZPts[Y_PTS][i] + yDelta;
	for(int i = 0; i < zDivs; i++) XYZPts[Z_PTS][(i+1)] = XYZPts[Z_PTS][i] + zDelta;

	XYZPts[X_PTS][numXPts-1] = target.max.x;
	XYZPts[Y_PTS][numYPts-1] = target.max.y;
	XYZPts[Z_PTS][numZPts-1] = target.max.z;

	int cnt = 0;
	for(int x = 0; x < numXPts-1; x++)
	{
		for(int y = 0; y < numYPts-1; y++)
		{
			for(int z = 0; z < numZPts-1; z++)
			{
				out->min.x = XYZPts[X_PTS][x];
				out->max.x = XYZPts[X_PTS][x+1];

				out->min.y = XYZPts[Y_PTS][y];
				out->max.y = XYZPts[Y_PTS][y+1];

				out->min.z = XYZPts[Z_PTS][z];
				out->max.z = XYZPts[Z_PTS][z+1];

				out++;
				cnt++;
			}
		}
	}

	assert(cnt == numOutAbbs);

	return numOutAbbs;
}

/* Scale an ABB about its center */
inline ABB_MaxMin ABB_Scale( const ABB_MaxMin &abb, float xScale, float yScale, float zScale )
{
	D3DXVECTOR3 v1 = abb.max - abb.min;
	D3DXVECTOR3 v2 = D3DXVECTOR3(v1.x*xScale, v1.y*yScale, v1.z*zScale);
	v1 -= v2;
	v1 /= 2.0f;

	ABB_MaxMin ret = abb;
	ret.max -= v1;
	ret.min += v1;

	return ret;	
}

/* Find the depth of the given ABB */
inline float ABB_CalcDepth( const ABB_MaxMin &abb )
{
	return fabs(abb.max.z - abb.min.z);
}

/* Find the width of the given ABB */
inline float ABB_CalcWidth( const ABB_MaxMin &abb )
{
	return fabs(abb.max.x - abb.min.x);
}

/*	Find the smallest ABB (from starting ABB) that includes the given point		*/
void ABB_GrowToInclude3DPoint(const Point_3D &p, ABB_MaxMin &abb)
{
	abb.min.x = fmin(abb.min.x, p.x);
	abb.min.y = fmin(abb.min.y, p.y);
	abb.min.z = fmin(abb.min.z, p.z);
	abb.max.x = fmax(abb.max.x, p.x);
	abb.max.y = fmax(abb.max.y, p.y);
	abb.max.z = fmax(abb.max.z, p.z);
}

/*  Calculates a world transform matrix that transforms an origin-centered 
	unit cube to a max,min ABB													*/
inline Matrix4x4 ABB_CalcUnitTransform( const ABB_MaxMin abb )
{
	// assume render object is origin centered cube x(0.5,-0.5) y(0.5,-0.5) z(0.5,-0.5)
	Matrix4x4 scale;
	float width = fabs(abb.max.x-abb.min.x);
	float height = fabs(abb.max.y-abb.min.y);
	float depth = fabs(abb.max.z-abb.min.z);
	Matrix4x4_Scale(&scale, width, height, depth);
	
	Matrix4x4 translate;
	Matrix4x4_Translate( &translate, abb.min.x+width/2.0f, abb.min.y+height/2.0f, abb.min.z+depth/2.0f );
	
	return ( scale * translate );
}

/* Calculate a sphere that contains the given ABB */
inline Sphere_PosRad ABB_CalcMinSphereContaining( const ABB_MaxMin abb )
{
	Sphere_PosRad s;

	// center point
	Vector_3 v;
	Vec3_Sub(&v, &abb.max, &abb.min);
	Vec3_Scale( &v, &v, 0.5f );
	Vec3_Add( &s.pos, &v, &abb.min ); 

	// radius
	s.radius = Vec3_Length( &v );
	
	return s;
}

inline Sphere_PosRad ABB_CalcMaxSphereContainedBy( const ABB_MaxMin abb )
{
	Sphere_PosRad s;

	// center point
	Vector_3 v;
	Vec3_Sub(&v, &abb.max, &abb.min);
	Vec3_Scale( &v, &v, 0.5f );
	Vec3_Add( &s.pos, &v, &abb.min ); 

	// radius
	s.radius = fmin( v.z, fmin( v.y, v.x ) );

	return s;
}

//////////////////////////////////////////////////////////////////////////
// Collision Geometry Tests
//////////////////////////////////////////////////////////////////////////

/* Collide Sphere to Sphere */
bool Collide_SphereToSphere(
	const Sphere_PosRad &s1,
	const Sphere_PosRad &s2
	)
{
	Vector_4 s1_to_s2 = Vector_4(s1.pos,0)-Vector_4(s2.pos,0);
	return Vec4_LengthSq( &s1_to_s2 ) < pow(s1.radius + s2.radius, 2);
}

/* Collide 3D Point to ABB */
bool Collide_PointToBox(
	const Point_3D &p, 
	const ABB_MaxMin &abb
	)
{
	return abb.max.x >= p.x && abb.max.y >= p.y && abb.max.z >= p.z &&
		abb.min.x < p.x && abb.min.y < p.y && abb.min.z < p.z;
}

/* Collide Sphere to ABB */
bool Collide_SphereToBox(
	const Sphere_PosRad &s, 
	const ABB_MaxMin &abb
	)
{
	return abb.max.x >= s.pos.x - s.radius && abb.max.y >= s.pos.y - s.radius && abb.max.z >= s.pos.z - s.radius &&
		abb.min.x <= s.pos.x + s.radius && abb.min.y <= s.pos.y + s.radius && abb.min.z <= s.pos.z + s.radius;
}

/* Collide Sphere to Capsule */
bool Collide_SphereToCapsule(
	 const Sphere_PosRad &s,
	 const Capsule &c
	 )
{
	/* S collides with C 
	if the distance from S's origin to C's line 
	is less than the sum of their radii */
	float dist = DistSq_Point3DFromLineSeg( s.pos, c.s.pos, Vector_3(c.s.pos + c.v) );
	return ( dist <= pow((s.radius + c.s.radius), 2) );
}

/* Collide 3D Point to Frustum */
bool Collide_PointToFrustum(
	Point_3D &testPt, 
	const Frustum_Camera &f, 
	bool ignoreY /*= false*/
	)
{
	// find the vector from the camera to the test point
	Vector_3 v = testPt - f.cameraPos;
	
	// find the test points distance along the camera Z axis 
	Vector_3 tempZ;
	if(ignoreY)
	{
		tempZ = Vector_3(f.cameraZ.x, 0, f.cameraZ.z);
		Vec3_Normalize(&tempZ, &tempZ);
	}
	else
		tempZ = f.cameraZ;
		
	float dZ = Vec3_Dot(&v, &tempZ);
	
	// calculate the width and height of the view frustum at the calculated Z distance
	// h = pc.z*2*tan(vertFovAngle/2)    w = h * widthRatio
	static float fovTang = tanf(0.5f * f.FOVANGLE); // this optmization will limit the ability to change FoV angle during runtime
	float height = dZ*fovTang;
	float width = height * f.ASPECT;

	// test if x is outside the width at the calcuated Z distance
	float dX = Vec3_Dot(&v, &f.cameraX);
	if(dX > width || dX < -width)
		return false;

	// if testing in 3D (octtree) then test Y height
	if(!ignoreY)
	{
		float dY = Vec3_Dot(&v, &f.cameraY);
		if(dY > height || dY < -height)
			return false;
	}

	return true;
}

/* Collide ABB to Frustum */
IntersectType Collide_ABBToFrustum(
	const ABB_MaxMin testABB, 
	const Frustum_Camera &f, 
	bool bTest2D_XZ /*= false*/
	)
{
		Point_3D testPt;
		int inCnt(0), outCnt(0);
		int nCheckVerts(8);
		if(bTest2D_XZ)
			nCheckVerts = 4;

		// for frustum clip check all AAB points against frustum volume in clip space
		for(int k = 0; k < nCheckVerts; k++)
		{
			testPt = testABB.min;
			if(k & 0x01)
				testPt.x = testABB.max.x;
			if(k & 0x02)
				testPt.z = testABB.max.z;
			if(k & 0x04)
				testPt.y = testABB.max.y;
		
			if(Collide_PointToFrustum(testPt, f, bTest2D_XZ))
				inCnt++;
			else
				outCnt++;

			if(inCnt > 0 && outCnt > 0)
				break;
		}	
		
		if(inCnt > 0 && outCnt > 0)
			return COLLIDE_INTERSECT;
		else if(outCnt > 0)
			return COLLIDE_OUTSIDE;
		else // totally inside
			 
			return COLLIDE_CONTAINED;
}

/* Collide Sphere to Frustum */
IntersectType Collide_SphereToFrustum(
	const Sphere_PosRad &s, 
	const Frustum_Camera &f,
	bool bTest2D_XZ
	)
{
	// find the vector from the camera to the test point
	Vector_3 v = s.pos - f.cameraPos;
	
	// find the test points distance along the camera Z axis 
	Vector_3 tempZ;
	if(bTest2D_XZ)
	{
		tempZ = Vector_3(f.cameraZ.x, 0, f.cameraZ.z);
		Vec3_Normalize(&tempZ, &tempZ);
	}
	else
		tempZ = f.cameraZ;
		
	float dZ = Vec3_Dot(&v, &tempZ);

	// if the z is not inside the near and far planes then reject
	if(dZ > f.FARDIST+s.radius || dZ < f.NEARDIST-s.radius)
		return COLLIDE_OUTSIDE;
	
	// calculate the width and height of the view frustum at the calculated Z distance
	// h = pc.z*2*tan(vertFovAngle/2)    w = h * widthRatio
	static float fovTang = tanf(0.5f*f.FOVANGLE); // this optmization will limit the ability to change FoV angle during runtime
	static float sphereFactorY = 1.0f/cos(0.5f*f.FOVANGLE);
	static float sphereFactorX = 1.0f/cos(0.5f*f.FOVANGLE*f.ASPECT);

	float height = dZ*fovTang;
	float width = height * f.ASPECT;

	// test if x is outside the width at the calcuated Z distance
	float dX = Vec3_Dot(&v, &f.cameraX);
	float modRadius = sphereFactorX * s.radius;
	if(dX > width+modRadius || dX < -width-modRadius)
		return COLLIDE_OUTSIDE;

	// if testing in 3D (octtree) then test Y height
	if(!bTest2D_XZ)
	{
		float dY = Vec3_Dot(&v, &f.cameraY);
		modRadius = sphereFactorY * s.radius;
		if(dY > height+modRadius || dY < -height-modRadius)
			return COLLIDE_OUTSIDE;
	}

	return COLLIDE_INTERSECT;
}

/* Collide Ray with Plane */
bool Collide_RayToPlane(const Ray_Vec3Pt &r, const Plane_Vec3PtNorm p)
{
	assert(false);
	return false;
}

/* Collide Sphere with Plane */
IntersectType Collide_SphereToPlane(const Sphere_PosRad &s, const Plane_Vec3PtNorm p, /*out*/Point_3D* sphereIntersect)
{
	*sphereIntersect = Point_3D(0,0,0);
	assert(false);

	

	return COLLIDE_OUTSIDE;
}

//////////////////////////////////////////////////////////////////////////
// Points and Lines
//////////////////////////////////////////////////////////////////////////
/* Squared Length of pq From Closest Point q on Line Segment ab */
float DistSq_Point3DFromLineSeg(Point_3D p, Point_3D a, Point_3D b)
{
	Vector_3 ab = b - a, ap = p - a;

	/* line seg = A + t*(B-A), where 0 <= t <= 1 */
	
	/* check if outside of A */
	float t = Vec3_Dot(&ap, &ab);
	if(t <= 0.0f) 
		return Vec3_Dot(&ap, &ap);

	/* check if outside of B */
	Vector_3 bp = p - b;
	float abDistSq = Vec3_Dot(&ab, &ab);
	if(t > abDistSq)
		return Vec3_Dot(&bp, &bp);
	
	/* find sq dist of point between A & B to P */
	return Vec3_Dot(&ap, &ap) - ( (t * t) / abDistSq);
}

//////////////////////////////////////////////////////////////////////////
// Polygons
//////////////////////////////////////////////////////////////////////////
/* Find the normal unit vector of the given polygon */
Vector_3 Polygon_CalcNormalVec( const Polygon_ABC &p )
{
	Vector_3 v1 = p.a - p.b;
	Vector_3 v2 = p.a - p.c;
	Vector_3 n;

	Vec3_Cross( &n, &v1, &v2 );

	return n;
}

//////////////////////////////////////////////////////////////////////////
// Spheres
//////////////////////////////////////////////////////////////////////////

/* Find a new sphere that contains spheres A and B */
Sphere_PosRad Sphere_GrowSphereToContainSphere( Sphere_PosRad a, Sphere_PosRad b )
{
	assert(a.radius >= 0 && b.radius >= 0);

	Sphere_PosRad ret;
	Vector_3 ab;
	Vec3_Sub(&ab, &b.pos, &a.pos);

	Vector_3 ab_norm;
	Vec3_Normalize(&ab_norm, &ab);

	// Do the calculation in AB space
	float ab_len = Vec3_Dot(&ab_norm, &ab);
	
	Vector_3 a_outer;
	Vec3_Scale(&a_outer, &ab_norm, a.radius);
	float outer = fmax(ab_len + b.radius, Vec3_Dot(&ab_norm, &a_outer));

	Vector_3 a_inner;
	Vec3_Scale(&a_inner, &a_outer, -1);
	float inner = fmin(ab_len - b.radius, Vec3_Dot(&ab_norm, &a_inner));

	ret.radius = (outer - inner)/2;
	float center = inner + ret.radius;

	Vec3_Scale(&ret.pos, &ab_norm, center);
	Vec3_Add(&ret.pos, &ret.pos, &a.pos);
	
	return ret;
}