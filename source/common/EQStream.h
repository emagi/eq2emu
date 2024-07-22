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
#ifndef _EQPROTOCOL_H
#define _EQPROTOCOL_H

#include <string>
#include <vector>
#include <deque>
#include <queue>

#include <map>
#include <set>
#ifndef WIN32
#include <netinet/in.h>
#endif
#include "EQPacket.h"
#include "Mutex.h"
#include "opcodemgr.h"
#include "misc.h"
#include "Condition.h"
#include "Crypto.h"
#include "zlib.h"
#include "timer.h"
#ifdef WRITE_PACKETS
#include <stdarg.h>
#endif

using namespace std;

typedef enum {
	ESTABLISHED,
	WAIT_CLOSE,
	CLOSING,
	DISCONNECTING,
	CLOSED
} EQStreamState;

#define FLAG_COMPRESSED	0x01
#define FLAG_ENCODED	0x04

#define RATEBASE	1048576 // 1 MB
#define DECAYBASE	78642	// RATEBASE/10

#ifndef RETRANSMIT_TIMEOUT_MULT
#define RETRANSMIT_TIMEOUT_MULT 3.0
#endif

#ifndef RETRANSMIT_TIMEOUT_MAX
#define RETRANSMIT_TIMEOUT_MAX 5000
#endif

#ifndef AVERAGE_DELTA_MAX
#define AVERAGE_DELTA_MAX 2500
#endif

#pragma pack(1)
struct SessionRequest {
	uint32 UnknownA;
	uint32 Session;
	uint32 MaxLength;
};

struct SessionResponse {
    uint32 Session;
	uint32 Key;
	uint8 UnknownA;
	uint8 Format;
	uint8 UnknownB;
	uint32 MaxLength;
	uint32 UnknownD;
};

//Deltas are in ms, representing round trip times
struct ClientSessionStats {
/*000*/	uint16 RequestID;
/*002*/	uint32 last_local_delta;
/*006*/	uint32 average_delta;
/*010*/	uint32 low_delta;
/*014*/	uint32 high_delta;
/*018*/	uint32 last_remote_delta;
/*022*/	uint64 packets_sent;
/*030*/	uint64 packets_recieved;
/*038*/
};

struct ServerSessionStats {
       uint16 RequestID;
       uint32 current_time;
       uint32 unknown1;
       uint32 received_packets;
       uint32 unknown2;
       uint32 sent_packets;
       uint32 unknown3;
       uint32 sent_packets2;
       uint32 unknown4;
       uint32 received_packets2;
};
	
#pragma pack()

class OpcodeManager;    
extern OpcodeManager *EQNetworkOpcodeManager;

class EQStreamFactory;

typedef enum {
	UnknownStream=0,
	LoginStream,
	WorldStream,
	ZoneStream,
	ChatOrMailStream,
	ChatStream,
	MailStream,
	EQ2Stream,
} EQStreamType;

class EQStream {
	protected:
		typedef enum {
			SeqPast,
			SeqInOrder,
			SeqFuture
		} SeqOrder;

		uint32 received_packets;
		uint32 sent_packets;
		uint32 remote_ip;
		uint16 remote_port;
		uint8 buffer[8192];
		unsigned char *oversize_buffer;
		uint32 oversize_offset,oversize_length;
		unsigned char *rogue_buffer;
		uint32 roguebuf_offset,roguebuf_size;
		uint8 app_opcode_size;
		EQStreamType StreamType;
		bool compressed,encoded;

		uint32 retransmittimer;
		uint32 retransmittimeout;
		//uint32 buffer_len;

		uint16 sessionAttempts;
		uint16 reconnectAttempt;
		bool streamactive;

		uint32 Session, Key;
		uint16 NextInSeq;
		uint16 NextOutSeq;
		uint16 SequencedBase;	//the sequence number of SequencedQueue[0]
		uint32  MaxLen;
		uint16 MaxSends;
		int8 timeout_delays;

