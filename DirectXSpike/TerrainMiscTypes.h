#pragma once

class CTerrainChunk;
typedef CTerrainChunk*	TerrainGrid[3][3];

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
