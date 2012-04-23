#pragma once
/********************************************************************
	created:	2012/04/23
	filename: 	FileIO.h
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
#include <fstream>

typedef std::ifstream InputFileType;
typedef std::ofstream OutputFileType;

typedef const char* FileNameType;
typedef struct _IOVector3
{
	float x;
	float y;
	float z;
} IOVector3;

class FileIO
{
public:
	static bool OpenFile(FileNameType name, bool binary, InputFileType* in_file);
	static void CloseFile(InputFileType* file);
	static bool ReadBytes(InputFileType* in, char* out_stream, size_t read_sz);

	static void ReadInt16(InputFileType* file, __int16* int16);
	static void ReadInt32(InputFileType* file, __int32* int32);
	static void ReadUint32(InputFileType* file, unsigned int* uint32);

	static void ReadNullTermString(InputFileType* file, char* out);
	static void ReadVec3(InputFileType* file, IOVector3* out);

	static void ReadSinglePrecision(InputFileType* file, float* out);
	static void ReadDoublePrecision(InputFileType* file, double* out);
};
