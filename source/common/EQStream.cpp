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
#include "debug.h"
#include <string>
#include <iomanip>
#include <iostream>
#include <vector>
#include <time.h>
#include <sys/types.h>
#ifdef WIN32
	#include <time.h>
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <sys/time.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <fcntl.h>
	#include <arpa/inet.h>
#endif
#include "EQPacket.h"
#include "EQStream.h"
#include "EQStreamFactory.h"
#include "misc.h"
#include "Mutex.h"
#include "op_codes.h"
#include "CRC16.h"
#include "packet_dump.h"
#ifdef LOGIN
	#include "../LoginServer/login_structs.h"
#endif
#include "EQ2_Common_Structs.h"
#include "Log.h"


//#define DEBUG_EMBEDDED_PACKETS 1
uint16 EQStream::MaxWindowSize=2048;

void EQStream::init(bool resetSession) {
	if (resetSession)
	{
		streamactive = false;
		sessionAttempts = 0;
	}

	timeout_delays = 0;

	MInUse.lock();
	active_users = 0;
	MInUse.unlock();

	Session=0;
	Key=0;
	MaxLen=0;
	NextInSeq=0;
	NextOutSeq=0;
	CombinedAppPacket=NULL;

	MAcks.lock();
	MaxAckReceived = -1;
	NextAckToSend = -1;
	LastAckSent = -1;
	MAcks.unlock();

	LastSeqSent=-1;
	MaxSends=5;
	LastPacket=Timer::GetCurrentTime2();
	oversize_buffer=NULL;
	oversize_length=0;
	oversize_offset=0;
	Factory = NULL;

	rogue_buffer=NULL;
	roguebuf_offset=0;
	roguebuf_size=0;

	MRate.lock();
	RateThreshold=RATEBASE/250;
	DecayRate=DECAYBASE/250;
	MRate.unlock();

	BytesWritten=0;
	SequencedBase = 0;
	AverageDelta = 500;

	crypto->setRC4Key(0);

	retransmittimer = Timer::GetCurrentTime2();
	retransmittimeout = 500 * RETRANSMIT_TIMEOUT_MULT;

	reconnectAttempt = 0;
	if (uint16(SequencedBase + SequencedQueue.size()) != NextOutSeq) {
		LogWrite(PACKET__DEBUG, 9, "Packet",  "init Invalid Sequenced queue: BS %u + SQ %u != NOS %u", SequencedBase, SequencedQueue.size(), NextOutSeq);
	}
}

EQStream::EQStream(sockaddr_in addr){ 
	crypto = new Crypto();
	resend_que_timer = new Timer(1000);
	combine_timer = new Timer(250); //250 milliseconds
	combine_timer->Start();
	resend_que_timer->Start();
	init(); 
	remote_ip=addr.sin_addr.s_addr; 
	remote_port=addr.sin_port; 
	State=CLOSED; 
	StreamType=UnknownStream; 
	compressed=true; 
	encoded=false; 
	app_opcode_size=2; 
	#ifdef WIN32
        ZeroMemory(&stream, sizeof(z_stream));
    #else
		bzero(&stream, sizeof(z_stream));
    #endif
	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;
	stream.opaque = (voidpf)0;
	deflateInit2(&stream, 9, Z_DEFLATED, 13, 9, Z_DEFAULT_STRATEGY);
	//deflateInit(&stream, 5);
	compressed_offset = 0;
	client_version = 0;
	received_packets = 0;
	sent_packets = 0;

#ifdef WRITE_PACKETS
	write_packets = 0;
	char write_packets_filename[64];
	snprintf(write_packets_filename, sizeof(write_packets_filename), "PacketLog%i.log", Timer::GetCurrentTime2());
	write_packets = fopen(write_packets_filename, "w+");
#endif
}

EQProtocolPacket* EQStream::ProcessEncryptedData(uchar* data, int32 size, int16 opcode){
	//cout << "B4:\n";
	//DumpPacket(data, size);
	/*if(size >= 2 && data[0] == 0 && data[1] == 0){
		cout << "Attempting to fix packet!\n";
		//Have to fix bad packet from client or it will screw up encryption :P
		size--;
		data++;
	}*/
	crypto->RC4Decrypt(data,size);
	int8 offset = 0;
	if(data[0] == 0xFF && size > 2){
			offset = 3;
			memcpy(&opcode, data+sizeof(int8), sizeof(int16));
	}
	else{
		offset = 1;
		memcpy(&opcode, data, sizeof(int8));
	}
	//cout << "After:\n";
	//DumpPacket(data, size);
	return new EQProtocolPacket(opcode, data+offset, size - offset);
}

EQProtocolPacket* EQStream::ProcessEncryptedPacket(EQProtocolPacket *p){
	EQProtocolPacket* ret = NULL;
	if(p->opcode == OP_Packet && p->size > 2)
		ret = ProcessEncryptedData(p->pBuffer+2, p->size-2, p->opcode);
	else
		ret = ProcessEncryptedData(p->pBuffer, p->size, p->opcode);
	return ret;
}

bool EQStream::ProcessEmbeddedPacket(uchar* pBuffer, int16 length,int8 opcode) {
	if(!pBuffer || !crypto->isEncrypted())
		return false;

	MCombineQueueLock.lock();
	EQProtocolPacket* newpacket = ProcessEncryptedData(pBuffer, length, opcode);
	MCombineQueueLock.unlock();
	
	if (newpacket) {
#ifdef DEBUG_EMBEDDED_PACKETS
		printf("Opcode: %u\n", newpacket->opcode);
		DumpPacket(newpacket->pBuffer, newpacket->size);
#endif
	
		EQApplicationPacket* ap = newpacket->MakeApplicationPacket(2);
		if (ap->version == 0)
			ap->version = client_version;
		InboundQueuePush(ap);
#ifdef WRITE_PACKETS
		WritePackets(ap->GetOpcodeName(), pBuffer, length, false);
#endif
		safe_delete(newpacket);
		return true;
	}
	
	return false;
}

bool EQStream::HandleEmbeddedPacket(EQProtocolPacket *p, int16 offset, int16 length){
	if(!p)
		return false;
	
#ifdef DEBUG_EMBEDDED_PACKETS
	// printf works better with DumpPacket
	printf( "Start Packet with offset %u, length %u, p->size %u\n", offset, length, p->size);
#endif
	
	if(p->size >= ((uint32)(offset+2))){	
		if(p->pBuffer[offset] == 0 && p->pBuffer[offset+1] == 0x19){
			uint32 data_length = 0;
			if(length == 0) {
				// Ensure there are at least 2 bytes after offset.
				if(p->size < offset + 2) {
					return false; // Not enough data.
				}
				data_length = p->size - offset - 2;
			} else {
				// Ensure provided length is at least 2.
				if(length < 2) {
					return false; // Provided length too short.
				}
				data_length = length - 2;
			}
#ifdef DEBUG_EMBEDDED_PACKETS
			printf( "Creating OP_AppCombined Packet with offset %u, length %u, p->size %u\n", offset, length, p->size);
			DumpPacket(p->pBuffer, p->size);
#endif
			// Verify that offset + 2 + data_length does not exceed p->size.
			if(offset + 2 + data_length > p->size) {
				return false; // Out-of-bounds.
			}
			EQProtocolPacket *subp = new EQProtocolPacket(OP_AppCombined, p->pBuffer + offset + 2, data_length);
			subp->copyInfo(p);
			ProcessPacket(subp, p);
			safe_delete(subp);
			return true;
		}
		else if (p->pBuffer[offset] == 0 && p->pBuffer[offset + 1] == 0) {
			if (length == 0)
				length = p->size - 1 - offset;
			else
				length--;

#ifdef DEBUG_EMBEDDED_PACKETS
			printf( "Creating Opcode 0 Packet!");
			DumpPacket(p->pBuffer + 1 + offset, length);
#endif
			uchar* buffer = (p->pBuffer + 1 + offset);
			bool valid = ProcessEmbeddedPacket(buffer, length);
			
			if(valid)
				return true;
		}
		else if(offset+4 < p->size && ntohl(*(uint32 *)(p->pBuffer+offset)) != 0xffffffff) {
#ifdef DEBUG_EMBEDDED_PACKETS
			uint16 seq = NextInSeq-1;
			sint8 check = 0;
			
			if(offset == 2) {
				seq=ntohs(*(uint16 *)(p->pBuffer));
				check=CompareSequence(NextInSeq,seq);
			}
			printf( "Unhandled Packet with offset %u, length %u, p->size %u, check: %i, nextinseq: %u, seq: %u\n", offset, length, p->size, check, NextInSeq, seq);
			DumpPacket(p->pBuffer, p->size);
#endif

			if(length == 0)
				length = p->size - offset;
				
				
			uchar* buffer = (p->pBuffer + offset);
			
			bool valid = ProcessEmbeddedPacket(buffer, length);
			
			if(valid)
				return true;
		}
		else if(p->pBuffer[offset] != 0xff && p->pBuffer[offset+1] == 0xff && p->size >= offset + 3) {
			// Read the first byte into a wider type to avoid underflow.
			uint16 total_length = p->pBuffer[offset]; // promote to uint16
			// Check that there is enough data: we expect offset+2+total_length == p->size.
			if(total_length + offset + 2 == p->size && total_length >= 2) {
				uint32 data_length = total_length - 2;
				// No additional bounds check needed because equality condition ensures it.
				EQProtocolPacket *subp = new EQProtocolPacket(p->pBuffer + offset + 2, data_length, OP_Packet);
				subp->copyInfo(p);
				ProcessPacket(subp, p);
				delete subp;
				return true;
			}
		}
	}
	return false;
}

