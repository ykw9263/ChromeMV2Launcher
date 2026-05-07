#pragma once
#include "Windows.h"
#ifndef MAX_PATH 
	#define MAX_PATH 260
#endif


int getTextSectionHeader(const char* filePath, PIMAGE_SECTION_HEADER textSectionOut);