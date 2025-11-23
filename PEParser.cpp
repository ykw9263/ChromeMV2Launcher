/*
 * Adapted from PEParser64.cpp by jhalon
 * URL: https://gist.github.com/jhalon/96ec0642014ef8bb75d37ea4cf022aca
 *
*/

#include <iostream>
#include "PEParser.h"

#define HEAP_ALLOC_FLAGS HEAP_GENERATE_EXCEPTIONS

int getTextSectionHeader(const char* filePath, PIMAGE_SECTION_HEADER textSectionOut)
{
    HANDLE hFile;
    DWORD lpNumberOfBytesRead, lpTotalNumberOfByteRead = 0;
    LARGE_INTEGER fileOffsetPointer = { 0 };
    PIMAGE_DOS_HEADER dosHeader = { 0 };
    PIMAGE_NT_HEADERS ntHeader = { 0 };
    PIMAGE_SECTION_HEADER sectionHeader = { 0 };

    // Acquire file handle to target file
    hFile = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("[!] Failed to open file handle: (0x%X)\n", GetLastError());
        CloseHandle(hFile);
        return 1;
    }

    // Prase DOS Header

    DWORD dosHeaderSize = sizeof(IMAGE_DOS_HEADER);
    dosHeader = (PIMAGE_DOS_HEADER)HeapAlloc(GetProcessHeap(), HEAP_ALLOC_FLAGS, dosHeaderSize);

    if (!ReadFile(hFile, dosHeader, dosHeaderSize, &lpNumberOfBytesRead, NULL))
    {
        printf("[!] Failed to read file: (0x%X)\n", GetLastError());
        CloseHandle(hFile);
        return 1;
    }
    lpTotalNumberOfByteRead += lpNumberOfBytesRead;
    lpNumberOfBytesRead = 0;


    //printf("\n========= DOS HEADER =========\n");
    //printf("%-38s 0x%X\n", "Magic number", dosHeader->e_magic);
    //printf("%-38s 0x%X\n", "Bytes on last page of file", dosHeader->e_cblp);
    //printf("%-38s 0x%X\n", "Pages in file", dosHeader->e_cp);
    //printf("%-38s 0x%X\n", "Relocations", dosHeader->e_crlc);
    //printf("%-38s 0x%X\n", "Size of header in paragraphs", dosHeader->e_cparhdr);
    //printf("%-38s 0x%X\n", "Minimum extra paragraphs needed", dosHeader->e_minalloc);
    //printf("%-38s 0x%X\n", "Maximum extra paragraphs needed", dosHeader->e_maxalloc);
    //printf("%-38s 0x%X\n", "Initial (relative) SS value", dosHeader->e_ss);
    //printf("%-38s 0x%X\n", "Initial SP value", dosHeader->e_sp);
    //printf("%-38s 0x%X\n", "Checksum", dosHeader->e_csum);
    //printf("%-38s 0x%X\n", "Initial IP value", dosHeader->e_ip);
    //printf("%-38s 0x%X\n", "Initial (relative) CS value", dosHeader->e_cs);
    //printf("%-38s 0x%X\n", "File address of relocation tabl", dosHeader->e_lfarlc);
    //printf("%-38s 0x%X\n", "Overlay Number", dosHeader->e_ovno);
    //printf("%-38s 0x%X\n", "OEM identifier (for e_oeminfo)", dosHeader->e_oemid);
    //printf("%-38s 0x%X\n", "OEM information; e_oemid specific", dosHeader->e_oeminfo);
    //printf("%-38s 0x%X\n", "File address of new exe header", dosHeader->e_lfanew);


    // Parse NT Header

    DWORD ntHeaderOffset = dosHeader->e_lfanew;
    fileOffsetPointer.LowPart = ntHeaderOffset;
    SetFilePointerEx(hFile, fileOffsetPointer, NULL, FILE_BEGIN);

    // free dos header buffer
    HeapFree(GetProcessHeap(), NULL, dosHeader);
    dosHeader = NULL;

    DWORD ntHeaderSize = sizeof(IMAGE_NT_HEADERS);
    ntHeader = (PIMAGE_NT_HEADERS)HeapAlloc(GetProcessHeap(), HEAP_ALLOC_FLAGS, ntHeaderSize);

    if (!ReadFile(hFile, ntHeader, ntHeaderSize, &lpNumberOfBytesRead, NULL))
    {
        printf("[!] Failed to read file: (0x%X)\n", GetLastError());
        CloseHandle(hFile);
        return 1;
    }
    lpTotalNumberOfByteRead += lpNumberOfBytesRead;
    lpNumberOfBytesRead = 0;


    //printf("\n========= NT HEADER =========\n");
    //printf("%-38s 0x%X\n", "Signature", ntHeader->Signature);

    //printf("\n========= FILE HEADER =========\n");
    //printf("%-38s 0x%X\n", "Machine", ntHeader->FileHeader.Machine);
    //printf("%-38s 0x%X\n", "NumberOfSections", ntHeader->FileHeader.NumberOfSections);
    //printf("%-38s 0x%X\n", "TimeDateStamp", ntHeader->FileHeader.TimeDateStamp);
    //printf("%-38s 0x%X\n", "PointerToSymbolTable", ntHeader->FileHeader.PointerToSymbolTable);
    //printf("%-38s 0x%X\n", "NumberOfSymbols", ntHeader->FileHeader.NumberOfSymbols);
    //printf("%-38s 0x%X\n", "SizeOfOptionalHeader", ntHeader->FileHeader.SizeOfOptionalHeader);
    //printf("%-38s 0x%X\n", "Characteristics", ntHeader->FileHeader.Characteristics);

    //// Prase Optional Header
    //printf("\n========= OPTIONAL HEADER =========\n");
    //printf("%-38s 0x%X\n", "Magic", ntHeader->OptionalHeader.Magic);
    //printf("%-38s 0x%X\n", "MajorLinkerVersion", ntHeader->OptionalHeader.MajorLinkerVersion);
    //printf("%-38s 0x%X\n", "MinorLinkerVersion", ntHeader->OptionalHeader.MinorLinkerVersion);
    //printf("%-38s 0x%X\n", "SizeOfCode", ntHeader->OptionalHeader.SizeOfCode);
    //printf("%-38s 0x%X\n", "SizeOfInitializedData", ntHeader->OptionalHeader.SizeOfInitializedData);
    //printf("%-38s 0x%X\n", "SizeOfUninitializedData", ntHeader->OptionalHeader.SizeOfUninitializedData);
    //printf("%-38s 0x%X\n", "AddressOfEntryPoint", ntHeader->OptionalHeader.AddressOfEntryPoint);
    //printf("%-38s 0x%X\n", "BaseOfCode", ntHeader->OptionalHeader.BaseOfCode);
    //printf("%-38s 0x%llx\n", "ImageBase", ntHeader->OptionalHeader.ImageBase);
    //printf("%-38s 0x%X\n", "SectionAlignment", ntHeader->OptionalHeader.SectionAlignment);
    //printf("%-38s 0x%X\n", "FileAlignment", ntHeader->OptionalHeader.FileAlignment);
    //printf("%-38s 0x%X\n", "MajorOperatingSystemVersion", ntHeader->OptionalHeader.MajorOperatingSystemVersion);
    //printf("%-38s 0x%X\n", "MinorOperatingSystemVersion", ntHeader->OptionalHeader.MinorOperatingSystemVersion);
    //printf("%-38s 0x%X\n", "MajorImageVersion", ntHeader->OptionalHeader.MajorImageVersion);
    //printf("%-38s 0x%X\n", "MinorImageVersion", ntHeader->OptionalHeader.MinorImageVersion);
    //printf("%-38s 0x%X\n", "MajorSubsystemVersion", ntHeader->OptionalHeader.MajorSubsystemVersion);
    //printf("%-38s 0x%X\n", "MinorSubsystemVersion", ntHeader->OptionalHeader.MinorSubsystemVersion);
    //printf("%-38s 0x%X\n", "Win32VersionValue", ntHeader->OptionalHeader.Win32VersionValue);
    //printf("%-38s 0x%X\n", "SizeOfImage", ntHeader->OptionalHeader.SizeOfImage);
    //printf("%-38s 0x%X\n", "SizeOfHeaders", ntHeader->OptionalHeader.SizeOfHeaders);
    //printf("%-38s 0x%X\n", "CheckSum", ntHeader->OptionalHeader.CheckSum);
    //printf("%-38s 0x%x\n", "Subsystem", ntHeader->OptionalHeader.Subsystem);
    //printf("%-38s 0x%X\n", "DllCharacteristics", ntHeader->OptionalHeader.DllCharacteristics);
    //printf("%-38s 0x%llx\n", "SizeOfStackReserve", ntHeader->OptionalHeader.SizeOfStackReserve);
    //printf("%-38s 0x%llx\n", "SizeOfStackCommit", ntHeader->OptionalHeader.SizeOfStackCommit);
    //printf("%-38s 0x%llx\n", "SizeOfHeapReserve", ntHeader->OptionalHeader.SizeOfHeapReserve);
    //printf("%-38s 0x%llx\n", "SizeOfHeapCommit", ntHeader->OptionalHeader.SizeOfHeapCommit);
    //printf("%-38s 0x%lx\n", "LoaderFlags", ntHeader->OptionalHeader.LoaderFlags);
    //printf("%-38s 0x%lx\n", "NumberOfRvaAndSizes", ntHeader->OptionalHeader.NumberOfRvaAndSizes);

    // get number of sections
    WORD numberOfSections = ntHeader->FileHeader.NumberOfSections;
    HeapFree(GetProcessHeap(), NULL, ntHeader);
    ntHeader = NULL;

    //// Calculate offset to the first section header
    DWORD sectionHeaderOffset = ntHeaderOffset + sizeof(IMAGE_NT_SIGNATURE) + sizeof(IMAGE_FILE_HEADER) + sizeof(IMAGE_OPTIONAL_HEADER);
    fileOffsetPointer.LowPart = sectionHeaderOffset;
    SetFilePointerEx(hFile, fileOffsetPointer, NULL, FILE_BEGIN);
    DWORD sectionHeaderSize = sizeof(IMAGE_SECTION_HEADER);

    // Prase Section Header
    //printf("\n========= .text SECTION HEADER =========\n");
    boolean textSectionFound = false;
    // Loop through all our section headers
    for (int i = 0; i < numberOfSections; i++)
    {
        sectionHeader = (PIMAGE_SECTION_HEADER)HeapAlloc(GetProcessHeap(), HEAP_ALLOC_FLAGS, sectionHeaderSize);
        if (!ReadFile(hFile, sectionHeader, sectionHeaderSize, &lpNumberOfBytesRead, NULL))
        {
            printf("[!] Failed to read file: (0x%X)\n", GetLastError());
            CloseHandle(hFile);
            return 1;
        }
        lpTotalNumberOfByteRead += lpNumberOfBytesRead;
        lpNumberOfBytesRead = 0;

        if (strncmp((char*)sectionHeader->Name, ".text", 5) != 0) {
            continue;
        }
        //printf("%s\n", sectionHeader->Name);
        //printf("\t%-38s 0x%X\n", "VirtualSize", sectionHeader->Misc.VirtualSize);
        //printf("\t%-38s 0x%X\n", "VirtualAddress", sectionHeader->VirtualAddress);
        //printf("\t%-38s 0x%X\n", "SizeOfRawData", sectionHeader->SizeOfRawData);
        //printf("\t%-38s 0x%X\n", "PointerToRawData", sectionHeader->PointerToRawData);
        //printf("\t%-38s 0x%X\n", "PointerToRelocations", sectionHeader->PointerToRelocations);
        //printf("\t%-38s 0x%X\n", "PointerToLinenumbers", sectionHeader->PointerToLinenumbers);
        //printf("\t%-38s 0x%X\n", "NumberOfRelocations", sectionHeader->NumberOfRelocations);
        //printf("\t%-38s 0x%X\n", "NumberOfLinenumbers", sectionHeader->NumberOfLinenumbers);
        //printf("\t%-38s 0x%X\n", "Characteristics", sectionHeader->Characteristics);
        textSectionFound = true;
        break;
    }

    if (!textSectionFound || sectionHeader == 0) {
        printf("[!] .text section not found\n");
        CloseHandle(hFile);
        return 1;
    }

    memcpy(textSectionOut, sectionHeader, sectionHeaderSize);
    HeapFree(GetProcessHeap(), NULL, sectionHeader);
    sectionHeader = NULL;

    // Cleanup
    CloseHandle(hFile);
    return 0;
}