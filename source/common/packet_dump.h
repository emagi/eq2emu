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
#ifndef PACKET_DUMP_H
#define PACKET_DUMP_H

#include <iostream>
using namespace std;

#include "../common/types.h"
#include "EQPacket.h"

class ServerPacket;

void DumpPacketAscii(const uchar* buf, int32 size, int32 cols=16, int32 skip=0);
void DumpPacketHex(const uchar* buf, int32 size, int32 cols=16, int32 skip=0);
void DumpPacketBin(const void* data, int32 len);
void DumpPacket(const uchar* buf, int32 size);
void DumpPacket(const ServerPacket* pack, bool iShowInfo = false);
void DumpPacketBin(const ServerPacket* pack);
void DumpPacketBin(int32 data);
void DumpPacketBin(int16 data);
void DumpPacketBin(int8 data);

#endif
