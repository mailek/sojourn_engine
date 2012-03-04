#include "StdAfx.h"

#include <iostream>
#include <sstream>
#include <d3dx9anim.h>

#include "BaseModel.h"
#include "Terrain.h"
#include "ShaderManager.h"
#include "vertextypes.h"
#include "texturemanager.h"
#include "gamestatestack.h"

#define MAX_STRUCT_NAME_CHARS	(50)
#define MAX_BONE_LIMIT			(50)
#define MAX_STRUCT_MATERIALS	(5)
#define MAX_INFLUENCES			(35)
#define MAX_BONE_DEPTH			(20)
#define MAX_VERTICES			(5000)

enum {
	INVALID_ANIMATION = 9999
};

typedef struct Bone : public D3DXFRAME
{
	char			_name[MAX_STRUCT_NAME_CHARS];
	D3DXMATRIX		matCombined;
	unsigned int	parentIndex;
	unsigned int	jointIndex;
	unsigned int	vertIndex;

} _Bone;

typedef struct MeshContainer : public D3DXMESHCONTAINER
{
	//General attributes
	char				_name[MAX_STRUCT_NAME_CHARS];

	//Mesh variables
    LPDIRECT3DTEXTURE9  _textures[MAX_STRUCT_MATERIALS];							// textures
	D3DMATERIAL9		_materials[MAX_STRUCT_MATERIALS];	// material values
	//D3DMATRIX			_boneOffsets[MAX_BONE_LIMIT];		// bone offsets
	LPD3DXMATRIX		_frameMatrices[MAX_INFLUENCES];		// references to frame matrix for influences
	LPD3DXMESH          _skinnedMesh;						// skin mesh
	
	//// Attribute table stuff
	//LPD3DXATTRIBUTERANGE pAttributeTable;	// The attribute table
 //   DWORD               NumAttributeGroups;// The number of attribute groups;
} _MeshContainer;

inline Sphere_PosRad CalculateMeshBoundSphere(LPD3DXMESH mesh)
{
	Sphere_PosRad ret;
	::ZeroMemory(&ret, sizeof(ret));

	// Calculate the bounding sphere
	LPVOID data;
	if(FAILED(mesh->LockVertexBuffer(D3DLOCK_READONLY, &data)))
	{
		assert(false);
		return ret;
	}

	/* Build the temporary array of vertex points */
	int numVerts = mesh->GetNumVertices();
	int vertexStride = mesh->GetNumBytesPerVertex();

	assert(numVerts < MAX_VERTICES);
	Point_3D pts[MAX_VERTICES];
	BYTE* pVert = (BYTE*)data;
	
	for(int i = 0; i < numVerts; i++)
	{
		D3DXVECTOR3* v = (D3DXVECTOR3*)pVert;
		pts[i] = *v;

		pVert += vertexStride;
	}

//	ret = Sphere_CalcBoundingSphereForPoints(pts, numVerts);
	ret = Sphere_CalcBoundingSphereForPointsIterative(pts, numVerts);

	return (ret);
}

////////////////////////////////////////////////////
// Hierarchy Allocator Class
////////////////////////////////////////////////////
class DefaultAllocator : public ID3DXAllocateHierarchy
{

    STDMETHOD(CreateFrame)(THIS_ LPCSTR Name, 
                            LPD3DXFRAME *ppNewFrame)
	{
		assert( strlen(Name) < MAX_STRUCT_NAME_CHARS );

		*ppNewFrame = NULL;
		
		Bone* bone = new Bone();
		::ZeroMemory( bone, sizeof(Bone) );
		
		strncpy_s( bone->_name, Name, sizeof(bone->_name) );
		bone->Name = &bone->_name[0];
		D3DXMatrixIdentity(&bone->matCombined);
		D3DXMatrixIdentity(&bone->TransformationMatrix);

		*ppNewFrame = (LPD3DXFRAME)bone;

		return S_OK;
	}

