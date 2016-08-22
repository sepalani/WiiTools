#pragma once

#include "endian.hh"

#define OPCODE_STACKUPDATE 0x94210000
#define OPCODE_BLR 0x4e800020

template<typename T>
bool in_mem1(T value)
{
	return value >= 0x80000000 && value <= 0x817fffff;
}

template<typename T>
bool in_mem2(T value)
{
	return value >= 0x90000000 && value <= 0x93ffffff;
}

template<typename T>
T* FindStackUpdate(T* buffer, u32 length)
{
	const u32* end = (const u32*)((const u8*)buffer + length);
	for (const u32* it = (const u32*)buffer; it < end; it += 1)
	{
		if ((Big32(it) & 0xffff0000) == OPCODE_STACKUPDATE)
			return (T*)it;
	}
	return (T*)0;
}

template<typename T>
T* FindStackUpdateReverse(T* buffer, u32 length)
{
	const u32* end = (const u32*)((const u8*)buffer - length);
	for (const u32* it = (const u32*)buffer; it > end; it -= 1)
	{
		if ((Big32(it) & 0xffff0000) == OPCODE_STACKUPDATE)
			return (T*)it;
	}
	return (T*)0;
}

template<typename T>
T* FindBlr(T* buffer, u32 length)
{
	const u32* end = (const u32*)((const u8*)buffer + length);
	for (const u32* it = (const u32*)buffer; it < end; it += 1)
	{
		if (Big32(it) == OPCODE_BLR)
			return (T*)it;
	}
	return (T*)0;
}

template<typename T>
T* FindBlrReverse(T* buffer, u32 length)
{
	const u32* end = (const u32*)((const u8*)buffer - length);
	for (const u32* it = (const u32*)buffer; it > end; it -= 1)
	{
		if (Big32(it) == OPCODE_BLR)
			return (T*)it;
	}
	return (T*)0;
}
