/********************************************************************
	created:	2012/05/22
	filename: 	TerrainPatch.cpp
	author:		Matthew Alford
	
	purpose:	Represents a 64x64 meter (world unit) square of
				terrain.  This entity can be collided with, and 
				rendered with texture mapping.  Patches use the 
				LevelLoader to load their subset of height-map values 
				from a gray-scale height texture.  The transforms are 
				baked into the vertices	positions at the time they 
				are loaded to the GPU, and the shader can omit the 
				world transform.  This is possible because the terrain 
				currently is constrained to be stationary.
*********************************************************************/
#include "StdAfx.h"

#include <fstream>

#include "TerrainPatch.h"
#include "ShaderManager.h"
#include "vertextypes.h"
#include "renderengine.h"
#include "fileio.h"
#include "texturemanager.h"
#include "gamestatestack.h"

// Uncomment this to draw surface normals
//#define DEBUGTERRAIN

//////////////////////////////////////////////////////////////////////////
// Setup Functions
//////////////////////////////////////////////////////////////////////////

CTerrainPatch::CTerrainPatch(void)
{
	collidableType = OBSTACLE;
	::ZeroMemory(m_textures, sizeof(m_textures));
	::ZeroMemory(&m_sampleData, sizeof(m_sampleData));
	::ZeroMemory(&m_transforms, sizeof(m_transforms));
	::ZeroMemory(&m_terrainModel, sizeof(m_terrainModel));
}

CTerrainPatch::~CTerrainPatch(void)
{
	LoadMesh(false); // unload
}

/************************************************
*   Name:   CTerrainPatch::LoadHeightMapArea
*   Desc:   Loads the heightmap for a given 
*			sub-area.  
*   Notes:  BlockSize = w = h.
************************************************/
void CTerrainPatch::LoadHeightMapArea( LPCSTR pFilename, int width, int height, float vertSpacing, float fYScale, TerrainTextureBlock area )
{
	/* the read area is increased by 1 pixel in width and height for shared vertices */
	TerrainTextureBlock readArea = area;
	readArea.wh.x++;
	readArea.wh.y++;
	DASSERT(readArea.wh.x == readArea.wh.y);

	/* update local state */
	m_sampleData.strFilename	= RESOURCE_FOLDER;
	m_sampleData.strFilename	+= pFilename;
	m_sampleData.blockSize		= (int)area.wh.x;

	m_transforms.fVertexSpacing	= vertSpacing;
	//m_transforms.fYScale		= fYScale;
	m_transforms.fYScale = 1.0f;

	m_terrainModel.numTriangles	= 2 * m_sampleData.blockSize * m_sampleData.blockSize;

	/* Load the height texture (displacement map).  Read the height 
	   info in from a gray-scale image (white is high vertical, dark is low) */
	LevelLoader::GetInstance()->ReadTerrainHeightMapFromRaw(m_sampleData.strFilename.c_str(), width, height, readArea, &m_sampleData.heightMap);
	
}

/************************************************
*   Name:   CTerrainPatch::LoadTextures
*   Desc:   Load the textures needed by this 
*			terrain patch.
************************************************/
void CTerrainPatch::LoadTextures()
{
	/* Get the current texture context and load the 
	   ground textures and materials. */
	TextureContextIdType texContext;
	CGameStateStack::GetInstance()->GetCurrentState()->HandleEvent(EVT_GETTEXCONTEXT, &texContext, sizeof(texContext));
	CTextureManager *texMgr = CTextureManager::GetInstance();

	/* setup a single texture layer for now */
	m_textures[0].texHnd = texMgr->GetTexture(texContext, "groundTexture.dds");
	m_textures[0].uvScale = m_transforms.fVertexSpacing / 30.0f/*old m_fUVScale*/;

	//"dirtTexture.dds"
	/*const float _uv2Spacing		= 1.0f / 50.0f;
	const float _uv3Spacing		= m_fVertexSpacing / m_fUVScale;*/
}