    STDMETHOD(CreateMeshContainer)(THIS_ 
        LPCSTR Name, 
        CONST D3DXMESHDATA *pMeshData, 
        CONST D3DXMATERIAL *pMaterials, 
        CONST D3DXEFFECTINSTANCE *pEffectInstances, 
        DWORD NumMaterials, 
        CONST DWORD *pAdjacency, 
        LPD3DXSKININFO pSkinInfo, 
        LPD3DXMESHCONTAINER *ppNewMeshContainer)
	{
		// Create a Temp mesh contianer
		// Using my drived struct here
		MeshContainer *container = new MeshContainer();
		*ppNewMeshContainer = container;

		::ZeroMemory( *ppNewMeshContainer, sizeof(MeshContainer) );

		if(Name)
		{
			strncpy_s( container->_name, Name, sizeof(container->_name) );
			container->Name = container->_name;
		}
		else
			container->Name = NULL;

		container->MeshData.Type = D3DXMESHTYPE_MESH;
		
		//// adjacency
		//DWORD dwFaces = pMeshData->pMesh->GetNumFaces();

		container->NumMaterials = NumMaterials;
		assert( NumMaterials < MAX_STRUCT_MATERIALS );

		////////// Multiply by 3 because there are three adjacent triangles
		////////pMeshContainer->pAdjacency = new DWORD[dwFaces*3];
		////////memcpy(pMeshContainer->pAdjacency, pAdjacency, sizeof(DWORD) * dwFaces*3);
		//
		LPDIRECT3DDEVICE9 pd3dDevice = NULL;// Direct3D Rendering device
		HR(pMeshData->pMesh->GetDevice(&pd3dDevice));
		
		// Copy the bind pose
		container->MeshData.pMesh = pMeshData->pMesh;
		container->MeshData.pMesh->AddRef();

		TextureContextIdType texContext;
		CGameStateStack::GetInstance()->GetCurrentState()->HandleEvent(EVT_GETTEXCONTEXT, &texContext, sizeof(texContext));
		CTextureManager* texMgr = CTextureManager::GetInstance();

		container->pMaterials = NULL;
		for(DWORD dw = 0; dw < NumMaterials; dw++)
		{
			container->_materials[dw] = pMaterials->MatD3D;
			container->_materials[dw].Ambient = container->_materials[dw].Diffuse;
			
			if(pMaterials[dw].pTextureFilename)
			{
				container->_textures[dw] = texMgr->GetTexture(texContext, pMaterials[dw].pTextureFilename);
			}
		}

		//Release the device
		COM_SAFERELEASE(pd3dDevice);

		if(pSkinInfo)
		{
			container->pSkinInfo = pSkinInfo;
			pSkinInfo->AddRef();

		//	// Will need an array of offset matrices to move the vertices from 
		//	//	the figure space to the bone's space
		//	UINT uBones = pSkinInfo->GetNumBones();
		//	for (UINT i = 0; i < uBones; i++)
		//		container->_boneOffsets[i] = *(container->pSkinInfo->GetBoneOffsetMatrix(i));
		//				
		}
		else
		{
			container->pSkinInfo = NULL;
		}

		container->_skinnedMesh = NULL;
		container->pEffects = NULL;

		return S_OK;
	}

    STDMETHOD(DestroyFrame)(THIS_ LPD3DXFRAME pFrameToFree)
	{

		Bone* pBoneToFree = (Bone*) pFrameToFree; /* unsafe up-cast */
		delete pBoneToFree;

		return S_OK;
	}

    STDMETHOD(DestroyMeshContainer)(THIS_ LPD3DXMESHCONTAINER pMeshContainerToFree)
	{
		MeshContainer* container = (MeshContainer*)pMeshContainerToFree;

		container->MeshData.pMesh->Release();
		container->MeshData.pMesh = NULL;
		
		delete container;

		return S_OK;
	}
};

////////////////////////////////////////////////////
// Base Model Class
////////////////////////////////////////////////////

static DefaultAllocator		s_allocator;

BaseModel::BaseModel(void) : m_mesh(NULL),
							m_debugAxesVB(NULL),
							m_bTransparency(false),
							m_meshType(eInvalid),
							m_skeletonVB(NULL),
							m_skeletonIB(NULL),
							m_animController(NULL),
							m_rootBone(NULL),
							m_headMeshContainer(NULL),
							m_animationCnt(0),
							m_activeAnimation(INVALID_ANIMATION),
							m_isAnimating(false),
							m_numOfBones(0),
							m_boundSphereOutOfDate(true)
{
	m_filename[0] = 0;
	::ZeroMemory(&m_sphereBounds, sizeof(m_sphereBounds));
}