void EQStream::ProcessPacket(EQProtocolPacket *p, EQProtocolPacket* lastp)
{
	uint32 processed=0,subpacket_length=0;

	if (p) {

		if (p->opcode!=OP_SessionRequest && p->opcode!=OP_SessionResponse && !Session) {
#ifdef EQN_DEBUG
			LogWrite(PACKET__ERROR, 0, "Packet", "*** Session not initialized, packet ignored ");
			//p->DumpRaw();
#endif
			return;
		}

 		//cout << "Received " << (int)p->opcode << ":\n";
		//DumpPacket(p->pBuffer, p->size);
		switch (p->opcode) {
			case OP_Combined: {
				processed=0;
				int8 offset = 0;
				int count = 0;
#ifdef LE_DEBUG
				printf( "OP_Combined:\n");
				DumpPacket(p);
#endif
				while(processed<p->size) {
					if ((subpacket_length=(unsigned char)*(p->pBuffer+processed))==0xff) {
						subpacket_length = ntohs(*(uint16*)(p->pBuffer + processed + 1));
						//printf("OP_Combined subpacket_length %u\n",subpacket_length);
						offset = 3;
					}
					else {
						offset = 1;
					}
					
					//printf("OP_Combined processed %u p->size %u subpacket length %u count %i\n",processed, p->size, subpacket_length, count);
					count++;
#ifdef LE_DEBUG
					printf( "OP_Combined Packet %i (%u) (%u):\n", count, subpacket_length, processed);
#endif
					bool isSubPacket = EQProtocolPacket::IsProtocolPacket(p->pBuffer + processed + offset, subpacket_length, false);
					if (isSubPacket) {
						EQProtocolPacket* subp = new EQProtocolPacket(p->pBuffer + processed + offset, subpacket_length);
						subp->copyInfo(p);
#ifdef LE_DEBUG
						printf( "Opcode %i:\n", subp->opcode);
						DumpPacket(subp);
#endif
						ProcessPacket(subp, p);
#ifdef LE_DEBUG
						DumpPacket(subp);
#endif
						delete subp;
					}
					else {
						offset = 1; // 0xFF in this case means it is actually 255 bytes of encrypted data after a 00 09 packet
						//Garbage packet?
						if(ntohs(*reinterpret_cast<uint16_t*>(p->pBuffer + processed + offset)) <= 0x1e) {
							subpacket_length=(unsigned char)*(p->pBuffer+processed);
							LogWrite(PACKET__ERROR, 0, "Packet", "!!!!!!!!!Garbage Packet Unknown Process as OP_Packet!!!!!!!!!!!!!\n");
							DumpPacket(p->pBuffer + processed + offset, subpacket_length);
							uchar* newbuf = p->pBuffer;
							newbuf += processed + offset;
							EQProtocolPacket *subp=new EQProtocolPacket(newbuf,subpacket_length);
							subp->copyInfo(p);
							ProcessPacket(subp, p);
							delete subp;
						}
						else {
							crypto->RC4Decrypt(p->pBuffer + processed + offset, subpacket_length);
							LogWrite(PACKET__ERROR, 0, "Packet", "!!!!!!!!!Garbage Packet!!!!!!!!!!!!! processed: %u, offset: %u, count: %i, subpacket_length: %u, offset_pos_1: %u, oversized_buffer_present: %u, offset size: %u, offset length: %u\n", 
							processed, offset, count, subpacket_length, p->pBuffer[processed + offset], oversize_buffer ? 1 : 0, oversize_offset, oversize_length);
							if(p->pBuffer[processed + offset] == 0xff)
							{
								uchar* newbuf = p->pBuffer;
								newbuf += processed + offset + 1;

								DumpPacket(p->pBuffer + processed + offset, subpacket_length);
								EQProtocolPacket *subp=new EQProtocolPacket(newbuf, subpacket_length, OP_Packet);
								subp->copyInfo(p);
								ProcessPacket(subp, p);
								delete subp;
							}
							else
								break; // bad packet
						}
					}
					processed+=subpacket_length+offset;
				}
				break;
			}
			case OP_AppCombined: {
				processed=0;
				EQProtocolPacket* newpacket = 0;
				int8 offset = 0;
#ifdef DEBUG_EMBEDDED_PACKETS
				printf( "OP_AppCombined: \n");
				DumpPacket(p);
#endif
				int count = 0;
				while(processed<p->size) {
					count++;
					if ((subpacket_length=(unsigned char)*(p->pBuffer+processed))==0xff) {
						subpacket_length=ntohs(*(uint16 *)(p->pBuffer+processed+1));
						offset = 3;
					} else
						offset = 1;
					
					if(crypto->getRC4Key()==0 && p && subpacket_length > 8+offset){
					#ifdef DEBUG_EMBEDDED_PACKETS
						DumpPacket(p->pBuffer, p->size);
					#endif
						p->pBuffer += offset;
						processRSAKey(p, subpacket_length);
						p->pBuffer -= offset;
					}
					else if(crypto->isEncrypted()){
#ifdef DEBUG_EMBEDDED_PACKETS
						printf( "OP_AppCombined Packet %i (%u) (%u): \n", count, subpacket_length, processed);
						DumpPacket(p->pBuffer+processed+offset, subpacket_length);
#endif
						if(!HandleEmbeddedPacket(p, processed + offset, subpacket_length)){
							uchar* buffer = (p->pBuffer + processed + offset);
							if(!ProcessEmbeddedPacket(buffer, subpacket_length, OP_AppCombined)) {
								LogWrite(PACKET__ERROR, 0, "Packet", "*** This is bad, ProcessEmbeddedPacket failed, report to Image!");
							}
						}
					}
					processed+=subpacket_length+offset;
				}
			}
			break;
			case OP_Packet: {
				if (!p->pBuffer || (p->Size() < 4))
				{
					break;
				}

				uint16 seq=ntohs(*(uint16 *)(p->pBuffer));
				sint8 check=CompareSequence(NextInSeq,seq);
				if (check == SeqFuture) {
#ifdef EQN_DEBUG
					LogWrite(PACKET__DEBUG, 1, "Packet", "*** Future packet: Expecting Seq=%i, but got Seq=%i", NextInSeq, seq);
					LogWrite(PACKET__DEBUG, 1, "Packet", "[Start]");
					p->DumpRawHeader(seq);
					LogWrite(PACKET__DEBUG, 1, "Packet", "[End]");
#endif				
					OutOfOrderpackets[seq] = p->Copy();

					// Image (2020): Removed as this is bad contributes to infinite loop
					//SendOutOfOrderAck(seq);
				} else if (check == SeqPast) {
#ifdef EQN_DEBUG
					LogWrite(PACKET__DEBUG, 1, "Packet", "*** Duplicate packet: Expecting Seq=%i, but got Seq=%i", NextInSeq, seq);
					LogWrite(PACKET__DEBUG, 1, "Packet", "[Start]");
					p->DumpRawHeader(seq);
					LogWrite(PACKET__DEBUG, 1, "Packet", "[End]");
#endif
					// Image (2020): Removed as this is bad contributes to infinite loop
					//OutOfOrderpackets[seq] = p->Copy();
					SendOutOfOrderAck(seq);
				} else {
					EQProtocolPacket* qp = RemoveQueue(seq);
					if (qp) {
						LogWrite(PACKET__DEBUG, 1, "Packet", "OP_Fragment: Removing older queued packet with sequence %i", seq);
						delete qp;
					}
					
					SetNextAckToSend(seq);
					NextInSeq++;
					
					if(HandleEmbeddedPacket(p))
						break;
					if(crypto->getRC4Key()==0 && p && p->size >= 69){
					#ifdef DEBUG_EMBEDDED_PACKETS
						DumpPacket(p->pBuffer, p->size);
					#endif
						processRSAKey(p);
					}
					else if(crypto->isEncrypted() && p){
						MCombineQueueLock.lock();
						EQProtocolPacket* newpacket = ProcessEncryptedPacket(p);
						MCombineQueueLock.unlock();
						if(newpacket){
							EQApplicationPacket* ap = newpacket->MakeApplicationPacket(2);
							if (ap->version == 0)
								ap->version = client_version;
#ifdef WRITE_PACKETS
							WritePackets(ap->GetOpcodeName(), p->pBuffer, p->size, false);
#endif
							InboundQueuePush(ap);
							safe_delete(newpacket);
						}
					}
				}
			}
			break;
			case OP_Fragment: {
				if (!p->pBuffer || (p->Size() < 4))
				{
					break;
				}

				uint16 seq=ntohs(*(uint16 *)(p->pBuffer));
				sint8 check=CompareSequence(NextInSeq,seq);
				if (check == SeqFuture) {
#ifdef EQN_DEBUG
					LogWrite(PACKET__DEBUG, 1, "Packet", "*** Future packet2: Expecting Seq=%i, but got Seq=%i", NextInSeq, seq);
					LogWrite(PACKET__DEBUG, 1, "Packet", "[Start]");
					//p->DumpRawHeader(seq);
					LogWrite(PACKET__DEBUG, 1, "Packet", "[End]");
#endif
					OutOfOrderpackets[seq] = p->Copy();
					//SendOutOfOrderAck(seq);
				} else if (check == SeqPast) {
#ifdef EQN_DEBUG
					LogWrite(PACKET__DEBUG, 1, "Packet", "*** Duplicate packet2: Expecting Seq=%i, but got Seq=%i", NextInSeq, seq);
					LogWrite(PACKET__DEBUG, 1, "Packet", "[Start]");
					//p->DumpRawHeader(seq);
					LogWrite(PACKET__DEBUG, 1, "Packet", "[End]");
#endif
					//OutOfOrderpackets[seq] = p->Copy();
					SendOutOfOrderAck(seq);
				} else {
					// In case we did queue one before as well.
					EQProtocolPacket* qp = RemoveQueue(seq);
					if (qp) {
						LogWrite(PACKET__DEBUG, 1, "Packet", "OP_Fragment: Removing older queued packet with sequence %i", seq);
						delete qp;
					}

					SetNextAckToSend(seq);
					NextInSeq++;
					if (oversize_buffer) {
						memcpy(oversize_buffer+oversize_offset,p->pBuffer+2,p->size-2);
						oversize_offset+=p->size-2;
						//cout << "Oversized is " << oversize_offset << "/" << oversize_length << " (" << (p->size-2) << ") Seq=" << seq << endl;
						if (oversize_offset==oversize_length) {
							if (*(p->pBuffer+2)==0x00 && *(p->pBuffer+3)==0x19) {
								EQProtocolPacket *subp=new EQProtocolPacket(oversize_buffer,oversize_offset);
								subp->copyInfo(p);
								ProcessPacket(subp, p);
								delete subp;
							} else {
								
								if(crypto->isEncrypted() && p && p->size > 2){
									MCombineQueueLock.lock();
									EQProtocolPacket* p2 = ProcessEncryptedData(oversize_buffer, oversize_offset, p->opcode);
									MCombineQueueLock.unlock();
									EQApplicationPacket* ap = p2->MakeApplicationPacket(2);
									ap->copyInfo(p);
									if (ap->version == 0)
										ap->version = client_version;
#ifdef WRITE_PACKETS
									WritePackets(ap->GetOpcodeName(), oversize_buffer, oversize_offset, false);
#endif
									ap->copyInfo(p);
									InboundQueuePush(ap);
									safe_delete(p2);
								}
							}
							delete[] oversize_buffer;
							oversize_buffer=NULL;
							oversize_offset=0;
						}
					} else if (!oversize_buffer) {
						oversize_length=ntohl(*(uint32 *)(p->pBuffer+2));
						oversize_buffer=new unsigned char[oversize_length];
						memcpy(oversize_buffer,p->pBuffer+6,p->size-6);
						oversize_offset=p->size-6;
						//cout << "Oversized is " << oversize_offset << "/" << oversize_length << " (" << (p->size-6) << ") Seq=" << seq << endl;
					}
				}
			}
			break;
			case OP_KeepAlive: {
#ifndef COLLECTOR
				NonSequencedPush(new EQProtocolPacket(p->opcode,p->pBuffer,p->size));
#endif
			}
			break;
			case OP_Ack: {
				if (!p->pBuffer || (p->Size() < 4))
				{
					LogWrite(PACKET__DEBUG, 9, "Packet", "Received OP_Ack that was of malformed size");
					break;
				}
				uint16 seq = ntohs(*(uint16*)(p->pBuffer));
				AckPackets(seq);
				retransmittimer = Timer::GetCurrentTime2();
			}
			break;
			case OP_SessionRequest: {
				if (p->Size() < sizeof(SessionRequest))
				{
					break;
				}

				if (GetState() == ESTABLISHED) {
					//_log(NET__ERROR, _L "Received OP_SessionRequest in ESTABLISHED state (%d) streamactive (%i) attempt (%i)" __L, GetState(), streamactive, sessionAttempts);

					// client seems to try a max of 4 times (initial +3 retries) then gives up, giving it a few more attempts just in case
					// streamactive means we identified the opcode, we cannot re-establish this connection
					if (streamactive || (sessionAttempts > 30))
					{
						SendDisconnect(false);
						SetState(CLOSED);
						break;
					}
				}

				sessionAttempts++;
				if(GetState() == WAIT_CLOSE) {
					printf("WAIT_CLOSE Reconnect with streamactive %u, sessionAttempts %u\n", streamactive, sessionAttempts);
					reconnectAttempt++;
				}
				init(GetState() != ESTABLISHED);
				OutboundQueueClear();
				SessionRequest *Request=(SessionRequest *)p->pBuffer;
				Session=ntohl(Request->Session);
				SetMaxLen(ntohl(Request->MaxLength));
#ifndef COLLECTOR
				NextInSeq=0;
				Key=0x33624702;
				SendSessionResponse();
#endif
				SetState(ESTABLISHED);
			}
			break;
			case OP_SessionResponse: {
				if (p->Size() < sizeof(SessionResponse))
				{
					break;
				}
				init();
				OutboundQueueClear();
				SetActive(true);
				SessionResponse *Response=(SessionResponse *)p->pBuffer;
				SetMaxLen(ntohl(Response->MaxLength));
				Key=ntohl(Response->Key);
				NextInSeq=0;
				SetState(ESTABLISHED);
				if (!Session)
					Session=ntohl(Response->Session);
				compressed=(Response->Format&FLAG_COMPRESSED);
				encoded=(Response->Format&FLAG_ENCODED);

				// Kinda kludgy, but trie for now
				if (compressed) {
					if (remote_port==9000 || (remote_port==0 && p->src_port==9000))
						SetStreamType(WorldStream);
					else
						SetStreamType(ZoneStream);
				} else if (encoded)
					SetStreamType(ChatOrMailStream);
				else
					SetStreamType(LoginStream);
			}
			break;
			case OP_SessionDisconnect: {
				//NextInSeq=0;
				SendDisconnect();
				//SetState(CLOSED);
			}
			break;
			case OP_OutOfOrderAck: {
				if (!p->pBuffer || (p->Size() < 4))
				{
					LogWrite(PACKET__DEBUG, 9, "Packet",  "Received OP_OutOfOrderAck that was of malformed size");
					break;
				}
				uint16 seq = ntohs(*(uint16*)(p->pBuffer));
				MOutboundQueue.lock();

				if (uint16(SequencedBase + SequencedQueue.size()) != NextOutSeq) {
					LogWrite(PACKET__DEBUG, 9, "Packet",  "Pre-OOA Invalid Sequenced queue: BS %u + SQ %u != NOS %u", SequencedBase, SequencedQueue.size(), NextOutSeq);
				}

				//if the packet they got out of order is between our last acked packet and the last sent packet, then its valid.
				if (CompareSequence(SequencedBase, seq) != SeqPast && CompareSequence(NextOutSeq, seq) == SeqPast) {
					uint16 sqsize = SequencedQueue.size();
					uint16 index = seq - SequencedBase;
					LogWrite(PACKET__DEBUG, 9, "Packet",  "OP_OutOfOrderAck marking packet acked in queue (queue index = %u, queue size = %u)", index, sqsize);
					if (index < sqsize) {
						SequencedQueue[index]->acked = true;
						// flag packets for a resend
						uint16 count = 0;
						uint32 timeout = AverageDelta * 2 + 100;
						for (auto sitr = SequencedQueue.begin(); sitr != SequencedQueue.end() && count < index; ++sitr, ++count) {
							if (!(*sitr)->acked && (*sitr)->sent_time > 0 && (((*sitr)->sent_time + timeout) < Timer::GetCurrentTime2())) {
								(*sitr)->sent_time = 0;
								LogWrite(PACKET__DEBUG, 9, "Packet",  "OP_OutOfOrderAck Flagging packet %u for retransmission", SequencedBase + count);
							}
						}
					}

					if (RETRANSMIT_TIMEOUT_MULT) {
						retransmittimer = Timer::GetCurrentTime2();
					}
				}
				else {
					LogWrite(PACKET__DEBUG, 9, "Packet",  "Received OP_OutOfOrderAck for out-of-window %u. Window (%u->%u)", seq, SequencedBase, NextOutSeq);
				}

				if (uint16(SequencedBase + SequencedQueue.size()) != NextOutSeq) {
					LogWrite(PACKET__DEBUG, 9, "Packet",  "Post-OOA Invalid Sequenced queue: BS %u + SQ %u != NOS %u", SequencedBase, SequencedQueue.size(), NextOutSeq);
				}

				MOutboundQueue.unlock();
			}
			break;
			case OP_ServerKeyRequest:{
				if (p->Size() < sizeof(ClientSessionStats))
				{
					//_log(NET__ERROR, _L "Received OP_SessionStatRequest that was of malformed size" __L);
					break;
				}
				
				ClientSessionStats* Stats = (ClientSessionStats*)p->pBuffer;
				int16 request_id = Stats->RequestID;
				AdjustRates(ntohl(Stats->average_delta));
				ServerSessionStats* stats=(ServerSessionStats*)p->pBuffer;
                memset(stats, 0, sizeof(ServerSessionStats));
				stats->RequestID = request_id;
				stats->current_time = ntohl(Timer::GetCurrentTime2());
				stats->sent_packets = ntohl(sent_packets);
				stats->sent_packets2 = ntohl(sent_packets);
				stats->received_packets = ntohl(received_packets);
				stats->received_packets2 = ntohl(received_packets);
				NonSequencedPush(new EQProtocolPacket(OP_SessionStatResponse,p->pBuffer,p->size));
				if(!crypto->isEncrypted())
					SendKeyRequest();
				else
					SendSessionResponse();
			}
			break;
			case OP_SessionStatResponse: {
				LogWrite(PACKET__INFO, 0, "Packet", "OP_SessionStatResponse");
			}
			break;
			case OP_OutOfSession: {
				LogWrite(PACKET__INFO, 0, "Packet", "OP_OutOfSession");
				SendDisconnect();
				SetState(CLOSED);
			}
			break;
			default:
				//EQApplicationPacket *ap = p->MakeApplicationPacket(app_opcode_size);
				//InboundQueuePush(ap);

				cout << "Orig Packet: " << p->opcode << endl;
				DumpPacket(p->pBuffer, p->size);
				if(p && p->size >= 69){
					processRSAKey(p);
				}
				MCombineQueueLock.lock();
				EQProtocolPacket* p2 = ProcessEncryptedData(p->pBuffer, p->size, OP_Packet);
				MCombineQueueLock.unlock();
				cout << "Decrypted Packet: " << p2->opcode << endl;
				DumpPacket(p2->pBuffer, p2->size);
				
				safe_delete(p2);
			/*	if(p2)
				{
					EQApplicationPacket* ap = p2->MakeApplicationPacket(2);
					if (ap->version == 0)
						ap->version = client_version;
					InboundQueuePush(ap);
					safe_delete(p2);
				}*/
				
				//EQProtocolPacket* puse = p2;
			/*	if (!rogue_buffer) {
						roguebuf_size=puse->size;
						rogue_buffer=new unsigned char[roguebuf_size];
						memcpy(rogue_buffer,puse->pBuffer,puse->size);
						roguebuf_offset=puse->size;
						cout << "RogueBuf is " << roguebuf_offset << "/" << roguebuf_size << " (" << (p->size-6) << ") NextInSeq=" << NextInSeq << endl;
					}
					else {
						int32 new_size = roguebuf_size + puse->size;
						uchar* tmp_buffer = new unsigned char[new_size];
						uchar* ptr = tmp_buffer;
						
						memcpy(ptr,rogue_buffer,roguebuf_size);
						ptr += roguebuf_size;
						memcpy(ptr,puse->pBuffer,puse->size);
						roguebuf_offset=puse->size;

						safe_delete_array(rogue_buffer);

						rogue_buffer = tmp_buffer;
						roguebuf_size = new_size;
						roguebuf_offset = new_size;
						cout << "RogueBuf is " << roguebuf_offset << "/" << roguebuf_size << " (" << (p->size-6) << ") NextInSeq=" << NextInSeq << endl;
				}*/
#ifdef WRITE_PACKETS
				WritePackets(ap->GetOpcodeName(), p->pBuffer, p->size, false);
#endif
				//InboundQueuePush(ap);
				LogWrite(PACKET__INFO, 0, "Packet", "Received unknown packet type, not adding to inbound queue");
				//safe_delete(p2);
				//SendDisconnect();
				break;
		}
	}
}

