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
#ifndef OPCODE_MANAGER_H
#define OPCODE_MANAGER_H

#include "types.h"
#include "Mutex.h"
#include "emu_opcodes.h"

#include <map>
using namespace std;

class OpcodeManager {
public:
	OpcodeManager();
	virtual ~OpcodeManager() {}
	
	virtual bool Mutable() { return(false); }
	virtual bool LoadOpcodes(const char *filename) = 0;
	virtual bool LoadOpcodes(map<string, uint16>* eq, std::string* missingOpcodes = nullptr) = 0;
	virtual bool ReloadOpcodes(const char *filename) = 0;
	
	virtual uint16 EmuToEQ(const EmuOpcode emu_op) = 0;
	virtual EmuOpcode EQToEmu(const uint16 eq_op) = 0;
	
	static const char *EmuToName(const EmuOpcode emu_op);
	const char *EQToName(const uint16 emu_op);
	EmuOpcode NameSearch(const char *name);
	
	//This has to be public for stupid visual studio
	class OpcodeSetStrategy {
		public:
			virtual void Set(EmuOpcode emu_op, uint16 eq_op) = 0;
			virtual ~OpcodeSetStrategy(){}
	};

protected:
	bool loaded;                    //true if all opcodes loaded
	Mutex MOpcodes;	//this only protects the local machine
					//in a shared manager, this dosent protect others
	
	static bool LoadOpcodesFile(const char *filename, OpcodeSetStrategy *s);
	static bool LoadOpcodesMap(map<string, uint16>* eq, OpcodeSetStrategy *s, std::string* missingOpcodes = nullptr);
};

class MutableOpcodeManager : public OpcodeManager {
public:
	MutableOpcodeManager() : OpcodeManager() {}
	virtual bool Mutable() { return(true); }
	virtual void SetOpcode(EmuOpcode emu_op, uint16 eq_op) = 0;
};

#ifdef SHARED_OPCODES	//quick toggle since only world and zone should possibly use this
//keeps opcodes in shared memory
class SharedOpcodeManager : public OpcodeManager {
public:
	virtual ~SharedOpcodeManager() {}
	
	virtual bool LoadOpcodes(const char *filename);
	virtual bool LoadOpcodes(map<string, uint16>* eq, std::string* missingOpcodes = nullptr);
	virtual bool ReloadOpcodes(const char *filename);
	
	virtual uint16 EmuToEQ(const EmuOpcode emu_op);
	virtual EmuOpcode EQToEmu(const uint16 eq_op);
	
protected:
	class SharedMemStrategy : public OpcodeManager::OpcodeSetStrategy {
	public:
		void Set(EmuOpcode emu_op, uint16 eq_op);
	};
	static bool DLLLoadOpcodesCallback(const char *filename);
};
#endif //SHARED_OPCODES

//keeps opcodes in regular heap memory
class RegularOpcodeManager : public MutableOpcodeManager {
public:
	RegularOpcodeManager();
	virtual ~RegularOpcodeManager();
	
	virtual bool Editable() { return(true); }
	virtual bool LoadOpcodes(const char *filename);
	virtual bool LoadOpcodes(map<string, uint16>* eq, std::string* missingOpcodes = nullptr);
	virtual bool ReloadOpcodes(const char *filename);
	
	virtual uint16 EmuToEQ(const EmuOpcode emu_op);
	virtual EmuOpcode EQToEmu(const uint16 eq_op);
	
	//implement our editing interface
	virtual void SetOpcode(EmuOpcode emu_op, uint16 eq_op);
	
protected:
	class NormalMemStrategy : public OpcodeManager::OpcodeSetStrategy {
	public:
		RegularOpcodeManager *it;
		void Set(EmuOpcode emu_op, uint16 eq_op);
	};
	friend class NormalMemStrategy;
	
	uint16 *emu_to_eq;
	EmuOpcode *eq_to_emu;
	uint32 EQOpcodeCount;
	uint32 EmuOpcodeCount;
};

//always resolves everything to 0 or OP_Unknown
class NullOpcodeManager : public MutableOpcodeManager {
public:
	NullOpcodeManager();
	
	virtual bool LoadOpcodes(const char *filename);
	virtual bool LoadOpcodes(map<string, uint16>* eq, std::string* missingOpcodes = nullptr);
	virtual bool ReloadOpcodes(const char *filename);
	
	virtual uint16 EmuToEQ(const EmuOpcode emu_op);
	virtual EmuOpcode EQToEmu(const uint16 eq_op);
	
	//fake it, just used for testing anyways
	virtual void SetOpcode(EmuOpcode emu_op, uint16 eq_op) {}
};

//starts as NullOpcodeManager, but remembers any mappings set
//could prolly have been implemented with an extension to regular,
//by overriding its load methods to be empty.
class EmptyOpcodeManager : public MutableOpcodeManager {
public:
	EmptyOpcodeManager();
	
	virtual bool LoadOpcodes(const char *filename);
	virtual bool LoadOpcodes(map<string, uint16>* eq, std::string* missingOpcodes = nullptr);
	virtual bool ReloadOpcodes(const char *filename);
	
	virtual uint16 EmuToEQ(const EmuOpcode emu_op);
	virtual EmuOpcode EQToEmu(const uint16 eq_op);
	
	//fake it, just used for testing anyways
	virtual void SetOpcode(EmuOpcode emu_op, uint16 eq_op);
protected:
	map<EmuOpcode, uint16> emu_to_eq;
	map<uint16, EmuOpcode> eq_to_emu;
};

#endif