BaseModel::~BaseModel(void)
{
	if( m_meshType == eSimpleMesh)
		COM_SAFERELEASE(m_mesh);
	else
		D3DXFrameDestroy(m_rootBone, &s_allocator);
	
	m_arrTexs.clear();
	COM_SAFERELEASE(m_skeletonVB);
}

//////////////////////////////////////////////////////////////////////////
// Setup Functions
//////////////////////////////////////////////////////////////////////////


bool BaseModel::LoadXMeshFromFile(LPCSTR pFilename, IDirect3DDevice9* pDevice)
{
	assert(pDevice);

	ID3DXBuffer *adjBuffer(NULL);
	ID3DXBuffer *matBuffer(NULL);
	DWORD numMats(0);

	HRESULT hr = D3DXLoadMeshFromX(pFilename, D3DXMESH_MANAGED, pDevice, &adjBuffer, 
		&matBuffer, 0/*effectinstances*/, &numMats, &m_mesh);

	if(FAILED(hr))
		return false;

	m_meshType = eSimpleMesh;
	TextureContextIdType texContext;
	CGameStateStack::GetInstance()->GetCurrentState()->HandleEvent(EVT_GETTEXCONTEXT, &texContext, sizeof(texContext));
	CTextureManager* texMgr = CTextureManager::GetInstance();

	D3DXMATERIAL* mats = (D3DXMATERIAL*)matBuffer->GetBufferPointer();
	for(int i = 0, j = numMats; i < j; i++)
	{
		memcpy( &mats[i].MatD3D.Ambient, &mats[i].MatD3D.Diffuse, sizeof(D3DCOLORVALUE) );
		m_arrMats.push_back(mats[i].MatD3D);

		if(mats[i].pTextureFilename != NULL)
		{
			LPDIRECT3DTEXTURE9 tex = texMgr->GetTexture(texContext, mats[i].pTextureFilename);
			m_arrTexs.push_back(tex);
		}
		else
			m_arrTexs.push_back(0);
	}

	matBuffer->Release();
	adjBuffer->Release();

	CreateDebugAxes();

	return true;
}

bool BaseModel::LoadXMeshHierarchyFromFile(LPCSTR pFilename, IDirect3DDevice9* pDevice)
{
	assert(pDevice);

	HR(D3DXLoadMeshHierarchyFromX( pFilename, 0, pDevice, 
		&s_allocator, NULL, (LPD3DXFRAME*)&m_rootBone, &m_animController ));

	m_meshType = eMeshHierarchy;
	m_animationCnt = m_animController->GetNumAnimationSets();

	m_numOfBones = 0;
	RecurseFillOutBone( m_rootBone, 0 );
	assert( m_numOfBones > 0 );

	//////////////////////
	// Vertices //////////
	const unsigned int vb_sz = (m_numOfBones+1) * sizeof(SkeletonVertex);
	HR(pDevice->CreateVertexBuffer( vb_sz/*length*/, D3DUSAGE_WRITEONLY/*usage*/, 
		SkeletonVertex::FVF/*fvf format*/, D3DPOOL_MANAGED/*pool*/, &m_skeletonVB, 0/*share handle*/ ));
	
	/////////////////////
	// Indices //////////
	const unsigned int ib_sz = 2 * m_numOfBones * sizeof(WORD);
	HR(pDevice->CreateIndexBuffer( ib_sz, D3DUSAGE_WRITEONLY/*usage*/, 
		D3DFMT_INDEX16/*format*/, D3DPOOL_MANAGED/*pool*/, &m_skeletonIB, 0/*share handle*/ ));

	UpdateBoneMatrices();
	if( m_headMeshContainer )
	{
		UpdateSkinnedMeshes( pDevice );
	}

	return true;
}

