#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include "common.hh"

void ascii(u8 character)
{
	std::cout << (char)(
		(character < 0x1e ||character > 0x7f) ? '.' : character
	);
}

void hexdump(void* pointer, u32 length)
{
#ifdef SHORT_DUMP
	u32 go_to = 10;
#else
	u32 go_to = (length + 0xF) >> 4;
#endif

#ifdef SAFETY
	if (go_to > 16)
		go_to = 16;
#endif

	u8* ptr = (u8*)pointer;
	for (u32 j = 0; j < go_to; j += 1)
	{
		for (u32 i = 0; i < 0x10; i += 1)
		{
			std::cout << std::setfill('0') << std::setw(2);
			std::cout << std::hex << ptr[i] << " ";
		}
		for (u32 i = 0; i < 0x10; i += 1)
			ascii(ptr[i]);
		std::cout << std::endl;
	}
}

void printStringVector(const std::vector<std::string>& lines)
{
	for (u32 i = 0; i < lines.size(); i += 1)
		std::cout << lines[i] << std::endl;
}

void printCharVector(const std::vector<char>& line)
{
	for (u32 i = 0; i < line.size(); i += 1)
		std::cout << line[i];
	std::cout << std::endl;
}

void stripCarriageReturns(std::string& StringToModify)
{
	if (StringToModify.empty())
		return;

	int startIndex = StringToModify.find_first_not_of("\r");
	int endIndex = StringToModify.find_last_not_of("\r");
	std::string tempString = StringToModify;

	StringToModify.erase();
	StringToModify = tempString.substr(startIndex, (endIndex - startIndex + 1));
}

u32 ReadFile(const char* filename, char* buffer)
{
	std::ifstream save(filename);

	if (!save)
	{
		std::cout << "File ";
		std::cout << filename << "could not be opened" << std::endl;
		exit(EXIT_FAILURE);
	}

	save.seekg(0, std::ifstream::end);
	u32 saveSize = save.tellg();
	save.seekg(0);

	buffer = new char[saveSize];
	save.read(buffer, saveSize);
	save.close();
	return saveSize;
}

std::vector<char> readLine1(const char* buffer, int len)
{
	std::vector<char> line;
	const char* end = buffer + len;
	for (; buffer < end; buffer += 1)
		if (*buffer != '\n')
			line.push_back(*buffer);
		else
			return line;
	return line;
}

#if 0
string readLine2(const char * buffer, int len)
{
	string line;
	line.empty();
	for (const char* spot = buffer; spot < buffer + len; spot++)
		if (*spot != '\n')
			line.append(*spot);
		else
			return line;
	return line;
}
#endif
