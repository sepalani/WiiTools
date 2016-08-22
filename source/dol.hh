#pragma once

#include "types.hh"

typedef struct
{
	u32 offsetText[7];		// 0	// 0x00
	u32 offsetData[11];		// 28	// 0x1c
	u32 addressText[7];		// 72	// 0x48
	u32 addressData[11];	// 100	// 0x64
	u32 sizeText[7];		// 144	// 0x90
	u32 sizeData[11];		// 172	// 0xac
	u32 addressBSS;			// 216	// 0xd8
	u32 sizeBSS;			// 220	// 0xdc
	u32 entrypoint;			// 224	// 0xe0
} dolheader;

void FixDolHeaderEndian(dolheader* header);
u32 GetMemoryAddressDol(const void* buffer, u32 offset);
u32 GetFileOffsetDol(const void* buffer, u32 address);
