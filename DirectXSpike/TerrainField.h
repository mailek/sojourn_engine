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
#include "TerrainPatch.h"
#include "GameEvents.h"

//////////////////////////////////////
// Forward Declarations
//////////////////////////////////////
class CTerrainPatch;
class CCollisionManager;
class CSceneManager;

using namespace GameEvents;

//////////////////////////////////////
// Types
//////////////////////////////////////

//////////////////////////////////////
// Constants
//////////////////////////////////////
static const int MAX_LOADED_PATCHES = 14;

//////////////////////////////////////
// Class
//////////////////////////////////////
class CTerrainField
{
	friend class TerrainFieldTest;
	typedef struct _SampleDefinition
	{
		unsigned int							width;
		unsigned int							height;
		std::string								filename;
		//int										numOfRows;
	} SampleDefinition;

public:
	CTerrainField(void);
	~CTerrainField(void);

	void Update( float elapsedMillis );
	CTerrainPatch* GetTerrainByLocation(WorldPosition pos, bool createIfNotFound = false);
	void SetAvatar( IEventHandler* avatar );
	void SetCollisionMgr( CCollisionManager* cm );
	void HandleTerrainGridChanged(CTerrainPatch* newCenter);
	void SubscribeToTerrainMgrEvents(IEventHandler* listener, bool subscribe=true);
	CTerrainContainer GetCurrentTerrain();
	void Setup(LPCSTR pFilename, int rows, int cols);

	inline void SetYOffset(float yOffset) {m_fYOffset = yOffset;}

private:
	TerrainGrid								m_currentGrid;
	CTerrainPatch							m_loadedPatches[MAX_LOADED_PATCHES];
	IEventHandler						   *m_pAvatar;
	std::list<IEventHandler*>				m_eventListeners;
	float									m_fYOffset;
	SampleDefinition						m_sampleDef;
	//int										m_numOfVerts;

	bool CheckForTerrainChange();
	CTerrainPatch* GetAvailablePatch();

	inline WorldPosition GetAvatarPosition() {WorldPosition pos; m_pAvatar->HandleEvent(EVT_GETPOSITIONVEC, &pos, sizeof(pos)); return pos;}
	TerrainTextureBlock CalculatePatchBoundsByPos( WorldPosition pos );
};