#pragma once
/********************************************************************
	created:	2012/04/23
	filename: 	Skybox.h
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
//////////////////////////////////////
// Includes
//////////////////////////////////////
#include "IRenderable.h"
#include "GameEvents.h"
#include "basemodel.h"

//////////////////////////////////////
// Forward Declarations
//////////////////////////////////////
class CShaderManager;
class CCamera;

//////////////////////////////////////
// Class Definition
//////////////////////////////////////
class CSkybox : public IRenderable
{
public:
	CSkybox(void);
	~CSkybox(void);
	void LoadSkyDome(float radius, UINT slices, UINT stacks);
	inline void SetObjectToFollow(GameEvents::IEventHandler* attach) {m_pAttach = attach;}
	
	virtual void Render( CRenderEngine &rndr );
	virtual D3DXMATRIX GetWorldTransform();
	virtual bool IsTransparent() {return false;}
	virtual void SetLastRenderFrame(UINT frameNum) {};
	virtual UINT GetLastRenderFrame() {return 0;}
	virtual Sphere_PosRad GetBoundingSphere() { Sphere_PosRad s; ::ZeroMemory(&s, sizeof(s)); return s; }

private:
	LPDIRECT3DTEXTURE9			m_texSkyTexture;
	LPD3DXMESH					m_mesh;
	//BaseModel					m_baseMesh;
	D3DMATERIAL9				m_skyMaterial;
	GameEvents::IEventHandler  *m_pAttach;
	float						m_sphereRadius;

};