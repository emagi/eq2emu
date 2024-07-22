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
#ifdef WIN32
#include <WinSock2.h>
#include <windows.h>
#endif
#include "EQEMuError.h"
#include "linked_list.h"
#include "Mutex.h"
#include "MiscFunctions.h"
#include <stdio.h>
#include <string.h>
#ifdef WIN32
	#include <conio.h>
#endif

void CatchSignal(int sig_num);

const char* EQEMuErrorText[EQEMuError_MaxErrorID] = { "ErrorID# 0, No Error",
	"MySQL Error #1405 or #2001 means your mysql server rejected the username and password you presented it.",
	"MySQL Error #2003 means you were unable to connect to the mysql server.",
	"MySQL Error #2005 means you there are too many connections on the mysql server. The server is overloaded.",
	"MySQL Error #2007 means you the server is out of memory. The server is overloaded.",
	};

LinkedList<char*>* EQEMuErrorList;
Mutex* MEQEMuErrorList;
AutoDelete< LinkedList<char*> > ADEQEMuErrorList(&EQEMuErrorList);
AutoDelete<Mutex> ADMEQEMuErrorList(&MEQEMuErrorList);

const char* GetErrorText(int32 iError) {
	if (iError >= EQEMuError_MaxErrorID)
		return "ErrorID# out of range";
	else
		return EQEMuErrorText[iError];
}

void AddEQEMuError(eEQEMuError iError, bool iExitNow) {
	if (!iError)
		return;
	if (!EQEMuErrorList) {
		EQEMuErrorList = new LinkedList<char*>;
		MEQEMuErrorList = new Mutex;
	}
	LockMutex lock(MEQEMuErrorList);

	LinkedListIterator<char*> iterator(*EQEMuErrorList);
	iterator.Reset();
	while (iterator.MoreElements()) {
		if (iterator.GetData()[0] == 1) {
			if (*((eEQEMuError*) &(iterator.GetData()[1])) == iError)
				return;
		}
		iterator.Advance();
	}
	
	char* tmp = new char[6];
	tmp[0] = 1;
	tmp[5] = 0;
	*((int32*) &tmp[1]) = iError;
	EQEMuErrorList->Append(tmp);

	if (iExitNow)
		CatchSignal(2);
}

void AddEQEMuError(char* iError, bool iExitNow) {
	if (!iError)
		return;
	if (!EQEMuErrorList) {
		EQEMuErrorList = new LinkedList<char*>;
		MEQEMuErrorList = new Mutex;
	}
	LockMutex lock(MEQEMuErrorList);
	char* tmp = strcpy(new char[strlen(iError) + 1], iError);
	EQEMuErrorList->Append(tmp);

	if (iExitNow)
		CatchSignal(2);
}

int32 CheckEQEMuError() {
	if (!EQEMuErrorList)
		return 0;
	int32 ret = 0;
	char* tmp = 0;
	bool HeaderPrinted = false;
	LockMutex lock(MEQEMuErrorList);

	while ((tmp = EQEMuErrorList->Pop() )) {
		if (!HeaderPrinted) {
			fprintf(stdout, "===============================\nRuntime errors:\n\n");
			HeaderPrinted = true;
		}
		if (tmp[0] == 1) {
			fprintf(stdout, "%s\n", GetErrorText(*((int32*) &tmp[1])));
		}
		else {
			fprintf(stdout, "%s\n\n", tmp);
		}
		safe_delete(tmp);
		ret++;
	}
	return ret;
}

void CheckEQEMuErrorAndPause() {
	if (CheckEQEMuError()) {
		fprintf(stdout, "Hit any key to exit\n");
		getchar();
	}
}


