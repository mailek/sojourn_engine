#pragma once
/********************************************************************
	created:	2012/04/23
	filename: 	MathUtil.h
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/

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
typedef D3DXQUATERNION Quaternion;

#define Vec4_LengthSq( vec4d ) \
	D3DXVec4LengthSq( vec4d )

#define Vec3_Length( vec3d ) \
	D3DXVec3Length( vec3d )

#define Vec3_LengthSq( vec3d ) \
	D3DXVec3LengthSq( vec3d )

#define Vec3_Sub( /* Vector_3* */outvec3d, /* Vector_3* */vec1, /* Vector_3* */vec2 ) \
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

#define Vec3_TransformCoord( vec3d_o, vec3d, matrix ) \
	D3DXVec3TransformCoord( vec3d_o, vec3d, matrix )

#define Matrix4x4_Scale( mat_out, x_scale, y_scale, z_scale ) \
	D3DXMatrixScaling( mat_out, x_scale, y_scale, z_scale )

#define Matrix4x4_Translate( mat_out, x_offset, y_offset, z_offset ) \
	D3DXMatrixTranslation( mat_out, x_offset, y_offset, z_offset )

#define Matrix4x4_LoadIdentity( mat_in_out ) \
	D3DXMatrixIdentity( mat_in_out )

#define Matrix4x4_LookAt( mat_out, eye, target, up ) \
	D3DXMatrixLookAtLH( mat_out, eye, target, up )

#define Matrix4x4_RotateAxis( mat_out, axis, angle_rad ) \
	D3DXMatrixRotationAxis( mat_out, axis, angle_rad )

Matrix4x4 Matrix4x4_LookAtQuaternion( Matrix4x4 *mat_out, Vector_3 *eye, Vector_3 *target );

/* Geometry Types */
typedef struct _Polygon_ABC
{
	Vector_3		a;
	Vector_3		b;
	Vector_3		c;
} Polygon_ABC;

typedef struct _ABB_MaxMin
{
	Vector_3		max;
	Vector_3		min;
} ABB_MaxMin;

typedef struct _Ray_Vec3Pt
{
	Vector_3		origin;
	Vector_3		normal;
} Ray_Vec3Pt;

typedef struct _Plane_Vec3PtNorm
{
	Vector_3		origin;
	Vector_3		normal;
} Plane_Vec3PtNorm;

typedef struct _Sphere_PosRad
{
	Vector_3		pos;
	float			radius;
} Sphere_PosRad;

typedef struct _Capsule 
{
	Sphere_PosRad	s;
	Vector_3		v;
} Capsule;

