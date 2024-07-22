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

#include "EQStreamFactory.h"
#include "Log.h"

#ifdef WIN32
	#include <WinSock2.h>
	#include <windows.h>
	#include <process.h>
	#include <io.h>
	#include <stdio.h>
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <sys/select.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <pthread.h>
#endif
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include "op_codes.h"
#include "EQStream.h"
#include "packet_dump.h"
#ifdef WORLD
	#include "../WorldServer/client.h"
#endif
using namespace std;

#ifdef WORLD
	extern ClientList client_list;
#endif
ThreadReturnType EQStreamFactoryReaderLoop(void *eqfs)
{
	if(eqfs){
		EQStreamFactory *fs=(EQStreamFactory *)eqfs;
		fs->ReaderLoop();
	}
	THREAD_RETURN(NULL);
}

ThreadReturnType EQStreamFactoryWriterLoop(void *eqfs)
{
	if(eqfs){
		EQStreamFactory *fs=(EQStreamFactory *)eqfs;
		fs->WriterLoop();
	}
	THREAD_RETURN(NULL);
}

ThreadReturnType EQStreamFactoryCombinePacketLoop(void *eqfs)
{
	if(eqfs){
		EQStreamFactory *fs=(EQStreamFactory *)eqfs;
		fs->CombinePacketLoop();
	}
	THREAD_RETURN(NULL);
}

EQStreamFactory::EQStreamFactory(EQStreamType type, int port)
{
	StreamType=type;
	Port=port;
	listen_ip_address = 0;
}

void EQStreamFactory::Close()
{
	CheckTimeout(true);
	Stop();
	if (sock != -1) {
#ifdef WIN32
		closesocket(sock);
#else
		close(sock);
#endif
		sock = -1;
	}
}
bool EQStreamFactory::Open()
{
struct sockaddr_in address;
#ifndef WIN32
	pthread_t t1, t2, t3;
#endif
	/* Setup internet address information.  
	This is used with the bind() call */
	memset((char *) &address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(Port);
#if defined(LOGIN) || defined(MINILOGIN)
	if(listen_ip_address)
		address.sin_addr.s_addr = inet_addr(listen_ip_address);
	else
		address.sin_addr.s_addr = htonl(INADDR_ANY);
#else
	address.sin_addr.s_addr = htonl(INADDR_ANY);
#endif
	/* Setting up UDP port for new clients */
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		return false;
	}

	if (::bind(sock, (struct sockaddr *) &address, sizeof(address)) < 0) {
		//close(sock);
		sock=-1;
		return false;
	}
	#ifdef WIN32
		unsigned long nonblock = 1;
		ioctlsocket(sock, FIONBIO, &nonblock);
	#else
		fcntl(sock, F_SETFL, O_NONBLOCK);
	#endif
	//moved these because on windows the output was delayed and causing the console window to look bad
#ifdef LOGIN
		LogWrite(LOGIN__DEBUG, 0, "Login", "Starting factory Reader");
		LogWrite(LOGIN__DEBUG, 0, "Login", "Starting factory Writer");
#elif WORLD
		LogWrite(WORLD__DEBUG, 0, "World", "Starting factory Reader");
		LogWrite(WORLD__DEBUG, 0, "World", "Starting factory Writer");
#endif
	#ifdef WIN32
		_beginthread(EQStreamFactoryReaderLoop,0, this);
		_beginthread(EQStreamFactoryWriterLoop,0, this);
		_beginthread(EQStreamFactoryCombinePacketLoop,0, this);
	#else
		pthread_create(&t1,NULL,EQStreamFactoryReaderLoop,this);
		pthread_create(&t2,NULL,EQStreamFactoryWriterLoop,this);
		pthread_create(&t3,NULL,EQStreamFactoryCombinePacketLoop,this);
		pthread_detach(t1);
		pthread_detach(t2);
		pthread_detach(t3);
	#endif
	return true;
}

EQStream *EQStreamFactory::Pop()
{
	if (!NewStreams.size())
		return NULL;

EQStream *s=NULL;
	//cout << "Pop():Locking MNewStreams" << endl;
	MNewStreams.lock();
	if (NewStreams.size()) {
		s=NewStreams.front();
		NewStreams.pop();
		s->PutInUse();
	}
	MNewStreams.unlock();
	//cout << "Pop(): Unlocking MNewStreams" << endl;

	return s;
}

