#include "stdafx.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

static char s_sbuffer[200];

////////////////////POINTER MACROS/////////////////////////
inline void COM_SAFERELEASE(IUnknown* ptr)
{
	if(ptr != NULL)
		ptr->Release();

	ptr = NULL;
}

/************************************************
*   Name:   _sprintf
*   Desc:   sprintf with buffered output.  Only
*			supports string of 200 characters.
************************************************/
inline const char* _sprintf(char* dest, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vsprintf_s(s_sbuffer, sizeof(s_sbuffer), format, args);
	va_end(args);
	if(dest) strcpy_s(dest, sizeof(dest), s_sbuffer);

	return s_sbuffer;
}

inline void CONSOLE(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vsprintf_s(s_sbuffer, sizeof(s_sbuffer), format, args);
	va_end(args);
	OutputDebugString(s_sbuffer);
}

inline void CONSOLE(char* msg)
{
	OutputDebugString(msg);
}