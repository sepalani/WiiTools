#include <cstring>

#include "dol.hh"
#include "endian.hh"

void FixDolHeaderEndian(dolheader* header)
{
	for (u32 i = 0; i < 7; i += 1)
	{
		header->offsetText[i] = be32(header->offsetText[i]);
		header->addressText[i] = be32(header->addressText[i]);
		header->sizeText[i]= be32(header->sizeText[i]);
	}
	for (u32 i = 0; i < 11; i += 1)
	{
		header->offsetData[i] = be32(header->offsetData[i]);
		header->addressData[i] = be32(header->addressData[i]);
		header->sizeData[i] = be32(header->sizeData[i]);
	}
	header->addressBSS = be32(header->addressBSS);
	header->sizeBSS = be32(header->sizeBSS);
	header->entrypoint = be32(header->entrypoint);
}

u32 GetMemoryAddressDol(const void* buffer, u32 offset)
{
	dolheader header;
	memcpy(&header, buffer, sizeof(header));

	FixDolHeaderEndian(&header);

	u32 addresses[18] = { 0 };
	u32 offsets[18] = { 0 };
	u32 counter = 0;
	for (u32 i = 0; i < 7; i += 1, counter += 1)
	{
		if (header.addressText[i] == 0)
			continue;
		offsets[counter] = header.offsetText[i];
		addresses[counter] = header.addressText[i];
	}

	for (u32 i = 0; i < 11; i += 1, counter += 1)
	{
		if (header.addressText[i] == 0)
			continue;
		offsets[counter] = header.offsetData[i];
		addresses[counter] = header.addressData[i];
	}
	
	for (u32 i = 0; i < counter; i += 1)
	{
		for (u32 j = 1; j < counter; j += 1)
		{
			if (offsets[j] < offsets[j - 1] || offsets[j - 1] == 0)
			{
				u32 temp = offsets[j - 1];
				offsets[j - 1] = offsets[j];
				offsets[j] = temp;
				temp = addresses[j - 1];
				addresses[j - 1] = addresses[j];
				addresses[j] = temp;
			}
		}
	}

	u32 mem = 0;
	u32 j = 0;
	for (u32 i = 0; i < counter; i++, j++)
	{
#ifdef DEBUG
		std::cout << "offsetText[" << i << "]: " << offsets[i];
		std::cout << "\t";
		std::cout << "addressText[" << i << "]: " << addresses[i];
		std::cout << endl;
#endif
		if (offsets[i] > offset || offsets[i] == 0)
			break;
	}
	j--;
	offset += addresses[j];
	offset -= offsets[j];
#ifdef DEBUG
	std::cout << "Memory address: 0x" << std::hex << offset << std::endl;
#endif
	mem = offset;
	return mem;
}

u32 GetFileOffsetDol(const void* buffer, u32 address)
{
	dolheader header;
	memcpy(&header, buffer, sizeof(header));

	FixDolHeaderEndian(&header);

	u32 addresses[18] = { 0 };
	u32 offsets[18] = { 0 };
	u32 counter = 0;
	for (u32 i = 0; i < 7; i += 1, counter += 1)
	{
		if (header.addressText[i] == 0)
			continue;
		offsets[counter] = header.offsetText[i];
		addresses[counter] = header.addressText[i];
	}

	for (u32 i = 0; i < 11; i += 1, counter += 1)
	{
		if (header.addressText[i] == 0)
			continue;
		offsets[counter] = header.offsetData[i];
		addresses[counter] = header.addressData[i];
	}
	
	for (u32 i = 0; i < counter; i += 1)
	{
		for (u32 j = 1; j < counter; j += 1)
		{
			if (addresses[j] < addresses[j - 1] || addresses[j - 1] == 0)
			{
				u32 temp = offsets[j - 1];
				offsets[j - 1] = offsets[j];
				offsets[j] = temp;
				temp = addresses[j - 1];
				addresses[j - 1] = addresses[j];
				addresses[j] = temp;
			}
		}
	}

	u32 mem = 0;
	u32 j = 0;
	for (u32 i = 0; i < counter; i += 1, j += 1)
	{
#ifdef DEBUG
		std::cout << "offsetText[" << i << "]: " << offsets[i];
		std::cout << "\t";
		std::cout << "addressText[" << i << "]: " << addresses[i];
		std::cout << endl;
#endif
		if (addresses[i] > address || addresses[i] == 0)
			break;
	}
	j -= 1;
	address -= addresses[j];
	address += offsets[j];
#ifdef DEBUG
	std::cout << "File offset: 0x" << std::hex << address << std::endl;
#endif
	mem = address;
	return mem;
}
