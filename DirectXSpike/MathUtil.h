#pragma once
//////////////////////////////////////////////////////////////////////////
// MathUtil.h - 2011 Matthew Alford
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////
// Includes
//////////////////////////////////////

//////////////////////////////////////
// Type Definitions
//////////////////////////////////////

/* DirectX Bindings */
typedef D3DXVECTOR3 Point_3D;
typedef D3DXVECTOR2 Point_2D;
typedef D3DXVECTOR3 Vector_3;
typedef D3DXVECTOR4 Vector_4;
typedef D3DXMATRIX	Matrix4x4;

#define Vec4_LengthSq( vec4d ) \
	D3DXVec4LengthSq( vec4d )

#define Vec3_Length( vec3d ) \
	D3DXVec3Length( vec3d )

#define Vec3_Sub( outvec3d, vec1, vec2 ) \
	D3DXVec3Subtract( outvec3d, vec1, vec2 )

#define Vec3_Add( outvec3d, vec1, vec2 ) \
	D3DXVec3Add( outvec3d, vec1, vec2 )

#define Vec3_Scale( outvec3d, vec1, scale ) \
	D3DXVec3Scale( outvec3d, vec1, scale )

#define Vec3_Normalize( outvec3d, vec3d ) \
	D3DXVec3Normalize( outvec3d, vec3d )

#define Vec3_Dot( vec3d_a, vec3d_b ) \
	D3DXVec3Dot( vec3d_a, vec3d_b )

#define Vec3_Cross( vec3d_o, vec3d_a, vec3d_b ) \
	D3DXVec3Cross( vec3d_o, vec3d_a, vec3d_b )

#define Matrix4x4_Scale( mat_out, x_scale, y_scale, z_scale ) \
	D3DXMatrixScaling( mat_out, x_scale, y_scale, z_scale )

#define Matrix4x4_Translate( mat_out, x_offset, y_offset, z_offset ) \
	D3DXMatrixTranslation( mat_out, x_offset, y_offset, z_offset )

/* Geometry Types */
typedef struct {
	Vector_3		a;
	Vector_3		b;
	Vector_3		c;
} Polygon_ABC;

typedef struct {
	Vector_3		max;
	Vector_3		min;
} ABB_MaxMin;

typedef struct {
	Vector_3		pos;
	float			radius;
} Sphere_PosRad;

typedef struct {
	Sphere_PosRad	s;
	Vector_3		v;
} Capsule;

typedef struct {
	Point_3D		cameraPos;		/* world position of the camera										*/
	Vector_3		cameraX;		/* normalized X axis world orientation 								*/
	Vector_3		cameraY;		/* normalized Y axis world orientation 								*/
	Vector_3		cameraZ;		/* normalized Z axis world orientation								*/
	float			FOVANGLE;		/* field of view vertical angle (in radians)						*/
	float			ASPECT;			/* aspect ratio														*/
	float			NEARDIST;		/* distance from camera position to near clip plane (usually 1.0f)	*/
	float			FARDIST;		/* distance from camera position to far clip plane					*/
} Frustum_Camera;

//////////////////////////////////////
// Constants
//////////////////////////////////////

/* Math */
const float PI			= 3.14159265358979f;
const float HALF_PI		= PI / 2.0f;

/* Collision */
typedef enum {
	COLLIDE_INVALID =0,
	COLLIDE_OUTSIDE,
	COLLIDE_INTERSECT,
	COLLIDE_CONTAINED
} IntersectType;

//////////////////////////////////////
// Function Declarations
//////////////////////////////////////

/* Float Math */
extern inline float lerp(float x, float x1, float s);
extern inline float fmax(float a, float b );
extern inline float fmin(float a, float b );
extern inline float frand(void);
extern inline float fclamp(float x, float min, float max);
extern inline bool fcompare(float a, float b);

/* ABB Routines */
extern inline ABB_MaxMin ABB_Scale( const ABB_MaxMin &abb, float xScale, float yScale, float zScale );
extern inline float ABB_CalcDepth( const ABB_MaxMin &abb );
extern inline float ABB_CalcWidth( const ABB_MaxMin &abb );
void ABB_GrowToInclude3DPoint(const Point_3D &p, ABB_MaxMin &abb);
extern inline D3DXMATRIX ABB_CalcUnitTransform( const ABB_MaxMin abb );
extern inline Sphere_PosRad ABB_CalcMinSphereContaining( const ABB_MaxMin abb );
extern inline Sphere_PosRad ABB_CalcMaxSphereContainedBy( const ABB_MaxMin abb );

/* Collision */
bool Collide_PointToBox(const Point_3D &p, const ABB_MaxMin &abb);
bool Collide_PointToFrustum(Point_3D &testPt, const Frustum_Camera &f, bool ignoreY = false);
IntersectType Collide_ABBToFrustum(const ABB_MaxMin testABB, const Frustum_Camera &f, bool bTest2DXZ /*= false*/);
IntersectType Collide_SphereToFrustum(const Sphere_PosRad &s, const Frustum_Camera &f, bool bTest2DXZ);
bool Collide_SphereToBox(const Sphere_PosRad &s, const ABB_MaxMin &abb);
bool Collide_SphereToSphere(const Sphere_PosRad &s1, const Sphere_PosRad &s2);
bool Collide_SphereToCapsule(const Sphere_PosRad &s, const Capsule &c);

/* Points and Lines */
float DistSq_Point3DFromLineSeg(Point_3D p, Point_3D a, Point_3D b);

/* Polygons */
Vector_3 Polygon_NormalVec( const Polygon_ABC &p );