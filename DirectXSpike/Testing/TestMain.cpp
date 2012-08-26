#include "StdAfx.h"
#include "TestMain.h"


TestMain::TestMain(void)
{
	memset(m_tests, 0, sizeof(m_tests));
}

TestMain::~TestMain(void)
{
}

void TestMain::RunSuite()
{
	CONSOLE("*****************************************\n");	
	CONSOLE("*              Unit Tests               *\n");
	CONSOLE("*****************************************\n");
	CONSOLE("Creating Test Suite.\n");
	CreateSuite();
	int testCnt;
	for(testCnt = 0; m_tests[testCnt] != 0; testCnt++);
	CONSOLE("Found %d Test(s).\n", testCnt);

	CONSOLE("Running Test Suite.\n\n");
	for(int i = 0; i<testCnt; i++)
	{	
		//CONSOLE("Starting %s.\n", m_tests[i]->GetTestName());
		CONSOLE("------[ %s ]------\n", m_tests[i]->GetTestName());
		m_tests[i]->Setup();
		m_tests[i]->RunTest();
		m_tests[i]->TearDown();
		CONSOLE("%s Completed.\n\n", m_tests[i]->GetTestName());
	}

	CONSOLE("*****************************************\n");
	CONSOLE("*           Testing Complete            *\n");
	CONSOLE("*****************************************\n");
}

void TestMain::CreateSuite()
{
	for(int i = 0; i < cnt_of_array(s_suite); i++)
	{
		m_tests[i] = (*s_suite[i])();
	}
}