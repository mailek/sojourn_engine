#pragma once
/********************************************************************
	created:	2012/04/16
	filename: 	BaseModel.h
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
//////////////////////////////////////
// Includes
//////////////////////////////////////
#include "MathUtil.h"

//////////////////////////////////////
// Forward Declarations
//////////////////////////////////////
class CShaderManagerEx;
class CTerrainChunk;
class CRenderEngine;
struct SkeletonVertex;
struct Bone;
class BaseModel;

//////////////////////////////////////
// Type Definitions
//////////////////////////////////////
typedef enum _EMeshType 
{
	eInvalid,
	eSimpleMesh,
	eMeshHierarchy
} EMeshType;

typedef std::vector<D3DMATERIAL9>::iterator				MatIterator;
typedef std::vector<IDirect3DTexture9*>::iterator		TexIterator;

//////////////////////////////////////
// Class Definition
//////////////////////////////////////

class IRenderFunc
{
public:
	IRenderFunc() : P(NULL) {};
	virtual void Render(CRenderEngine& rndr, D3DXMATRIX worldTransform, CShaderManagerEx &shaderMgr) =0;

	BaseModel		*P;
};

class rfLightAndTexture : public IRenderFunc
{
public:
	virtual void Render(CRenderEngine& rndr, D3DXMATRIX worldTransform, CShaderManagerEx &shaderMgr);
};

class rfLightAndColored : public IRenderFunc
{
public:
	virtual void Render(CRenderEngine& rndr, D3DXMATRIX worldTransform, CShaderManagerEx &shaderMgr);
};

class rfXPLightAndColored : public IRenderFunc
{
public:
	virtual void Render(CRenderEngine& rndr, D3DXMATRIX worldTransform, CShaderManagerEx &shaderMgr);
};

class BaseModel
{
	/* everyone needs a friend */
	friend class CMeshManager;
	friend class rfLightAndTexture;
	friend class rfLightAndColored;
	friend class rfXPLightAndColored;
public:
	BaseModel(void);
	virtual ~BaseModel(void);

private:
	std::vector<LPDIRECT3DTEXTURE9>		m_arrTexs;
	std::vector<D3DMATERIAL9>			m_arrMats;
	bool								m_bTransparency;	
	Sphere_PosRad						m_sphereBounds;
	unsigned int						m_numOfBones;
	LPDIRECT3DVERTEXBUFFER9				m_skeletonVB;					/* Renderable bone vertices (joints)	*/
	LPDIRECT3DINDEXBUFFER9				m_skeletonIB;					/* Renderable bone indices				*/
	LPD3DXANIMATIONCONTROLLER			m_animController;
	DWORD								m_animationCnt;					/* Number of animations in model file	*/
	DWORD								m_activeAnimation;				/* Id of current animation playing		*/
	bool								m_isAnimating;					/* Stop or start the animation updates	*/
	bool								m_boundSphereOutOfDate;			/* Sphere bound recalc flag				*/
	char								m_filename[MAX_FILENAME_LEN];	/* Filename of the model				*/
	ComplexTransform					m_meshTransform;    			/* Local transforms specific to model	*/
	IRenderFunc*						m_pRenderFunc;					/* Current render technique				*/

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
	void UpdateSkinnedMeshes();

public:
	bool LoadXMeshFromFile(LPCSTR pFilename, IDirect3DDevice9* pDevice);
	bool LoadXMeshHierarchyFromFile(LPCSTR pFilename, IDirect3DDevice9* pDevice);
	void LoadTeapot();
	void LoadCenteredUnitCube();
	void LoadCenteredUnitCylinder();
	void LoadCenteredUnitSphere();
	void LoadScreenOrientedQuad();
	void Render(CRenderEngine& rndr, D3DXMATRIX worldTransform, CShaderManagerEx &shaderMgr);
	Sphere_PosRad GetSphereBounds();
	void Update( float elapsedMillis );
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
	inline void SetRenderFunc( IRenderFunc& rf ) {m_pRenderFunc = &rf;}
	inline bool IsTransparent() {return m_bTransparency;}

	struct {
		rfLightAndTexture		lightAndTexture;
		rfLightAndColored		lightAndColored;
		rfXPLightAndColored		xpLightAndColored;
	}
	RenderFuncs;

};