		uint8 active_users;	//how many things are actively using this
		Mutex MInUse;

#ifdef WRITE_PACKETS
		FILE* write_packets = NULL;
		char GetChar(uchar in);
		void WriteToFile(char* pFormat, ...);
		void WritePackets(const char* opcodeName, uchar* data, int32 size, bool outgoing);
		void WritePackets(EQ2Packet* app, bool outgoing);
		Mutex MWritePackets;
#endif

		EQStreamState State;
		Mutex MState;

		uint32 LastPacket;
		Mutex MVarlock;

		EQApplicationPacket* CombinedAppPacket;
		Mutex MCombinedAppPacket;

		long LastSeqSent;
		Mutex MLastSeqSent;
		void SetLastSeqSent(uint32);

		// Ack sequence tracking.
		long MaxAckReceived,NextAckToSend,LastAckSent;
		long GetMaxAckReceived();
		long GetNextAckToSend();
		long GetLastAckSent();
		void SetMaxAckReceived(uint32 seq);
		void SetNextAckToSend(uint32);
		void SetLastAckSent(uint32);

		Mutex MAcks;

		// Packets waiting to be sent
		queue<EQProtocolPacket*> NonSequencedQueue;
		deque<EQProtocolPacket*> SequencedQueue;
		map<uint16, EQProtocolPacket *> OutOfOrderpackets;
		Mutex MOutboundQueue;

		// Packes waiting to be processed
		deque<EQApplicationPacket *> InboundQueue;
		Mutex MInboundQueue;

		static uint16 MaxWindowSize;

		sint32 BytesWritten;

		Mutex MRate;
		sint32 RateThreshold;
		sint32 DecayRate;
		uint32 AverageDelta;

		EQStreamFactory *Factory;

	public:
		Mutex MCombineQueueLock;
		bool CheckCombineQueue();
		deque<EQ2Packet*> combine_queue;
		Timer*	combine_timer;

		Crypto* crypto;
		int8 EQ2_Compress(EQ2Packet* app, int8 offset = 3);
		z_stream stream;
		uchar*	stream_buffer;
		int32	stream_buffer_size;
		bool	eq2_compressed;
		int8	compressed_offset;
		int16	client_version;
		int16	GetClientVersion(){ return client_version; }
		void	SetClientVersion(int16 version){ client_version = version; }
		void	ResetSessionAttempts() { reconnectAttempt = 0; }
		bool	HasSessionAttempts() { return reconnectAttempt>0; }
		EQStream() { init(); remote_ip = 0; remote_port = 0; State = CLOSED; StreamType = UnknownStream; compressed = true; 
		encoded = false; app_opcode_size = 2;}
		EQStream(sockaddr_in addr);
		virtual ~EQStream() { 
			MOutboundQueue.lock();
			SetState(CLOSED);
			MOutboundQueue.unlock();
			RemoveData(); 
			safe_delete(crypto);
			safe_delete(combine_timer);
			safe_delete(resend_que_timer);
			safe_delete_array(oversize_buffer);
			safe_delete_array(rogue_buffer);
			deque<EQ2Packet*>::iterator cmb;
			MCombineQueueLock.lock();
			for (cmb = combine_queue.begin(); cmb != combine_queue.end(); cmb++){
				safe_delete(*cmb);
			}
			MCombineQueueLock.unlock();
			deflateEnd(&stream);
			map<int16, EQProtocolPacket*>::iterator oop;
			for (oop = OutOfOrderpackets.begin(); oop != OutOfOrderpackets.end(); oop++){
				safe_delete(oop->second);
			}
#ifdef WRITE_PACKETS			
			if (write_packets)
				fclose(write_packets);
#endif
		}
		inline void SetFactory(EQStreamFactory *f) { Factory=f; }
		void init(bool resetSession = true);
		void SetMaxLen(uint32 length) { MaxLen=length; }
		int8 getTimeoutDelays(){ return timeout_delays; }
		void addTimeoutDelay(){ timeout_delays++; }
		void EQ2QueuePacket(EQ2Packet* app, bool attempted_combine = false);
		void PreparePacket(EQ2Packet* app, int8 offset = 0);
		void UnPreparePacket(EQ2Packet* app);
		void EncryptPacket(EQ2Packet* app, int8 compress_offset, int8 offset);
		void FlushCombinedPacket();
		void SendPacket(EQApplicationPacket *p);
		void QueuePacket(EQProtocolPacket *p);
		void SendPacket(EQProtocolPacket *p);
		vector<EQProtocolPacket *> convert(EQApplicationPacket *p);
		void NonSequencedPush(EQProtocolPacket *p);
		void SequencedPush(EQProtocolPacket *p);

