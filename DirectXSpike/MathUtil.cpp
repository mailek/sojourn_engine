/********************************************************************
	created:	2012/04/23
	filename: 	MathUtil.cpp
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
#include "stdafx.h"
#include "mathutil.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
//////////////////////////////////////////////////////////////////////////
static void FarthestPointsABB(const Point_3D pts[], const int numPts, int *minOut, int *maxOut);
static Sphere_PosRad CalcSphereFromPointsByABBDistance(const Point_3D pts[], const int numPts);
static void RandomizePointArray( Point_3D pts[], const int numPts );

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

/* Calculate the largest sphere that will fit inside the given ABB */
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

/* Calculate the two most distant points on the boundary of the ABB containing the given point cloud */
static void FarthestPointsABB( const Point_3D pts[], const int numPts, int *minOut, int *maxOut )
{
	int xMin(0), xMax(0), yMin(0), yMax(0), zMin(0), zMax(0);

	/* find the extreme points along each axes */
	for(int i=1; i < numPts; i++)
	{
		if(pts[i].x < pts[xMin].x) xMin = i;
		if(pts[i].x > pts[xMax].x) xMax = i;
		if(pts[i].y < pts[yMin].y) yMin = i;
		if(pts[i].y > pts[yMax].y) yMax = i;
		if(pts[i].z < pts[zMin].z) zMin = i;
		if(pts[i].z > pts[zMax].z) zMax = i;
	}

	/* calculate the squared distance between each pair of 
	   extreme points along a particular axis */
	Point_3D temp;
	temp = pts[xMax] - pts[xMin];
	float xDistSq = Vec3_Dot(&temp, &temp);

	temp = pts[yMax] - pts[yMin];
	float yDistSq = Vec3_Dot(&temp, &temp);

	temp = pts[zMax] - pts[zMin];
	float zDistSq = Vec3_Dot(&temp, &temp);

	/* return pair of points farthest apart */
	*minOut = xMin;
	*maxOut = xMax;
	if(yDistSq > xDistSq && yDistSq > zDistSq)
	{
		*minOut = yMin;
		*maxOut = yMax;
	}
	if(zDistSq > xDistSq && zDistSq > yDistSq)
	{
		*minOut = zMin;
		*maxOut = zMax;
	}
}

//////////////////////////////////////////////////////////////////////////
// Sphere Routines
//////////////////////////////////////////////////////////////////////////

/* Calculate bounding sphere of a given point cloud using the Ritter method */
Sphere_PosRad Sphere_CalcBoundingSphereForPoints( const Point_3D pts[], const int numPts )
{
	Sphere_PosRad ret;

	/* Find the approximate bounding sphere using ABB, to obtain estimated center point */
	ret = CalcSphereFromPointsByABBDistance(pts, numPts);

	/* Grow sphere radius to include every point in the array */
	for(int i=0; i < numPts; i++)
		Sphere_GrowSphereToContainPoint(&ret, pts[i]);

	return ret;
}

/* Iteratively calculate a better bounding sphere for a given point cloud using 
   multiple Ritter iterations 
   Note: input points will be randomized upon output */
Sphere_PosRad Sphere_CalcBoundingSphereForPointsIterative( Point_3D pts[], const int numPts )
{
	const int NUM_OF_ITERATIONS = 10;
	const float SHRINK_MOD = 0.9f;

	/* Calculate the initial guess */
	Sphere_PosRad ret = Sphere_CalcBoundingSphereForPoints( pts, numPts );
	Sphere_PosRad attempt = ret;

	/* Recursively shrink the sphere then grow to contain points */
	for(int k = 0; k < NUM_OF_ITERATIONS; k++)
	{
		attempt.radius = attempt.radius * SHRINK_MOD;
		RandomizePointArray( pts, numPts );
		for(int i = 0; i < numPts; i++)
			Sphere_GrowSphereToContainPoint(&attempt, pts[i]);

		if(attempt.radius < ret.radius)
			ret = attempt;
	}

	return ret;
}

/* Grow the given Sphere to include the given Point */
void Sphere_GrowSphereToContainPoint( Sphere_PosRad *inOut, const Point_3D p )
{
	// Find squared distance between sphere center and point
	Vector_3 d = p - inOut->pos;
	float distSq = Vec3_Dot(&d, &d);

	// Only grow sphere if point is outside bounds
	if(distSq > (inOut->radius * inOut->radius))
	{
		float dist = sqrt(distSq);
		float newRadius = (dist + inOut->radius) * 0.5f;
		float k = (newRadius - inOut->radius) / dist;
		
		inOut->radius = newRadius;
		inOut->pos += (d * k);
	}
}

/* Grow the given Sphere to include a different sphere */
void Sphere_GrowSphereToContainSphere( Sphere_PosRad *inOut, const Sphere_PosRad s )
{
	assert(inOut->radius >= 0 && s.radius >= 0);

	Vector_3 ab;
	Sphere_PosRad a = *inOut;
	Sphere_PosRad b = s;

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

	inOut->radius = (outer - inner)/2;
	float center = inner + inOut->radius;

	Vec3_Scale(&inOut->pos, &ab_norm, center);
	Vec3_Add(&inOut->pos, &inOut->pos, &a.pos);
}

