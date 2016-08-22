#pragma once

//#define DEBUG 1
//#define DEBUG_BINARY 1
//#define QUITME 1
#define FUNCTION_NAME_LIMIT 20

#include <string>
#include <vector>

#include "types.hh"

void ascii(u8 character);
void hexdump(const void* pointer, u32 length);
void printStringVector(const std::vector<std::string>& lines);
void printCharVector(const std::vector<char>& line);
void stripCarriageReturns(std::string& StringToModify);
u32 ReadFile(const char* filename, char* buffer);
std::vector<char> readLine1(char* buffer, int len);
#if 0
std::string readLine2(char* buffer, int len);
#endif
