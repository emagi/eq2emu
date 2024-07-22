/*  
    EQ2Emulator:  Everquest II Server Emulator
    Copyright (C) 2007  EQ2EMulator Development Team (http://www.eq2emulator.net)

    This file is part of EQ2Emulator.

    EQ2Emulator is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    EQ2Emulator is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with EQ2Emulator.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef TYPES_H
#define TYPES_H

#include <string>

using namespace std;

//atoi is not int32 or uint32 safe!!!!
#define atoul(str) strtoul(str, NULL, 10)
#ifdef WIN32
	#define atoi64(str) _atoi64(str)
#else
	#define atoi64(str) strtoll(str, NULL, 10)
#endif
typedef unsigned char		int8;
typedef unsigned short		int16;
typedef unsigned int		int32;

typedef unsigned char		uint8;
typedef  signed  char		sint8;
typedef unsigned short		uint16;
typedef  signed  short		sint16;
typedef unsigned int		uint32;
typedef  signed  int		sint32;

#ifdef WIN32
	#if defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 64
		typedef unsigned __int64	int64;
		typedef unsigned __int64	uint64;
		typedef signed __int64		sint64;
	#else
		#error __int64 not supported
	#endif
#else
typedef unsigned long long	int64;
typedef unsigned long long	uint64;
typedef signed long long	sint64;
//typedef __u64				int64;
//typedef __u64				uint64;
//typedef __s64				sint64;
#endif

typedef unsigned long		ulong;
typedef unsigned short		ushort;
typedef unsigned char		uchar;

#ifdef WIN32
	#define snprintf	_snprintf
	#define vsnprintf	_vsnprintf
	#define strncasecmp	_strnicmp
	#define strcasecmp  _stricmp
	typedef void ThreadReturnType;
//	#define THREAD_RETURN(x) return;
	#define THREAD_RETURN(x) _endthread(); return; 
#else
	typedef void* ThreadReturnType;
	typedef int SOCKET;
	#define THREAD_RETURN(x) return(x);
#endif

#define safe_delete(d) if(d) { delete d; d=0; }
#define safe_delete_array(d) if(d) { delete[] d; d=0; }
#define L32(i)	((int32) i)
#define H32(i)	((int32) (i >> 32))
#define L16(i)	((int16) i)

#ifndef WIN32
// More WIN32 compatability
	typedef unsigned long DWORD;
	typedef unsigned char BYTE;
	typedef char CHAR;
	typedef unsigned short WORD;
	typedef float FLOAT;
	typedef FLOAT *PFLOAT;
	typedef BYTE *PBYTE,*LPBYTE;
	typedef int *PINT,*LPINT;
	typedef WORD *PWORD,*LPWORD;
	typedef long *LPLONG, LONG;
	typedef DWORD *PDWORD,*LPDWORD;
	typedef int INT;
	typedef unsigned int UINT,*PUINT,*LPUINT;
#endif


#ifdef WIN32
#define DLLFUNC extern "C" __declspec(dllexport)
#else
#define DLLFUNC extern "C"
#endif


#pragma pack(1)
struct uint16_breakdown {
	union {
		uint16 all;
		struct {
			uint8 b1;
			uint8 b2;
		} bytes;
	};
	inline uint16&	operator=(const uint16& val) { return (all=val); }
	inline uint16*	operator&() { return &all; }
	inline operator	uint16&() { return all; }
	inline uint8&	b1()	{ return bytes.b1; }
	inline uint8&	b2()	{ return bytes.b2; }
};

struct uint32_breakdown {
	union {
		uint32 all;
		struct {
			uint16 w1;
			uint16 w2;
		} words;
		struct {
			uint8 b1;
			union {
				struct {
					uint8 b2;
					uint8 b3;
				} middle;
				uint16 w2_3; // word bytes 2 to 3
			};
			uint8 b4;
		} bytes;
	};
	inline uint32&	operator=(const uint32& val) { return (all=val); }
	inline uint32*	operator&() { return &all; }
	inline operator	uint32&() { return all; }

	inline uint16&	w1()	{ return words.w1; }
	inline uint16&	w2()	{ return words.w2; }
	inline uint16&	w2_3()	{ return bytes.w2_3; }
	inline uint8&	b1()	{ return bytes.b1; }
	inline uint8&	b2()	{ return bytes.middle.b2; }
	inline uint8&	b3()	{ return bytes.middle.b3; }
	inline uint8&	b4()	{ return bytes.b4; }
};

struct EQ2_32BitString{
	int32	size;
	string	data;
};
struct EQ2_16BitString{
	int16	size;
	string	data;
};
struct EQ2_8BitString{
	int8	size;
	string	data;
};

struct EQ2_Color{
	int8	red;
	int8	green;
	int8	blue;
};

struct WorldTime{
	int16	year;
	int		month;
	int		day;
	int		hour;
	int		minute;
};

typedef enum QUERY_TYPE{ Q_SELECT, Q_UPDATE, Q_REPLACE, Q_INSERT, Q_DELETE, Q_DBMS} QUERY_TYPE;

#pragma pack()


#endif
