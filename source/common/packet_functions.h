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
#ifndef PACKET_FUNCTIONS_H
#define PACKET_FUNCTIONS_H
#include "types.h"
#include "EQPacket.h"

int32 roll(int32 in, int8 bits);
int64 roll(int64 in, int8 bits);
int32 rorl(int32 in, int8 bits);
int64 rorl(int64 in, int8 bits);

void EncryptProfilePacket(EQApplicationPacket* app);
void EncryptProfilePacket(uchar* pBuffer, int32 size);

#define EncryptSpawnPacket EncryptZoneSpawnPacket
//void EncryptSpawnPacket(EQApplicationPacket* app);
//void EncryptSpawnPacket(uchar* pBuffer, int32 size);

void EncryptZoneSpawnPacket(EQApplicationPacket* app);
void EncryptZoneSpawnPacket(uchar* pBuffer, int32 size);

int DeflatePacket(unsigned char* in_data, int in_length, unsigned char* out_data, int max_out_length);
uint32 InflatePacket(uchar* indata, uint32 indatalen, uchar* outdata, uint32 outdatalen, bool iQuiet = false);
uint32 GenerateCRC(int32 b, int32 bufsize, uchar *buf);
uint32 GenerateCRCRecipe(uint32 b, void* buf, uint32 bufsize);

#endif
