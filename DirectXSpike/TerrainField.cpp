/********************************************************************
	created:	2012/05/22
	filename: 	TerrainField.cpp
	author:		Matthew Alford
	
	purpose:	Collection of Terrain Patches in a 3x3 grid.  Patches
				are chosen according to the avatar's position, and
				the field constrains avatar to center chunk.  Routes
				collision queries to the appropriate patch.  Broadcasts
				grid change events to listeners, allowing for the scene
				tree to be updated.  Maintains a cache of previously
				visited patches to allow collision to function after
				the patch's mesh was unloaded.
*********************************************************************/
#include "StdAfx.h"
#include "TerrainField.h"
#include "TerrainMiscTypes.h"
#include "CollisionManager.h"
#include "TerrainPatch.h"
#include "SceneManager.h"

static const float Y_SCALE		= 2.0f;
static const float VERT_SPACING = 5.0f;
static const int BLOCK_SIZE		= 64;		/* 64x64 blocks */

compiler_assert(TERRAIN_GRID_ROWS&1 && TERRAIN_GRID_COLS&1, terrain_field_cpp);

#define CENTER_CHUNK (m_currentGrid[(TERRAIN_GRID_ROWS+1)>>1][(TERRAIN_GRID_COLS+1)>>1])

//////////////////////////////////////////////////////////////////////////
// CTerrainContainer class
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// Setup Functions
//////////////////////////////////////////////////////////////////

CTerrainContainer::CTerrainContainer(TerrainGrid grid)
{
	for(int i = 0; i < cnt_of_array(grid[0]); i++)
		for(int j = 0; j < cnt_of_array(grid[i]); j++)
		{
			m_terrainGrid[i][j] = grid[i][j];
		}
}

//////////////////////////////////////////////////////////////////
// Update Functions
//////////////////////////////////////////////////////////////////

float CTerrainContainer::GetTerrainHeightAtXZ( float xCoord, float zCoord )
{
	Point_3D pt;
	for(int i = 0; i < cnt_of_array(m_terrainGrid[0]); i++)
		for(int j = 0; j < cnt_of_array(m_terrainGrid[i]); j++)
		{
			if(!m_terrainGrid[i][j])
				continue;

			pt.x = xCoord;
			pt.z = zCoord;
			pt.y = m_terrainGrid[i][j]->GetBoundingSphere().pos.y;
			if(ABB_PointInsideABB(pt, m_terrainGrid[i][j]->GetBoundingBox()))
			{
				return m_terrainGrid[i][j]->GetTerrainHeightAtXZ(xCoord,zCoord);
			}
		}

	return 0;
}

ABB_MaxMin CTerrainContainer::CalculateBoundingBox(void)
{
	assert(m_terrainGrid[1][1]);
	ABB_MaxMin abb = m_terrainGrid[1][1]->GetBoundingBox();

	for(int i = 0; i < cnt_of_array(m_terrainGrid[0]); i++)
		for(int j = 0; j < cnt_of_array(m_terrainGrid[i]); j++)
		{
			if(!m_terrainGrid[i][j])
				continue;

			ABB_GrowToIncludeABB(m_terrainGrid[i][j]->GetBoundingBox(), abb);
		}

	return abb;
}

//////////////////////////////////////////////////////////////////////////
// CTerrainField Class
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// Setup Functions
//////////////////////////////////////////////////////////////////
CTerrainField::CTerrainField(void) : m_fYOffset(0.0f),
									m_pAvatar(NULL)
{
	::ZeroMemory(m_currentGrid, sizeof(TerrainGrid));
	::ZeroMemory(&m_sampleDef, sizeof(m_sampleDef));
}

CTerrainField::~CTerrainField(void)
{
}

//////////////////////////////////////////////////////////////////
// Update Functions
//////////////////////////////////////////////////////////////////
void CTerrainField::Update( float elapsedMillis )
{
	CheckForTerrainChange();
}

