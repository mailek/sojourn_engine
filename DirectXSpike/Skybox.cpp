/********************************************************************
	created:	2012/04/23
	filename: 	Skybox.cpp
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
#include "StdAfx.h"
#include "Skybox.h"
#include "ShaderManager.h"
#include "Camera.h"
#include "vertextypes.h"
#include "renderengine.h"
#include "texturemanager.h"
#include "gamestatestack.h"

// tweak variables
#define PULL_DOWN_MOD	(0.8f) // 'pulls' the texture uv's down over the round edge to crop outside ring of texture
#define SPHEREYSINK_MOD (0.2f) // sinks the sphere down from the camera position in order to hide horizon banding

//////////////////////////////////////////////////////////////////////////
// Setup Functions
//////////////////////////////////////////////////////////////////////////

CSkybox::CSkybox(void) : m_texSkyTexture(NULL),
						m_mesh(NULL),
						m_pAttach(NULL),
						m_sphereRadius(0.0f)
{
}

CSkybox::~CSkybox(void)
{
}

void CSkybox::LoadSkyDome( float radius, UINT slices, UINT stacks )
{
	// texture
	TextureContextIdType texContext;
	CGameStateStack::GetInstance()->GetCurrentState()->HandleEvent(EVT_GETTEXCONTEXT, &texContext, sizeof(texContext));
	CTextureManager* texMgr = CTextureManager::GetInstance();
	m_texSkyTexture = texMgr->GetTexture(texContext, "skyTexture.dds");

	// model
	dxCreateSphere(radius, slices, stacks, &m_mesh, NULL);
	m_sphereRadius = radius;

	LPD3DXMESH texturedMesh;
    dxCloneMeshFVF(m_mesh, D3DXMESH_SYSTEMMEM, D3DFVF_XYZ | D3DFVF_TEX1, &texturedMesh);
    m_mesh->Release();
	m_mesh = texturedMesh;

    // lock the vertex buffer & fill out the new uv's
    SkydomeVertex* vertices;
    texturedMesh->LockVertexBuffer(0, (void**)&vertices);
	int numVerts=m_mesh->GetNumVertices();

	D3DXMATRIX rotate;
	D3DXMatrixRotationX(&rotate, PI/2);
	// loop through the vertices
	for (int i=0;i<numVerts;i++) 
	{
		D3DXVec3TransformCoord(&vertices->_p, &vertices->_p, &rotate);
		// calculate texture coordinates
		vertices->_tex1.x = PULL_DOWN_MOD*vertices->_p.x/(2*radius)+0.5f;
		vertices->_tex1.y = PULL_DOWN_MOD*vertices->_p.z/(2*radius)+0.5f;
		vertices++;
	}

	m_mesh->UnlockVertexBuffer();

	D3DXCOLOR matColor		= D3DXCOLOR(0.8f, 0.8f, 1.0f, 1.0f);
	m_skyMaterial.Ambient	= matColor*50.0f;
	m_skyMaterial.Diffuse	= matColor;
}

//////////////////////////////////////////////////////////////////////////
// Render Functions
//////////////////////////////////////////////////////////////////////////

void CSkybox::Render( CRenderEngine &rndr )
{
	D3DXMATRIX world = GetWorldTransform();
	CShaderManagerEx &shaderMgr = rndr.GetShaderManager();
	LPDIRECT3DDEVICE9 device = rndr.GetDevice();
	CCamera& camera = rndr.GetSceneManager()->GetDefaultCamera();
	
	shaderMgr.SetEffect(EFFECT_SKYDOME);
	shaderMgr.SetTechnique("FlatTexture");
	shaderMgr.SetViewProjectionEx(camera.GetViewMatrix(), camera.GetProjectionMatrix());
	shaderMgr.SetWorldTransformEx(world);
	shaderMgr.SetTexture("diffuseTex", m_texSkyTexture);

	int num = shaderMgr.BeginEffect();
	for(int i=0; i<num; i++)
	{
		shaderMgr.Pass(i);
		m_mesh->DrawSubset(0);
		shaderMgr.FinishPass();
	}

	shaderMgr.FinishEffect();
}

D3DXMATRIX CSkybox::GetWorldTransform() 
{
	Vector_3 parentPos;
	m_pAttach->HandleEvent(EVT_GETPOSITIONVEC, &parentPos, sizeof(parentPos));

	D3DXMATRIX worldMatrix;
	D3DXMatrixIdentity(&worldMatrix);
	D3DXMatrixTranslation(&worldMatrix, parentPos.x, parentPos.y - m_sphereRadius*SPHEREYSINK_MOD, parentPos.z);
	return worldMatrix;
}