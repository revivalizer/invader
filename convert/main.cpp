#include <windows.h>
#include <cstdlib>
#include <cstdio>

#include "pch.h"

int main(int argc, char* argv[])
{
	if (argc > 1)
	{
		const char* path = argv[1];
		printf("Processing %s\n", path);

		ZRenoiseSong song;
		int res = song.Read(path);

		if (res==0)
		{
			printf("Ok\n");

			// Generate header path by appending ".h"
			int pathLen = strlen(path);
			char* headerPath = new char[pathLen+3];
			headerPath[0] = '\0';
			strcat(headerPath, path);
			strcat(headerPath, ".h");

			// Write header
			ZExeSong* exeSong = song.MakeExeSong();
			ZExeSong* packedExeSong = ZExeSong::MakeBlob(exeSong)->Pack();

			WriteDataToHeader(headerPath, (uint8_t*)packedExeSong, packedExeSong->size);
		}
	}
	else
	{
		printf("No file specified as argument.");
		return -1;
	}

	return 0;
}