/************************************************
*   Name:   CTerrainPatch::LoadMesh
*   Desc:   Calculate the mesh vertices and load 
*			the mesh data to VRAM.
*	Notes:	Calculates the vertex position,
*			normals, and UV coordinates.
************************************************/
void CTerrainPatch::LoadMesh(bool load/*= true*/)
{
	const int numVertsPerRowCol = m_sampleData.blockSize+1;

	if(!load)
	{
		COM_SAFERELEASE(m_terrainModel.vertexBuffer);
		COM_SAFERELEASE(m_terrainModel.indexBuffer);
		m_terrainModel.isLoaded = false;
		return;
	}

	const float _width			= m_sampleData.blockSize * m_transforms.fVertexSpacing;
	const float _height			= _width;
	const float _startXCoord	= -_width / 2;
	const float _startZCoord	= _height / 2;
	const int numVerts			= numVertsPerRowCol * numVertsPerRowCol;

	/* create the vram vertex buffer */
	dxCreateVertexBuffer(numVerts*sizeof(TerrainVertex), D3DUSAGE_WRITEONLY, TerrainVertex::FVF, D3DPOOL_MANAGED, &m_terrainModel.vertexBuffer, 0);
	TerrainVertex *vertices;
	HR( m_terrainModel.vertexBuffer->Lock(0, 0, (void**)&vertices, 0) );

	/* iterate through rows */
	float zPos = _startZCoord;
	const float yScale = m_transforms.fYScale;
	const float yTranslate = m_transforms.fYOffset;
	::ZeroMemory(&m_abbBounds, sizeof(m_abbBounds));

	/* Make a copy so we don't lose the initial address */
	TerrainVertex *workingVertices = vertices;
	{
		for(int i = 0; i < numVertsPerRowCol; i++)
		{
			/* iterate through cols in each row */
			float xPos = _startXCoord;
			//float uPos = 0, u2Pos = 0, u3Pos = 0;
			for(int j = 0; j < numVertsPerRowCol; j++)
			{
				/* fill vertex out */
				TerrainVertex vertex;
				vertex._p.x = xPos;
				vertex._p.z = zPos;
				vertex._p.y = yTranslate + yScale * (float)GetHeightMapEntry(j, i);
				
				/* pre-calculate bounding box */
				ABB_GrowToInclude3DPoint(vertex._p, m_abbBounds);

				/* add normals and texture coords */
				vertex._n = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

				for(int t = 0; t < TEX_LAYERS_CNT; t++)
				{
					vertex._t[t] = D3DXVECTOR2(m_textures[t].uvScale * j, m_textures[t].uvScale * i);
				}
				
				/* add the vertex definition */
				(*workingVertices) = vertex;
				workingVertices++;
				
				/* go to next cell */
				xPos += m_transforms.fVertexSpacing;
			}

			/* go to the next row */
			zPos -= m_transforms.fVertexSpacing;
		}
	}

	/* calculate # of indices */
	int numIndices = 3 * m_terrainModel.numTriangles;
	dxCreateIndexBuffer(numIndices * sizeof(__int16), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_terrainModel.indexBuffer, 0);
	__int16 *indices = NULL;
	HR( m_terrainModel.indexBuffer->Lock(0, 0, (void**)&indices, 0) );
	
	/* loop through each row */
	int baseIndex = 0;
	const int size = m_sampleData.blockSize;
	for(int i = 0; i < size; i++)
	{
		/* loop through each cell in row */
		for(int k = 0; k < size; k++)
		{
			/* top triangle */
			indices[baseIndex]		= k			+ size*(i+1);	// 0,	1 k,	i+row
			indices[baseIndex+1]	= k			+ i*size;		// 0,	0 k,	i
			indices[baseIndex+2]	= k+1		+ i*size;		// 1,	0 k+1,	i

			/* bottom triangle */
			indices[baseIndex+3]	= k			+ size*(i+1);	// 0,	1 k,	i+row
			indices[baseIndex+4]	= k+1		+ i*size;		// 1,	0 k+1,	i
			indices[baseIndex+5]	= k+1		+ size*(i+1);	// 1,	1 k+1,	i+row

			baseIndex += 6;
		}
	}
	
	/* build the normals by averaging the face normals of each triangle vertices */
	Polygon_ABC poly;
	for(unsigned int i = 0; i < m_terrainModel.numTriangles; i++)
	{
		unsigned int idx = 3/*stride*/ * i;
		int indexVert0 = indices[idx];
		int indexVert1 = indices[idx + 1];
		int indexVert2 = indices[idx + 2];

		poly.a = vertices[indexVert0]._p;
		poly.b = vertices[indexVert1]._p;
		poly.c = vertices[indexVert2]._p;

		D3DXVECTOR3 vecNorm = Polygon_CalcNormalVec( poly );
		vertices[indexVert0]._n += vecNorm;
		vertices[indexVert1]._n += vecNorm;
		vertices[indexVert2]._n += vecNorm;
	}
		
	for(int i = 0; i < numVerts; i++)
	{
		D3DXVec3Normalize(&vertices[i]._n, &vertices[i]._n);
	}

	m_terrainModel.vertexBuffer->Unlock();
	m_terrainModel.indexBuffer->Unlock();

	m_terrainModel.isLoaded = true;

#ifdef DEBUGTERRAIN
	//DEBUG DRAW NORMALS
	HR( device->CreateVertexBuffer(m_numOfVerts*6*(sizeof(float)), D3DUSAGE_WRITEONLY, D3DFVF_XYZ, D3DPOOL_MANAGED, &m_debugNormalsVB, 0) );
	D3DXVECTOR3* coord = NULL;
	HR( m_debugNormalsVB->Lock(0, 0, (void**)&coord, 0) );
	
	for(int j = 0; j < m_numOfVerts; j++)
	{
		int index = 2*j;
		coord[index] = vertices[j]._p;
		coord[index+1] = vertices[j]._p + 2*vertices[j]._n;
	}

	m_debugNormalsVB->Unlock();
#endif
}

