#pragma once
/********************************************************************
	created:	2012/04/23
	filename: 	LevelLoader.h
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
#include "fileio.h"
#include "skybox.h"
#include "Singleton.h"

#define MAX_OBJ_DEF_FILENAME_LEN	(50)
#define MAX_OBJ_DEF_TYPE_ID_LEN		(40)
#define MAX_LEVEL_NAME_LEN			(40)
#define MAX_LEVEL_TERRAIN_NAME_LEN	(40)
#define MAX_NUM_OBJ_TYPES			(50)
#define MAX_LEVEL_VER_LEN			(20)
#define MAX_NUM_LVL_OBJS			(100)
#define MAX_NUM_TERRAIN_SAMPLES		(65536)/* 256x256 */
#define MAX_NUM_TERRAINS			(2)

typedef char ObjectDefIDType[MAX_OBJ_DEF_TYPE_ID_LEN]; 
typedef char ObjectDefFilenameType[MAX_OBJ_DEF_FILENAME_LEN];
typedef CSkybox LevelSkybox;
typedef char LevelNameType[MAX_LEVEL_NAME_LEN];
typedef char LevelVersion[MAX_LEVEL_VER_LEN];
typedef char LevelTerrainName[MAX_LEVEL_TERRAIN_NAME_LEN];
typedef std::vector<BYTE> LevelHeightMap;

typedef struct _LevelTerrain
{
	BYTE					heightMap[MAX_NUM_TERRAIN_SAMPLES];
	int						rows;
	int						cols;
	LevelTerrainName		terrainName;
} LevelTerrain;

typedef struct _LevelObjectDefinitionLink
{
	ObjectDefFilenameType	fileName;
	ObjectDefIDType			typeId;
} LevelObjectDefinitionLink;

typedef struct _LevelObjectDefinition
{
} LevelObjectDefinition;

typedef struct _LevelObjectPalette
{
	int						numObjTypes;
	LevelObjectDefinition	palette[MAX_NUM_OBJ_TYPES];
} LevelObjectPalette;

typedef struct _LevelObjectInstance
{
} LevelObjectInstance;

typedef struct _LevelDefinition
{
	LevelSkybox				skybox;
	LevelObjectPalette		palette;
	LevelObjectInstance		instances[MAX_NUM_LVL_OBJS];
	int						numInstances;
	LevelNameType			levelName;
	LevelVersion			levelVersion;
	LevelTerrain			terrains[MAX_NUM_TERRAINS];
	int						numTerrains;
} LevelDefinition;

interface ILevelLoader
{
	virtual void LoadLevelFile(const char* levelFileName, LevelDefinition* level)=0;
	virtual void ReadTerrainHeightMapFromRaw(LPCSTR pFilename, int width, int height, Rect readArea, LevelHeightMap *outMap)=0;
};

class LevelLoader : public ILevelLoader, public SingleThreadSingleton<LevelLoader, ILevelLoader>
{
	friend class SingleThreadSingleton<LevelLoader, ILevelLoader>;
private:
	LevelLoader(void) {};
	~LevelLoader(void) {};

private:
	static void ReadLevelObjectPalette(InputFile* level, LevelObjectPalette* palette);
	static void ReadTerrain(InputFile* level, LevelTerrain* terrain);
	static void ReadObjectInstance(InputFile* level, LevelObjectInstance* levelObj);
	static void ReadLevelObjectDefinition(InputFile* defFile, LevelObjectDefinition* def);

/* ILevelLoader */
public:
	virtual void LoadLevelFile(const char* levelFileName, LevelDefinition* level);
	virtual void ReadTerrainHeightMapFromRaw(LPCSTR pFilename, int width, int height, Rect readArea, LevelHeightMap *outMap);
};