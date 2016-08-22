#include <iostream>
#include <sstream>

#include "common.hh"
#include "dol.hh"
#include "endian.hh"
#include "functions.hh"

std::vector<u32> GetU32Vector(std::string code)
{
	std::vector<u32> binary;
	for(u32 ii = 0; ii < code.length(); ii+=8)
	{
		std::string sub = code.substr(ii, 8);
#ifdef DEBUG_BINARY
		std::cout << sub << std::endl;
#endif
		const char * codeptr = sub.c_str();
		u32 dword = strtoul(codeptr, NULL, 16);
#ifdef DEBUG_BINARY
		std::cout << "0x" << std::hex << dword << std::endl;
#endif
		binary.push_back( dword );
	}
#ifdef DEBUG_BINARY
	std::cout << std::endl;
#endif
	return binary;
}

void* FindFunction(char* buffer, u32 length, std::vector<u32> findme)
{
	u32 findme_length = findme.size();
	for (u32* location = (u32*)buffer; (u8*)location < (u8*)buffer + length - findme_length * 4; location++)
	{
		u32 i = 0;
		for (u32* check = location; check < location + findme_length; check++, i++) {
			if ((findme[i] > 0x10) && (be32(*check) != findme[i]))
				break;
		}
		if (i == findme_length)
			return (void*)location;
	}
	return NULL;
}

char * FindFunction( char * start , char * end , const u32 * binary , u32 length )
{
	if( length > (u32)( end - start ) ) return NULL;
	for( char * search = start ; search < ( end - length ) ; search++ )
	{
		u32 * check = (u32*)search;
		u32 i = 0;
		for( i = 0 ; i < (length / 4) ; i++ )
		{
			if(*(binary+i) < 0x10)
				continue;
			if( Big32(binary+i) != check[i] )
				break;
		}
		if ( i == (length / 4) )
			return search;
	}
	return NULL;
}

char* FindFunction(char* buffer, u32 length, const u32* findme, u32 findme_length)
{
	for (u32* location = (u32*)buffer; (u8*)location < (u8*)buffer + length - findme_length * 4; location++)
	{
		u32 i = 0;
		for (u32* check = location; check < location + findme_length; check++, i++) {
			if((findme[i]>0x10) && (be32(*check) != findme[i]))
				break;
		}
		if (i == findme_length)
			return (char*)location;
	}
	return NULL;
}

char* CheckFunction(char* buffer, u32 length, const u32* findme, u32 findme_length)
{
	u32 i = 0;
	for(u32* check = (u32*)buffer; check < (u32*)buffer + findme_length; check++, i++)
	{
		std::cout << std::hex << be32(*check)
			<< "\t" << std::hex << findme[i];
		if ((findme[i] > 0x10) && (be32(*check) != findme[i]))
			std::cout << " NOT MATCHING!!!";
		std::cout << std::endl;
	}
	if (i == findme_length)
		return (char*)buffer;
	else
		std::cout << "missed at insn " << i << std::endl;
	return NULL;
}

void FindSig( char* buffer, u32 length, std::string sig, bool dol )
{
	if ( sig.length() < 5 )
		return;
	//FIXME
	if ( sig == "---" )
		return;

	std::stringstream ss(sig);
	std::string code , unk1 , funcName;
	char space = ' ';
	std::getline( ss , code , space );
	std::getline( ss , unk1 , space );
	std::getline( ss , funcName , space );
	stripCarriageReturns( funcName );
	if ( FUNCTION_NAME_LIMIT < funcName.length() )
		return;
	std::vector< std::pair<int, std::string> > refs;
	while( !ss.eof() )
	{
		//FIXME
		std::string tempNum, tempString;
		std::getline( ss , tempNum , space );
		u32 num = strtoul(tempNum.c_str() + 1, NULL, 16);
		std::getline( ss , tempString , space );
		stripCarriageReturns( tempString );
		refs.push_back( std::pair<int, std::string>(num, tempString) );
		//refs.push_back( tempString );
	}
#ifdef DEBUG
	std::cout << "Size of refs: " << std::dec << refs.size() << std::endl;
	std::cout << funcName << std::endl;
#endif
	std::vector<u32> binary = GetU32Vector( code );

	void * func = NULL;
	func = FindFunction( buffer , length , binary );
	if ( func )
	{
		u32 offs = 0;
		u32 file_offs = (u32)((char*)func - buffer);
		if ( dol )
			offs = GetMemoryAddressDol(buffer, file_offs);
		else
			offs = file_offs + 0x80000000;
		std::cout << funcName;
		std::cout << " at ";
		std::cout << "0x" << std::hex << offs;
		std::cout << std::endl;
		for(u32 ii=0; ii < refs.size(); ii++)
		{
			std::cout << std::dec << refs[ii].first << "\t"
				<< refs[ii].second;

			char* ref_offs = (char*)func + refs[ii].first;
			u32 insn = Big32(ref_offs);
			u32 b_amt = insn ^ 0x48000000;
			if ( b_amt & 0x2000000 )
				b_amt = b_amt | 0xfd000000;
			b_amt &= 0xfffffffe;
			u32 ref_address = offs + refs[ii].first;
			ref_address += b_amt;
			if ( ( insn & 0x48000000 ) == 0x48000000 )
				std::cout << "\t" << std::hex << ref_address;

			//u32 val = GetFileOffsetDol(buffer, address);
			std::cout << std::endl;
		}
		/* show xrefs */
	}
#ifdef QUITME
	exit(1);
#endif
}

