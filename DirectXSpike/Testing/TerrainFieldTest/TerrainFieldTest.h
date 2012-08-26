#pragma once
#include "../TestCommon.h"
#include "../../TerrainField.h"

class TerrainFieldTest : 
	public ITest
{
public:
	TerrainFieldTest(void);
	~TerrainFieldTest(void);

	static ITest* CreateTest() { return new TerrainFieldTest(); }

public:
	/* ITest Members */
	virtual void Setup();
	virtual void TearDown();
	virtual void RunTest();
	TEST_NAME("TerrainFieldTest");

private:
};