//////////////////////////////////////////////////////////////////////////
// Update Functions
//////////////////////////////////////////////////////////////////////////

/************************************************
*   Name:   CTerrainPatch::GetTerrainHeightAtXZ
*   Desc:   Calculate the y-value at a particular
*			(x,z) world position.
************************************************/
float CTerrainPatch::GetTerrainHeightAtXZ(float xCoord, float zCoord)
{
	/* Find which subcell we are in */
	const int blockSize			= m_sampleData.blockSize;
	const float yScale			= m_transforms.fYScale;
	const float yTranslate		= m_transforms.fYOffset;
	const float _width			= (blockSize - 1) * m_transforms.fVertexSpacing;
	const float _height			= (blockSize - 1) * m_transforms.fVertexSpacing;
	D3DXVECTOR3 clampPointWorking(xCoord, 0.0f, -zCoord); // flip the z axis so it matches up with the height map texture space
	clampPointWorking += D3DXVECTOR3(_width/2.0f, 0.0f, _height/2.0f); // translate top left point to origin
	if(clampPointWorking.x < 0 || clampPointWorking.x > _width || clampPointWorking.z < 0 || clampPointWorking.z > _height)
		return 0.0f;

	clampPointWorking /= m_transforms.fVertexSpacing;  // unscale the collision point, so we don't have to continue to factor it

	int colSubcell = (int)::floorf(clampPointWorking.x); 
	int rowSubcell = (int)::floorf(clampPointWorking.z);

	/* find which triangle we are in	*/
	/*									*/
	/*		TL---TR---> +X				*/
	/*		|    /|						*/
	/*		|  /  |						*/
	/*		|/    |						*/
	/*		BL---BR						*/
	/*		|							*/
	/*		V +Z						*/
	D3DXVECTOR3 cellOriginOffset((float)colSubcell, 0.0f, (float)rowSubcell);
	
	/* translate the collision point to cell local space */
	clampPointWorking -= cellOriginOffset; 

	/* find the shared vertices and translate them to cell local space */
	D3DXVECTOR3 bottomLeft((float)colSubcell, yTranslate + yScale * (float)GetHeightMapEntry(colSubcell, rowSubcell+1), (float)(rowSubcell+1));
	D3DXVECTOR3 topRight((float)(colSubcell+1), yTranslate + yScale * (float)GetHeightMapEntry(colSubcell+1, rowSubcell), (float)rowSubcell);
	bottomLeft -= cellOriginOffset;
	topRight -= cellOriginOffset;
	float retYCoord(0.0f);
	if(clampPointWorking.x + clampPointWorking.z < 1.0f)
	{
		/* top left triangle */
		D3DXVECTOR3 topLeft((float)colSubcell, yTranslate + yScale * (float)GetHeightMapEntry(colSubcell, rowSubcell), (float)rowSubcell);
		topLeft -= cellOriginOffset;
		retYCoord = lerp(0.0f, topRight.y - topLeft.y, clampPointWorking.x) + lerp(0.0f, bottomLeft.y - topLeft.y, clampPointWorking.z) + topLeft.y;
	}
	else
	{
		/* bottom right triangle */
		D3DXVECTOR3 bottomRight((float)(colSubcell+1), yTranslate + yScale * (float)GetHeightMapEntry(colSubcell+1, rowSubcell+1), (float)(rowSubcell+1));
		bottomRight -= cellOriginOffset;
		retYCoord = lerp(0.0f, bottomLeft.y - bottomRight.y, 1-clampPointWorking.x) + lerp(0.0f, topRight.y - bottomRight.y, 1-clampPointWorking.z) + bottomRight.y;
	}	

	return retYCoord;
}

