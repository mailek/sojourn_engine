#pragma once
#include "../TestCommon.h"
#include "../../LevelLoader.h"

class LevelLoaderTest :
	public ITest
{
public:
	LevelLoaderTest(void);
	~LevelLoaderTest(void);

	static ITest* CreateTest() { return new LevelLoaderTest(); }

public:
	/* ITest Members */
	virtual void Setup();
	virtual void TearDown();
	virtual void RunTest();
	TEST_NAME("LevelLoaderTest");

private:

};
