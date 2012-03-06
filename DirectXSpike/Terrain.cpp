#include "StdAfx.h"

#include <fstream>

#include "Terrain.h"
#include "ShaderManager.h"
#include "mathutil.h"
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

CTerrain::CTerrain(void) : m_VertexBuffer(NULL),
							m_IndexBuffer(NULL),
							m_fVertexSpacing(0.0f),
							m_fYScale(1.0f),
							m_fUVScale(1.0f),
							m_fYOffset(0.0f),
							m_debugNormalsVB(NULL),
							m_texGroundTexture(NULL),
							m_texDirtTexture(NULL)
{
	collidableType = OBSTACLE;
}

CTerrain::~CTerrain(void)
{
	COM_SAFERELEASE(m_VertexBuffer);
	COM_SAFERELEASE(m_IndexBuffer);
}

ABB_MaxMin CTerrain::CalculateBoundingBox(void)
{
	return m_abbBounds;
}

/* Deprecated - Use LevelLoader::LoadTerrain for future work */
bool CTerrain::LoadTerrain(LPCSTR pFilename, LPCSTR pTextureFilename, int rows, int cols, float vertSpacing, float fYScale, float uvScale, LPDIRECT3DDEVICE9 device)
{
	assert(device);
	assert(pTextureFilename != NULL);

	m_strFilename		= RESOURCE_FOLDER;
	m_strFilename       += pFilename;
	
	m_numOfRows			= rows;
	m_numVertsPerRow	= cols;
	m_numOfVerts		= m_numVertsPerRow * m_numOfRows;
	m_fVertexSpacing	= vertSpacing;
	m_fYScale			= fYScale;
	m_fUVScale			= uvScale;
	m_numTriangles		= 2 * (m_numVertsPerRow-1)*(m_numOfRows-1);

	//Load the height texture
	// texture
	TextureContextIdType texContext;
	CGameStateStack::GetInstance()->GetCurrentState()->HandleEvent(EVT_GETTEXCONTEXT, &texContext, sizeof(texContext));
	CTextureManager* texMgr = CTextureManager::GetInstance();
	m_texGroundTexture = texMgr->GetTexture(texContext, pTextureFilename);

	//Load the ground texture
	m_texDirtTexture = texMgr->GetTexture(texContext, "dirtTexture.dds");
	
	// read the height info in from a grayscale image (white is high vertical, dark is low)
	std::vector<BYTE> arrBinary(m_numOfVerts);
	InputFileType terrainFile;
	VERIFY(FileIO::OpenFile(m_strFilename.c_str(), true, &terrainFile));
	VERIFY(FileIO::ReadBytes(&terrainFile, (char*)&arrBinary[0], arrBinary.size()));
	FileIO::CloseFile(&terrainFile);

	m_heightMap.resize(m_numOfVerts);
	for(int i = 0, j = m_heightMap.size(); i < j; i++)
	{
		m_heightMap[i] = (int)arrBinary[i];
	}

	if(SetupMesh(device) == false)
		return false;

	return true;
}

int CTerrain::GetHeightMapEntry(int row, int col)
{
	if(row > m_numOfRows - 1 || col > m_numVertsPerRow - 1 || row < 0 || col < 0)
		return 0;

	unsigned int index = row * m_numVertsPerRow + col;

	return m_heightMap[index];
}

void CTerrain::SetHeightMapEntry(int row, int col, int &value)
{
	int index = row * m_numVertsPerRow + col;
	m_heightMap[index] = value;
}