int8 EQStream::EQ2_Compress(EQ2Packet* app, int8 offset){

#ifdef LE_DEBUG
	printf( "Before Compress in %s, line %i:\n", __FUNCTION__, __LINE__);
	DumpPacket(app);
#endif


	uchar* pDataPtr = app->pBuffer + offset;
	int xpandSize = app->size * 2;
	uchar* deflate_buff = new uchar[xpandSize];
	MCompressData.lock();
	stream.next_in = pDataPtr;
	stream.avail_in = app->size - offset;
	stream.next_out = deflate_buff;
	stream.avail_out = xpandSize;

	int ret = deflate(&stream, Z_SYNC_FLUSH);

	if (ret != Z_OK)
	{
		printf("ZLIB COMPRESSION RETFAIL: %i, %i (Ret: %i)\n", app->size, stream.avail_out, ret);
		MCompressData.unlock();
		safe_delete_array(deflate_buff);
		return 0;
	}

	int32 newsize = xpandSize - stream.avail_out;
	safe_delete_array(app->pBuffer);
	app->size = newsize + offset;
	app->pBuffer = new uchar[app->size];
	app->pBuffer[(offset - 1)] = 1;
	memcpy(app->pBuffer + offset, deflate_buff, newsize);
	MCompressData.unlock();
	safe_delete_array(deflate_buff);

#ifdef LE_DEBUG
	printf( "After Compress in %s, line %i:\n", __FUNCTION__, __LINE__);
	DumpPacket(app);
#endif

	return offset - 1;
}

