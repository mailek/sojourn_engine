/********************************************************************
	created:	2012/04/23
	filename: 	LevelLoader.cpp
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
#include "StdAfx.h"
#include "LevelLoader.h"

//LevelLoader::LevelLoader(void)
//{
//}
//
//LevelLoader::~LevelLoader(void)
//{
//}

void LevelLoader::LoadLevelFile(const char* levelFileName, LevelDefinition* level)
{
	InputFileType levelFile;
	FileIO::OpenFile(levelFileName, true, &levelFile);

	/* File Version Stamp */
	FileIO::ReadNullTermString(&levelFile, level->levelVersion);

	/* Level Attributes */
	FileIO::ReadNullTermString(&levelFile, level->levelName);

	/* Object Type Palette */
	ReadLevelObjectPalette(&levelFile, &level->palette);

	/* Level Terrain */
	FileIO::ReadInt32(&levelFile, &level->numTerrains);
	for(int i=0; i < level->numTerrains; i++)
	{
		ReadTerrain(&levelFile, &level->terrains[i]);
	}

	/* Level Object Instances */
	level->numInstances = 0;
	FileIO::ReadInt32(&levelFile, &level->numInstances);
	for(int i=0; i < level->numInstances; i++)
	{
		ReadObjectInstance(&levelFile, &level->instances[i]);
	}

	FileIO::CloseFile(&levelFile);

}

void LevelLoader::ReadLevelObjectPalette(InputFileType* level, LevelObjectPalette* palette)
{
	unsigned int numDefs = 0;
	FileIO::ReadUint32(level, &numDefs);
	palette->numObjTypes = numDefs;

	/* Load the definition links */
	LevelObjectDefinitionLink links[MAX_NUM_OBJ_TYPES];
	memset(links, 0, numDefs*sizeof(LevelObjectDefinitionLink));
	for(unsigned int i=0; i < numDefs; i++)
	{
		FileIO::ReadNullTermString(level, links[i].fileName);
		FileIO::ReadNullTermString(level, links[i].typeId);
	}

	/* Load the individual object definitions */
	for(unsigned int i = 0; i < numDefs; i++)
	{
		LevelObjectDefinitionLink* link = &links[i];
		InputFileType defFile;
		FileIO::OpenFile(link->fileName, true, &defFile);
		ReadLevelObjectDefinition(&defFile, &palette->palette[i]);
		FileIO::CloseFile(&defFile);
	}

}

void LevelLoader::ReadTerrain(InputFileType* level, LevelTerrain* terrain)
{
	
}

void LevelLoader::ReadObjectInstance(InputFileType* level, LevelObjectInstance* levelObj)
{

}

void LevelLoader::ReadLevelObjectDefinition(InputFileType* defFile, LevelObjectDefinition* def)
{

}