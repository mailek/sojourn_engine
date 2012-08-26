/********************************************************************
	created:	2012/05/22
	filename: 	AssertUtil.h
	author:		Matthew Alford
	
	purpose:	Assert logic and utility functions
*********************************************************************/

#include <assert.h>

//////////////////////////////////////
// Verify and Debug Assert
//////////////////////////////////////
#ifndef _DEBUG
#define VERIFY(x) x
#define DASSERT(x)
#else
#define VERIFY(x) assert(x)
#define DASSERT(x) assert(x)
#endif

//////////////////////////////////////
// Verify HRESULT
//////////////////////////////////////
#define HR(com) \
	if( FAILED(com) ) \
		{assert(false);}

//////////////////////////////////////
// Compile-time Assert
//////////////////////////////////////
#define compiler_assert(test, file) _COMPILER_ASSERT(test, __LINE__, file)
#define CONCAT(a, b) a##b
#define _COMPILER_ASSERT(test, line, file) \
	typedef char CONCAT(compiler_assert_failed_##file, line)[2*!!(test)-1]