int16 EQStream::processRSAKey(EQProtocolPacket *p, uint16 subpacket_length){
	/*int16 limit = 0;
	int8 offset = 13;
	int8 offset2 = 0;
	if(p->pBuffer[2] == 0)
		limit = p->pBuffer[9];
	else{
		limit = p->pBuffer[5];
		offset2 = 5;
		offset-=1;
	}
	crypto->setRC4Key(Crypto::RSADecrypt(p->pBuffer + offset + (limit-8), 8));
	return (limit + offset +1) - offset2;*/
	if(subpacket_length)
		crypto->setRC4Key(Crypto::RSADecrypt(p->pBuffer + subpacket_length - 8, 8));
	else
		crypto->setRC4Key(Crypto::RSADecrypt(p->pBuffer + p->size - 8, 8));
	
	return 0;
}

void EQStream::SendKeyRequest(){
	int32 crypto_key_size = 60;
	int16 size = sizeof(KeyGen_Struct) + sizeof(KeyGen_End_Struct) + crypto_key_size;
	EQ2Packet *outapp=new EQ2Packet(OP_WSLoginRequestMsg,NULL,size);
	memcpy(&outapp->pBuffer[0], &crypto_key_size, sizeof(int32));
	memset(&outapp->pBuffer[4], 0xFF, crypto_key_size);
	memset(&outapp->pBuffer[size-5], 1, 1);
	memset(&outapp->pBuffer[size-1], 1, 1);
	EQ2QueuePacket(outapp, true);
}

void EQStream::EncryptPacket(EQ2Packet* app, int8 compress_offset, int8 offset){
	if(app->size>2 && crypto->isEncrypted()){
		app->packet_encrypted = true;
		uchar* crypt_buff = app->pBuffer;
		if(app->eq2_compressed)
			crypto->RC4Encrypt(crypt_buff + compress_offset, app->size - compress_offset);
		else
			crypto->RC4Encrypt(crypt_buff + 2 + offset, app->size - 2 - offset);
	}
}

