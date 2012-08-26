#pragma once

#include "LevelLoaderTest/LevelLoaderTest.h"
#include "TerrainFieldTest/TerrainFieldTest.h"

typedef ITest*(*CreateTestFunc)(void);

static CreateTestFunc s_suite[] = 
{
	LevelLoaderTest::CreateTest,
	TerrainFieldTest::CreateTest
};