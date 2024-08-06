
#include "utlis.h"
#include "stdafx.h"

int Readfile(const char* filename, char** buffer) {

	char sFilename[256] = "";
	sprintf_s(sFilename, sizeof(sFilename), ".%s", filename);

	FILE* file;
	errno_t err = fopen_s(&file, sFilename, "rb");
	if (err != 0 || file == NULL) {
		printf("Could not open file %s\n", sFilename);
		return -1;
	}

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	rewind(file);

	*buffer = (char*)malloc(fileSize + 1);
	if (*buffer == NULL) {
		printf("Memory allocation failed\n");
		return -1;
	}

	size_t bytesRead = fread(*buffer, 1, fileSize, file);
	if (bytesRead != fileSize) {
		printf("File read error\n");
		free(*buffer);
		fclose(file);
		return -1;
	}

	(*buffer)[fileSize] = '\0';
	fclose(file);
	return fileSize;
}