void EQStream::EQ2QueuePacket(EQ2Packet* app, bool attempted_combine){
	if(CheckActive()){
		if(!attempted_combine){
			MCombineQueueLock.lock();
			combine_queue.push_back(app);
			MCombineQueueLock.unlock();
		}
		else{
			MCombineQueueLock.lock();
			PreparePacket(app);
			MCombineQueueLock.unlock();
#ifdef LE_DEBUG
			printf( "After B in %s, line %i:\n", __FUNCTION__, __LINE__);
			DumpPacket(app);
#endif
			SendPacket(app);
		}
	}
}

void EQStream::UnPreparePacket(EQ2Packet* app){
	if(app->pBuffer[2] == 0 && app->pBuffer[3] == 19){
		uchar* new_buffer = new uchar[app->size-3];
		memcpy(new_buffer+2, app->pBuffer+5, app->size-3);
		delete[] app->pBuffer;
		app->size-=3;
		app->pBuffer = new_buffer;
	}
}

#ifdef WRITE_PACKETS
char EQStream::GetChar(uchar in)
{
	if (in < ' ' || in > '~')
		return '.';
	return (char)in;
}
void EQStream::WriteToFile(char* pFormat, ...) {
	va_list args;
	va_start(args, pFormat);
	vfprintf(write_packets, pFormat, args);
	va_end(args);
}

void EQStream::WritePackets(const char* opcodeName, uchar* data, int32 size, bool outgoing) {
	MWritePackets.lock();
	struct in_addr ip_addr;
	ip_addr.s_addr = remote_ip;
	char timebuffer[80];
	time_t rawtime;
	struct tm* timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(timebuffer, 80, "%m/%d/%Y %H:%M:%S", timeinfo);
	if (outgoing)
		WriteToFile("-- %s --\n%s\nSERVER -> %s\n", opcodeName, timebuffer, inet_ntoa(ip_addr));
	else
		WriteToFile("-- %s --\n%s\n%s -> SERVER\n", opcodeName, timebuffer, inet_ntoa(ip_addr));
	int i;
	int nLines = size / 16;
	int nExtra = size % 16;
	uchar* pPtr = data;
	for (i = 0; i < nLines; i++)
	{
		WriteToFile("%4.4X:\t%2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n", i * 16, pPtr[0], pPtr[1], pPtr[2], pPtr[3], pPtr[4], pPtr[5], pPtr[6], pPtr[7], pPtr[8], pPtr[9], pPtr[10], pPtr[11], pPtr[12], pPtr[13], pPtr[14], pPtr[15], GetChar(pPtr[0]), GetChar(pPtr[1]), GetChar(pPtr[2]), GetChar(pPtr[3]), GetChar(pPtr[4]), GetChar(pPtr[5]), GetChar(pPtr[6]), GetChar(pPtr[7]), GetChar(pPtr[8]), GetChar(pPtr[9]), GetChar(pPtr[10]), GetChar(pPtr[11]), GetChar(pPtr[12]), GetChar(pPtr[13]), GetChar(pPtr[14]), GetChar(pPtr[15]));
		pPtr += 16;
	}
	if (nExtra)
	{
		WriteToFile("%4.4X\t", nLines * 16);
		for (i = 0; i < nExtra; i++)
		{
			WriteToFile("%2.2X ", pPtr[i]);
		}
		for (i; i < 16; i++)
			WriteToFile("   ");
		for (i = 0; i < nExtra; i++)
		{
			WriteToFile("%c", GetChar(pPtr[i]));
		}
		WriteToFile("\n");
	}
	WriteToFile("\n\n");
	fflush(write_packets);
	MWritePackets.unlock();
}

void EQStream::WritePackets(EQ2Packet* app, bool outgoing) {
	if (app->version == 0)
		app->version = client_version;
	WritePackets(app->GetOpcodeName(), app->pBuffer, app->size, outgoing);
}
#endif

void EQStream::PreparePacket(EQ2Packet* app, int8 offset){
	app->setVersion(client_version);
	compressed_offset = 0;

#ifdef LE_DEBUG
	printf( "Before A in %s, line %i:\n", __FUNCTION__, __LINE__);
	DumpPacket(app);
#endif
	if(!app->packet_prepared){
		if(app->PreparePacket(MaxLen) == 255) //invalid version
			return;
	}

#ifdef LE_DEBUG
	printf( "After Prepare in %s, line %i:\n", __FUNCTION__, __LINE__);
	DumpPacket(app);
#endif
#ifdef WRITE_PACKETS
	if (!app->eq2_compressed && !app->packet_encrypted)
		WritePackets(app, true);
#endif

	if(!app->eq2_compressed && app->size>128){
		compressed_offset = EQ2_Compress(app);
		if (compressed_offset)
			app->eq2_compressed = true;
	}
	if(!app->packet_encrypted){
		EncryptPacket(app, compressed_offset, offset);
		if(app->size > 2 && app->pBuffer[2] == 0){
			uchar* new_buffer = new uchar[app->size+1];
			new_buffer[2] = 0;
			memcpy(new_buffer+3, app->pBuffer+2, app->size-2);
			delete[] app->pBuffer;
			app->pBuffer = new_buffer;
			app->size++;
		}
	}

#ifdef LE_DEBUG
	printf( "After A in %s, line %i:\n", __FUNCTION__, __LINE__);
	DumpPacket(app);
#endif

}

void EQStream::SendPacket(EQProtocolPacket *p)
{
	uint32 chunksize,used;
	uint32 length;

	// Convert the EQApplicationPacket to 1 or more EQProtocolPackets
	if (p->size>( MaxLen-8)) { // proto-op(2), seq(2), app-op(2) ... data ... crc(2)
		uchar* tmpbuff=p->pBuffer;
		length=p->size - 2;

		EQProtocolPacket *out=new EQProtocolPacket(OP_Fragment,NULL,MaxLen-4);
		*(uint32 *)(out->pBuffer+2)=htonl(length);
		used=MaxLen-10;
		memcpy(out->pBuffer+6,tmpbuff+2,used);

#ifdef LE_DEBUG
		printf("(%s, %i) New Fragment:\n ", __FUNCTION__, __LINE__);
		DumpPacket(out);
#endif

		SequencedPush(out);

		while (used<length) {
			chunksize=min(length-used,MaxLen-6);
			out=new EQProtocolPacket(OP_Fragment,NULL,chunksize+2);
			//memcpy(out->pBuffer+2,tmpbuff,1);
			memcpy(out->pBuffer+2,tmpbuff+used+2,chunksize);
#ifdef LE_DEBUG
		printf("Chunk: \n");
		DumpPacket(out);
#endif
			SequencedPush(out);
			used+=chunksize;

		}

#ifdef LE_DEBUG
		printf( "ChunkDelete: \n");
		DumpPacket(out);
		//cerr << "1: Deleting 0x" << hex << (uint32)(p) << dec << endl;
#endif

		delete p;
	} else {
		SequencedPush(p);
	}
}
void EQStream::SendPacket(EQApplicationPacket *p)
{
uint32 chunksize,used;
uint32 length;

	// Convert the EQApplicationPacket to 1 or more EQProtocolPackets
	if (p->size>(MaxLen-8)) { // proto-op(2), seq(2), app-op(2) ... data ... crc(2)
		//cout << "Making oversized packet for: " << endl;
		//cout << p->size << endl;
		//p->DumpRawHeader();
		//dump_message(p->pBuffer,p->size,timestamp());
		//cout << p->size << endl;
		unsigned char *tmpbuff=new unsigned char[p->size+2];
		//cout << hex << (int)tmpbuff << dec << endl;
		length=p->serialize(tmpbuff);

		EQProtocolPacket *out=new EQProtocolPacket(OP_Fragment,NULL,MaxLen-4);
		*(uint32 *)(out->pBuffer+2)=htonl(p->Size());
		memcpy(out->pBuffer+6,tmpbuff,MaxLen-10);
		used=MaxLen-10;
		SequencedPush(out);
		//cout << "Chunk #" << ++i << " size=" << used << ", length-used=" << (length-used) << endl;
		while (used<length) {
			out=new EQProtocolPacket(OP_Fragment,NULL,MaxLen-4);
			chunksize=min(length-used,MaxLen-6);
			memcpy(out->pBuffer+2,tmpbuff+used,chunksize);
			out->size=chunksize+2;
			SequencedPush(out);
			used+=chunksize;
			//cout << "Chunk #"<< ++i << " size=" << chunksize << ", length-used=" << (length-used) << endl;
		}
		//cerr << "1: Deleting 0x" << hex << (uint32)(p) << dec << endl;
		delete p;
		delete[] tmpbuff;
	} else {
		EQProtocolPacket *out=new EQProtocolPacket(OP_Packet,NULL,p->Size()+2);
		p->serialize(out->pBuffer+2);
		SequencedPush(out);
		//cerr << "2: Deleting 0x" << hex << (uint32)(p) << dec << endl;
		delete p;
	}
}