bool CTerrain::SetupMesh(LPDIRECT3DDEVICE9 device)
{
	HR(device->CreateVertexBuffer(m_numOfVerts*sizeof(TerrainVertex), D3DUSAGE_WRITEONLY, TerrainVertex::FVF, D3DPOOL_MANAGED, &m_VertexBuffer, 0));
	TerrainVertex *vertices;
	m_VertexBuffer->Lock(0, 0, (void**)&vertices, 0);

	const float _width			= (m_numVertsPerRow - 1) * m_fVertexSpacing;
	const float _height			= (m_numOfRows - 1) * m_fVertexSpacing;
	const float _startXCoord	= -_width / 2;
	const float _startZCoord	= _height / 2;
	const float _uvSpacing		= m_fVertexSpacing / m_fUVScale;
	const float _uv2Spacing		= 1.0f / 50.0f;
	const float _uv3Spacing		= m_fVertexSpacing / m_fUVScale;

	// iterate through rows
	float zPos = _startZCoord;
	float vPos = 0, v2Pos = 0, v3Pos = 0;
	::ZeroMemory(&m_abbBounds, sizeof(m_abbBounds));
	/* Make a copy so we don't lose the initial address */
	TerrainVertex *workingVertices = vertices; 
	{
		for(int i = 0, j = m_numOfRows; i < j; i++)
		{
			// iterate through cols in each row
			float xPos = _startXCoord;
			float uPos = 0, u2Pos = 0, u3Pos = 0;
			for(int k = 0, l = m_numVertsPerRow; k < l; k++)
			{
				// fill vertex out
				TerrainVertex vertex;
				vertex._p.x = xPos;
				vertex._p.z = zPos;
				vertex._p.y = m_fYOffset + m_fYScale * (float)GetHeightMapEntry(k, i);
				
				// bounding box
				ABB_GrowToInclude3DPoint(vertex._p, m_abbBounds);

				// normals and textures
				vertex._n = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
				vertex._t1 = D3DXVECTOR2(uPos, vPos);
				vertex._t2 = D3DXVECTOR2(u2Pos, v2Pos);
				vertex._t3 = D3DXVECTOR2(u3Pos, v3Pos);
				
				(*workingVertices) = vertex;
				workingVertices++;
				
				uPos += _uvSpacing;
				u2Pos += _uv2Spacing;
				u3Pos += _uv3Spacing;
				xPos += m_fVertexSpacing;
			}

			vPos += _uvSpacing;
			v2Pos += _uv2Spacing;
			v3Pos += _uv3Spacing;
			zPos -= m_fVertexSpacing;
		}
	}

	// calculate # of indices
	int numIndices = 3 * m_numTriangles;
	HR( device->CreateIndexBuffer(numIndices * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_IndexBuffer, 0) );
	WORD *indices;
	HR( m_IndexBuffer->Lock(0, 0, (void**)&indices, 0) );
	
	// loop through each row
	int baseIndex = 0;
	for(int i = 0, j = m_numOfRows-1; i < j; i++)
	{
		// loop through each cell in row
		for(int k = 0, l = m_numVertsPerRow-1; k < l; k++)
		{
			// top triangle
			indices[baseIndex]		= k			+ m_numVertsPerRow*(i+1); // 0,1 k, i+row
			indices[baseIndex+1]	= k			+ i*m_numVertsPerRow; // 0,0 k, i
			indices[baseIndex+2]	= k+1		+ i*m_numVertsPerRow; // 1,0 k+1, i

			//bottom triangle
			indices[baseIndex+3]	= k			+ m_numVertsPerRow*(i+1); // 0,1 k, i+row
			indices[baseIndex+4]	= k+1		+ i*m_numVertsPerRow; // 1,0 k+1, i
			indices[baseIndex+5]	= k+1		+ m_numVertsPerRow*(i+1); // 1,1 k+1, i+row

			baseIndex += 6;
		}
	}
	
	// Now build the normals by averaging the face normals of each triangle vertices
	Polygon_ABC poly;
	for(unsigned int i = 0; i < m_numTriangles; i++)
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
		
	for(int i = 0; i < m_numOfVerts; i++)
	{
		D3DXVec3Normalize(&vertices[i]._n, &vertices[i]._n);
	}

	m_VertexBuffer->Unlock();
	m_IndexBuffer->Unlock();

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

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Update Functions
//////////////////////////////////////////////////////////////////////////

float CTerrain::GetTerrainHeightAtXZ(float xCoord, float zCoord)
{
	// Find which subcell we are in
	const float _width			= (m_numVertsPerRow - 1) * m_fVertexSpacing;
	const float _height			= (m_numOfRows - 1) * m_fVertexSpacing;
	D3DXVECTOR3 clampPointWorking(xCoord, 0.0f, -zCoord); // flip the z axis so it matches up with the height map texture space
	clampPointWorking += D3DXVECTOR3(_width/2.0f, 0.0f, _height/2.0f); // translate top left point to origin
	if(clampPointWorking.x < 0 || clampPointWorking.x > _width || clampPointWorking.z < 0 || clampPointWorking.z > _height)
		return 0.0f;

	clampPointWorking /= m_fVertexSpacing;  // unscale the collision point, so we don't have to continue to factor it

	int colSubcell = (int)::floorf(clampPointWorking.x); 
	int rowSubcell = (int)::floorf(clampPointWorking.z);

	// find which triangle we are in
	//TL---TR--->+x
	//|    /|
	//|  /  |
	//|/    |
	//BL---BR
	//|
	//V +Z
	D3DXVECTOR3 cellOriginOffset((float)colSubcell, 0.0f, (float)rowSubcell);
	
	// translate the collision point to cell local space
	clampPointWorking -= cellOriginOffset; 

	// find the shared vertices and translate them to cell local space
	D3DXVECTOR3 bottomLeft((float)colSubcell, m_fYOffset + m_fYScale * (float)GetHeightMapEntry(colSubcell, rowSubcell+1), (float)(rowSubcell+1));
	D3DXVECTOR3 topRight((float)(colSubcell+1), m_fYOffset + m_fYScale * (float)GetHeightMapEntry(colSubcell+1, rowSubcell), (float)rowSubcell);
	bottomLeft -= cellOriginOffset;
	topRight -= cellOriginOffset;
	float retYCoord(0.0f);
	if(clampPointWorking.x + clampPointWorking.z < 1.0f)
	{
		// top left triangle
		D3DXVECTOR3 topLeft((float)colSubcell, m_fYOffset + m_fYScale * (float)GetHeightMapEntry(colSubcell, rowSubcell), (float)rowSubcell);
		topLeft -= cellOriginOffset;
		retYCoord = lerp(0.0f, topRight.y - topLeft.y, clampPointWorking.x) + lerp(0.0f, bottomLeft.y - topLeft.y, clampPointWorking.z) + topLeft.y;
	}
	else
	{
		// bottom right triangle
		D3DXVECTOR3 bottomRight((float)(colSubcell+1), m_fYOffset + m_fYScale * (float)GetHeightMapEntry(colSubcell+1, rowSubcell+1), (float)(rowSubcell+1));
		bottomRight -= cellOriginOffset;
		retYCoord = lerp(0.0f, bottomLeft.y - bottomRight.y, 1-clampPointWorking.x) + lerp(0.0f, topRight.y - bottomRight.y, 1-clampPointWorking.z) + bottomRight.y;
	}	

	return retYCoord;
}

//////////////////////////////////////////////////////////////////////////
// Render Functions
//////////////////////////////////////////////////////////////////////////

void CTerrain::Render( CRenderEngine &rndr )
{
	LPDIRECT3DDEVICE9 device = rndr.GetDevice();
	CShaderManagerEx &shaderMgr = rndr.GetShaderManager();
	CCamera &camera = rndr.GetSceneManager().GetDefaultCamera();
	assert(device);
	
	D3DXMATRIX worldMatrix;
	D3DXMatrixIdentity(&worldMatrix);
	shaderMgr.SetWorldTransformEx(worldMatrix);

	device->SetStreamSource(0, m_VertexBuffer, 0, sizeof(TerrainVertex));
	device->SetIndices(m_IndexBuffer);
	device->SetFVF(TerrainVertex::FVF);

	device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
	device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
	device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
	device->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	device->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	device->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
	device->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
	device->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
	//device->SetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	//device->SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	//device->SetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
	//device->SetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
	//device->SetSamplerState(2, D3DSAMP_MIPFILTER, D3DTEXF_POINT);

	D3DXCOLOR matColor = D3DXCOLOR(0.8f, 1.0f, 0.8f, 1.0f);
	D3DMATERIAL9 terrainMaterial;
	terrainMaterial.Ambient		= matColor;
	terrainMaterial.Diffuse		= matColor;
	shaderMgr.SetMaterialEx(terrainMaterial);
	
	device->SetTexture(0, m_texGroundTexture);
//	device->SetTexture(1, m_texHeightTexture);
	device->SetTexture(1, m_texDirtTexture);

	assert(false);	// TODO: add begin/end effect and draw support

	device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_numOfVerts, 0, m_numTriangles);

	device->SetTexture(0, 0);
	device->SetTexture(1, 0);
	//device->SetTexture(2, 0);

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