int CompareSigs( std::string sig1, std::string sig2 )
{
	//FIXME
	if ( sig1 == "---" )
		return CMP_BAD_SIG;
	if ( sig2 == "---" )
		return CMP_BAD_SIG;

	// SIG1
	std::stringstream ss(sig1);
	std::string code , unk1 , funcName;
	char space = ' ';
	std::getline( ss , code , space );
	std::getline( ss , unk1 , space );
	std::getline( ss , funcName , space );
	stripCarriageReturns( funcName );
	std::vector< std::pair<int, std::string> > refs;
	while( !ss.eof() )
	{
		//FIXME
		std::string tempNum, tempString;
		std::getline( ss , tempNum , space );
		u32 num = strtoul(tempNum.c_str() + 1, NULL, 16);
		std::getline( ss , tempString , space );
		stripCarriageReturns( tempString );
		refs.push_back( std::pair<int, std::string>(num, tempString) );
		//refs.push_back( tempString );
	}
	
	// SIG2
	std::stringstream ss2(sig2);
	std::string code2 , unk12 , funcName2;
	std::getline( ss2 , code2 , space );
	std::getline( ss2 , unk12 , space );
	std::getline( ss2 , funcName2 , space );
	stripCarriageReturns( funcName2 );
	std::vector< std::pair<int, std::string> > refs2;
	while( !ss2.eof() )
	{
		//FIXME
		std::string tempNum, tempString;
		std::getline( ss2 , tempNum , space );
		u32 num = strtoul(tempNum.c_str() + 1, NULL, 16);
		std::getline( ss2 , tempString , space );
		stripCarriageReturns( tempString );
		refs2.push_back( std::pair<int, std::string>(num, tempString) );
		//refs.push_back( tempString );
	}

	if(code != code2)
		return CMP_BAD_CODE;

	for(u32 ii=0; ii < refs.size(); ii++)
	{
		if(funcName != funcName2)
			return CMP_BAD_NAME;
		if(refs[ii].first != refs2[ii].first)
			return CMP_BAD_REFNAMES;
		if(refs[ii].second != refs2[ii].second)
			return CMP_BAD_REFOFFS;
	}

	return CMP_MATCHING;
}

void ShowSigCodeDiff(std::string sig1, std::string sig2, bool stop)
{
	//FIXME
	if ( sig1 == "---" )
		return;
	if ( sig2 == "---" )
		return;

	// SIG1
	std::stringstream ss(sig1);
	std::string code , unk1 , funcName;
	char space = ' ';
	std::getline( ss , code , space );
	std::getline( ss , unk1 , space );
	std::getline( ss , funcName , space );
	stripCarriageReturns( funcName );
	std::vector< std::pair<int, std::string> > refs;
	while( !ss.eof() )
	{
		//FIXME
		std::string tempNum, tempString;
		std::getline( ss , tempNum , space );
		u32 num = strtoul(tempNum.c_str() + 1, NULL, 16);
		std::getline( ss , tempString , space );
		stripCarriageReturns( tempString );
		refs.push_back( std::pair<int, std::string>(num, tempString) );
		//refs.push_back( tempString );
	}
	
	// SIG2
	std::stringstream ss2(sig2);
	std::string code2 , unk12 , funcName2;
	std::getline( ss2 , code2 , space );
	std::getline( ss2 , unk12 , space );
	std::getline( ss2 , funcName2 , space );
	stripCarriageReturns( funcName2 );
	std::vector< std::pair<int, std::string> > refs2;
	while( !ss2.eof() )
	{
		//FIXME
		std::string tempNum, tempString;
		std::getline( ss2 , tempNum , space );
		u32 num = strtoul(tempNum.c_str() + 1, NULL, 16);
		std::getline( ss2 , tempString , space );
		stripCarriageReturns( tempString );
		refs2.push_back( std::pair<int, std::string>(num, tempString) );
		//refs.push_back( tempString );
	}

	std::vector<u32> vec1 = GetU32Vector( code );
	std::vector<u32> vec2 = GetU32Vector( code2 );

#if 0
	std::cout << "\t\tin:" << std::endl;
#endif
	for(u32 ii = 0; ii < vec1.size(); ii++)
	{
#if 0
		std::cout << "\t\t\tchecked" << std::hex << vec1[ii] <<
			"\t" << vec2[ii] << std::endl;
#endif
		if(vec1[ii] != vec2[ii])
		{
			std::cout << "\t" << std::hex << ii <<
				" doesn't match " << 
				vec1[ii] << "\t" << vec2[ii] << std::endl;
			if(stop)
				return;
		}
	}
	return;
}

