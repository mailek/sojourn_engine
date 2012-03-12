#pragma once
//////////////////////////////////////////////////////////////////////////
// Player.h - 2011 Matthew Alford
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////
// Includes
//////////////////////////////////////
#include "MathUtil.h"

//////////////////////////////////////
// Forward Declarations
//////////////////////////////////////
class CShaderManagerEx;
class CTerrain;
class CRenderEngine;
struct SkeletonVertex;
struct Bone;

//////////////////////////////////////
// Type Definitions
//////////////////////////////////////
typedef enum EMeshType 
{
	eInvalid,
	eSimpleMesh,
	eMeshHierarchy
};

typedef std::vector<D3DMATERIAL9>::iterator				MatIterator;
typedef std::vector<IDirect3DTexture9*>::iterator		TexIterator;

//////////////////////////////////////
// Class Definition
//////////////////////////////////////
class BaseModel
{
	friend class CMeshManager;
public:
	BaseModel(void);
	virtual ~BaseModel(void);

private:
	std::vector<LPDIRECT3DTEXTURE9>		m_arrTexs;
	std::vector<D3DMATERIAL9>			m_arrMats;
	bool								m_bTransparency;	
	Sphere_PosRad						m_sphereBounds;
	unsigned int						m_numOfBones;
	LPDIRECT3DVERTEXBUFFER9				m_skeletonVB;		/* renderable bone vertices (joints) */
	LPDIRECT3DINDEXBUFFER9				m_skeletonIB;		/* renderable bone indices */
	LPD3DXANIMATIONCONTROLLER			m_animController;
	DWORD								m_animationCnt;		/* number of animations in model file */
	DWORD								m_activeAnimation;/* id of current animation playing */
	bool								m_isAnimating;		/* stop or start the animation updates */
	bool								m_boundSphereOutOfDate;
	char								m_filename[MAX_FILENAME_LEN]; /* filename of the model */
	ComplexTransform					m_meshTransform;    /* local transforms specific to model */

	EMeshType							m_meshType;
	/* ///////// if simple mesh /////////// */
	LPD3DXMESH							m_mesh;	
	/* ///////// if mesh hierachy ///////// */
	Bone*								m_rootBone;
	LPD3DXMESHCONTAINER					m_headMeshContainer;

	LPDIRECT3DVERTEXBUFFER9				m_debugAxesVB;
	
	void CreateDebugAxes(void);
	void RecurseFillOutBone( Bone* bone, unsigned int parentIndex );
	void RecurseCalculateBoneMatrices( Bone* bone, LPD3DXMATRIX parentTransform, SkeletonVertex* arrVertices, WORD* arrIndices );
	void UpdateBoneMatrices();
	void UpdateSkinnedMeshes( LPDIRECT3DDEVICE9 device );

public:
	bool LoadXMeshFromFile(LPCSTR pFilename, IDirect3DDevice9* pDevice);
	bool LoadXMeshHierarchyFromFile(LPCSTR pFilename, IDirect3DDevice9* pDevice);
	void LoadTeapot(IDirect3DDevice9* pDevice);
	void LoadCenteredUnitCube(IDirect3DDevice9* device);
	void LoadCenteredUnitCylinder(IDirect3DDevice9* device);
	void LoadCenteredUnitSphere(IDirect3DDevice9* pDevice);
	void LoadScreenOrientedQuad(IDirect3DDevice9* pDevice);
	void Render(CRenderEngine& rndr, D3DXMATRIX worldTransform, CShaderManagerEx &shaderMgr);
	Sphere_PosRad GetSphereBounds();
	void Update( LPDIRECT3DDEVICE9 device, float elapsedMillis );
	void SetAnimation( DWORD animationId );
	void SetDrawColor( ColorRGBA32 clr );
	
	inline DWORD GetFVF( void ) { assert(m_meshType == eSimpleMesh); return m_mesh->GetFVF(); }
	inline HRESULT GetVertexBuffer(LPDIRECT3DVERTEXBUFFER9* buffer) { assert(m_meshType == eSimpleMesh); return m_mesh->GetVertexBuffer(buffer); }
	inline HRESULT GetIndexBuffer(LPDIRECT3DINDEXBUFFER9* buffer) { assert(m_meshType == eSimpleMesh); return m_mesh->GetIndexBuffer(buffer); }
	inline DWORD GetVertexSizeInBytes() { assert(m_meshType == eSimpleMesh); return m_mesh->GetNumBytesPerVertex(); }
	inline UINT GetNumVertices() { assert(m_meshType == eSimpleMesh); return m_mesh->GetNumVertices(); }

	inline Matrix4x4 GetMeshMatrix() {return m_meshTransform.GetTransform();}
	inline void SetLocalPosition(D3DXVECTOR3 &translation) {m_meshTransform.SetTranslation(translation); m_boundSphereOutOfDate=true;}
	inline void SetScale(D3DXVECTOR3 &scale) {m_meshTransform.SetScale(scale); m_boundSphereOutOfDate=true;}
	inline void SetXRotationRadians(float xRot) {m_meshTransform.SetXRotationRadians(xRot); m_boundSphereOutOfDate=true;}
	inline void SetYRotationRadians(float yRot) {m_meshTransform.SetYRotationRadians(yRot); m_boundSphereOutOfDate=true;}
	inline void SetZRotationRadians(float zRot) {m_meshTransform.SetZRotationRadians(zRot); m_boundSphereOutOfDate=true;}
	inline void SetTransparency(bool transparent) {m_bTransparency = transparent;}
	inline bool IsTransparent() {return m_bTransparency;}

};