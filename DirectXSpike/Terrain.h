#pragma once
//////////////////////////////////////////////////////////////////////////
// Terrain.h - 2011 Matthew Alford
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////
// Includes
//////////////////////////////////////
#include "MathUtil.h"
#include "IRenderable.h"
#include "ICollidable.h"

//////////////////////////////////////
// Forward Declarations
//////////////////////////////////////
class CShaderManager;

//////////////////////////////////////
// Class Definition
//////////////////////////////////////
class CTerrain : public IRenderable, public ICollidable
{
public:
	CTerrain(void);
	~CTerrain(void);

private:
	int GetHeightMapEntry(int row, int col);
	void SetHeightMapEntry(int row, int col, int &value);
	bool SetupMesh(LPDIRECT3DDEVICE9 device);

	std::vector<int>			m_heightMap;
	std::string					m_strFilename;
	int							m_numVertsPerRow;
	int							m_numOfRows;
	int							m_numOfVerts;
	float						m_fVertexSpacing;
	float						m_fYScale;
	float						m_fUVScale;
	float						m_fYOffset;
	UINT						m_numTriangles;
	ABB_MaxMin					m_abbBounds;
	
	LPDIRECT3DVERTEXBUFFER9		m_VertexBuffer;
	LPDIRECT3DINDEXBUFFER9		m_IndexBuffer;
	LPDIRECT3DTEXTURE9			m_texGroundTexture;
	//LPDIRECT3DTEXTURE9			m_texHeightTexture;
	LPDIRECT3DTEXTURE9			m_texDirtTexture;
	LPDIRECT3DVERTEXBUFFER9		m_debugNormalsVB;
	
public:
	float GetTerrainHeightAtXZ(float xCoord, float ZCoord);
	/*Deprecated*/bool LoadTerrain(LPCSTR pFilename, LPCSTR pTextureFilename, int rows, int cols, float vertSpacing, float fYScale, float uvScale, LPDIRECT3DDEVICE9 device);
	void SetYOffset(float offset) {m_fYOffset = offset;}
	ABB_MaxMin CalculateBoundingBox(void);
	
	virtual D3DXMATRIX GetWorldTransform() {D3DXMATRIX worldMatrix;	D3DXMatrixIdentity(&worldMatrix); return worldMatrix;}
	virtual void Render( CRenderEngine &rndr );
	virtual bool IsTransparent() {return false;}
	virtual void SetLastRenderFrame(UINT frameNum) {};
	virtual UINT GetLastRenderFrame() {return 0;}
	virtual Sphere_PosRad GetBoundingSphere() { Sphere_PosRad s; ::ZeroMemory(&s, sizeof(s)); return s; }

	// ICollidable
	virtual void GetCollideSphere( Sphere_PosRad& out ) {out = GetBoundingSphere();}
	virtual void HandleCollision( ICollidable* other ) {};
	};