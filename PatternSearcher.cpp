#include <iostream>
#include <vector>
#include <utility>
#include <string_view>
#include <algorithm>
#include <functional>
#include "PatternSearcher.h"

#define HEAP_ALLOC_FLAGS HEAP_GENERATE_EXCEPTIONS
constexpr UINT16 WILDCARD = 0x1234;

const std::vector<UINT16> needle1 = {
    0x41, 0x56,
    0x56,
    0x57,
    0x53,
    0x48, 0x81, 0xec,  WILDCARD, WILDCARD, WILDCARD, WILDCARD,
    0x48, 0x8b, 0x05,  WILDCARD, WILDCARD, WILDCARD, WILDCARD,
    0x48, 0x31, 0xe0,
    0x48, 0x89, 0x84, 0x24, WILDCARD, WILDCARD, WILDCARD, WILDCARD,
    // we want to insert breakpoint here and overwrite r8d with 0x03
    0x41, 0x83, 0xf8, 0x02,
    0x7f, WILDCARD,
    0x44, 0x89, 0xce,
    0x44, 0x89, 0xc3,
    0x48, 0x89, 0xd7,
    0x49, 0x89, 0xce,
    0x41, 0x83, 0xf9, 0x01,
    0x75, WILDCARD,
};

struct Pattern {
    std::vector<UINT16> pattern;
    ULONG offset;
};

const std::vector<Pattern> needles = {
    { needle1, 0x1e },
};

//const std::vector<std::pair<std::vector<UINT16>, UINT16>> needles = { 
//    { needle1, 0x1e },
//};




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
        fprintf(stderr, "[!] Failed to open file handle: (0x%X)\n", GetLastError());
        CloseHandle(hFile);
        exit(1);
    }
    //SetFilePointerEx(hFile, fileOffsetPointer, NULL, FILE_BEGIN);


    hFileMap = CreateFileMappingA(hFile, NULL, PAGE_READONLY, 0, startOffset + size, "chromeDLLMap");
    if (hFileMap == NULL)
    {
        fprintf(stderr, "[!] Failed to create file mapping: (0x%X)\n", GetLastError());
        CloseHandle(hFile);
        exit(1);
    }

    textSectionView = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, startOffset + size);
    if (textSectionView == NULL)
    {
        fprintf(stderr, "[!] Failed to open file mapping view: (0x%X)\n", GetLastError());
        CloseHandle(hFileMap);
        CloseHandle(hFile);
        exit(1);
    }

    std::string_view sectionStrView((char*) textSectionView, size);

    for (const auto& needle : needles)
    {
        std::vector<UINT16> needlePattern =  needle.pattern;
        auto it = std::search(sectionStrView.begin(), sectionStrView.end(),
            needlePattern.begin(), needlePattern.end(), wildcardPredicate);
        if (it != sectionStrView.end())
        {
            ULONG64 foundOffset = it - sectionStrView.begin();
            printf("Pattern found at offset: 0x%.16llx \n", foundOffset);
            CloseHandle(hFileMap);
            CloseHandle(hFile);
            UnmapViewOfFile(textSectionView);
            ULONG64 targetOffset = foundOffset + needle.offset;
            return targetOffset;
        }
    }

    CloseHandle(hFileMap);
    CloseHandle(hFile);
    UnmapViewOfFile(textSectionView);
    return -1;
}