// Computes the combined bone transformation and increments the bone count
void BaseModel::RecurseFillOutBone( Bone* bone, unsigned int parentIndex )
{
	static int s_boneDepth = 0;
	unsigned int jointIndex = m_numOfBones;
	m_numOfBones++;
	assert( m_numOfBones < MAX_BONE_LIMIT );

	// save the skeleton verts and indices for rendering
	bone->vertIndex = m_numOfBones;
	bone->parentIndex = parentIndex;
	bone->jointIndex = jointIndex;

	// link the meshes influences to the actual frame combined matrix
	MeshContainer* pMeshContainer = (MeshContainer*)bone->pMeshContainer;
	if(pMeshContainer != NULL)
	{
		// if this is the first mesh container, save it for rendering
		m_headMeshContainer = ( m_headMeshContainer ? m_headMeshContainer : pMeshContainer );

		const DWORD numBones = pMeshContainer->pSkinInfo->GetNumBones();
		for( DWORD i = 0; i < numBones; i++ )
		{
			assert( i < MAX_INFLUENCES );
			Bone* boneRef = (Bone*)D3DXFrameFind( (LPD3DXFRAME)m_rootBone, pMeshContainer->pSkinInfo->GetBoneName(i) );
			assert(boneRef);
			pMeshContainer->_frameMatrices[i] = &boneRef->matCombined;

		}

		LPDIRECT3DDEVICE9 device;
		HR(pMeshContainer->MeshData.pMesh->GetDevice( &device ));
		// Copy the bind pose reference mesh to a workable skinned mesh
		pMeshContainer->MeshData.pMesh->CloneMeshFVF( D3DXMESH_VB_MANAGED/*options*/, pMeshContainer->MeshData.pMesh->GetFVF(), 
			device, &pMeshContainer->_skinnedMesh );
	}
	
	// siblings
	if( bone->pFrameSibling != NULL )
		RecurseFillOutBone( (Bone*) bone->pFrameSibling, parentIndex );

	// kids
	if( s_boneDepth < MAX_BONE_DEPTH )
	{
		s_boneDepth++;
		if( bone->pFrameFirstChild != NULL )
			RecurseFillOutBone( (Bone*) bone->pFrameFirstChild, m_numOfBones );

		s_boneDepth--;
	}

}

void BaseModel::LoadTeapot(IDirect3DDevice9* pDevice)
{
	assert(pDevice);

	HR(D3DXCreateTeapot(pDevice, &m_mesh, 0));
	m_meshType = eSimpleMesh;
	
	// Teapot Material
	D3DXCOLOR matColor = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	D3DMATERIAL9 mat;
	mat.Ambient		= matColor;
	mat.Diffuse		= matColor;

	m_arrMats.push_back(mat);
	m_arrTexs.push_back(0);
}

void BaseModel::LoadCenteredUnitCube(IDirect3DDevice9* pDevice)
{
	assert(pDevice);

	HR(D3DXCreateBox( pDevice, 1.0f, 1.0f, 1.0f, &m_mesh, NULL ));
	m_meshType = eSimpleMesh;

	// Cube Material
	D3DXCOLOR matColor = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	D3DMATERIAL9 mat;
	mat.Ambient		= matColor;
	mat.Diffuse		= matColor;

	m_arrMats.push_back(mat);
	m_arrTexs.push_back(0);
}

void BaseModel::LoadCenteredUnitCylinder(IDirect3DDevice9* pDevice)
{
	assert(pDevice);

	HR(D3DXCreateCylinder( pDevice, 0.5f, 0.5f, 1.0f, 16, 2, &m_mesh, NULL ));
	m_meshType = eSimpleMesh;

	// Cylinder Material
	D3DXCOLOR matColor = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	D3DMATERIAL9 mat;
	mat.Ambient		= matColor;
	mat.Diffuse		= matColor;

	m_arrMats.push_back(mat);
	m_arrTexs.push_back(0);
}

void BaseModel::LoadCenteredUnitSphere(IDirect3DDevice9* pDevice)
{
	assert(pDevice);

	HR(D3DXCreateSphere( pDevice, 1.0f, 32, 32, &m_mesh, NULL ));
	m_meshType = eSimpleMesh;

	// Sphere Material
	D3DXCOLOR matColor = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	D3DMATERIAL9 mat;
	mat.Ambient		= matColor;
	mat.Diffuse		= matColor;

	m_arrMats.push_back(mat);
	m_arrTexs.push_back(0);
}

