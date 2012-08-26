#include "StdAfx.h"
#include "TerrainFieldTest.h"

//////////////////////////////////////////////////////////////////////////
// Mocks
//////////////////////////////////////////////////////////////////////////
class LevelLoaderMock : public ILevelLoader
{
/*ILevelLoader*/
public:
	virtual void LoadLevelFile(const char* levelFileName, LevelDefinition* level) {};
	virtual void ReadTerrainHeightMapFromRaw(LPCSTR pFilename, int width, int height, Rect readArea, LevelHeightMap *outMap) {};
};

//////////////////////////////////////////////////////////////////////////
// Tests
//////////////////////////////////////////////////////////////////////////

TerrainFieldTest::TerrainFieldTest(void)
{
}

TerrainFieldTest::~TerrainFieldTest(void)
{
}

void TerrainFieldTest::Setup()
{
}

void TerrainFieldTest::TearDown()
{
}

void TerrainFieldTest::RunTest()
{
	/* Test target position is at the origin. */
	{
		EXPERIMENT("Test calculate texture bounds with various locations.");

		/* __Init__ */
		CTerrainField field;
		TerrainTextureBlock bounds;
		// block size is 64x64, texture size needs to be a multiple of this plus one row and column.
		// vert spacing is currently 5.0f, meaning there is 5 meters of world space between each terrain vertex  */
		const float blockExtent = 64.0f;

		field.Setup("test_file.lvl", 129, 129 );

		/* __Run__ */
		TEST_TIMER("Calculate Patch Bound Box Test - Origin");
		bounds = field.CalculatePatchBoundsByPos(WorldPosition(0,0,0));
		STOP_TIMER();

		/* __Verify__ */
		a(bounds.wh.x == blockExtent);
		a(bounds.wh.y == blockExtent);
		a(bounds.pos.x == 64.0f);
		a(bounds.pos.y == 64.0f);

		/* __Run__ */
		TEST_TIMER("Calculate Patch Bound Box Test - NW");
		bounds = field.CalculatePatchBoundsByPos(WorldPosition(-0.5f,-0.5f,0));
		STOP_TIMER();

		/* __Verify__ */
		a(bounds.wh.x == blockExtent);
		a(bounds.wh.y == blockExtent);
		a(bounds.pos.x == 0.0f);
		a(bounds.pos.y == 0.0f);

		/* __Run__ */
		TEST_TIMER("Calculate Patch Bound Box Test - SW");
		bounds = field.CalculatePatchBoundsByPos(WorldPosition(-0.5f,0.5f,0));
		STOP_TIMER();

		/* __Verify__ */
		a(bounds.wh.x == blockExtent);
		a(bounds.wh.y == blockExtent);
		a(bounds.pos.x == 0.0f);
		a(bounds.pos.y == 64.0f);

		/* __Run__ */
		TEST_TIMER("Calculate Patch Bound Box Test - NE");
		bounds = field.CalculatePatchBoundsByPos(WorldPosition(0.5f,-0.5f,0));
		STOP_TIMER();

		/* __Verify__ */
		a(bounds.wh.x == blockExtent);
		a(bounds.wh.y == blockExtent);
		a(bounds.pos.x == 64.0f);
		a(bounds.pos.y == 0.0f);

	}

	/* Test get terrain chunk from current grid by world position. */
	{
		EXPERIMENT("Test get terrain chunk from current grid by world position.");

		/* __Init__ */
		CTerrainField field;
		CTerrainPatch *patch;
		LevelLoaderMock mockLoader;
		LevelLoader::SetMockInstance(&mockLoader);
		
		field.Setup("test_file.lvl", 129, 129 );

		/* __Run__ */
		TEST_TIMER("Try to fetch the origin's terrain chunk from an empty terrain grid.");
		patch = field.GetTerrainByLocation(WorldPosition(0,0,0));
		STOP_TIMER();

		/* __Verify__ */
		
		a(patch == NULL);

		/* __Run__ */
		TEST_TIMER("Try to fetch the origin again, but this time have a new instance created.");
		patch = field.GetTerrainByLocation(WorldPosition(0,0,0), true);
		STOP_TIMER();

		/* __Verify__ */

		a(patch!= NULL);

		/* __Run__ */
		TEST_TIMER("Verify the newly created terrain chunk was not placed in the current grid.");
		patch = field.GetTerrainByLocation(WorldPosition(0,0,0));
		STOP_TIMER();

		/* __Verify__ */

		a(patch ==NULL);

		/* __Tear_Down__ */
		LevelLoader::SetMockInstance(NULL);
	}
}