void EQStreamFactory::Push(EQStream *s)
{
	//cout << "Push():Locking MNewStreams" << endl;
	MNewStreams.lock();
	NewStreams.push(s);
	MNewStreams.unlock();
	//cout << "Push(): Unlocking MNewStreams" << endl;
}

void EQStreamFactory::ReaderLoop()
{
fd_set readset;
map<string,EQStream *>::iterator stream_itr;
int num;
int length;
unsigned char buffer[2048];
sockaddr_in from;
int socklen=sizeof(sockaddr_in);
timeval sleep_time;
	ReaderRunning=true;
	while(sock!=-1) {
		MReaderRunning.lock();
		if (!ReaderRunning)
			break;
		MReaderRunning.unlock();

		FD_ZERO(&readset);
		FD_SET(sock,&readset);

		sleep_time.tv_sec=30;
		sleep_time.tv_usec=0;
		if ((num=select(sock+1,&readset,NULL,NULL,&sleep_time))<0) {
			// What do we wanna do?
		} else if (num==0)
			continue;

		if (FD_ISSET(sock,&readset)) {
#ifdef WIN32
			if ((length=recvfrom(sock,(char*)buffer,sizeof(buffer),0,(struct sockaddr*)&from,(int *)&socklen))<2)
#else
			if ((length=recvfrom(sock,buffer,2048,0,(struct sockaddr *)&from,(socklen_t *)&socklen))<2)
#endif
			{
				// What do we wanna do?
			} else {
				char temp[25];
				sprintf(temp,"%u.%d",ntohl(from.sin_addr.s_addr),ntohs(from.sin_port));
				MStreams.lock();
				if ((stream_itr=Streams.find(temp))==Streams.end() || buffer[1]==OP_SessionRequest) {
					MStreams.unlock();
					if (buffer[1]==OP_SessionRequest) {
						if(stream_itr != Streams.end() && stream_itr->second)
							stream_itr->second->SetState(CLOSED);
						EQStream *s=new EQStream(from);
						s->SetFactory(this);
						s->SetStreamType(StreamType);
						Streams[temp]=s;
						WriterWork.Signal();
						Push(s);
						s->Process(buffer,length);
						s->SetLastPacketTime(Timer::GetCurrentTime2());
					}
				} else {
					EQStream *curstream = stream_itr->second;
					//dont bother processing incoming packets for closed connections
					if(curstream->CheckClosed())
						curstream = NULL;
					else
						curstream->PutInUse();
					MStreams.unlock();
					
					if(curstream) {
						curstream->Process(buffer,length);
						curstream->SetLastPacketTime(Timer::GetCurrentTime2());
						curstream->ReleaseFromUse();
					}
				}
			}
		}
	}
}

void EQStreamFactory::CheckTimeout(bool remove_all)
{
	//lock streams the entire time were checking timeouts, it should be fast.
	MStreams.lock();
	
	unsigned long now=Timer::GetCurrentTime2();
	map<string,EQStream *>::iterator stream_itr;
	
	for(stream_itr=Streams.begin();stream_itr!=Streams.end();) {
		EQStream *s = stream_itr->second;
		EQStreamState state = s->GetState();
		
		if (state==CLOSING && !s->HasOutgoingData()) {
			stream_itr->second->SetState(CLOSED);
			state = CLOSED;
		} else if (s->CheckTimeout(now, STREAM_TIMEOUT)) { 
			const char* stateString;
			switch (state){
			case ESTABLISHED:
				stateString = "Established";
				break;
			case CLOSING:
				stateString = "Closing";
				break;
			case CLOSED:
				stateString = "Closed";
				break;
			case WAIT_CLOSE:
				stateString = "Wait-Close";
				break;
			default:
				stateString = "Unknown";
				break;
			}
			LogWrite(WORLD__DEBUG, 0, "World", "Timeout up!, state=%s (%u)", stateString, state);
			if (state==ESTABLISHED) {
				s->Close();
			}
			else if (state == WAIT_CLOSE) {
				s->SetState(CLOSING);
				state = CLOSING;
			}
			else if (state == CLOSING) {
				//if we time out in the closing state, just give up
				s->SetState(CLOSED);
				state = CLOSED;
			}
		}
		//not part of the else so we check it right away on state change
		if (remove_all || state==CLOSED) {
			if (!remove_all && s->getTimeoutDelays()<2) {
				s->addTimeoutDelay();
				//give it a little time for everybody to finish with it
			} else {
				//everybody is done, we can delete it now

#ifdef LOGIN
				LogWrite(LOGIN__DEBUG, 0, "Login", "Removing connection...");
#else
				LogWrite(WORLD__DEBUG, 0, "World", "Removing connection...");
#endif
				map<string,EQStream *>::iterator temp=stream_itr;
				stream_itr++;
				//let whoever has the stream outside delete it
				#ifdef WORLD
				client_list.RemoveConnection(temp->second);
				#endif
				EQStream* stream = temp->second;
				Streams.erase(temp);
				delete stream;
				continue;
			}
		}

		stream_itr++;
	}
	MStreams.unlock();
}