CTerrainPatch* CTerrainField::GetTerrainByLocation( WorldPosition pos, bool createIfNotFound /*= false*/ )
{
	/* look inside the grid first */
	CTerrainPatch *ret = NULL, *t = NULL;
	for(int i = 0; i < TERRAIN_GRID_ROWS && !ret; i++)
	{
		for(int j = 0; j < TERRAIN_GRID_COLS && !ret; j++)
		{
			t = m_currentGrid[i][j];
			ret = ( t && Collide_PointToBox(pos, t->GetBoundingBox(), TEST_2D_XZ) ) ? t : NULL; 
		}
	}

	/* if still not found, then look through any loaded patches */
	for( int i = 0; i < MAX_LOADED_PATCHES && !ret; i++ )
	{
		t = &m_loadedPatches[i];
		if(!t->IsTerrainSamplesLoaded())
			continue;

		ret = ( t && Collide_PointToBox(pos, t->GetBoundingBox(), TEST_2D_XZ) ) ? t : NULL;
	}

	/* create on request - check if position is inside the field at all (sanity check), then create the new patch. */
	const unsigned int texWidth = m_sampleDef.width, texHeight = m_sampleDef.height;
	if(!ret && createIfNotFound)
	{
		Rect fieldBounds;
		fieldBounds.wh = Vector_2((texWidth-1)*VERT_SPACING, (texHeight-1)*VERT_SPACING);
		fieldBounds.pos = -fieldBounds.wh*0.5f;

		/* verify the point is inside this field */
		if(Collide_PointToRect(pos.x, pos.z, fieldBounds))
		{
			ret = GetAvailablePatch();

			/* load the new patch */
			TerrainTextureBlock bounds = CalculatePatchBoundsByPos(pos);
			ret->LoadHeightMapArea(m_sampleDef.filename.c_str(), texWidth, texHeight, VERT_SPACING, Y_SCALE, bounds);
		}
	}

	return ret;
}

void CTerrainField::SetAvatar( GameEvents::IEventHandler *avatar )
{
	m_pAvatar = avatar;
	CheckForTerrainChange();
}

void CTerrainField::SetCollisionMgr( CCollisionManager* cm )
{
	cm->RegisterStaticCollidable((ICollidable*)m_currentGrid[1][1]);
}

void CTerrainField::HandleTerrainGridChanged(CTerrainPatch* newCenter)
{
	/* create the new grid, and load any height-map data for new patches */
	TerrainGrid newGrid;
	::ZeroMemory(newGrid, sizeof(newGrid));
	newGrid[TERRAIN_GRID_ROWS>>1][TERRAIN_GRID_COLS>>1] = newCenter;
	
	ABB_MaxMin abb		= newCenter->GetBoundingBox();
	Rect rect;
	rect.pos.x			= abb.min.x;
	rect.pos.y			= abb.min.z;
	rect.wh.x			= abb.max.x - abb.min.x;
	rect.wh.y			= abb.max.z - abb.min.z;

	Vector_2 centerPos	= rect.pos + 0.5f * rect.wh;
	Vector_2 xy			= centerPos - Vector_2((TERRAIN_GRID_COLS>>1) * rect.wh.x, (TERRAIN_GRID_ROWS>>1) * rect.wh.y );
	float startX		= xy.x;
	WorldPosition pos3D;
	
	// this code needs debugged, the items being loaded with the following passage, get immediately unloaded by the next piece
	
	for(int i = 0; i < TERRAIN_GRID_ROWS; i++)
	{
		for(int j = 0; j < TERRAIN_GRID_COLS; j++)
		{
			if(newGrid[i][j] != newCenter)
			{
				pos3D.x = xy.x;
				pos3D.z = xy.y;
				pos3D.y = 0;
				newGrid[i][j] = GetTerrainByLocation(pos3D, true);			
			}
			
			xy.x += rect.wh.x;
		}

		xy.y += rect.wh.y;
		xy.x = startX;
	}

	/* save the new grid */
	memcpy( m_currentGrid, newGrid, sizeof(m_currentGrid));

	/* unload any meshes that are not in the new grid */
	CTerrainPatch *pch = NULL;
	for(int k = 0; k < cnt_of_array(m_loadedPatches); k++)
	{
		bool found = false;
		pch = &m_loadedPatches[k];
		for(int i = 0; i < TERRAIN_GRID_ROWS && !found; i++)
		{
			for(int j = 0; j < TERRAIN_GRID_COLS && !found; j++)
			{
				found = (pch == m_currentGrid[i][j]);
			}
		}

		if(!found)
		{
			pch->LoadMesh(false); //unload
		}
	}

	/* load all the new grid's meshes */
	for(int i = 0; i < TERRAIN_GRID_ROWS; i++)
	{
		for(int j = 0; j < TERRAIN_GRID_COLS; j++)
		{
			if(!m_currentGrid[i][j])
				continue;

			m_currentGrid[i][j]->LoadTextures();
			m_currentGrid[i][j]->LoadMesh();
		}
	}

	/* notify all listeners of the event */
	CTerrainContainer terrContainer(m_currentGrid);
	for(std::list<IEventHandler*>::iterator it = m_eventListeners.begin(), _it = m_eventListeners.end(); it != _it; ++it)
	{
		(*it)->HandleEvent(EVT_TERRAIN_GRIDCHANGED, &terrContainer, sizeof(terrContainer));
	}

	// see above comment about needing to debug
	assert(false);
}

