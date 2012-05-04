#pragma once
/********************************************************************
	created:	2012/04/23
	filename: 	TerrainChunk.h
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
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
class CTerrainChunk : public IRenderable, public ICollidable
{
public:
	CTerrainChunk(void);
	~CTerrainChunk(void);

private:
	int GetHeightMapEntry(int row, int col);
	void SetHeightMapEntry(int row, int col, int &value);
	bool SetupMesh();

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
	Matrix4x4					m_worldMatrix;
	
	LPDIRECT3DVERTEXBUFFER9		m_VertexBuffer;
	LPDIRECT3DINDEXBUFFER9		m_IndexBuffer;
	LPDIRECT3DTEXTURE9			m_texGroundTexture;
	//LPDIRECT3DTEXTURE9			m_texHeightTexture;
	LPDIRECT3DTEXTURE9			m_texDirtTexture;
	LPDIRECT3DVERTEXBUFFER9		m_debugNormalsVB;
	
public:
	float GetTerrainHeightAtXZ(float xCoord, float ZCoord);
	void SetYOffset(float offset) {m_fYOffset = offset;}
	ABB_MaxMin CalculateBoundingBox(void);
	/* deprecated */bool LoadTerrain( LPCSTR pFilename, LPCSTR pTextureFilename, int rows, int cols, float vertSpacing, float fYScale, float uvScale );
	
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