		Mutex MResendQue;
		Mutex MCompressData;
		deque<EQProtocolPacket*>resend_que;
		void CheckResend(int eq_fd);

		void AckPackets(uint16 seq);
		void Write(int eq_fd);

		void SetActive(bool val) { streamactive = val; }

		void WritePacket(int fd,EQProtocolPacket *p);

		void EncryptPacket(uchar* data, int16 size);
		uint32 GetKey() { return Key; }
		void SetKey(uint32 k) { Key=k; }
		void SetSession(uint32 s) { Session=s; }
		void SetLastPacketTime(uint32 t) {LastPacket=t;}

		void Process(const unsigned char *data, const uint32 length);
		void ProcessPacket(EQProtocolPacket *p, EQProtocolPacket* lastp=NULL);
		
		bool ProcessEmbeddedPacket(uchar* pBuffer, uint16 length, int8 opcode = OP_Packet);
		bool HandleEmbeddedPacket(EQProtocolPacket *p, int16 offset = 2, int16 length = 0);

		EQProtocolPacket * ProcessEncryptedPacket(EQProtocolPacket *p);
		EQProtocolPacket * ProcessEncryptedData(uchar* data, int32 size, int16 opcode);

		virtual void DispatchPacket(EQApplicationPacket *p) { p->DumpRaw(); }

		void SendSessionResponse();
		void SendSessionRequest();
		void SendDisconnect(bool setstate = true);
		void SendAck(uint16 seq);
		void SendOutOfOrderAck(uint16 seq);

		bool CheckTimeout(uint32 now, uint32 timeout=30) { return  (LastPacket && (now-LastPacket) > timeout); }
		bool Stale(uint32 now, uint32 timeout=30) { return  (LastPacket && (now-LastPacket) > timeout); }

		void InboundQueuePush(EQApplicationPacket *p);
		EQApplicationPacket *PopPacket(); // InboundQueuePop
		void InboundQueueClear();

		void OutboundQueueClear();
		bool HasOutgoingData();
		void SendKeyRequest();
		int16 processRSAKey(EQProtocolPacket *p, uint16 subpacket_length = 0);
		void RemoveData() { InboundQueueClear(); OutboundQueueClear(); if (CombinedAppPacket) delete CombinedAppPacket; }

		//
		inline bool IsInUse() { bool flag; MInUse.lock(); flag=(active_users>0); MInUse.unlock(); return flag; }
		inline void PutInUse() { MInUse.lock(); active_users++; MInUse.unlock(); }
		inline void ReleaseFromUse() { MInUse.lock(); if(active_users > 0) active_users--; MInUse.unlock(); }

		static SeqOrder CompareSequence(uint16 expected_seq, uint16 seq);

		inline EQStreamState GetState() { return State; }
		inline void SetState(EQStreamState state) { MState.lock();  State = state; MState.unlock(); }

		inline uint32 GetRemoteIP() { return remote_ip; }
		inline uint32 GetrIP() { return remote_ip; }
		inline uint16 GetRemotePort() { return remote_port; }
		inline uint16 GetrPort() { return remote_port; }
		

		static EQProtocolPacket *Read(int eq_fd, sockaddr_in *from);

		void Close() { SendDisconnect(); }
		bool CheckActive() { return (GetState()==ESTABLISHED); }
		bool CheckClosed() { return GetState()==CLOSED; }
		void SetOpcodeSize(uint8 s) { app_opcode_size = s; }
		void SetStreamType(EQStreamType t);
		inline const EQStreamType GetStreamType() const { return StreamType; }

		void ProcessQueue();
		EQProtocolPacket* RemoveQueue(uint16 seq);

		void Decay();
		void AdjustRates(uint32 average_delta);
		Timer* resend_que_timer;
};

#endif
