#pragma once

#define TERRAIN_GRID_COLS (3)
#define TERRAIN_GRID_ROWS TERRAIN_GRID_COLS

class CTerrainPatch;
typedef CTerrainPatch*	TerrainGrid[TERRAIN_GRID_ROWS][TERRAIN_GRID_COLS];

class CTerrainContainer
{
public:
	CTerrainContainer(TerrainGrid grid);
	~CTerrainContainer(void){};

public:
	TerrainGrid			m_terrainGrid;

	float GetTerrainHeightAtXZ(float xCoord, float zCoord);
	ABB_MaxMin CalculateBoundingBox(void);
};