void DumpSigInfo( std::string sig )
{
	//FIXME
	if ( sig == "---" )
		return;

	std::stringstream ss(sig);
	std::string code , unk1 , funcName;
	char space = ' ';
	std::getline( ss , code , space );
	std::getline( ss , unk1 , space );
	std::getline( ss , funcName , space );
	stripCarriageReturns( funcName );
	if ( FUNCTION_NAME_LIMIT < funcName.length() )
		return;
	std::vector< std::pair<int, std::string> > refs;
	while( !ss.eof() )
	{
		//FIXME
		std::string tempNum, tempString;
		std::getline( ss , tempNum , space );
		u32 num = strtoul(tempNum.c_str() + 1, NULL, 16);
		std::getline( ss , tempString , space );
		stripCarriageReturns( tempString );
		refs.push_back( std::pair<int, std::string>(num, tempString) );
		//refs.push_back( tempString );
	}
#ifdef DEBUG
	std::cout << "Size of refs: " << std::dec << refs.size() << std::endl;
#endif
	std::cout << funcName << std::endl;
	for(u32 ii=0; ii < refs.size(); ii++)
	{
		std::cout << "\t" << std::dec << refs[ii].first << "\t"
			<< refs[ii].second << std::endl;
	}
}

std::string GetSigName( std::string sig )
{
	if ( sig == "---" )
		return "---";

	std::stringstream ss(sig);
	std::string code , unk1 , funcName;
	char space = ' ';
	std::getline( ss , code , space );
	std::getline( ss , unk1 , space );
	std::getline( ss , funcName , space );
	stripCarriageReturns( funcName );

	std::vector< std::pair<int, std::string> > refs;
	while( !ss.eof() )
	{
		//FIXME
		std::string tempNum, tempString;
		std::getline( ss , tempNum , space );
		u32 num = strtoul(tempNum.c_str() + 1, NULL, 16);
		std::getline( ss , tempString , space );
		stripCarriageReturns( tempString );
		refs.push_back( std::pair<int, std::string>(num, tempString) );
		//refs.push_back( tempString );
	}
	return funcName;
}

bool FindSigByName( std::string sig, std::string sigName )
{
	if ( sig == "---" )
		return false;

	std::stringstream ss(sig);
	std::string code , unk1 , funcName;
	char space = ' ';
	std::getline( ss , code , space );
	std::getline( ss , unk1 , space );
	std::getline( ss , funcName , space );
	stripCarriageReturns( funcName );

	std::vector< std::pair<int, std::string> > refs;
	while( !ss.eof() )
	{
		//FIXME
		std::string tempNum, tempString;
		std::getline( ss , tempNum , space );
		u32 num = strtoul(tempNum.c_str() + 1, NULL, 16);
		std::getline( ss , tempString , space );
		stripCarriageReturns( tempString );
		refs.push_back( std::pair<int, std::string>(num, tempString) );
		//refs.push_back( tempString );
	}
	if(sigName == funcName)
		return true;
	else
		return false;
}

char * FindBinary( char * start , u32 buffer_len , char * binary , u32 length )
{
	for ( u32 i = 0 ; i < ( buffer_len - length ) ; i++ )
		if ( !memcmp( start + i , binary , length ) )
			return start + i;
	return NULL;
}

char * FindBinary( char * start , u32 buffer_len , const u32 * binary , u32 length )
{
	for ( u32 i = 0 ; i < ( buffer_len - length ) ; i++ )
	{
		u32 * check = (u32*)(start + i);
		u32 j = 0;
		for ( j = 0 ; j < length / 4 ; j++ )
			if ( check[j] != Big32(binary + j) )
				break;
		if ( j == length / 4 )
			return (start + i);
	}
	return NULL;
}

