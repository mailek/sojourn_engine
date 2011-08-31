#include "stdafx.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

////////////////////POINTER MACROS/////////////////////////
inline void COM_SAFERELEASE(IUnknown* ptr)
{
	if(ptr != NULL)
		ptr->Release();

	ptr = NULL;
}
