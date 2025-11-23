#include <iostream>
#include <vector>
#include <string_view>
#include <algorithm>
#include <functional>
#include "PatternSearcher.h"

#define HEAP_ALLOC_FLAGS HEAP_GENERATE_EXCEPTIONS
constexpr UINT16 WILDCARD = 0x1234;

const std::vector<UINT16> needle = { 
    0x41,0x57,
    0x41,0x56,
    0x56,
    0x57,
    0x53,
    0x48,0x83,0xec,0x30,
    0x48,0x89,0xce,
    0x48,WILDCARD,WILDCARD,WILDCARD,WILDCARD,WILDCARD,WILDCARD,
    0x48,0x31,0xe0,
    0x48,0x89,0x44,0x24,0x28,
    0xe8,WILDCARD,WILDCARD,WILDCARD,WILDCARD,
    0x48,0x89,0xf1,
    0xe8,WILDCARD,WILDCARD,WILDCARD,WILDCARD,
    0x83,0x7e,0x10,0x00,
    0x0f,0x84,0x98,0x00,0x00,0x00,
};

static bool wildcardPredicate(const char& _Left, const UINT16& _Right) {
    if (_Right == WILDCARD)
        return true;
    return (UINT8)_Left == (UINT8)_Right;
}


ULONG64 searchPattern(const char* filePath, DWORD startOffset, DWORD size) {
    printf("view startOffset: %d\n", startOffset);

    HANDLE hFile, hFileMap;
    //LARGE_INTEGER fileOffsetPointer = { 0 };
    DWORD lpNumberOfBytesRead, lpTotalNumberOfByteRead = 0;
    LPVOID textSectionView = nullptr;

    //fileOffsetPointer.LowPart = startOffset;

    printf("opening file: %s\n", filePath);
    // Acquire file handle to target file
    hFile = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("[!] Failed to open file handle: (0x%X)\n", GetLastError());
        CloseHandle(hFile);
        exit(1);
    }
    //SetFilePointerEx(hFile, fileOffsetPointer, NULL, FILE_BEGIN);


    hFileMap = CreateFileMappingA(hFile, NULL, PAGE_READONLY, 0, startOffset + size, "chromeDLLMap");
    if (hFileMap == NULL)
    {
        printf("[!] Failed to create file mapping: (0x%X)\n", GetLastError());
        CloseHandle(hFile);
        exit(1);
    }

    textSectionView = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, startOffset + size);
    if (textSectionView == NULL)
    {
        printf("[!] Failed to open file mapping view: (0x%X)\n", GetLastError());
        CloseHandle(hFileMap);
        CloseHandle(hFile);
        exit(1);
    }

    std::string_view sectionStrView((char*) textSectionView, size);

    auto it = std::search(sectionStrView.begin(), sectionStrView.end(),
        needle.begin(), needle.end(), wildcardPredicate);
    if (it == sectionStrView.end())
    {
        printf("Pattern not found\n");
        CloseHandle(hFileMap);
        CloseHandle(hFile);
        UnmapViewOfFile(textSectionView);
        return -1;
    }

    ULONG64 foundOffset = it - sectionStrView.begin();
    printf("Pattern found at offset: 0x%.16llx \n", foundOffset);

    // clean up
    CloseHandle(hFileMap);
    CloseHandle(hFile);
    UnmapViewOfFile(textSectionView);
    return foundOffset;
}