void CreateIDC( char* buffer, u32 length, std::string sig, bool dol )
{
	if ( sig.length() < 5 )
		return;
	//FIXME
	if ( sig == "---" )
		return;

	std::stringstream ss(sig);
	std::string code , unk1 , funcName;
	char space = ' ';
	std::getline( ss , code , space );
	std::getline( ss , unk1 , space );
	std::getline( ss , funcName , space );
	stripCarriageReturns( funcName );
	if ( FUNCTION_NAME_LIMIT < funcName.length() )
		return;
	std::vector< std::pair<int, std::string> > refs;
	while( !ss.eof() )
	{
		//FIXME
		std::string tempNum, tempString;
		std::getline( ss , tempNum , space );
		u32 num = strtoul(tempNum.c_str() + 1, NULL, 16);
		std::getline( ss , tempString , space );
		stripCarriageReturns( tempString );
		refs.push_back( std::pair<int, std::string>(num, tempString) );
		//refs.push_back( tempString );
	}
#ifdef DEBUG
	std::cout << "Size of refs: " << std::dec << refs.size() << std::endl;
	std::cout << funcName << std::endl;
#endif
	std::vector<u32> binary = GetU32Vector( code );

	void * func = NULL;
	func = FindFunction( buffer , length , binary );
	if ( func )
	{
		u32 offs = 0;
		u32 file_offs = (u32)((char*)func - buffer);
		if ( dol )
			offs = GetMemoryAddressDol(buffer, file_offs);
		else
			offs = file_offs + 0x80000000;
		std::cout << "MakeFunction(0x" << std::hex << offs <<
			", BADADDR); MakeName(0x" << std::hex << offs <<
			", \"" << funcName << "\");" << std::endl;
		for(u32 ii=0; ii < refs.size(); ii++)
		{
			/*
			std::cout << std::dec << refs[ii].first << "\t"
				<< refs[ii].second;
			*/

			char* ref_offs = (char*)func + refs[ii].first;
			u32 insn = Big32(ref_offs);
			u32 b_amt = insn ^ 0x48000000;
			if ( b_amt & 0x2000000 )
				b_amt = b_amt | 0xfd000000;
			b_amt &= 0xfffffffe;
			u32 ref_address = offs + refs[ii].first;
			ref_address += b_amt;
			if ( ( insn & 0x48000000 ) == 0x48000000 )
			{
				//std::cout << "\t" << std::hex << ref_address;
				std::cout << "MakeFunction(0x" << std::hex <<
					ref_address << ", BADADDR); MakeName(0x" <<
					std::hex << ref_address << ", \"" <<
					refs[ii].second << "\");" << std::endl;
			}

			//u32 val = GetFileOffsetDol(buffer, address);
			//std::cout << std::endl;
		}
		/* show xrefs */
	}
#ifdef QUITME
	exit(1);
#endif
}

m_sig ParseMegaLine(std::string sig)
{
	m_sig msig;
	msig.code = "";
	msig.unk1 = "0000:";
	msig.funcName = "";
	std::vector< std::pair<int, std::string> > refs;
	msig.refs = refs;
	if ( sig.length() < 5 )
		return msig;
	//FIXME
	if ( sig == "---" )
		return msig;

	std::stringstream ss(sig);
	std::string code , unk1 , funcName;
	char space = ' ';
	std::getline( ss , code , space );
	std::getline( ss , unk1 , space );
	std::getline( ss , funcName , space );
	stripCarriageReturns( funcName );
	msig.code = code;
	msig.unk1 = unk1;
	msig.funcName = funcName;

	while( !ss.eof() )
	{
		//FIXME
		std::string tempNum, tempString;
		std::getline( ss , tempNum , space );
		u32 num = strtoul(tempNum.c_str() + 1, NULL, 16);
		std::getline( ss , tempString , space );
		stripCarriageReturns( tempString );
		refs.push_back( std::pair<int, std::string>(num, tempString) );
		//refs.push_back( tempString );
	}
	msig.refs = refs;
	return msig;
}

function_instance FindMSig(char* buffer, u32 length, u32 offset, m_sig sig, bool dol)
{
#if 0
	std::cout << "buffer: " << std::hex << (u32)buffer << " [DEBUG]" << std::endl
		<< "length: " << std::hex << length << " [DEBUG]" << std::endl
		<< "offset: " << std::hex << offset << " [DEBUG]" << std::endl
		<< "code: " << sig.code << " [DEBUG]" << std::endl;
#endif
	function_instance instance;
	instance.sig = sig;
	instance.buffer_location = NULL;
	instance.memory_address = 0;
	std::vector<u32> binary = GetU32Vector( sig.code );

	void * func = NULL;
	func = FindFunction( buffer , length , binary );
#if 0
	std::cout << sig.funcName << ": " << std::hex << (u32)func
		<< " [DEBUG]" << std::endl;
#endif
	if ( func )
	{
		instance.buffer_location = (char*)func;
		u32 offs = 0;
		u32 file_offs = (u32)((char*)func - buffer) + offset;
		if ( dol )
			offs=GetMemoryAddressDol(buffer-offset, file_offs);
		else
			offs = file_offs + 0x80000000;
		instance.memory_address = offs;
	}
	return instance;
}

