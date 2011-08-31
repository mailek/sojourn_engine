#include "StdAfx.h"

#include <iostream>
#include <sstream>
#include <d3dx9anim.h>

#include "BaseModel.h"
#include "Terrain.h"
#include "ShaderManager.h"
#include "vertextypes.h"

#define MAX_STRUCT_NAME_CHARS	(50)
#define MAX_BONE_LIMIT			(50)
#define MAX_STRUCT_MATERIALS	(5)
#define MAX_INFLUENCES			(35)
#define MAX_BONE_DEPTH			(20)
#define INVALID_ANIMATION		(-1)

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

		container->pMaterials = NULL;
		for(DWORD dw = 0; dw < NumMaterials; dw++)
		{
			container->_materials[dw] = pMaterials->MatD3D;
			container->_materials[dw].Ambient = container->_materials[dw].Diffuse;
			
			if(pMaterials[dw].pTextureFilename && strlen(pMaterials[dw].pTextureFilename) > 0)
			{
				std::ostringstream stream;
				stream << "Resources\\" << pMaterials[dw].pTextureFilename;
				HR(D3DXCreateTextureFromFile( pd3dDevice, stream.str().c_str(), &container->_textures[dw] ));				
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
							m_vecScale(1.0f, 1.0f, 1.0f),
							m_vecRotation(0.0f, 0.0f, 0.0f),
							m_vecPos(0.0f, 0.0f, 0.0f),
							m_meshType(eInvalid),
							m_skeletonVB(NULL),
							m_animController(NULL),
							m_rootBone(NULL),
							m_headMeshContainer(NULL),
							m_animationCnt(0),
							m_activeAnimation(INVALID_ANIMATION),
							m_isAnimating(false),
							m_sphereCalculated(false)
{

}

BaseModel::~BaseModel(void)
{
	if( m_meshType == eSimpleMesh)
		COM_SAFERELEASE(m_mesh);
	else
		D3DXFrameDestroy(m_rootBone, &s_allocator);

	// release textures
	for(TexIterator it = m_arrTexs.begin(), _it = m_arrTexs.end(); it != _it; it++)
		COM_SAFERELEASE(*it);
	
	m_arrTexs.clear();
	COM_SAFERELEASE(m_skeletonVB);
	
}

//////////////////////////////////////////////////////////////////////////
// Setup Functions
//////////////////////////////////////////////////////////////////////////

Sphere_PosRad BaseModel::GetSphereBounds()
{
	Sphere_PosRad s;
	::ZeroMemory(&s, sizeof(s));
	if(m_mesh == NULL)
		return s;
	
	if(!m_sphereCalculated)
	{
		// Calculate the bounding sphere
//		LPDIRECT3DVERTEXBUFFER9 vb;
		LPVOID data;
		if(FAILED(m_mesh->LockVertexBuffer(D3DLOCK_READONLY, &data)))
		{
			assert(false);
			return m_sphereBounds;
		}

		D3DXVECTOR3 center;
		float radius; 

		if(FAILED(D3DXComputeBoundingSphere((D3DXVECTOR3*)data, m_mesh->GetNumVertices(), m_mesh->GetNumBytesPerVertex(), &center, &radius)))
			return m_sphereBounds;

		m_sphereBounds.pos = center;
		m_sphereBounds.radius = radius;

		m_sphereCalculated = true;
	}

	return m_sphereBounds;
}

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

	D3DXMATERIAL* mats = (D3DXMATERIAL*)matBuffer->GetBufferPointer();
	for(int i = 0, j = numMats; i < j; i++)
	{
		memcpy( &mats[i].MatD3D.Ambient, &mats[i].MatD3D.Diffuse, sizeof(D3DCOLORVALUE) );
		m_arrMats.push_back(mats[i].MatD3D);

		if(mats[i].pTextureFilename != NULL)
		{
			std::ostringstream stream;
			stream << "Resources\\" << mats[i].pTextureFilename;
		
			LPDIRECT3DTEXTURE9 tex(NULL);
			D3DXCreateTextureFromFile(pDevice, std::string(stream.str()).c_str(), &tex);
			m_arrTexs.push_back(tex);
		}
		else
			m_arrTexs.push_back(0);
	}

	matBuffer->Release();
	adjBuffer->Release();

	//hr = pDevice->CreateVertexBuffer(6*sizeof(DebugAxesVertex), D3DUSAGE_WRITEONLY, DebugAxesVertex::FVF, D3DPOOL_MANAGED, &m_debugAxesVB, 0);
	//if(FAILED(hr))
	//	return false;

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

	// TODO: TESTING _ REMOVE THIS
	D3DXTRACK_DESC desc;
	m_animController->GetTrackDesc( 0, &desc );
	//LPD3DXANIMATIONSET animSet = NULL;
	//m_animController->GetAnimationSet(2, &animSet);
	//m_animController->SetTrackAnimationSet(0, animSet );
	m_animController->SetTrackSpeed(0, 2.0f);
	
	// END TODO

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
		pMeshContainer->MeshData.pMesh->CloneMeshFVF( 0/*options*/, pMeshContainer->MeshData.pMesh->GetFVF(), 
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

bool BaseModel::LoadTeapot(IDirect3DDevice9* pDevice)
{
	assert(pDevice);

	HRESULT hr = D3DXCreateTeapot(pDevice, &m_mesh, 0);
	if(FAILED(hr))
		return false;

	m_meshType = eSimpleMesh;
	
	// Teapot Material
	D3DXCOLOR matColor = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	D3DMATERIAL9 teapotMaterial;
	teapotMaterial.Ambient		= matColor;
	teapotMaterial.Diffuse		= matColor;

	m_arrMats.push_back(teapotMaterial);
	m_arrTexs.push_back(0);

#ifdef _DEBUG
	//hr = pDevice->CreateVertexBuffer(6*sizeof(DebugAxesVertex), D3DUSAGE_WRITEONLY, DebugAxesVertex::FVF, D3DPOOL_MANAGED, &m_debugAxesVB, 0);
	//if(FAILED(hr))
	//	return false;
	//
	//CreateDebugAxes();
#endif

	return true;
}

bool BaseModel::LoadCenteredCube(IDirect3DDevice9* pDevice)
{
	assert(pDevice);

	HRESULT hr = D3DXCreateBox( pDevice, 1,1,1/*0.999f, 0.999f, 0.999f*/, &m_mesh, NULL );

	if(FAILED(hr))
		return false;

	m_meshType = eSimpleMesh;

	// Cube Material
	D3DXCOLOR matColor = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	D3DMATERIAL9 cubeMaterial;
	cubeMaterial.Ambient		= matColor;
	cubeMaterial.Diffuse		= matColor;

	m_arrMats.push_back(cubeMaterial);
	m_arrTexs.push_back(0);

	return true;
}

bool BaseModel::LoadScreenOrientedQuad(IDirect3DDevice9* pDevice)
{
	assert(pDevice);

	D3DVERTEXELEMENT9 vertDecl[10];
	D3DXDeclaratorFromFVF( RenderQuadVertex::FVF, vertDecl );
	HRESULT hr = D3DXCreateMesh( 2, 6, 0, vertDecl , pDevice, &m_mesh );
	if(FAILED(hr))
		return false;

	m_meshType = eSimpleMesh;

	RenderQuadVertex* coord = NULL;
	LPDIRECT3DVERTEXBUFFER9 vb =NULL;
	m_mesh->GetVertexBuffer(&vb);

	hr = vb->Lock(0, 0, (void**)&coord, 0);
	if(FAILED(hr))
		return false;

	coord[0]._p  = D3DXVECTOR3(-1.0f, -1.0f, 1.0f);
	coord[1]._p  = D3DXVECTOR3(-1.0f, 1.0f, 1.0f);
	coord[2]._p  = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
	coord[3]._p  = D3DXVECTOR3(-1.0f, -1.0f, 1.0f);
	coord[4]._p  = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
	coord[5]._p  = D3DXVECTOR3(1.0f, -1.0f, 1.0f);

	// Note: Billboard Quad's UVs are calculated in vertex shader

	vb->Unlock();
	COM_SAFERELEASE(vb);

	return true;
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
	_right._p = D3DXVECTOR3(1.0f/m_vecScale.x, 0.0f, 0.0f);
	_right._c = D3DCOLOR_XRGB(255, 0, 0);

	_up._p = D3DXVECTOR3(0.0f, 1.0f/m_vecScale.y, 0.0f);
	_up._c = D3DCOLOR_XRGB(0, 0, 255);

	_facing._p = D3DXVECTOR3(0.0f, 0.0f, 1.0f/m_vecScale.z);
	_facing._c = D3DCOLOR_XRGB(0, 255, 0);
	
	_pos._p =  D3DXVECTOR3(0.0f, 0.0f, 0.0f);
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

void BaseModel::Render(LPDIRECT3DDEVICE9 device, D3DXMATRIX worldTransform, CShaderManager &shaderMgr) const
{
	assert( m_meshType );

	// apply the mesh specific transforms before rendering
	D3DXMATRIX scale, worldMatrix;
	D3DXMatrixIdentity(&worldMatrix);
	D3DXMatrixScaling(&scale, m_vecScale.x, m_vecScale.y, m_vecScale.z);
	worldMatrix *= scale;

	D3DXMATRIX rotate;
	D3DXMatrixRotationZ(&rotate, m_vecRotation.z);
	worldMatrix *= rotate;

	D3DXMatrixIdentity(&rotate);
	D3DXMatrixRotationX(&rotate, m_vecRotation.x);
	worldMatrix *= rotate;

	D3DXMatrixIdentity(&rotate);
	D3DXMatrixRotationY(&rotate, m_vecRotation.y);
	worldMatrix *= rotate;

	D3DXMATRIX translate;
	D3DXMatrixIdentity(&translate);
	D3DXMatrixTranslation(&translate, m_vecPos.x, m_vecPos.y, m_vecPos.z);
	worldMatrix *= translate;

	worldMatrix *= worldTransform;
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
		device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

		for(int i = 0, j = m_arrMats.size(); i < j; i++)
		{
			shaderMgr.SetMaterial(PASS_DEFAULT, m_arrMats[i]);
			device->SetFVF(m_mesh->GetFVF());
			device->SetTexture(0, m_arrTexs[i]);
			m_mesh->DrawSubset(i);
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

//////////////////////////////////////////////////////////////////////////
// Update Functions
//////////////////////////////////////////////////////////////////////////

void BaseModel::Update( LPDIRECT3DDEVICE9 device, float elapsedMillis )
{
	if( m_meshType == eMeshHierarchy )
	{
		if(m_animController && m_isAnimating )
			m_animController->AdvanceTime( elapsedMillis, NULL );

		UpdateBoneMatrices();
		if( m_headMeshContainer )
		{
			UpdateSkinnedMeshes( device );
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