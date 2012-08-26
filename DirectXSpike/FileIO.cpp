/********************************************************************
	created:	2012/04/23
	filename: 	FileIO.cpp
	author:		Matthew Alford
	
	purpose:	
*********************************************************************/
#include "StdAfx.h"
#include "FileIO.h"

bool FileIO::OpenFile(FileNameType name, EFileReadMode mode, InputFile* in_file)
{
	// replace forward slashes with backslashes
	char filename[MAX_FILENAME_LEN], f[MAX_FILENAME_LEN], *nextToken;
	f[0] = 0;
	strcpy_s(filename, name);
	for( char* stok = strtok_s(filename, "/", &nextToken); stok != NULL; stok = strtok_s(NULL, "/", &nextToken) )
	{
		strcat_s(f, stok);
		if(strlen(nextToken)) strcat_s(f, "\\");
	}

	in_file->open(name, (mode == FILE_BINARY ? std::ios_base::binary : 0));
	return !in_file->fail();
}

bool FileIO::ReadBytes(InputFile* in, char* out_stream, size_t read_sz)
{
	in->read(out_stream, read_sz);
	return !in->fail();
}

void FileIO::SeekForward(InputFile* in, int offset)
{
	in->seekg(offset, std::ios_base::cur);
}

void FileIO::CloseFile(InputFile* file)
{
	if(file->is_open())
	{
		file->close();
	}
}

void FileIO::ReadInt16(InputFile* file, __int16* int16)
{
	BYTE buffer[2];
	ReadBytes(file, (char*)buffer, sizeof(__int16));
	*int16 = (__int16)buffer;
}

void FileIO::ReadNullTermString(InputFile* file, char* out)
{
	char readChar = 1;
	while(readChar != NULL)
	{
		ReadBytes(file, &readChar, 1);
		*out = readChar;
		out++;
	}

	*out = NULL;
}

void FileIO::ReadInt32(InputFile* file, __int32* int32)
{
	BYTE buffer[4];
	ReadBytes(file, (char*)buffer, sizeof(__int32));
	*int32 = (__int32)buffer;
}

void FileIO::ReadUint32(InputFile* file, unsigned int* uint32)
{
	ReadInt32(file, (int*)uint32);
}

void FileIO::ReadVec3(InputFile* file, IOVector3* out)
{
	ReadSinglePrecision(file, &out->x);
	ReadSinglePrecision(file, &out->y);
	ReadSinglePrecision(file, &out->z);
}

void FileIO::ReadSinglePrecision(InputFile* file, float* out)
{
	ReadBytes(file, (char*)out, 2);
}

void FileIO::ReadDoublePrecision(InputFile* file, double* out)
{
	ReadBytes(file, (char*)out, 4);
}
