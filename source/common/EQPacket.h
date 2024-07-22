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
#ifndef _EQPACKET_H
#define _EQPACKET_H

#include "types.h"
#include <stdio.h>
#include <string.h>

#ifdef WIN32
	#include <time.h>
	#include <WinSock2.h>
#else
	#include <sys/time.h>
	#include <netinet/in.h>
#endif

#include "emu_opcodes.h"
#include "op_codes.h"
#include "packet_dump.h"

class OpcodeManager;

class EQStream;

class EQPacket {
	friend class EQStream;
public:
	unsigned char *pBuffer;
	uint32 size;
	uint32 src_ip,dst_ip;
	uint16 src_port,dst_port;
	uint32 priority;
	timeval timestamp;
	int16  version;
	~EQPacket();
	void DumpRawHeader(uint16 seq=0xffff, FILE *to = stdout) const;
	void DumpRawHeaderNoTime(uint16 seq=0xffff, FILE *to = stdout) const;
	void DumpRaw(FILE *to = stdout) const;
	const char* GetOpcodeName();
	
	void setVersion(int16 new_version){ version = new_version; }
	void setSrcInfo(uint32 sip, uint16 sport) { src_ip=sip; src_port=sport; }
	void setDstInfo(uint32 dip, uint16 dport) { dst_ip=dip; dst_port=dport; }
	void setTimeInfo(uint32 ts_sec, uint32 ts_usec) { timestamp.tv_sec=ts_sec; timestamp.tv_usec=ts_usec; }
	void copyInfo(const EQPacket *p) { src_ip=p->src_ip; src_port=p->src_port;  dst_ip=p->dst_ip; dst_port=p->dst_port; timestamp.tv_sec=p->timestamp.tv_sec; timestamp.tv_usec=p->timestamp.tv_usec; }
	uint32 Size() const { return size+2; }

//no reason to have this method in zone or world

	uint16 GetRawOpcode() const { return(opcode); }


	inline bool operator<(const EQPacket &rhs) {
		return (timestamp.tv_sec < rhs.timestamp.tv_sec || (timestamp.tv_sec==rhs.timestamp.tv_sec && timestamp.tv_usec < rhs.timestamp.tv_usec));
	}
	void SetProtocolOpcode(int16 new_opcode){
		opcode = new_opcode;
	}
	
protected:
	uint16 opcode;

	EQPacket(const uint16 op, const unsigned char *buf, const uint32 len);
	EQPacket(const EQPacket &p) { version = 0; }
	EQPacket() { opcode=0; pBuffer=NULL; size=0; version = 0; setTimeInfo(0, 0); }

};

class EQApplicationPacket;

class EQProtocolPacket : public EQPacket {
public:
	EQProtocolPacket(uint16 op, const unsigned char *buf, uint32 len) : EQPacket(op,buf,len) { 
		eq2_compressed = false; 
		packet_prepared = false;
		packet_encrypted = false;
		sequence = 0;
		sent_time = 0;
		attempt_count = 0;
		acked = false;
	} 
	EQProtocolPacket(const unsigned char *buf, uint32 len, int in_opcode = -1);
	bool combine(const EQProtocolPacket *rhs);
	uint32 serialize (unsigned char *dest, int8 offset = 0) const;
	static bool ValidateCRC(const unsigned char *buffer, int length, uint32 Key);
	static uint32 Decompress(const unsigned char *buffer, const uint32 length, unsigned char *newbuf, uint32 newbufsize);
	static uint32 Compress(const unsigned char *buffer, const uint32 length, unsigned char *newbuf, uint32 newbufsize);
	static void ChatDecode(unsigned char *buffer, int size, int DecodeKey);
	static void ChatEncode(unsigned char *buffer, int size, int EncodeKey);
	static bool IsProtocolPacket(const unsigned char* in_buff, uint32_t len, bool bTrimCRC);

