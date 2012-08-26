#pragma once
/********************************************************************
	created:	2012/04/23
	filename: 	TerrainPatch.h
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
//////////////////////////////////////
// Includes
//////////////////////////////////////
#include "IRenderable.h"
#include "ICollidable.h"
#include "LevelLoader.h"
#include "VertexTypes.h"

//////////////////////////////////////
// Forward Declarations
//////////////////////////////////////
class CShaderManager;

//////////////////////////////////////
// Constants
//////////////////////////////////////

//////////////////////////////////////
// Types
//////////////////////////////////////
typedef Rect TerrainTextureBlock;

//////////////////////////////////////
// Class Definition
//////////////////////////////////////
class CTerrainPatch : public IRenderable, public ICollidable
{
	typedef struct _SampleData
	{
		LevelHeightMap				heightMap;
		std::string					strFilename;
		//int							m_numVertsPerRow;
		int							blockSize;	
	} SampleData;

	typedef struct _TerrainTransforms
	{
		/* scale */
		float						fVertexSpacing;
		float						fYScale;

		/* translation */
		float						fYOffset;
		Matrix4x4					worldMatrix;
	} TerrainTransforms;

	typedef struct _TerrainModel
	{
		LPDIRECT3DVERTEXBUFFER9		vertexBuffer;
		LPDIRECT3DINDEXBUFFER9		indexBuffer;
		UINT						numTriangles;
		bool						isLoaded;
		LPDIRECT3DVERTEXBUFFER9		debugNormalsVB;
	} TerrainModel;

	typedef struct _TextureDefinition
	{
		LPDIRECT3DTEXTURE9			texHnd;
		float						uvScale;
	} TextureDefinition;

public:
	CTerrainPatch(void);
	~CTerrainPatch(void);

private:
	int GetHeightMapEntry(int row, int col);
	void SetHeightMapEntry(int row, int col, int &value) { m_sampleData.heightMap[row * m_sampleData.blockSize + col] = value;}

	/* collision */
	ABB_MaxMin					m_abbBounds;
	
	TextureDefinition			m_textures[TEX_LAYERS_CNT];
	SampleData					m_sampleData;
	TerrainModel				m_terrainModel;
	TerrainTransforms			m_transforms;

	//LPDIRECT3DTEXTURE9		m_texGroundTexture;
	//LPDIRECT3DTEXTURE9		m_texHeightTexture;
	
public:
	float GetTerrainHeightAtXZ(float xCoord, float ZCoord);
	void LoadHeightMapArea( LPCSTR pFilename, int width, int height, float vertSpacing, float fYScale, TerrainTextureBlock area );
	void LoadMesh(bool load = true);
	void LoadTextures();

	inline ABB_MaxMin GetBoundingBox(void) {return m_abbBounds;}
	inline void SetYOffset(float offset) {m_transforms.fYOffset = offset;}
	inline bool IsRenderMeshLoaded() {return m_terrainModel.isLoaded;}
	inline bool IsTerrainSamplesLoaded() {return m_sampleData.strFilename.length()>0;}
	
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