typedef struct _Frustum_Camera
{
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
const float TWO_PI		= 2 * PI;

/* Collision */
typedef enum _EIntersectType {
	COLLIDE_INVALID =0,
	COLLIDE_OUTSIDE,
	COLLIDE_INTERSECT,
	COLLIDE_CONTAINED
} EIntersectType;

//////////////////////////////////////
// Function Declarations
//////////////////////////////////////

/* Float Math */
extern inline float lerp(float x, float x1, float s);
extern inline float fmax(float a, float b );
extern inline float fmin(float a, float b );
extern inline float frand(void);
extern inline float fclamp(float x, float min, float max);
extern inline bool feq(float a, float b);

/* Collision */
typedef enum _ECollideAxesType
{
	TEST_1D_X	= 1,
	TEST_1D_Y	= 2,
	TEST_1D_Z	= 4,
	TEST_2D_XY	= 3,
	TEST_2D_XZ	= 5,
	TEST_2D_YZ	= 6,
	TEST_3D_XYZ	= 7
} ECollideAxesType;

bool Collide_PointToBox(const Point_3D &p, const ABB_MaxMin &abb, WORD axes=TEST_3D_XYZ);
bool Collide_PointToFrustum(Point_3D &testPt, const Frustum_Camera &f, bool ignoreY = false);
EIntersectType Collide_ABBToFrustum(const ABB_MaxMin testABB, const Frustum_Camera &f, bool bTest2DXZ /*= false*/);
EIntersectType Collide_SphereToFrustum(const Sphere_PosRad &s, const Frustum_Camera &f, bool bTest2DXZ);
bool Collide_SphereToBox(const Sphere_PosRad &s, const ABB_MaxMin &abb);
bool Collide_SphereToSphere(const Sphere_PosRad &s1, const Sphere_PosRad &s2);
bool Collide_SphereToCapsule(const Sphere_PosRad &s, const Capsule &c);
EIntersectType Collide_SphereToPlane(const Sphere_PosRad &s, const Plane_Vec3PtNorm p, /*out*/Point_3D* sphereIntersect);
bool Collide_RayToPlane(const Ray_Vec3Pt &r, const Plane_Vec3PtNorm p);

/* ABB Routines */
extern inline int ABB_Divide( const ABB_MaxMin target, int xDivs, int yDivs, int zDivs, ABB_MaxMin* out );
extern inline ABB_MaxMin ABB_Scale( const ABB_MaxMin &abb, float xScale, float yScale, float zScale );
extern inline float ABB_CalcDepth( const ABB_MaxMin &abb );
extern inline float ABB_CalcWidth( const ABB_MaxMin &abb );
extern inline void ABB_GrowToInclude3DPoint(const Point_3D &p, ABB_MaxMin &abb);
extern inline void ABB_GrowToIncludeABB(const ABB_MaxMin &add, ABB_MaxMin &abb);
extern inline bool ABB_PointInsideABB(const Point_3D &p, const ABB_MaxMin &abb);
extern inline D3DXMATRIX ABB_CalcUnitTransform( const ABB_MaxMin abb );
extern inline Sphere_PosRad ABB_CalcMinSphereContaining( const ABB_MaxMin abb );
extern inline Sphere_PosRad ABB_CalcMaxSphereContainedBy( const ABB_MaxMin abb );

/* Points and Lines */
float DistSq_Point3DFromLineSeg(Point_3D p, Point_3D a, Point_3D b);

/* Polygons */
Vector_3 Polygon_CalcNormalVec( const Polygon_ABC &p );

/* Spheres */
void Sphere_GrowSphereToContainSphere( Sphere_PosRad *inOut, const Sphere_PosRad s );
void Sphere_GrowSphereToContainPoint( Sphere_PosRad *inOut, const Point_3D p );
extern inline void Sphere_CalcXfmHalfUnitSphere( Sphere_PosRad &in, Matrix4x4 &out );
Sphere_PosRad Sphere_CalcBoundingSphereForPoints( const Point_3D pts[], const int numPts );
Sphere_PosRad Sphere_CalcBoundingSphereForPointsIterative( Point_3D pts[], const int numPts );

/* Closest Computations */
inline Point_3D ClosestPoint_PlaneFromPoint(Point_3D pt, Plane_Vec3PtNorm plane)
{
 return Point_3D();
}

//////////////////////////////////////
// Class Declarations
//////////////////////////////////////

/*	Utility class that emcompasses scale, rotation, 
	and translation - and provides lazy computatcion of matrix */
class ComplexTransform
{
public:
	enum {
		ROTATE_XYZ			= (1),
		ROTATE_XZY			= (2),
		ROTATE_YXZ 			= (3),
		ROTATE_YZX 			= (4),
		ROTATE_ZXY 			= (5),
		ROTATE_ZYX 			= (6),
		ROTATION_ORDER_MASK = (0x7),
		XFM_OUT_OF_DATE		= (0x8)
	} ETransformFlags;

	ComplexTransform(): flags(ROTATE_ZYX|XFM_OUT_OF_DATE), rotationEuler(0,0,0), translation(0,0,0), scale(1,1,1){};
	Matrix4x4 GetTransform();
	inline void SetRotationOrder(int order){ flags = (flags & !(ROTATION_ORDER_MASK)) | order | XFM_OUT_OF_DATE;}
	inline void SetXRotationRadians(float xr) {rotationEuler.x = xr; flags |= XFM_OUT_OF_DATE;}
	inline void SetYRotationRadians(float yr) {rotationEuler.y = yr; flags |= XFM_OUT_OF_DATE;}
	inline void SetZRotationRadians(float zr) {rotationEuler.z = zr; flags |= XFM_OUT_OF_DATE;}
	inline void SetTranslation(Vector_3 &t) {translation=t; flags |= XFM_OUT_OF_DATE;}
	inline void SetScale(Vector_3 &s) {scale = s; flags |= XFM_OUT_OF_DATE;}
	inline Vector_3 GetScale() {return scale;}
	inline Vector_3 GetTranslation() {return translation;}
	inline Vector_3 GetRotationEulers() {return rotationEuler;}

private:
	Matrix4x4		matrix;
	Vector_3		translation;
	Vector_3		rotationEuler;	/* in radians */
	Vector_3		scale;
	int				flags;
};