	EQProtocolPacket *Copy() { 
		EQProtocolPacket* new_packet = new EQProtocolPacket(opcode,pBuffer,size);
		new_packet->eq2_compressed = this->eq2_compressed;
		new_packet->packet_prepared = this->packet_prepared;
		new_packet->packet_encrypted = this->packet_encrypted;
		return new_packet; 
	}
	EQApplicationPacket *MakeApplicationPacket(uint8 opcode_size=0) const;
	bool eq2_compressed;
	bool packet_prepared;
	bool packet_encrypted;
	bool acked;
	int32 sent_time;
	int8  attempt_count;
	int32 sequence;

private:
	EQProtocolPacket(const EQProtocolPacket &p) { }
	//bool dont_combine;
};
class EQ2Packet : public EQProtocolPacket {
public:
	EQ2Packet(const EmuOpcode in_login_op, const unsigned char *buf, uint32 len) : EQProtocolPacket(OP_Packet,buf,len){ 
		login_op = in_login_op;
		eq2_compressed = false;
		packet_prepared = false;
		packet_encrypted = false;
	}
	bool AppCombine(EQ2Packet* rhs);
	EQ2Packet* Copy() { 
		EQ2Packet* new_packet = new EQ2Packet(login_op,pBuffer,size); 
		new_packet->eq2_compressed = this->eq2_compressed;
		new_packet->packet_prepared = this->packet_prepared;
		new_packet->packet_encrypted = this->packet_encrypted;
		return new_packet;
	}
	int8 PreparePacket(int16 MaxLen);
	const char* GetOpcodeName();
	EmuOpcode login_op;
};
class EQApplicationPacket : public EQPacket {
	friend class EQProtocolPacket;
	friend class EQStream;
public:
	EQApplicationPacket() : EQPacket(0,NULL,0) { emu_opcode = OP_Unknown; app_opcode_size=default_opcode_size; }
	EQApplicationPacket(const EmuOpcode op) : EQPacket(0,NULL,0) { SetOpcode(op); app_opcode_size=default_opcode_size; }
	EQApplicationPacket(const EmuOpcode op, const uint32 len) : EQPacket(0,NULL,len) { SetOpcode(op); app_opcode_size=default_opcode_size; }
	EQApplicationPacket(const EmuOpcode op, const unsigned char *buf, const uint32 len) : EQPacket(0,buf,len) { SetOpcode(op); app_opcode_size=default_opcode_size; }
	bool combine(const EQApplicationPacket *rhs);
	uint32 serialize (unsigned char *dest) const;
	uint32 Size() const { return size+app_opcode_size; }
	EQApplicationPacket *Copy() const {
		EQApplicationPacket *it = new EQApplicationPacket;
		try {
			it->pBuffer= new unsigned char[size];
			memcpy(it->pBuffer,pBuffer,size);
			it->size=size;
			it->opcode = opcode;
			it->emu_opcode = emu_opcode;
			it->version = version;
			return(it);
		}
		catch( bad_alloc &ba )
		{
			cout << ba.what() << endl;
			if( NULL != it )
				delete it;
		}
		return NULL;
	}
	
	void SetOpcodeSize(uint8 s) { app_opcode_size=s; }
	void SetOpcode(EmuOpcode op);
	const EmuOpcode GetOpcodeConst() const;
	inline const EmuOpcode GetOpcode() const { return(GetOpcodeConst()); }
	//caching version of get
	inline const EmuOpcode GetOpcode() { EmuOpcode r = GetOpcodeConst(); emu_opcode = r; return(r); }

	static uint8 default_opcode_size;

protected:
	//this is just a cache so we dont look it up several times on Get()
	EmuOpcode emu_opcode;

private:
	//this constructor should only be used by EQProtocolPacket, as it
	//assumes the first two bytes of buf are the opcode.
	EQApplicationPacket(const unsigned char *buf, uint32 len, uint8 opcode_size=0);
	EQApplicationPacket(const EQApplicationPacket &p) { emu_opcode = OP_Unknown; app_opcode_size=default_opcode_size; }

	uint8 app_opcode_size;
};

void DumpPacketHex(const EQApplicationPacket* app);
void DumpPacket(const EQProtocolPacket* app);
void DumpPacketAscii(const EQApplicationPacket* app);
void DumpPacket(const EQApplicationPacket* app, bool iShowInfo = false);
void DumpPacketBin(const EQApplicationPacket* app);

#endif
