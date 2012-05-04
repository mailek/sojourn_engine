#include "StdAfx.h"
#include "TerrainField.h"
#include "TerrainMiscTypes.h"
#include "CollisionManager.h"
#include "TerrainChunk.h"
#include "SceneManager.h"

//////////////////////////////////////////////////////////////////////////
// CTerrainContainer class
//////////////////////////////////////////////////////////////////////////
CTerrainContainer::CTerrainContainer(TerrainGrid grid)
{
	for(int i = 0; i < cnt_of_array(grid[0]); i++)
		for(int j = 0; j < cnt_of_array(grid[i]); j++)
		{
			m_terrainGrid[i][j] = grid[i][j];
		}
}

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
			if(ABB_PointInsideABB(pt, m_terrainGrid[i][j]->CalculateBoundingBox()))
			{
				return m_terrainGrid[i][j]->GetTerrainHeightAtXZ(xCoord,zCoord);
			}
		}

	return 0;
}

ABB_MaxMin CTerrainContainer::CalculateBoundingBox(void)
{
	ABB_MaxMin abb = m_terrainGrid[1][1]->CalculateBoundingBox();

	for(int i = 0; i < cnt_of_array(m_terrainGrid[0]); i++)
		for(int j = 0; j < cnt_of_array(m_terrainGrid[i]); j++)
		{
			if(!m_terrainGrid[i][j])
				continue;

			ABB_GrowToIncludeABB(m_terrainGrid[i][j]->CalculateBoundingBox(), abb);
		}

	return abb;
}

//////////////////////////////////////////////////////////////////////////
// CTerrainField Class
//////////////////////////////////////////////////////////////////////////

CTerrainField::CTerrainField(void)
{
}

CTerrainField::~CTerrainField(void)
{
}

void CTerrainField::Update( float elapsedMillis )
{
	bool terrainChanged = false;

	if( terrainChanged )
	{
		this->HandleTerrainGridChanged();
	}
}

CTerrainChunk* CTerrainField::GetTerrainByLocation( Vector_3 pos )
{
	return m_currentTerrain[1][1];
}

void CTerrainField::SetAvatar( GameEvents::IEventHandler *avatar )
{
	m_pAvatar = avatar;
}

void CTerrainField::SetCollisionMgr( CCollisionManager* cm )
{
	cm->RegisterStaticCollidable((ICollidable*)m_currentTerrain[1][1]);
}

void CTerrainField::HandleTerrainGridChanged()
{
	TerrainGrid temp;
	memcpy( temp, m_currentTerrain, sizeof(temp));
	::ZeroMemory(m_currentTerrain, sizeof(m_currentTerrain));
	assert(false); // need to add support for loading terrain data

	/* notify all listeners of the event */
	CTerrainContainer terrContainer(m_currentTerrain);
	for(std::list<IEventHandler*>::iterator it = m_eventListeners.begin(), _it = m_eventListeners.end(); it != _it; it++)
	{
		(*it)->HandleEvent(EVT_TERRAIN_GRIDCHANGED, &terrContainer, sizeof(terrContainer));
	}
}

void CTerrainField::SubscribeToTerrainMgrEvents(IEventHandler* listener, bool unsubscribe/*=false*/)
{
	if(!unsubscribe)
	{
		m_eventListeners.push_back(listener);
	}
	else
	{
		m_eventListeners.remove(listener);
	}
}

CTerrainContainer CTerrainField::GetCurrentTerrain()
{
	return CTerrainContainer(m_currentTerrain);
}

void CTerrainField::Load(LPCSTR pFilename, LPCSTR pTextureFilename, int rows, int cols, float vertSpacing, float fYScale, float uvScale)
{
	memset( m_currentTerrain, 0, sizeof(m_currentTerrain));
	m_currentTerrain[1][1] = &m_terrainChunk;
	m_terrainChunk.LoadTerrain(pFilename, pTextureFilename, rows, cols, vertSpacing, fYScale, uvScale);
}