void EQStream::SequencedPush(EQProtocolPacket *p)
{
	p->setVersion(client_version);
	MOutboundQueue.lock();
	*(uint16 *)(p->pBuffer)=htons(NextOutSeq);
	SequencedQueue.push_back(p);
	p->sequence = NextOutSeq;
	NextOutSeq++;
	MOutboundQueue.unlock();
}

void EQStream::NonSequencedPush(EQProtocolPacket *p)
{
	p->setVersion(client_version);
	MOutboundQueue.lock();
	NonSequencedQueue.push(p);
	MOutboundQueue.unlock();
}

void EQStream::SendAck(uint16 seq)
{
	uint16 Seq=htons(seq);
	SetLastAckSent(seq);
	NonSequencedPush(new EQProtocolPacket(OP_Ack,(unsigned char *)&Seq,sizeof(uint16)));
}

void EQStream::SendOutOfOrderAck(uint16 seq)
{
	uint16 Seq=htons(seq);
	NonSequencedPush(new EQProtocolPacket(OP_OutOfOrderAck,(unsigned char *)&Seq,sizeof(uint16)));
}

bool EQStream::CheckCombineQueue(){
	bool ret = true; //processed all packets
	MCombineQueueLock.lock();
	if(combine_queue.size() > 0){
		EQ2Packet* first = combine_queue.front();
		combine_queue.pop_front();
		if(combine_queue.size() == 0){ //nothing to combine this with
			EQ2QueuePacket(first, true);
		}
		else{
			PreparePacket(first);
			EQ2Packet* second = 0;
			bool combine_worked = false;
			int16 count = 0;
			while(combine_queue.size()){
				count++;
				second = combine_queue.front();
				combine_queue.pop_front();
				PreparePacket(second);
				/*if(first->GetRawOpcode() != OP_AppCombined && first->pBuffer[2] == 0){
					EQ2Packet* tmp = second;
					second = first;
					first = tmp;
				}*/
				if(!first->AppCombine(second)){
					first->SetProtocolOpcode(OP_Packet);
					if(combine_worked){
						SequencedPush(first);
					}
					else{
						EQ2QueuePacket(first, true);
					}
					first = second;
					combine_worked = false;
				}
				else{
					combine_worked = true;
					//DumpPacket(first);
				}
				if(count >= 60 || first->size > 4000){ //other clients need packets too
					ret = false;
					break;
				}
			}
			if(first){
				first->SetProtocolOpcode(OP_Packet);
				if(combine_worked){
					SequencedPush(first);
				}
				else{
					EQ2QueuePacket(first, true);
				}
			}
		}
	}
	MCombineQueueLock.unlock();
	return ret;
}

void EQStream::CheckResend(int eq_fd){
	int32 curr = Timer::GetCurrentTime2();
	EQProtocolPacket* packet = 0;
	deque<EQProtocolPacket*>::iterator itr;
	MResendQue.lock();
	for(itr=resend_que.begin();itr!=resend_que.end();itr++){
		packet = *itr;
		if(packet->attempt_count >= 5){//tried to resend this packet 5 times, client must already have it but didnt ack it
			safe_delete(packet);
			itr = resend_que.erase(itr);
			if(itr == resend_que.end())
				break;
		}
		else{
			if((curr - packet->sent_time) < 1000)
				continue;
			packet->sent_time -=1000;
			packet->attempt_count++;
			WritePacket(eq_fd, packet);
		}
	}
	MResendQue.unlock();
}



//returns SeqFuture if `seq` is later than `expected_seq`
EQStream::SeqOrder EQStream::CompareSequence(uint16 expected_seq, uint16 seq)
{
	if (expected_seq == seq) {
		// Curent
		return SeqInOrder;
	}
	else if ((seq > expected_seq && (uint32)seq < ((uint32)expected_seq + EQStream::MaxWindowSize)) || seq < (expected_seq - EQStream::MaxWindowSize)) {
		// Future
		return SeqFuture;
	}
	else {
		// Past
		return SeqPast;
	}
}

void EQStream::AckPackets(uint16 seq)
{
	std::deque<EQProtocolPacket*>::iterator itr, tmp;

	MOutboundQueue.lock();

	SeqOrder ord = CompareSequence(SequencedBase, seq);
	if (ord == SeqInOrder) {
		//they are not acking anything new...
		LogWrite(PACKET__DEBUG, 9, "Packet",  "Received an ack with no window advancement (seq %u)", seq);
	}
	else if (ord == SeqPast) {
		//they are nacking blocks going back before our buffer, wtf?
		LogWrite(PACKET__DEBUG, 9, "Packet",  "Received an ack with backward window advancement (they gave %u, our window starts at %u). This is bad" , seq, SequencedBase);
	}
	else {
		LogWrite(PACKET__DEBUG, 9, "Packet",  "Received an ack up through sequence %u. Our base is %u", seq, SequencedBase);


		//this is a good ack, we get to ack some blocks.
		seq++;	//we stop at the block right after their ack, counting on the wrap of both numbers.
		while (SequencedBase != seq) {
			if (SequencedQueue.empty()) {
				LogWrite(PACKET__DEBUG, 9, "Packet",  "OUT OF PACKETS acked packet with sequence %u. Next send is %u before this", (unsigned long)SequencedBase, SequencedQueue.size());
				SequencedBase = NextOutSeq;
				break;
			}
			LogWrite(PACKET__DEBUG, 9, "Packet",  "Removing acked packet with sequence %u", (unsigned long)SequencedBase);
			//clean out the acked packet
			delete SequencedQueue.front();
			SequencedQueue.pop_front();
			//advance the base sequence number to the seq of the block after the one we just got rid of.
			SequencedBase++;
		}
		if (uint16(SequencedBase + SequencedQueue.size()) != NextOutSeq) {
			LogWrite(PACKET__DEBUG, 9, "Packet",  "Post-Ack on %u Invalid Sequenced queue: BS %u + SQ %u != NOS %u", seq, SequencedBase, SequencedQueue.size(), NextOutSeq);
		}
	}

	MOutboundQueue.unlock();
}

