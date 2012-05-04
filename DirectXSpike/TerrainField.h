#pragma once
/********************************************************************
	created:	2012/04/23
	filename: 	TerrainField.h
	author:		Matthew Alford
	
	purpose:	Handles loading the nearby terrain chunks and spawning
				environment flora.

*********************************************************************/

//////////////////////////////////////
// Include
//////////////////////////////////////
#include "GameEvents.h"
#include "TerrainMiscTypes.h"
#include "TerrainChunk.h"

//////////////////////////////////////
// Forward Declarations
//////////////////////////////////////
class CTerrainChunk;
class CCollisionManager;
class CSceneManager;

//////////////////////////////////////
// Types
//////////////////////////////////////

//////////////////////////////////////
// Class
//////////////////////////////////////
class CTerrainManager
{
public:
	CTerrainManager(void);
	~CTerrainManager(void);

	void Update( float elapsedMillis );
	CTerrainChunk* GetTerrainByLocation(Vector_3 pos);
	void SetAvatar( GameEvents::IEventHandler* avatar );
	void SetCollisionMgr( CCollisionManager* cm );
	void HandleTerrainGridChanged();
	void SubscribeToTerrainMgrEvents(GameEvents::IEventHandler* listener, bool unsubscribe=false);
	CTerrainContainer GetCurrentTerrain();
	inline void SetYOffset(float yOffset) {m_fYOffset = yOffset;}
	void Load(LPCSTR pFilename, LPCSTR pTextureFilename, int rows, int cols, float vertSpacing, float fYScale, float uvScale);
private:
	TerrainGrid								m_currentTerrain;
	GameEvents::IEventHandler			   *m_pAvatar;
	std::list<GameEvents::IEventHandler*>	m_eventListeners;
	float									m_fYOffset;
	CTerrainChunk							m_terrainChunk;
};