void EQStreamFactory::CombinePacketLoop(){
	deque<EQStream*> combine_que;
	CombinePacketRunning = true;
	bool packets_waiting = false;
	while(sock!=-1) {
		if (!CombinePacketRunning)
			break;
		MStreams.lock();
		map<string,EQStream *>::iterator stream_itr;
		for(stream_itr=Streams.begin();stream_itr!=Streams.end();stream_itr++) {
			if(!stream_itr->second){
				continue;
			}
			if(stream_itr->second->combine_timer && stream_itr->second->combine_timer->Check())
				combine_que.push_back(stream_itr->second);
		}
		EQStream* stream = 0;
		packets_waiting = false;
		while(combine_que.size()){
			stream = combine_que.front();
			if(stream->CheckActive()){
				if(!stream->CheckCombineQueue())
					packets_waiting = true;
			}
			combine_que.pop_front();
		}
		MStreams.unlock();
		if(!packets_waiting)
			Sleep(25);

		Sleep(1);
	}
}

void EQStreamFactory::WriterLoop()
{
map<string,EQStream *>::iterator stream_itr;
vector<EQStream *> wants_write;
vector<EQStream *>::iterator cur,end;
deque<EQStream*> resend_que;
bool decay=false;
uint32 stream_count;

Timer DecayTimer(20);
	
	WriterRunning=true;
	DecayTimer.Enable();
	while(sock!=-1) {
		Timer::SetCurrentTime();
		//if (!havework) {
			//WriterWork.Wait();
		//}
		MWriterRunning.lock();
		if (!WriterRunning)
			break;
		MWriterRunning.unlock();
		
		wants_write.clear();

		decay=DecayTimer.Check();
		
		//copy streams into a seperate list so we dont have to keep
		//MStreams locked while we are writting
		MStreams.lock();
		for(stream_itr=Streams.begin();stream_itr!=Streams.end();stream_itr++) {
			// If it's time to decay the bytes sent, then let's do it before we try to write
			if(!stream_itr->second){
				Streams.erase(stream_itr);
				break;
			}
			if (decay)
				stream_itr->second->Decay();

			if (stream_itr->second->HasOutgoingData()) {
				stream_itr->second->PutInUse();
				wants_write.push_back(stream_itr->second);
			}
			if(stream_itr->second->resend_que_timer->Check())
				resend_que.push_back(stream_itr->second);
		}
		MStreams.unlock();
		
		//do the actual writes
		cur = wants_write.begin();
		end = wants_write.end();
		for(; cur != end; cur++) {
			(*cur)->Write(sock);
			(*cur)->ReleaseFromUse();
		}
		while(resend_que.size()){
			resend_que.front()->CheckResend(sock);
			resend_que.pop_front();
		}
		Sleep(10);

		MStreams.lock();
		stream_count=Streams.size();
		MStreams.unlock();
		if (!stream_count) {
			//cout << "No streams, waiting on condition" << endl;
			WriterWork.Wait();
			//cout << "Awake from condition, must have a stream now" << endl;
		}
	}
}