void CTerrainField::SubscribeToTerrainMgrEvents( IEventHandler* listener, bool subscribe/*=true*/ )
{
	if(subscribe)
		m_eventListeners.push_back(listener);
	else
		m_eventListeners.remove(listener);
}

CTerrainContainer CTerrainField::GetCurrentTerrain()
{
	return CTerrainContainer(m_currentGrid);
}

void CTerrainField::Setup( LPCSTR pFilename, int rows, int cols )
{
	m_sampleDef.width = cols;
	m_sampleDef.height = rows;
	m_sampleDef.filename = pFilename;

	/* verify the height map can be broken into even 64x64 pieces */
	DASSERT( ((rows-1) % BLOCK_SIZE == 0) && ((cols-1) % BLOCK_SIZE == 0) );
}

bool CTerrainField::CheckForTerrainChange()
{
	CTerrainPatch* terrain = GetTerrainByLocation(GetAvatarPosition(), true);
	if(terrain && terrain != CENTER_CHUNK)
	{
		terrain->LoadMesh();
		HandleTerrainGridChanged(terrain);
	}

	return false;
}

CTerrainPatch* CTerrainField::GetAvailablePatch()
{
	CTerrainPatch *ret = NULL;

	/* look for a empty terrain chunk */
	for(int i = 0; i < MAX_LOADED_PATCHES && !ret; i++)
	{
		ret = !m_loadedPatches[i].IsTerrainSamplesLoaded() ? &m_loadedPatches[i] : NULL;
	}

	for(int i = 0; i < MAX_LOADED_PATCHES && !ret; i++)
	{
		ret = !m_loadedPatches[i].IsRenderMeshLoaded() ? &m_loadedPatches[i] : NULL;
	}

	/* if an empty terrain was not available, then try to unload one */
	for(int i = 0; i < MAX_LOADED_PATCHES && !ret; i++)
	{
		/* if outside the current grid then unload */
		bool found = false;
		for(int j = 0; j < TERRAIN_GRID_ROWS && !found; j++)
		{
			for(int k = 0; k < TERRAIN_GRID_COLS && !found; k++)
			{
				found = (m_currentGrid[j][k] == &m_loadedPatches[i]);
			}
		}

		if(!found)
		{
			ret = &m_loadedPatches[i];
			*ret = CTerrainPatch();
		}
	}

	return ret;
}

TerrainTextureBlock CTerrainField::CalculatePatchBoundsByPos( WorldPosition pos )
{
	TerrainTextureBlock ret;
	const float tex_w	= (float)m_sampleDef.width-1;
	const float tex_h	= (float)m_sampleDef.height-1;
	const float x		= floorf((pos.x + 0.5f*tex_w*VERT_SPACING) / (BLOCK_SIZE*VERT_SPACING));
	const float y		= floorf((pos.y + 0.5f*tex_h*VERT_SPACING) / (BLOCK_SIZE*VERT_SPACING));

	ret.pos				= Vector_2(x*BLOCK_SIZE, y*BLOCK_SIZE);
	ret.wh				= Vector_2((float)BLOCK_SIZE, (float)BLOCK_SIZE);

	return ret;
}
