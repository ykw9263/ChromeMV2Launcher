#include <iostream>
#include "ChromeDLLScanner.h"
#include "PEParser.h"
#include "PatternSearcher.h"

constexpr UINT64 BP_OP_OFFSET = 0x0e;

UINT64 scanTgtBreakPointOffset(char* pathToDLL)
{   
    if (strnlen_s(pathToDLL, MAX_PATH) >= MAX_PATH) {
        fprintf(stderr, "Path too long\n");
        exit(ERROR_MRM_FILEPATH_TOO_LONG);
    }
    char filePath[MAX_PATH] = { 0 };
    strncpy_s(filePath, MAX_PATH, pathToDLL, MAX_PATH);
    IMAGE_SECTION_HEADER textSectionHeader;
    
    if (getTextSectionHeader(filePath, &textSectionHeader)) {
        fprintf(stderr, "[!] .text section not found\n");
        exit(1);
    }

    printf("\n found .text section\n");
    printf("%s\n", textSectionHeader.Name);
    printf("\t%-38s 0x%X\n", "VirtualAddress", textSectionHeader.VirtualAddress);
    printf("\t%-38s 0x%X\n", "PointerToRawData", textSectionHeader.PointerToRawData);
    printf("\t%-38s 0x%X\n", "SizeOfRawData", textSectionHeader.SizeOfRawData);

    UINT64 opcodeOffset = 0;
    opcodeOffset = searchPattern(filePath, textSectionHeader.PointerToRawData, textSectionHeader.SizeOfRawData);
    if (opcodeOffset == -1) {
        fprintf(stderr, "[!] pattern not found\n");
        exit(1);
    }

    UINT64 opcodeVAOffset = opcodeOffset + textSectionHeader.VirtualAddress - textSectionHeader.PointerToRawData;
    UINT64 breakpointOffset = opcodeVAOffset + BP_OP_OFFSET;
    printf("Calculated breakpoint offset: 0x%llx\n", breakpointOffset);

    return breakpointOffset;

}