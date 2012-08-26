/********************************************************************
	created:	2012/04/23
	filename: 	LevelLoader.cpp
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
#include "StdAfx.h"
#include "LevelLoader.h"



void LevelLoader::LoadLevelFile(const char* levelFileName, LevelDefinition* level)
{
	InputFile levelFile;
	FileIO::OpenFile(levelFileName, FILE_BINARY, &levelFile);

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

void LevelLoader::ReadLevelObjectPalette(InputFile* level, LevelObjectPalette* palette)
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
		InputFile defFile;
		FileIO::OpenFile(link->fileName, FILE_BINARY, &defFile);
		ReadLevelObjectDefinition(&defFile, &palette->palette[i]);
		FileIO::CloseFile(&defFile);
	}

}

void LevelLoader::ReadTerrain(InputFile* level, LevelTerrain* terrain)
{
	
}

void LevelLoader::ReadTerrainHeightMapFromRaw(LPCSTR pFilename, int width, int height, Rect readArea, LevelHeightMap *outMap)
{
	const int rectWidth	= (int)floorf(readArea.wh.x), rectHeight = (int)floorf(readArea.wh.y);
	const int rectX		= (int)floorf(readArea.pos.x), rectY = (int)floorf(readArea.pos.y);

	/* sanity checks */
	DASSERT( readArea.pos.x >= 0.0f 
		&& readArea.pos.y >= 0.0f 
		&& rectX + rectWidth <= width 
		&& rectY + rectHeight <= height 
		&& readArea.wh.x > 0 
		&& readArea.wh.y > 0 );

	/* open the file for read */
	outMap->clear();
	*outMap = LevelHeightMap(rectHeight*rectWidth);
	InputFile terrainFile;
	VERIFY(FileIO::OpenFile(pFilename, FILE_BINARY, &terrainFile));

	/* scan to the first entry in the read area */
	int offset = rectX + rectY * width;
	FileIO::SeekForward(&terrainFile, offset);

	/* read each of the scan lines in the requested read area rectangle */
	offset = width - rectWidth;
	int lines = rectHeight;
	int j = 0;
	while(lines)
	{
		VERIFY( FileIO::ReadBytes(&terrainFile, (char*)&(*outMap)[j], rectWidth) );

		j += rectWidth;
		lines--;

		if(lines)
			FileIO::SeekForward(&terrainFile, offset);
	}

	/* clean up */
	terrainFile.Close();
	
	DASSERT( outMap->size() == rectWidth * rectHeight );
}

void LevelLoader::ReadObjectInstance(InputFile* level, LevelObjectInstance* levelObj)
{

}

void LevelLoader::ReadLevelObjectDefinition(InputFile* defFile, LevelObjectDefinition* def)
{

}