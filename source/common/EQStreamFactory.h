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
#ifndef _EQSTREAMFACTORY_H

#define _EQSTREAMFACTORY_H

#include <queue>
#include <map>
#include "../common/EQStream.h"
#include "../common/Condition.h"
#include "../common/opcodemgr.h"
#include "../common/timer.h"

#define STREAM_TIMEOUT 45000 //in ms

class EQStreamFactory {
	private:
		int sock;
		int Port;

		bool ReaderRunning;
		Mutex MReaderRunning;
		bool WriterRunning;
		Mutex MWriterRunning;
		bool CombinePacketRunning;
		Mutex MCombinePacketRunning;

		Condition WriterWork;

		EQStreamType StreamType;
		
		queue<EQStream *> NewStreams;
		Mutex MNewStreams;

		map<string,EQStream *> Streams;
		Mutex MStreams;

		

		Timer *DecayTimer;

	public:
		char*	listen_ip_address;
		void CheckTimeout(bool remove_all = false);
		EQStreamFactory(EQStreamType type) { ReaderRunning=false; WriterRunning=false; StreamType=type; }
		EQStreamFactory(EQStreamType type, int port);
		~EQStreamFactory(){
			safe_delete_array(listen_ip_address);
		}

		EQStream *Pop();
		void Push(EQStream *s);

		bool loadPublicKey();
		bool Open();
		bool Open(unsigned long port) { Port=port; return Open(); }
		void Close();
		void ReaderLoop();
		void WriterLoop();
		void CombinePacketLoop();
		void Stop() { StopReader(); StopWriter(); StopCombinePacket(); }
		void StopReader() { MReaderRunning.lock(); ReaderRunning=false; MReaderRunning.unlock(); }
		void StopWriter() { MWriterRunning.lock(); WriterRunning=false; MWriterRunning.unlock(); WriterWork.Signal(); }
		void StopCombinePacket() { MCombinePacketRunning.lock(); CombinePacketRunning=false; MCombinePacketRunning.unlock(); }
		void SignalWriter() { WriterWork.Signal(); }

};

#endif