/************************************************
*   Name:   CTerrainPatch::GetHeightMapEntry
*   Desc:   Lookup a height map value from the 
*			database by the x/y indices.
************************************************/
int CTerrainPatch::GetHeightMapEntry(int row, int col)
{
	if(row > m_sampleData.blockSize || col > m_sampleData.blockSize || row < 0 || col < 0)
		return 0;

	unsigned int index = row * m_sampleData.blockSize + col;

	return m_sampleData.heightMap[index];
}


//////////////////////////////////////////////////////////////////////////
// Render Functions
//////////////////////////////////////////////////////////////////////////

/************************************************
*   Name:   CTerrainPatch::Render
*   Desc:   Render this terrain chunk
************************************************/
void CTerrainPatch::Render( CRenderEngine &rndr )
{
	/* set the shading technique */
	CShaderManagerEx &shaderMgr = rndr.GetShaderManager();
	shaderMgr.SetEffect( EFFECT_LIGHTTEX );
	shaderMgr.SetDefaultTechnique();

	/* set up the geometry streams */
	dxSetStreamIndexedSource(0, m_terrainModel.indexBuffer, m_terrainModel.vertexBuffer, 0, sizeof(TerrainVertex));
	dxFVF(TerrainVertex::FVF);

	/* set the render-states */
	dxCullMode(D3DCULL_CCW);

	/* set light constants */
	D3DXVECTOR3 toLight = D3DXVECTOR3(0,0,0) - rndr.GetLightDirection();
	shaderMgr.SetLightDirection(*D3DXVec3Normalize(&toLight, &toLight));
	
	/* set transforms */
	Matrix4x4 world;
	Matrix4x4_LoadIdentity(&world);
	shaderMgr.SetWorldTransformEx(world); // TODO: remove from terrain shader

	CCamera &camera = rndr.GetSceneManager()->GetDefaultCamera();
	shaderMgr.SetViewProjectionEx( camera.GetViewMatrix(), camera.GetProjectionMatrix() );

	/* set the texture units */
	for(int i = 0; i < TEX_LAYERS_CNT; i++)
	{
		shaderMgr.SetTexture(_sprintf(0, "tex%d", i), m_textures[i].texHnd);
	}
	
	//device->SetTexture(1, m_texHeightTexture);
	//device->SetTexture(1, m_texDirtTexture);

	/* texture sampler state */
	dxSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	dxSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	dxSetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
	dxSetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
	dxSetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
	//device->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	//device->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	//device->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
	//device->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
	//device->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
	//device->SetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	//device->SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	//device->SetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
	//device->SetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
	//device->SetSamplerState(2, D3DSAMP_MIPFILTER, D3DTEXF_POINT);

	/* set material constants */
	D3DXCOLOR matColor = D3DXCOLOR(0.8f, 1.0f, 0.8f, 1.0f);
	D3DMATERIAL9 terrainMaterial;
	terrainMaterial.Ambient		= matColor;
	terrainMaterial.Diffuse		= matColor;
	shaderMgr.SetMaterialEx(terrainMaterial);

	/* Render each pass */
	int numPasses = shaderMgr.BeginEffect();
	const int blockSize = m_sampleData.blockSize;
	for( int i = 0; i < numPasses; i++ )
	{
		shaderMgr.Pass(i);
		dxDrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, blockSize*blockSize, 0, m_terrainModel.numTriangles);
		shaderMgr.FinishPass();
	}
	shaderMgr.FinishEffect();

#ifdef DEBUGTERRAIN
	//DEBUG DRAW NORMALS
	shaderMgr.SetVertexShader(PASS_PRIM_COLORED);
	shaderMgr.SetPixelShader(PASS_PRIM_COLORED);
	shaderMgr.SetViewProjection(PASS_PRIM_COLORED, camera.GetViewMatrix(), camera.GetProjectionMatrix());
	shaderMgr.SetWorldTransform(PASS_PRIM_COLORED, worldMatrix);
	shaderMgr.SetDrawColor(PASS_PRIM_COLORED, D3DXCOLOR(0.2f, 0.7f, 0.2f, 1.0f));
		
	device->SetStreamSource(0, m_debugNormalsVB, 0, 3*sizeof(float));
	device->SetFVF(D3DFVF_XYZ);
	device->DrawPrimitive(D3DPT_LINELIST, 0, m_numOfVerts);

	shaderMgr.SetVertexShader(PASS_DEFAULT);
	shaderMgr.SetPixelShader(PASS_DEFAULT);
#endif
}