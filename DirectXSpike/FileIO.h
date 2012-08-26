#pragma once
/********************************************************************
	created:	2012/04/23
	filename: 	FileIO.h
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
#include <fstream>

//////////////////////////////////////
// Types
//////////////////////////////////////

typedef std::ofstream OutputFileType;

typedef const char* FileNameType;
typedef struct _IOVector3
{
	float x;
	float y;
	float z;
} IOVector3;

typedef enum _EFileReadMode {
	FILE_BINARY,
	FILE_ACSII
} EFileReadMode;

class InputFile;

//////////////////////////////////////
// Functions
//////////////////////////////////////
class FileIO
{
public:
	static bool OpenFile(FileNameType name, EFileReadMode mode, InputFile* in_file);
	static void CloseFile(InputFile* file);
	static bool ReadBytes(InputFile* in, char* out_stream, size_t read_sz);
	static void SeekForward(InputFile* in,  int offset);

	static void ReadInt16(InputFile* file, __int16* int16);
	static void ReadInt32(InputFile* file, __int32* int32);
	static void ReadUint32(InputFile* file, unsigned int* uint32);

	static void ReadNullTermString(InputFile* file, char* out);
	static void ReadVec3(InputFile* file, IOVector3* out);

	static void ReadSinglePrecision(InputFile* file, float* out);
	static void ReadDoublePrecision(InputFile* file, double* out);
};

//////////////////////////////////////
// Classes
//////////////////////////////////////
class InputFile : public std::ifstream
{
	bool bOpen;
public:
	InputFile() : bOpen(true) {};
	~InputFile() {Close();}
	void Close() { if(bOpen) {FileIO::CloseFile(this); bOpen = false;} }
};