#include "StdAfx.h"
#include "FileIO.h"

bool FileIO::OpenFile(FileNameType name, bool binary, InputFileType* in_file)
{
	in_file->open(name, (binary ? std::ios_base::binary : 0));
	return !in_file->fail();
}

bool FileIO::ReadBytes(InputFileType* in, char* out_stream, size_t read_sz)
{
	in->read(out_stream, read_sz);
	return !in->fail();
}

void FileIO::CloseFile(InputFileType* file)
{
	if(file->is_open())
	{
		file->close();
	}
}

void FileIO::ReadInt16(InputFileType* file, __int16* int16)
{
	BYTE buffer[2];
	ReadBytes(file, (char*)buffer, sizeof(__int16));
	*int16 = (__int16)buffer;
}

void FileIO::ReadNullTermString(InputFileType* file, char* out)
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

void FileIO::ReadInt32(InputFileType* file, __int32* int32)
{
	BYTE buffer[4];
	ReadBytes(file, (char*)buffer, sizeof(__int32));
	*int32 = (__int32)buffer;
}

void FileIO::ReadUint32(InputFileType* file, unsigned int* uint32)
{
	ReadInt32(file, (int*)uint32);
}

void FileIO::ReadVec3(InputFileType* file, IOVector3* out)
{
	ReadSinglePrecision(file, &out->x);
	ReadSinglePrecision(file, &out->y);
	ReadSinglePrecision(file, &out->z);
}

void FileIO::ReadSinglePrecision(InputFileType* file, float* out)
{
	ReadBytes(file, (char*)out, 2);
}

void FileIO::ReadDoublePrecision(InputFileType* file, double* out)
{
	ReadBytes(file, (char*)out, 4);
}