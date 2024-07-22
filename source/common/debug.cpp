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

/*
	JA: File rendered obsolete (2011-08-12)

#include "debug.h"

#include <iostream>
using namespace std;
#include <time.h>
#include <string.h>
#ifdef WIN32
	#include <process.h>

	#define snprintf	_snprintf
	#define vsnprintf	_vsnprintf
	#define strncasecmp	_strnicmp
	#define strcasecmp  _stricmp
#else
	#include <sys/types.h>
	#include <unistd.h>
	#include <stdarg.h>
#endif
#include "../common/MiscFunctions.h"

EQEMuLog* LogFile = new EQEMuLog;
AutoDelete<EQEMuLog> adlf(&LogFile);

static const char* FileNames[EQEMuLog::MaxLogID] = { "logs/eq2emu", "logs/eq2emu", "logs/eq2emu_error", "logs/eq2emu_debug", "logs/eq2emu_quest", "logs/eq2emu_commands" };
static const char* LogNames[EQEMuLog::MaxLogID] = { "Status", "Normal", "Error", "Debug", "Quest", "Command" };

EQEMuLog::EQEMuLog() {
	for (int i=0; i<MaxLogID; i++) {
		fp[i] = 0;
#if EQDEBUG >= 2
		pLogStatus[i] = 1 | 2;
#else
		pLogStatus[i] = 0;
#endif
		logCallbackFmt[i] = NULL;
		logCallbackBuf[i] = NULL;
	}
#if EQDEBUG < 2
	pLogStatus[Status] = 3;
	pLogStatus[Error] = 3;
	pLogStatus[Debug] = 3;
	pLogStatus[Quest] = 2;
	pLogStatus[Commands] = 2;
#endif
}

EQEMuLog::~EQEMuLog() {
	for (int i=0; i<MaxLogID; i++) {
		if (fp[i])
			fclose(fp[i]);
	}
}

bool EQEMuLog::open(LogIDs id) {
	if (id >= MaxLogID) {
		return false;
    }
	LockMutex lock(&MOpen);
	if (pLogStatus[id] & 4) {
		return false;
    }
    if (fp[id]) {
        return true;
    }

	char exename[200] = "";
#if defined(WORLD)
	snprintf(exename, sizeof(exename), "_world");
#elif defined(ZONE)
	snprintf(exename, sizeof(exename), "_zone");
#endif
	char filename[200];
#ifndef NO_PIDLOG
	snprintf(filename, sizeof(filename), "%s%s_%04i.log", FileNames[id], exename, getpid());
#else
	snprintf(filename, sizeof(filename), "%s%s.log", FileNames[id], exename);
#endif
    fp[id] = fopen(filename, "a");
    if (!fp[id]) {
		cerr << "Failed to open log file: " << filename << endl;
		pLogStatus[id] |= 4; // set file state to error
        return false;
    }
    fputs("---------------------------------------------\n",fp[id]);
    return true;
}

bool EQEMuLog::write(LogIDs id, const char *fmt, ...) {
	char buffer[4096];

	if (!this) {
		return false;
    }
	if (id >= MaxLogID) {
		return false;
    }
	bool dofile = false;
	if (pLogStatus[id] & 1) {
		dofile = open(id);
	}
	if (!(dofile || pLogStatus[id] & 2))
		return false;
	LockMutex lock(&MLog[id]);

    time_t aclock;
    struct tm *newtime;
    
    time( &aclock );                 //Get time in seconds
    newtime = localtime( &aclock );  //Convert time to struct

	if (dofile){
#ifndef NO_PIDLOG
		fprintf(fp[id], "[%04d%02d%02d %02d:%02d:%02d] ", newtime->tm_year+1900, newtime->tm_mon+1, newtime->tm_mday, newtime->tm_hour, newtime->tm_min, newtime->tm_sec);
#else
		fprintf(fp[id], "%04i [%04d%02d%02d %02d:%02d:%02d] ", getpid(), newtime->tm_year+1900, newtime->tm_mon+1, newtime->tm_mday, newtime->tm_hour, newtime->tm_min, newtime->tm_sec);
#endif
	}

	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, argptr);
	va_end(argptr);
    if (dofile)
		fprintf(fp[id], "%s\n", buffer);
	if(logCallbackFmt[id]) {
		msgCallbackFmt p = logCallbackFmt[id];
		p(id, fmt, argptr );
	}

    if (pLogStatus[id] & 2) {
		if (pLogStatus[id] & 8) {
			fprintf(stderr, "[%04d%02d%02d %02d:%02d:%02d] [%s] ", newtime->tm_year+1900, newtime->tm_mon+1, newtime->tm_mday, newtime->tm_hour, newtime->tm_min, newtime->tm_sec, LogNames[id]);
			fprintf(stderr, "%s\n", buffer);
		}
		else {
			fprintf(stdout, "[%04d%02d%02d %02d:%02d:%02d] [%s] ", newtime->tm_year+1900, newtime->tm_mon+1, newtime->tm_mday, newtime->tm_hour, newtime->tm_min, newtime->tm_sec, LogNames[id]);
			fprintf(stdout, "%s\n", buffer);
		}
	}
    if (dofile)
		fprintf(fp[id], "\n");
    if (pLogStatus[id] & 2) {
		if (pLogStatus[id] & 8)
			fprintf(stderr, "\n");
		else
			fprintf(stdout, "\n");
	}
    if(dofile)
      fflush(fp[id]);
    return true;
}

bool EQEMuLog::writebuf(LogIDs id, const char *buf, int8 size, int32 count) {
	if (!this) {
		return false;
    }
	if (id >= MaxLogID) {
		return false;
    }
	bool dofile = false;
	if (pLogStatus[id] & 1) {
		dofile = open(id);
	}
	if (!(dofile || pLogStatus[id] & 2))
		return false;
	LockMutex lock(&MLog[id]);

    time_t aclock;
    struct tm *newtime;
    
    time( &aclock );                 // Get time in seconds
    newtime = localtime( &aclock );  // Convert time to struct

	if (dofile){
#ifndef NO_PIDLOG
		fprintf(fp[id], "[%02d.%02d. - %02d:%02d:%02d] ", newtime->tm_mon+1, newtime->tm_mday, newtime->tm_hour, newtime->tm_min, newtime->tm_sec);
#else
		fprintf(fp[id], "%04i [%02d.%02d. - %02d:%02d:%02d] ", getpid(), newtime->tm_mon+1, newtime->tm_mday, newtime->tm_hour, newtime->tm_min, newtime->tm_sec);
#endif
	}

	if (dofile) {
		fwrite(buf, size, count, fp[id]);
		fprintf(fp[id], "\n");
	}
	if(logCallbackBuf[id]) {
		msgCallbackBuf p = logCallbackBuf[id];
		p(id, buf, size, count);
	}
    if (pLogStatus[id] & 2) {
		if (pLogStatus[id] & 8) {
			fprintf(stderr, "[%s] ", LogNames[id]);
			fwrite(buf, size, count, stderr);
			fprintf(stderr, "\n");
		} else {
			fprintf(stdout, "[%s] ", LogNames[id]);
			fwrite(buf, size, count, stdout);
			fprintf(stdout, "\n");
		}
	}
    if(dofile)
      fflush(fp[id]);
    return true;
}

bool EQEMuLog::writeNTS(LogIDs id, bool dofile, const char *fmt, ...) {
	char buffer[4096];
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, argptr);
	va_end(argptr);
	if (dofile)
		fprintf(fp[id], "%s\n", buffer);
    if (pLogStatus[id] & 2) {
		if (pLogStatus[id] & 8)
			fprintf(stderr, "%s\n", buffer);
		else
			fprintf(stdout, "%s\n", buffer);
	}
    return true;
};

bool EQEMuLog::Dump(LogIDs id, int8* data, int32 size, int32 cols, int32 skip) {
	if (!this) {
#if EQDEBUG >= 10
    cerr << "Error: Dump() from null pointer"<<endl;
#endif
		return false;
    }
	if (size == 0)
		return true;
	if (!LogFile)
		return false;
	if (id >= MaxLogID)
		return false;
	bool dofile = false;
	if (pLogStatus[id] & 1) {
		dofile = open(id);
	}
	if (!(dofile || pLogStatus[id] & 2))
		return false;
	LockMutex lock(&MLog[id]);
	write(id, "Dumping Packet: %i", size);
	// Output as HEX
	int j = 0; char* ascii = new char[cols+1]; memset(ascii, 0, cols+1);
	int32 i;
    for(i=skip; i<size; i++) {
		if ((i-skip)%cols==0) {
			if (i != skip)
				writeNTS(id, dofile, " | %s\n", ascii);
			writeNTS(id, dofile, "%4i: ", i-skip);
			memset(ascii, 0, cols+1);
			j = 0;
		}
		else if ((i-skip)%(cols/2) == 0) {
			writeNTS(id, dofile, "- ");
		}
		writeNTS(id, dofile, "%02X ", (unsigned char)data[i]);

		if (data[i] >= 32 && data[i] < 127)
			ascii[j++] = data[i];
		else
			ascii[j++] = '.';
    }
	int32 k = ((i-skip)-1)%cols;
	if (k < 8)
		writeNTS(id, dofile, "  ");
	for (int32 h = k+1; h < cols; h++) {
		writeNTS(id, dofile, "   ");
	}
	writeNTS(id, dofile, " | %s\n", ascii);
	if (dofile)
		fflush(fp[id]);
	safe_delete_array(ascii);
	return true;
}
	
void EQEMuLog::SetCallback(LogIDs id, msgCallbackFmt proc) {
	if (!this)
		return;
	if (id >= MaxLogID) {
		return;
    }
    logCallbackFmt[id] = proc;
}

void EQEMuLog::SetCallback(LogIDs id, msgCallbackBuf proc) {
	if (!this)
		return;
	if (id >= MaxLogID) {
		return;
    }
    logCallbackBuf[id] = proc;
}

void EQEMuLog::SetAllCallbacks(msgCallbackFmt proc) {
	if (!this)
		return;
	int r;
	for(r = Status; r < MaxLogID; r++) {
		SetCallback((LogIDs)r, proc);
	}
}

void EQEMuLog::SetAllCallbacks(msgCallbackBuf proc) {
	if (!this)
		return;
	int r;
	for(r = Status; r < MaxLogID; r++) {
		SetCallback((LogIDs)r, proc);
	}
}
*/