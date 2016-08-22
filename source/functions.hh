#pragma once

#include <string>
#include <vector>

#include "types.hh"

// SigCompare defines
#define CMP_MATCHING 0
#define CMP_BAD_CODE -1
#define CMP_BAD_NAME -2
#define CMP_BAD_REFNAMES -3
#define CMP_BAD_REFOFFS -4
#define CMP_BAD_SIG -99

typedef struct
{
	std::string	code;
	std::string unk1;
	std::string	funcName;
	std::vector< std::pair<int, std::string> > refs;
} m_sig;

typedef struct
{
	m_sig sig;
	char* buffer_location;
	u32   memory_address;
} function_instance;

std::vector<u32> GetU32Vector(std::string code);
void* FindFunction(char* buffer, u32 length, std::vector<u32> findme);
char* FindFunction(char* start, char* end, const u32* binary, u32 length);
char* FindFunction(char* buffer, u32 length, const u32* findme, u32 findme_length);
char* CheckFunction(char* buffer, u32 length, const u32* findme, u32 findme_length);
void FindSig(char* buffer, u32 length, std::string sig, bool dol);
int CompareSigs(std::string sig1, std::string sig2);
void ShowSigCodeDiff(std::string sig1, std::string sig2, bool stop=false);
void DumpSigInfo(std::string sig);
std::string GetSigName(std::string sig);
bool FindSigByName(std::string sig, std::string sigName);
char* FindBinary(char* start, u32 buffer_len, char* binary, u32 length);
char* FindBinary(char* start, u32 buffer_len, const u32* binary, u32 length);
void CreateIDC(char* buffer, u32 length, std::string sig, bool dol);
m_sig ParseMegaLine(std::string sig);
function_instance FindMSig(char* buffer, u32 length, u32 offset, m_sig sig, bool isDol);