void EQStream::Write(int eq_fd)
{
	queue<EQProtocolPacket *> ReadyToSend;
	long maxack;

	// Check our rate to make sure we can send more
	MRate.lock();
	sint32 threshold=RateThreshold;
	MRate.unlock();
	if (BytesWritten > threshold) {
		//cout << "Over threshold: " << BytesWritten << " > " << threshold << endl;
		return;
	}

	MCombinedAppPacket.lock();
	EQApplicationPacket *CombPack=CombinedAppPacket;
	CombinedAppPacket=NULL;
	MCombinedAppPacket.unlock();

	if (CombPack) {
		SendPacket(CombPack);
	}

	// If we got more packets to we need to ack, send an ack on the highest one
	MAcks.lock();
	maxack=MaxAckReceived;
	// Added from peaks findings
	if (NextAckToSend>LastAckSent || LastAckSent == 0x0000ffff)
		SendAck(NextAckToSend);
	MAcks.unlock();

	// Lock the outbound queues while we process
	MOutboundQueue.lock();

	// Adjust where we start sending in case we get a late ack
	//if (maxack>LastSeqSent)
	//	LastSeqSent=maxack;

	// Place to hold the base packet t combine into
	EQProtocolPacket *p=NULL;
	std::deque<EQProtocolPacket*>::iterator sitr;

	// Find the next sequenced packet to send from the "queue"
	sitr = SequencedQueue.begin();

	uint16 count = 0;
	// get to start of packets
	while (sitr != SequencedQueue.end() && (*sitr)->sent_time > 0) {
		++sitr;
		++count;
	}

	bool SeqEmpty = false, NonSeqEmpty = false;
	// Loop until both are empty or MaxSends is reached
	while (!SeqEmpty || !NonSeqEmpty) {

		// See if there are more non-sequenced packets left
		if (!NonSequencedQueue.empty()) {
			if (!p) {
				// If we don't have a packet to try to combine into, use this one as the base
				// And remove it form the queue
				p = NonSequencedQueue.front();
				LogWrite(PACKET__DEBUG, 9, "Packet", "Starting combined packet with non-seq packet of len %u",p->size);
				NonSequencedQueue.pop();
			}
			else if (!p->combine(NonSequencedQueue.front())) {
				// Trying to combine this packet with the base didn't work (too big maybe)
				// So just send the base packet (we'll try this packet again later)
				LogWrite(PACKET__DEBUG, 9, "Packet", "Combined packet full at len %u, next non-seq packet is len %u", p->size, (NonSequencedQueue.front())->size);
				ReadyToSend.push(p);
				BytesWritten += p->size;
				p = nullptr;

				if (BytesWritten > threshold) {
					// Sent enough this round, lets stop to be fair
					LogWrite(PACKET__DEBUG, 9, "Packet", "Exceeded write threshold in nonseq (%u > %u)", BytesWritten, threshold);
					break;
				}
			}
			else {
				// Combine worked, so just remove this packet and it's spot in the queue
				LogWrite(PACKET__DEBUG, 9, "Packet", "Combined non-seq packet of len %u, yeilding %u combined", (NonSequencedQueue.front())->size, p->size);
				delete NonSequencedQueue.front();
				NonSequencedQueue.pop();
			}
		}
		else {
			// No more non-sequenced packets
			NonSeqEmpty = true;
		}

		if (sitr != SequencedQueue.end()) {
			uint16 seq_send = SequencedBase + count;	//just for logging...

			if (SequencedQueue.empty()) {
				LogWrite(PACKET__DEBUG, 9, "Packet", "Tried to write a packet with an empty queue (%u is past next out %u)", seq_send, NextOutSeq);
				SeqEmpty = true;
				continue;
			}

				if ((*sitr)->acked || (*sitr)->sent_time != 0) {
					++sitr;
					++count;
					if (p) {
						LogWrite(PACKET__DEBUG, 9, "Packet", "Final combined packet not full, len %u", p->size);
						ReadyToSend.push(p);
						BytesWritten += p->size;
						p = nullptr;
					}
					LogWrite(PACKET__DEBUG, 9, "Packet", "Not retransmitting seq packet %u because already marked as acked", seq_send);
				}
				else if (!p) {
					// If we don't have a packet to try to combine into, use this one as the base
					// Copy it first as it will still live until it is acked
					p = (*sitr)->Copy();
					LogWrite(PACKET__DEBUG, 9, "Packet", "Starting combined packet with seq packet %u of len %u", seq_send, p->size);
					(*sitr)->sent_time = Timer::GetCurrentTime2();
					++sitr;
					++count;
				}
				else if (!p->combine(*sitr)) {
					// Trying to combine this packet with the base didn't work (too big maybe)
					// So just send the base packet (we'll try this packet again later)
					LogWrite(PACKET__DEBUG, 9, "Packet", "Combined packet full at len %u, next seq packet %u is len %u", p->size, seq_send + 1, (*sitr)->size);
					ReadyToSend.push(p);
					BytesWritten += p->size;
					p = nullptr;
					if ((*sitr)->opcode != OP_Fragment && BytesWritten > threshold) {
						// Sent enough this round, lets stop to be fair
						LogWrite(PACKET__DEBUG, 9, "Packet", "Exceeded write threshold in seq (%u > %u)", BytesWritten, threshold);
						break;
					}
				}
				else {
					// Combine worked
					LogWrite(PACKET__DEBUG, 9, "Packet", "Combined seq packet %u of len %u, yeilding %u combined", seq_send, (*sitr)->size, p->size);
					(*sitr)->sent_time = Timer::GetCurrentTime2();
					++sitr;
					++count;
				}

			if (uint16(SequencedBase + SequencedQueue.size()) != NextOutSeq) {
				LogWrite(PACKET__DEBUG, 9, "Packet", "Post send Invalid Sequenced queue: BS %u + SQ %u != NOS %u", SequencedBase, SequencedQueue.size(), NextOutSeq);
			}
		}
		else {
			// No more sequenced packets
			SeqEmpty = true;
		}
	}
	MOutboundQueue.unlock();	// Unlock the queue

	// We have a packet still, must have run out of both seq and non-seq, so send it
	if (p) {
		LogWrite(PACKET__DEBUG, 9, "Packet",  "Final combined packet not full, len %u", p->size);
		ReadyToSend.push(p);
		BytesWritten += p->size;
	}

	// Send all the packets we "made"
	while (!ReadyToSend.empty()) {
		p = ReadyToSend.front();
		WritePacket(eq_fd, p);
		delete p;
		ReadyToSend.pop();
	}

	//see if we need to send our disconnect and finish our close
	if (SeqEmpty && NonSeqEmpty) {
		//no more data to send
		if (GetState() == CLOSING) {
			MOutboundQueue.lock();
			if (SequencedQueue.size() > 0 ) {
				// retransmission attempts
			}
			else
			{
				LogWrite(PACKET__DEBUG, 9, "Packet", "All outgoing data flushed, disconnecting client.");
				//we are waiting for the queues to empty, now we can do our disconnect.
				//this packet will not actually go out until the next call to Write().
				SendDisconnect();
				//SetState(CLOSED);
			}	
			MOutboundQueue.unlock();
		}
	}
}

void EQStream::WritePacket(int eq_fd, EQProtocolPacket *p)
{
uint32 length = 0;
sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr=remote_ip;
	address.sin_port=remote_port;
#ifdef NOWAY
	uint32 ip=address.sin_addr.s_addr;
	cout << "Sending to: " 
		<< (int)*(unsigned char *)&ip
		<< "." << (int)*((unsigned char *)&ip+1)
		<< "." << (int)*((unsigned char *)&ip+2)
		<< "." << (int)*((unsigned char *)&ip+3)
		<< "," << (int)ntohs(address.sin_port) << "(" << p->size << ")" << endl;

	p->DumpRaw();
	cout << "-------------" << endl;
#endif
	length=p->serialize(buffer);
	if (p->opcode!=OP_SessionRequest && p->opcode!=OP_SessionResponse) {
		if (compressed) {
			BytesWritten -= p->size;
			uint32 newlen=EQProtocolPacket::Compress(buffer,length,write_buffer,2048);
			memcpy(buffer,write_buffer,newlen);
			length=newlen;
			BytesWritten += newlen;
		}
		if (encoded) {
			EQProtocolPacket::ChatEncode(buffer,length,Key);
		}
		*(uint16 *)(buffer+length)=htons(CRC16(buffer,length,Key));
		length+=2;
	}
	sent_packets++;
	//dump_message_column(buffer,length,"Writer: ");
	//cout << "Raw Data:\n";
	//DumpPacket(buffer, length);
	sendto(eq_fd,(char *)buffer,length,0,(sockaddr *)&address,sizeof(address));
}

EQProtocolPacket *EQStream::Read(int eq_fd, sockaddr_in *from)
{
int socklen;
int length=0;
unsigned char buffer[2048];
EQProtocolPacket *p=NULL;
char temp[15];

	socklen=sizeof(sockaddr);
#ifdef WIN32
	length=recvfrom(eq_fd, (char *)buffer, 2048, 0, (struct sockaddr*)from, (int *)&socklen);
#else
	length=recvfrom(eq_fd, buffer, 2048, 0, (struct sockaddr*)from, (socklen_t *)&socklen);
#endif
	if (length>=2) {
		DumpPacket(buffer, length);
		p=new EQProtocolPacket(buffer[1],&buffer[2],length-2);
		//printf("Read packet: opcode %i length %u, expected-length: %u\n",buffer[1], length, p->size);
		uint32 ip=from->sin_addr.s_addr;
		sprintf(temp,"%d.%d.%d.%d:%d",
			*(unsigned char *)&ip,
			*((unsigned char *)&ip+1),
			*((unsigned char *)&ip+2),
			*((unsigned char *)&ip+3),
			ntohs(from->sin_port));
		//cout << timestamp() << "Data from: " << temp << " OpCode 0x" << hex << setw(2) << setfill('0') << (int)p->opcode << dec << endl;
		//dump_message(p->pBuffer,p->size,timestamp());
		
	}
	return p;
}

void EQStream::SendSessionResponse()
{
EQProtocolPacket *out=new EQProtocolPacket(OP_SessionResponse,NULL,sizeof(SessionResponse));
	SessionResponse *Response=(SessionResponse *)out->pBuffer;
	Response->Session=htonl(Session);
	Response->MaxLength=htonl(MaxLen);
	Response->UnknownA=2;
	Response->Format=0;
	if (compressed)
		Response->Format|=FLAG_COMPRESSED;
	if (encoded)
		Response->Format|=FLAG_ENCODED;
	Response->Key=htonl(Key);

	out->size=sizeof(SessionResponse);

	NonSequencedPush(out);
}

void EQStream::SendSessionRequest()
{
	EQProtocolPacket *out=new EQProtocolPacket(OP_SessionRequest,NULL,sizeof(SessionRequest));
	SessionRequest *Request=(SessionRequest *)out->pBuffer;
	memset(Request,0,sizeof(SessionRequest));
	Request->Session=htonl(time(NULL));
	Request->MaxLength=htonl(512);

	NonSequencedPush(out);
}

void EQStream::SendDisconnect(bool setstate)
{
	try{
		if(GetState() != ESTABLISHED && GetState() != WAIT_CLOSE)
			return;
		
		EQProtocolPacket *out=new EQProtocolPacket(OP_SessionDisconnect,NULL,sizeof(uint32)+sizeof(int16));
		*(uint32 *)out->pBuffer=htonl(Session);
		out->pBuffer[4] = 0;
		out->pBuffer[5] = 6;
		NonSequencedPush(out);
		if(setstate)
			SetState(CLOSING);
	}
	catch(...){}
}