void BaseModel::LoadScreenOrientedQuad(IDirect3DDevice9* pDevice)
{
	assert(pDevice);

	D3DVERTEXELEMENT9 vertDecl[10];
	HR(D3DXDeclaratorFromFVF( RenderQuadVertex::FVF, vertDecl ));
	HR(D3DXCreateMesh( 2, 6, 0, vertDecl , pDevice, &m_mesh ));
	m_meshType = eSimpleMesh;

	RenderQuadVertex* coord = NULL;
	LPDIRECT3DVERTEXBUFFER9 vb =NULL;
	m_mesh->GetVertexBuffer(&vb);
	HR(vb->Lock(0, 0, (void**)&coord, 0));
	
	coord[0]._p  = D3DXVECTOR3(-1.0f, -1.0f, 1.0f);
	coord[1]._p  = D3DXVECTOR3(-1.0f, 1.0f, 1.0f);
	coord[2]._p  = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
	coord[3]._p  = D3DXVECTOR3(-1.0f, -1.0f, 1.0f);
	coord[4]._p  = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
	coord[5]._p  = D3DXVECTOR3(1.0f, -1.0f, 1.0f);

	// Note: Billboard Quad's UVs are calculated in vertex shader

	vb->Unlock();
	COM_SAFERELEASE(vb);
}

void BaseModel::CreateDebugAxes()
{
	if(m_debugAxesVB == NULL)
		return;

	// debug origin lines
	DebugAxesVertex* coord = NULL;
	HRESULT hr = m_debugAxesVB->Lock(0, 0, (void**)&coord, 0);
	if(FAILED(hr))
		return;

	DebugAxesVertex _right, _up, _facing, _pos;
	Vector_3 scale = m_meshTransform.GetScale();
	_right._p = D3DXVECTOR3(1.0f/scale.x, 0.0f, 0.0f);
	_right._c = D3DCOLOR_XRGB(255, 0, 0);

	_up._p = D3DXVECTOR3(0.0f, 1.0f/scale.y, 0.0f);
	_up._c = D3DCOLOR_XRGB(0, 0, 255);

	_facing._p = D3DXVECTOR3(0.0f, 0.0f, 1.0f/scale.z);
	_facing._c = D3DCOLOR_XRGB(0, 255, 0);
	
	_pos._p = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	_pos._c = D3DCOLOR_XRGB(255, 255, 255);

	coord[0] = _pos;
	coord[1] = _right;
	coord[2] = _pos;
	coord[3] = _up;
	coord[4] = _pos;
	coord[5] = _facing;
	
	m_debugAxesVB->Unlock();
}

//////////////////////////////////////////////////////////////////////////
// Render Functions
//////////////////////////////////////////////////////////////////////////

void BaseModel::Render(LPDIRECT3DDEVICE9 device, D3DXMATRIX worldTransform, CShaderManager &shaderMgr)
{
	assert( m_meshType );

	// apply the mesh specific transforms before rendering
	Matrix4x4 worldMatrix = GetMeshMatrix() * worldTransform;
	shaderMgr.SetWorldTransform(PASS_DEFAULT, worldMatrix);

	// Render Skeleton
	if( m_meshType == eMeshHierarchy )
	{
		// Draw Skeleton
		//HR(device->SetTexture( 0, 0 ));
		//HR(device->SetStreamSource( 0, m_skeletonVB, 0, sizeof(SkeletonVertex) ));
		//HR(device->SetIndices( m_skeletonIB ));
		//HR(device->SetFVF( SkeletonVertex::FVF ));
		//HR(device->DrawIndexedPrimitive(D3DPT_LINELIST, 0, 0, m_numOfBones+1, 10, m_numOfBones ));
		
		// Draw Skin
		device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
		MeshContainer* pDrawContainer = (MeshContainer*)m_headMeshContainer;
		while( pDrawContainer != NULL )
		{
			for( DWORD i = 0; i < pDrawContainer->NumMaterials; i++ )
			{
				// TODO:  Note that the reason for hard-coding the ambient is
				//		  per the sample tiny model is dark for outside scene
				shaderMgr.SetMaterial(PASS_DEFAULT, pDrawContainer->_materials[i]);
				HR(device->SetFVF( pDrawContainer->pSkinInfo->GetFVF() ));
				HR(device->SetTexture(0, pDrawContainer->_textures[i]));
				HR((pDrawContainer->_skinnedMesh->DrawSubset(i)));
			}
			pDrawContainer = (MeshContainer*)pDrawContainer->pNextMeshContainer;
		}

	}
	else
	{
		assert(m_meshType == eSimpleMesh);
		device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

		for(int i = 0, j = m_arrMats.size(); i < j; i++)
		{
			shaderMgr.SetMaterial(PASS_DEFAULT, m_arrMats[i]);
			device->SetFVF(m_mesh->GetFVF());
			device->SetTexture(0, m_arrTexs[i]);
			HR(m_mesh->DrawSubset(i));
		}
		
		device->SetTexture(0, 0);
	}

#ifdef _DEBUG
	// draw debug origin lines		
	if(m_debugAxesVB)
	{
		device->SetTexture(0,0);
		device->SetStreamSource(0, m_debugAxesVB, 0, sizeof(DebugAxesVertex));
		device->SetFVF(DebugAxesVertex::FVF);
		device->DrawPrimitive(D3DPT_LINELIST, 0, 3);
	}
#endif
	
}

