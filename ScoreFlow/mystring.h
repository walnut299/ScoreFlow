#pragma once

char** Split(char* src, const char* gb, int* splitSize);
void FreeSplit(char** split, int splitSize);
int StartsWith(const char* str, const char* prefix);