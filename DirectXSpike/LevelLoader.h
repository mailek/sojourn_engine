#pragma once

#include "fileio.h"
#include "skybox.h"

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

struct LevelTerrain
{
	BYTE					heightMap[MAX_NUM_TERRAIN_SAMPLES];
	int						rows;
	int						cols;
	LevelTerrainName		terrainName;
};

struct LevelObjectDefinitionLink
{
	ObjectDefFilenameType	fileName;
	ObjectDefIDType			typeId;
};

struct LevelObjectDefinition
{
};

struct LevelObjectPalette
{
	int						numObjTypes;
	LevelObjectDefinition	palette[MAX_NUM_OBJ_TYPES];
};

struct LevelObjectInstance
{
};

struct LevelDefinition
{
	LevelSkybox				skybox;
	LevelObjectPalette		palette;
	LevelObjectInstance		instances[MAX_NUM_LVL_OBJS];
	int						numInstances;
	LevelNameType			levelName;
	LevelVersion			levelVersion;
	LevelTerrain			terrains[MAX_NUM_TERRAINS];
	int						numTerrains;
};

class LevelLoader
{
public:
	//LevelLoader(void);
	//~LevelLoader(void);

	static void LoadLevelFile(const char* levelFileName, LevelDefinition* level);

private:
	static void ReadLevelObjectPalette(InputFileType* level, LevelObjectPalette* palette);
	static void ReadTerrain(InputFileType* level, LevelTerrain* terrain);
	static void ReadObjectInstance(InputFileType* level, LevelObjectInstance* levelObj);
	static void ReadLevelObjectDefinition(InputFileType* defFile, LevelObjectDefinition* def);
};