Sphere_PosRad BaseModel::GetSphereBounds()
{
	/* Recalculate the sphere bound if needed */
	if(m_boundSphereOutOfDate)
	{
		Sphere_PosRad newSphere;
		if(m_meshType == eMeshHierarchy)
		{
			for(MeshContainer* container = (MeshContainer*)m_headMeshContainer; container != NULL; container = (MeshContainer*)container->pNextMeshContainer)
			{
				Sphere_PosRad tempSphere = CalculateMeshBoundSphere(container->_skinnedMesh);

				if(container == m_headMeshContainer)
					newSphere = tempSphere;
				else
					Sphere_GrowSphereToContainSphere(&newSphere, tempSphere);
			}
		}
		else
		{
			assert(m_meshType == eSimpleMesh);
			newSphere = CalculateMeshBoundSphere(m_mesh);
		}

		/* Apply local transforms */
		newSphere.radius *= fmax(fmax(m_meshTransform.GetScale().x, m_meshTransform.GetScale().y), m_meshTransform.GetScale().z);
		Matrix4x4 meshTransforms = GetMeshMatrix();
		Vec3_TransformCoord(&newSphere.pos, &newSphere.pos, &meshTransforms);

		m_sphereBounds = newSphere;
		m_boundSphereOutOfDate = false;
	}

	return (m_sphereBounds);
}

//////////////////////////////////////////////////////////////////////////
// Update Functions
//////////////////////////////////////////////////////////////////////////

void BaseModel::Update( LPDIRECT3DDEVICE9 device, float elapsedMillis )
{
	if( m_meshType == eMeshHierarchy )
	{
		if(m_animController && m_isAnimating )
		{
			m_animController->AdvanceTime( elapsedMillis, NULL );

			UpdateBoneMatrices();
			if( m_headMeshContainer )
			{
				UpdateSkinnedMeshes( device );
			}
		}
	}
}

void BaseModel::UpdateBoneMatrices()
{
	const unsigned int vb_sz = (m_numOfBones+1) * sizeof(SkeletonVertex);
	const unsigned int ib_sz = 2 * m_numOfBones * sizeof(WORD);

	WORD indices[ MAX_BONE_LIMIT<<1 ]; // 2 verts per line (bone)
	::ZeroMemory( &indices, sizeof(indices) );
	SkeletonVertex verts[ MAX_BONE_LIMIT ];	
	::ZeroMemory( &verts, sizeof(verts) );

	// Calcuate the frame matrices
	D3DXMATRIX mat;
	D3DXMatrixIdentity( &mat );
	RecurseCalculateBoneMatrices( m_rootBone, &mat, verts, indices );

	SkeletonVertex* pVB = NULL;
	HR(m_skeletonVB->Lock( 0, 0, (void**)&pVB, 0));
	memcpy( pVB, verts, vb_sz );

	WORD *pIndex = NULL;
	HR(m_skeletonIB->Lock( 0, 0, (void**)&pIndex, 0 ));
	memcpy( pIndex, indices, ib_sz );

	HR(m_skeletonVB->Unlock());
	HR(m_skeletonIB->Unlock());
}

