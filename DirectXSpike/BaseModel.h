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
class CShaderManager;
class CTerrain;
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
public:
	BaseModel(void);
	virtual ~BaseModel(void);

private:
	std::vector<LPDIRECT3DTEXTURE9>		m_arrTexs;
	std::vector<D3DMATERIAL9>			m_arrMats;
	D3DXVECTOR3							m_vecScale;
	D3DXVECTOR3							m_vecRotation;		/* in radians */
	D3DXVECTOR3							m_vecPos;			/* local offsets in model coordinate space */
	bool								m_bTransparency;	
	Sphere_PosRad						m_sphereBounds;
	unsigned int						m_numOfBones;
	LPDIRECT3DVERTEXBUFFER9				m_skeletonVB;		/* renderable bone vertices (joints) */
	LPDIRECT3DINDEXBUFFER9				m_skeletonIB;		/* renderable bone indices */
	LPD3DXANIMATIONCONTROLLER			m_animController;
	DWORD								m_animationCnt;		/* number of animations in model file */
	DWORD								m_activeAnimation;/* id of current animation playing */
	bool								m_isAnimating;		/* stop or start the animation updates */
	bool								m_sphereCalculated;

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
	void LoadCenteredUnitSphere(IDirect3DDevice9* pDevice);
	void LoadScreenOrientedQuad(IDirect3DDevice9* pDevice);
	void Render(LPDIRECT3DDEVICE9 device, D3DXMATRIX worldTransform, CShaderManager &shaderMgr) const;
	Sphere_PosRad GetSphereBounds();
	void Update( LPDIRECT3DDEVICE9 device, float elapsedMillis );
	void SetAnimation( DWORD animationId );
	void SetDrawColor( ColorRGBA32 clr );
	
	inline DWORD GetFVF( void ) { assert(m_meshType == eSimpleMesh); return m_mesh->GetFVF(); }
	inline HRESULT GetVertexBuffer(LPDIRECT3DVERTEXBUFFER9* buffer) { assert(m_meshType == eSimpleMesh); return m_mesh->GetVertexBuffer(buffer); }
	inline HRESULT GetIndexBuffer(LPDIRECT3DINDEXBUFFER9* buffer) { assert(m_meshType == eSimpleMesh); return m_mesh->GetIndexBuffer(buffer); }
	inline DWORD GetVertexSizeInBytes() { assert(m_meshType == eSimpleMesh); return m_mesh->GetNumBytesPerVertex(); }
	inline UINT GetNumVertices() { assert(m_meshType == eSimpleMesh); return m_mesh->GetNumVertices(); }

	inline void Set3DPosition(D3DXVECTOR3 &translation) {m_vecPos=translation;}
	inline void SetScale(D3DXVECTOR3 &scale) {m_vecScale = scale; CreateDebugAxes();}
	inline void SetXRotationRadians(float xRot) {m_vecRotation.x = xRot;}
	inline void SetYRotationRadians(float yRot) {m_vecRotation.y = yRot;}
	inline void SetZRotationRadians(float zRot) {m_vecRotation.z = zRot;}
	inline void SetTransparency(bool transparent) {m_bTransparency = transparent;}
	inline bool IsTransparent() {return m_bTransparency;}

};