void EQStream::InboundQueuePush(EQApplicationPacket *p)
{
	MInboundQueue.lock();
	InboundQueue.push_back(p);
	MInboundQueue.unlock();
}

EQApplicationPacket *EQStream::PopPacket()
{
EQApplicationPacket *p=NULL;

	MInboundQueue.lock();
	if (InboundQueue.size()) {
		p=InboundQueue.front();
		InboundQueue.pop_front();
	}
	MInboundQueue.unlock();
	if(p)
		p->setVersion(client_version);
	return p;
}

void EQStream::InboundQueueClear()
{
	MInboundQueue.lock();
	while(InboundQueue.size()){
		delete InboundQueue.front();
		InboundQueue.pop_front();
	}
	MInboundQueue.unlock();
}
void EQStream::EncryptPacket(uchar* data, int16 size){
	if(size>6){
		
	}
}
bool EQStream::HasOutgoingData()
{
bool flag;
	
	//once closed, we have nothing more to say
	if(CheckClosed())
		return(false);
	
	MOutboundQueue.lock();
	flag=(!NonSequencedQueue.empty());
	if (!flag) {
		flag = (!SequencedQueue.empty());
	}
	MOutboundQueue.unlock();

	if (!flag) {
		MAcks.lock();
		flag= (NextAckToSend>LastAckSent);
		MAcks.unlock();
	}

	if (!flag) {
		MCombinedAppPacket.lock();
		flag=(CombinedAppPacket!=NULL);
		MCombinedAppPacket.unlock();
	}

	return flag;
}

void EQStream::OutboundQueueClear()
{
	MOutboundQueue.lock();
	while(NonSequencedQueue.size()) {
		delete NonSequencedQueue.front();
		NonSequencedQueue.pop();
	}
	while(SequencedQueue.size()) {
		delete SequencedQueue.front();
		SequencedQueue.pop_front();
	}
	MOutboundQueue.unlock();
}

void EQStream::Process(const unsigned char *buffer, const uint32 length)
{
	received_packets++;
static unsigned char newbuffer[2048];
uint32 newlength=0;

#ifdef LE_DEBUG
printf("ProcessBuffer:\n");
DumpPacket(buffer, length);
#endif

	if (EQProtocolPacket::ValidateCRC(buffer,length,Key)) {
		if (compressed) {
			newlength=EQProtocolPacket::Decompress(buffer,length,newbuffer,2048);
#ifdef LE_DEBUG
			printf("ProcessBufferDecompress:\n");
			DumpPacket(buffer, newlength);
#endif
		} else {
			memcpy(newbuffer,buffer,length);
			newlength=length;
			if (encoded)
				EQProtocolPacket::ChatDecode(newbuffer,newlength-2,Key);
		}

#ifdef LE_DEBUG
		printf("ResultProcessBuffer:\n");
		DumpPacket(buffer, newlength);
#endif
		uint16 opcode=ntohs(*(const uint16 *)newbuffer);
		//printf("Read packet: opcode %i newlength %u, newbuffer2len: %u, newbuffer3len: %u\n",opcode, newlength, newbuffer[2], newbuffer[3]);
		if(opcode > 0 && opcode <= OP_OutOfSession)
		{
			if (buffer[1]!=0x01 && buffer[1]!=0x02 && buffer[1]!=0x1d)
				newlength-=2;
			
			EQProtocolPacket p(newbuffer,newlength);
			ProcessPacket(&p);
		}
		else
		{
			cout << "2Orig Packet: " << opcode << endl;
			DumpPacket(newbuffer, newlength);
			ProcessEmbeddedPacket(newbuffer, newlength, OP_Fragment);
		}
		ProcessQueue();
	} else {
		cout << "Incoming packet failed checksum:" <<endl;
		dump_message_column(const_cast<unsigned char *>(buffer),length,"CRC failed: ");
	}
}

long EQStream::GetMaxAckReceived()
{
	MAcks.lock();
	long l=MaxAckReceived;
	MAcks.unlock();

	return l;
}

long EQStream::GetNextAckToSend()
{
	MAcks.lock();
	long l=NextAckToSend;
	MAcks.unlock();

	return l;
}

long EQStream::GetLastAckSent()
{
	MAcks.lock();
	long l=LastAckSent;
	MAcks.unlock();

	return l;
}

void EQStream::SetMaxAckReceived(uint32 seq)
{
	deque<EQProtocolPacket *>::iterator itr;

	MAcks.lock();
	MaxAckReceived=seq;
	MAcks.unlock();
	MOutboundQueue.lock();
	if (long(seq) > LastSeqSent)
		LastSeqSent=seq;
	MResendQue.lock();
	EQProtocolPacket* packet = 0;
	for(itr=resend_que.begin();itr!=resend_que.end();itr++){
		packet = *itr;
		if(packet && packet->sequence <= seq){
			safe_delete(packet);
			itr = resend_que.erase(itr);
			if(itr == resend_que.end())
				break;
		}
	}
	MResendQue.unlock();
	MOutboundQueue.unlock();
}

void EQStream::SetNextAckToSend(uint32 seq)
{
	MAcks.lock();
	NextAckToSend=seq;
	MAcks.unlock();
}

void EQStream::SetLastAckSent(uint32 seq)
{
	MAcks.lock();
	LastAckSent=seq;
	MAcks.unlock();
}

void EQStream::SetLastSeqSent(uint32 seq)
{
	MOutboundQueue.lock();
	LastSeqSent=seq;
	MOutboundQueue.unlock();
}

void EQStream::SetStreamType(EQStreamType type)
{
	StreamType=type;
	switch (StreamType) {
		case LoginStream:
			app_opcode_size=1;
			compressed=false;
			encoded=false;
			break;
		case EQ2Stream:
			app_opcode_size=2;
			compressed=false;
			encoded=false;
			break;
		case ChatOrMailStream:
		case ChatStream:
		case MailStream:
			app_opcode_size=1;
			compressed=false;
			encoded=true;
			break;
		case ZoneStream:
		case WorldStream:
		default:
			app_opcode_size=2;
			compressed=true;
			encoded=false;
			break;
	}
}

void EQStream::ProcessQueue()
{
	if (OutOfOrderpackets.empty()) {
		return;
	}

	EQProtocolPacket* qp = NULL;
	while ((qp = RemoveQueue(NextInSeq)) != NULL) {
		//_log(NET__DEBUG, _L "Processing Queued Packet: Seq=%d" __L, NextInSeq);
		ProcessPacket(qp);
		delete qp;
		//_log(NET__APP_TRACE, _L "OP_Packet Queue size=%d" __L, PacketQueue.size());
	}
}

EQProtocolPacket* EQStream::RemoveQueue(uint16 seq)
{
	map<unsigned short, EQProtocolPacket*>::iterator itr;
	EQProtocolPacket* qp = NULL;
	if ((itr = OutOfOrderpackets.find(seq)) != OutOfOrderpackets.end()) {
		qp = itr->second;
		OutOfOrderpackets.erase(itr);
	//_log(NET__APP_TRACE, _L "OP_Packet Queue size=%d" __L, PacketQueue.size());
	}
	return qp;
}

void EQStream::Decay()
{
	MRate.lock();
	uint32 rate=DecayRate;
	MRate.unlock();
	if (BytesWritten>0) {
		BytesWritten-=rate;
		if (BytesWritten<0)
			BytesWritten=0;
	}

	int count = 0;
	MOutboundQueue.lock();
	for (auto sitr = SequencedQueue.begin(); sitr != SequencedQueue.end(); ++sitr, count++) {
		if (!(*sitr)->acked && (*sitr)->sent_time > 0 && ((*sitr)->sent_time + retransmittimeout) < Timer::GetCurrentTime2()) {
			(*sitr)->sent_time = 0;
			LogWrite(PACKET__DEBUG, 9, "Packet", "Timeout exceeded for seq %u.  Flagging packet for retransmission", SequencedBase + count);
		}
	}
	MOutboundQueue.unlock();
}

void EQStream::AdjustRates(uint32 average_delta)
{
	if (average_delta && (average_delta <= AVERAGE_DELTA_MAX)) {
		MRate.lock();
		AverageDelta = average_delta;
		RateThreshold = RATEBASE / average_delta;
		DecayRate = DECAYBASE / average_delta;
		if (BytesWritten > RateThreshold)
			BytesWritten = RateThreshold + DecayRate;
		MRate.unlock();
	}
	else {
		AverageDelta = AVERAGE_DELTA_MAX;
	}
}
