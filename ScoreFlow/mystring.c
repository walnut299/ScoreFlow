#include "stdafx.h"

char** Split(char* src, const char* gb, int* splitSize)
{
	int srcLen = strlen(src);
	int gbLen = strlen(gb);
	char** ret = (char**)malloc(srcLen * sizeof(char*));
	int ssplitSize = 0;
	memset(ret, 0, srcLen);
	int i;	
	int s = 0;
	for (i = 0; src[i] != '\0'; i++) {
		int k = i;
		BOOL mapping = TRUE;
		int j;
		for (j = 0; gb[j] != '\0'; j++) {
			if (src[k] == '\0') {
				mapping = FALSE;
				break;
			}
			if (src[k] != gb[j]) {
				mapping = FALSE;
				break;
			}
			k++;
		}
		if (mapping) {
			char* temp = (char*)malloc(i - s + 1);
			memset(temp, 0, i - s + 1);
			memcpy(temp, &src[s], i - s);
			ret[ssplitSize++] = temp;
			i = i + j - 1;
			s = i+1;
			continue;
		}	
		int t = i + j;
		i = t;
	}

	if (i != s) {
		char* temp = (char*)malloc(i - s + 1);
		memset(temp, 0, i - s + 1);
		memcpy(temp, &src[s], i - s);
		ret[ssplitSize++] = temp;
	}
	*splitSize = ssplitSize;
	return ret;
}

void FreeSplit(char** split, int splitSize) {
	for (int i = 0; i < splitSize; i++) {
		free(split[i]);
	}
	free(split);
}

int StartsWith(const char* str, const char* prefix){
	size_t lenPrefix = strlen(prefix);
	size_t lenStr = strlen(str);

	if (lenStr < lenPrefix) {
		return 0; 
	}

	return strncmp(str, prefix, lenPrefix) == 0;
}