void BaseModel::UpdateSkinnedMeshes( LPDIRECT3DDEVICE9 device )
{
	MeshContainer *container = (MeshContainer*) this->m_headMeshContainer;

	for( ; container != NULL; container = (MeshContainer*)container->pNextMeshContainer )
	{
		if( container->pSkinInfo == NULL )
			continue;

		// software skinning
		LPD3DXMESH srcMesh = container->MeshData.pMesh;
		LPD3DXMESH destMesh = container->_skinnedMesh;

		D3DXMATRIX boneTransforms[MAX_INFLUENCES];
		memset( &boneTransforms, 0, sizeof(boneTransforms) );
		for( int i = 0, j = container->pSkinInfo->GetNumBones(); i < j; i++ )
		{
			D3DXMATRIX frame = *container->_frameMatrices[i];
			D3DXMATRIX boneOffset = *container->pSkinInfo->GetBoneOffsetMatrix( i );
			boneTransforms[i] = boneOffset * frame;
		}
		
		LPVOID srcBuffer, destBuffer;

		srcMesh->LockVertexBuffer( D3DLOCK_READONLY, &srcBuffer );
		destMesh->LockVertexBuffer( 0, &destBuffer );
		container->pSkinInfo->UpdateSkinnedMesh( boneTransforms, NULL, srcBuffer, destBuffer );

		srcMesh->UnlockVertexBuffer();
		destMesh->UnlockVertexBuffer();

	}

	// update the bound sphere
	m_boundSphereOutOfDate = true;
}

void BaseModel::RecurseCalculateBoneMatrices( Bone* bone, LPD3DXMATRIX parentTransform, SkeletonVertex* arrVertices, WORD* arrIndices )
{
	static int s_boneDepth = 0;
	
	D3DXMatrixMultiply( &bone->matCombined, &bone->TransformationMatrix, parentTransform );
	D3DXVec3TransformCoord( &arrVertices[bone->vertIndex]._p, &D3DXVECTOR3(0,0,0), &bone->matCombined );
	//arrVertices[bone->vertIndex]._c = D3DCOLOR_XRGB(255, 255, 255);
	
	arrIndices[bone->jointIndex<<1] = bone->parentIndex;
	arrIndices[(bone->jointIndex<<1)+1] = bone->vertIndex;

	// siblings
	if( bone->pFrameSibling != NULL )
		RecurseCalculateBoneMatrices( (Bone*) bone->pFrameSibling, parentTransform, arrVertices, arrIndices );

	// kids
	if( s_boneDepth < MAX_BONE_DEPTH )
	{
		s_boneDepth++;
		if( bone->pFrameFirstChild != NULL )
			RecurseCalculateBoneMatrices( (Bone*) bone->pFrameFirstChild, &bone->matCombined, arrVertices, arrIndices );

		s_boneDepth--;
	}

}

void BaseModel::SetAnimation( DWORD animationId )
{
	if(m_meshType == eSimpleMesh)
		return;

	if(animationId == -1)
	{
		m_isAnimating = false;
		m_activeAnimation = -1;
		m_animController->SetTrackPosition(0, 0.0f);
		m_animController->ResetTime();
	} else if( animationId != m_activeAnimation && animationId < m_animationCnt ) 
	{ 
		m_isAnimating = true;
		m_activeAnimation = animationId;
		LPD3DXANIMATIONSET animSet = NULL;
		m_animController->GetAnimationSet(animationId, &animSet);
		m_animController->SetTrackAnimationSet(0, animSet);
		COM_SAFERELEASE(animSet);
	}
}

void BaseModel::SetDrawColor( ColorRGBA32 clr )
{
	assert( m_meshType == eSimpleMesh );

	m_arrMats.clear();
	m_arrTexs.clear();

	D3DMATERIAL9 mat;
	mat.Ambient = clr;
	mat.Diffuse = clr;

	m_arrMats.push_back(mat);
	m_arrTexs.push_back(CTextureManager::GetInstance()->GetTexture(GLOBAL_TEX_CONTEXT, DEFAULT_TEXTURE));
}