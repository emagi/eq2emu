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
#ifndef EQDEBUG_H
#define EQDEBUG_H

// Debug Levels
/*
	1 = Normal
	3 = Some extended debug info
	5 = Light DETAIL info
	7 = Heavy DETAIL info
	9 = DumpPacket/PrintPacket
	You should use even numbers too, to define any subset of the above basic template
*/
#ifndef EQDEBUG
	#define EQDEBUG 1
#endif


#if defined(DEBUG) && defined(WIN32)
	//#ifndef _CRTDBG_MAP_ALLOC
		#include <stdlib.h>
		#include <crtdbg.h>
		#if (_MSC_VER < 1300)
			#include <new>
			#include <memory>
			#define _CRTDBG_MAP_ALLOC
			#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
			#define malloc(s) _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
		#endif
	//#endif
#endif

#ifndef ThrowError
	void CatchSignal(int);
	#if defined(CATCH_CRASH) || defined(_EQDEBUG)
		#define ThrowError(errstr)	{ cout << "Fatal error: " << errstr << " (" << __FILE__ << ", line " << __LINE__ << ")" << endl; LogWrite(WORLD__ERROR, 0, "Debug", "Thrown Error: %s (%s:%i)", errstr, __FILE__, __LINE__); throw errstr; }
	#else
		#define ThrowError(errstr)	{ cout << "Fatal error: " << errstr << " (" << __FILE__ << ", line " << __LINE__ << ")" << endl; LogWrite(WORLD__ERROR, 0, "Debug", "Thrown Error: %s (%s:%i)", errstr, __FILE__, __LINE__); CatchSignal(0); }
	#endif
#endif

#ifdef WIN32
	// VS6 doesn't like the length of STL generated names: disabling
	#pragma warning(disable:4786)
#endif

#ifndef WIN32
	#define DebugBreak()			if(0) {}
#endif

#ifdef WIN32
	#include <WinSock2.h>
	#include <windows.h>
#endif

#include "../common/Mutex.h"
#include <stdio.h>
#include <stdarg.h>


class EQEMuLog {
public:
	EQEMuLog();
	~EQEMuLog();

	enum LogIDs {
		Status = 0,	//this must stay the first entry in this list
		Normal,
		Error,
		Debug,
		Quest,
		Commands,
		MaxLogID
	};
	
	//these are callbacks called for each
	typedef void (* msgCallbackBuf)(LogIDs id, const char *buf, int8 size, int32 count);
	typedef void (* msgCallbackFmt)(LogIDs id, const char *fmt, va_list ap);
	
	void SetAllCallbacks(msgCallbackFmt proc);
	void SetAllCallbacks(msgCallbackBuf proc);
	void SetCallback(LogIDs id, msgCallbackFmt proc);
	void SetCallback(LogIDs id, msgCallbackBuf proc);
	
	bool writebuf(LogIDs id, const char *buf, int8 size, int32 count);
	bool write(LogIDs id, const char *fmt, ...);
	bool Dump(LogIDs id, int8* data, int32 size, int32 cols=16, int32 skip=0);
private:
	bool open(LogIDs id);
	bool writeNTS(LogIDs id, bool dofile, const char *fmt, ...); // no error checking, assumes is open, no locking, no timestamp, no newline

	Mutex	MOpen;
	Mutex	MLog[MaxLogID];
	FILE*	fp[MaxLogID];
/* LogStatus: bitwise variable
	1 = output to file
	2 = output to stdout
	4 = fopen error, dont retry
	8 = use stderr instead (2 must be set)
*/
	int8	pLogStatus[MaxLogID];
	
	msgCallbackFmt logCallbackFmt[MaxLogID];
	msgCallbackBuf logCallbackBuf[MaxLogID];
};

//extern EQEMuLog* LogFile;

#ifdef _EQDEBUG
class PerformanceMonitor {
public:
	PerformanceMonitor(sint64* ip) {
		p = ip;
		QueryPerformanceCounter(&tmp);
	}
	~PerformanceMonitor() {
		LARGE_INTEGER tmp2;
		QueryPerformanceCounter(&tmp2);
		*p += tmp2.QuadPart - tmp.QuadPart;
	}
	LARGE_INTEGER tmp;
	sint64* p;
};
#endif
#endif
