#include "StdAfx.h"
#include "LevelLoaderTest.h"

#define TEST_RESOURCES	TEST_ROOT_FOLDER"LevelLoaderTest/"

LevelLoaderTest::LevelLoaderTest(void)
{
}

LevelLoaderTest::~LevelLoaderTest(void)
{
}

void LevelLoaderTest::Setup()
{
	//m_allocator = new CStackMemoryAllocator();
}

void LevelLoaderTest::TearDown()
{
	//delete m_allocator;
}

void LevelLoaderTest::RunTest()
{
	/* Test Reading a chunk of solid color from center of texture. */
	{
	EXPERIMENT("Read chunk in center of heightmap");
		
	/* __Init__ */
	LevelHeightMap map;
	const unsigned int WHITE_PIXEL = 255;
	const unsigned int BLACK_PIXEL = 0;
	// pos: 212, 235 
	// wh:  95, 101

	/* __Run__ */
	Rect readArea;
	readArea.pos.x = 211;
	readArea.pos.y = 234;
	readArea.wh.x = 97;
	readArea.wh.y = 103;

	TEST_TIMER("Read Chunk Test");
	LevelLoader::GetInstance()->ReadTerrainHeightMapFromRaw(TEST_RESOURCES"test_600x600.raw", 600, 600, readArea, &map );
	STOP_TIMER();

	/* __Verify__ */
	LevelHeightMap::iterator it = map.begin();

	a(map.size() == readArea.wh.y * readArea.wh.x);
	for( int i = 0; i < readArea.wh.x; i++, it++ ) 
	{
		a(*it == BLACK_PIXEL);
	}
	for( int j = 0; j < readArea.wh.y-2; j++ )
	{
		a(*it == BLACK_PIXEL); it++;
		for( int i = 0; i < readArea.wh.x-2; i++, it++ ) 
		{
			a(*it == WHITE_PIXEL);
		}
		a(*it == BLACK_PIXEL); it++;
	}
	for( int i = 0; i < readArea.wh.x; i++, it++ ) 
	{
		a(*it == BLACK_PIXEL);
	}

	} // EXPERIMENT

	/* Test Reading a chunk of stipled color from heightmap. */
	{
		EXPERIMENT("Read a stiple pattern chunk from heightmap");

		/* __Init__ */
		LevelHeightMap map;
		const unsigned int WHITE_PIXEL = 255;
		const unsigned int BLACK_PIXEL = 0;
		// pos: 450, 286 
		// wh:  70, 24

		/* __Run__ */
		Rect readArea;
		readArea.pos.x = 450;
		readArea.pos.y = 286;
		readArea.wh.x = 70;
		readArea.wh.y = 24;

		TEST_TIMER("Read Stipple Pattern Test");
		LevelLoader::GetInstance()->ReadTerrainHeightMapFromRaw(TEST_RESOURCES"test_600x600.raw", 600, 600, readArea, &map );
		STOP_TIMER();

		/* __Verify__ */
		LevelHeightMap::iterator it = map.begin();

		a(map.size() == readArea.wh.y * readArea.wh.x);
		for( int i = 0; i < readArea.wh.x; i++, it++ ) 
		{
			a(*it == (((i%2) < 1)  ? BLACK_PIXEL : WHITE_PIXEL));
		}
		for( int i = 0; i < readArea.wh.x; i++, it++ ) 
		{
			a(*it == (((i%4) < 2)  ? BLACK_PIXEL : WHITE_PIXEL));
		}
		for( int i = 0; i < readArea.wh.x; i++, it++ ) 
		{
			a(*it == (((i%6) < 3)  ? BLACK_PIXEL : WHITE_PIXEL));
		}
		for( int i = 0; i < readArea.wh.x; i++, it++ ) 
		{
			a(*it == (((i%8) < 4)  ? BLACK_PIXEL : WHITE_PIXEL));
		}
		for( int i = 0; i < readArea.wh.x; i++, it++ ) 
		{
			a(*it == (((i%10) < 5)  ? BLACK_PIXEL : WHITE_PIXEL));
		}
		for(LevelHeightMap::iterator _it = map.end(); it != _it; it++)
		{
			a(*it == WHITE_PIXEL);
		}

	}// EXPERIMENT

}