/* Calculate an approximate bounding sphere for the given point cloud, using the ABB axis method */
static Sphere_PosRad CalcSphereFromPointsByABBDistance(const Point_3D pts[], const int numPts)
{
	Sphere_PosRad s;

	/* Find the most distant two points in the point cloud */
	int max, min;
	FarthestPointsABB( pts, numPts, &min, &max);

	/* Calculate the smallest sphere that encompasses this pair of points */
	Point_3D temp;
	temp = pts[max] + pts[min];
	Vec3_Scale(&s.pos, &temp, 0.5f);
	
	temp = pts[max] - s.pos;
	s.radius = Vec3_Length(&temp);
	
	return s;
}

//////////////////////////////////////////////////////////////////////////
// Point Routines
//////////////////////////////////////////////////////////////////////////

/* Randomize the order of a set of points */
static void RandomizePointArray( Point_3D pts[], const int numPts )
{
	Point_3D swap;
	int j;
	for(int i = 0; i < numPts; i++)
	{
		j = rand() % numPts;
		swap = pts[i];
		pts[i] = pts[j];
		pts[j] = swap;
	}
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
	const ABB_MaxMin &abb,
	WORD axes
	)
{
	bool ret = true;

	if( axes & TEST_1D_X )
	{
		ret &= (abb.max.x >= p.x && abb.min.x < p.x);
	}

	if( axes & TEST_1D_Y )
	{
		ret &= (abb.max.y >= p.y && abb.min.y < p.y);
	}

	if( axes & TEST_1D_Z )
	{
		ret &= (abb.max.z >= p.z && abb.min.z < p.z);
	}
	/*return abb.max.x >= p.x && abb.max.y >= p.y && abb.max.z >= p.z &&
		abb.min.x < p.x && abb.min.y < p.y && abb.min.z < p.z;*/

	return ret;
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
EIntersectType Collide_ABBToFrustum(
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
EIntersectType Collide_SphereToFrustum(
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
EIntersectType Collide_SphereToPlane(const Sphere_PosRad &s, const Plane_Vec3PtNorm p, /*out*/Point_3D* sphereIntersect)
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
// Complex Transform Class
//////////////////////////////////////////////////////////////////////////

Matrix4x4 ComplexTransform::GetTransform()
{
#define FIRST	0
#define SECOND	1
#define THIRD	2
#define X_ITEM	0
#define Y_ITEM	1
#define Z_ITEM	2

	if(flags & XFM_OUT_OF_DATE)
	{
		D3DXMATRIX s, workingMatrix;
		D3DXMatrixIdentity(&workingMatrix);

		/* scaling */
		D3DXMatrixScaling(&s, scale.x, scale.y, scale.z);
		workingMatrix *= s;

		/* rotation */
		D3DXMATRIX rXYZ[3];
		D3DXMatrixRotationX(&rXYZ[X_ITEM], rotationEuler.x);
		D3DXMatrixRotationY(&rXYZ[Y_ITEM], rotationEuler.y);
		D3DXMatrixRotationZ(&rXYZ[Z_ITEM], rotationEuler.z);

		D3DXMATRIX* ordered[3];
		int orderFlag = (flags & ROTATION_ORDER_MASK);
		switch(orderFlag)
		{
		case ROTATE_XYZ:
			ordered[FIRST]	= &rXYZ[X_ITEM];
			ordered[SECOND] = &rXYZ[Y_ITEM];
			ordered[THIRD]	= &rXYZ[Z_ITEM];
			break;
		case ROTATE_XZY:
			ordered[FIRST]	= &rXYZ[X_ITEM];
			ordered[SECOND] = &rXYZ[Z_ITEM];
			ordered[THIRD]	= &rXYZ[Y_ITEM];
			break;
		case ROTATE_YXZ:
			ordered[FIRST]	= &rXYZ[Y_ITEM];
			ordered[SECOND] = &rXYZ[X_ITEM];
			ordered[THIRD]	= &rXYZ[Z_ITEM];
			break;
		case ROTATE_YZX:
			ordered[FIRST]	= &rXYZ[Y_ITEM];
			ordered[SECOND] = &rXYZ[Z_ITEM];
			ordered[THIRD]	= &rXYZ[X_ITEM];
			break;
		case ROTATE_ZXY:
			ordered[FIRST]	= &rXYZ[Z_ITEM];
			ordered[SECOND] = &rXYZ[X_ITEM];
			ordered[THIRD]	= &rXYZ[Y_ITEM];
			break;
		case ROTATE_ZYX:
			ordered[FIRST]	= &rXYZ[Z_ITEM];
			ordered[SECOND] = &rXYZ[Y_ITEM];
			ordered[THIRD]	= &rXYZ[X_ITEM];
			break;
		default:
			assert(false);
			break;
		}

		for(int i=0; i < cnt_of_array(rXYZ); i++)
			workingMatrix *= (*ordered[i]);

		/* translation */
		D3DXMATRIX t;
		D3DXMatrixTranslation(&t, translation.x, translation.y, translation.z);
		workingMatrix *= t;

		/* save the matrix for lazy calc */
		matrix = workingMatrix;
		flags &= ~XFM_OUT_OF_DATE;
	}

#undef FIRST	
#undef SECOND	
#undef THIRD	
#undef X_ITEM	
#undef Y_ITEM	
#undef Z_ITEM	

	return matrix;
}

Matrix4x4 Matrix4x4_LookAtQuaternion( Matrix4x4 *mat_out, Vector_3 *eye, Vector_3 *target )
{
	Matrix4x4 ret;
	Vec3_Normalize(eye, eye);
	Vec3_Normalize(target, target);

	Vector_3 norm;
	Vec3_Cross(&norm, eye, target);
	float rot = acos(Vec3_Dot(eye, target));

	Matrix4x4_RotateAxis(&ret, &norm, rot);

	return ret;
}