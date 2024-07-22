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
#include "../common/debug.h"
#include <iostream>
#include <iomanip>
#include <stdio.h>

using namespace std;

#include "packet_dump.h"
#include "EQStream.h"
#include "../common/servertalk.h"

void DumpPacketAscii(const uchar* buf, int32 size, int32 cols, int32 skip) {
	// Output as ASCII
	for(int32 i=skip; i<size; i++)
	{
		if ((i-skip)%cols==0)
		{
			cout << endl << setw(3) << setfill(' ') << i-skip << ":";
		}
		else if ((i-skip)%(cols/2)==0)
		{
			cout << " - ";
		}
		if (buf[i] > 32 && buf[i] < 127)
		{
			cout << buf[i];
		}
		else
		{
			cout << '.';
		}
	}
	cout << endl << endl;
}

void DumpPacketHex(const uchar* buf, int32 size, int32 cols, int32 skip) {
	if (size == 0 || size > 39565)
		return;
	// Output as HEX
	char output[4];
	int j = 0; char* ascii = new char[cols+1]; memset(ascii, 0, cols+1);
	int32 i;
    for(i=skip; i<size; i++)
    {
		if ((i-skip)%cols==0) {
			if (i != skip)
				cout << " | " << ascii << endl;
			cout << setw(4) << setfill(' ') << i-skip << ": ";
			memset(ascii, 0, cols+1);
			j = 0;
		}
		else if ((i-skip)%(cols/2) == 0) {
			cout << "- ";
		}
		sprintf(output, "%02X ", (unsigned char)buf[i]);
		cout << output;

		if (buf[i] >= 32 && buf[i] < 127) {
			ascii[j++] = buf[i];
		}
		else {
			ascii[j++] = '.';
		}
//		cout << setfill(0) << setw(2) << hex << (int)buf[i] << " ";
    }
	int32 k = ((i-skip)-1)%cols;
	if (k < 8)
		cout << "  ";
	for (int32 h = k+1; h < cols; h++) {
		cout << "   ";
	}
	cout << " | " << ascii << endl;
	safe_delete_array(ascii);
}

void DumpPacket(const uchar* buf, int32 size)
{
	DumpPacketHex(buf, size);
//	DumpPacketAscii(buf,size);
}

void DumpPacket(const ServerPacket* pack, bool iShowInfo) {
	if (iShowInfo) {
		cout << "Dumping ServerPacket: 0x" << hex << setfill('0') << setw(4) << pack->opcode << dec;
		cout << " size:" << pack->size << endl;
	}
	DumpPacketHex(pack->pBuffer, pack->size);
}

void DumpPacketBin(const ServerPacket* pack) {
	DumpPacketBin(pack->pBuffer, pack->size);
}

void DumpPacketBin(int32 data) {
	DumpPacketBin((uchar*)&data, sizeof(int32));
}

void DumpPacketBin(int16 data) {
	DumpPacketBin((uchar*)&data, sizeof(int16));
}

void DumpPacketBin(int8 data) {
	DumpPacketBin((uchar*)&data, sizeof(int8));
}


void DumpPacketBin(const void* iData, int32 len) {
	if (!len)
		return;
	const int8* data = (const int8*) iData;
	int32 k=0;
	for (k=0; k<len; k++) {
		if (k % 4 == 0) {
			if (k != 0) {
				cout << " | " << hex << setw(2) << setfill('0') << (int) data[k-4] << dec;
				cout << " " << hex << setw(2) << setfill('0') << (int) data[k-3] << dec;
				cout << " " << hex << setw(2) << setfill('0') << (int) data[k-2] << dec;
				cout << " " << hex << setw(2) << setfill('0') << (int) data[k-1] << dec;
				cout << endl;
			}
			cout << setw(4) << setfill('0') << k << ":";
		}
		else if (k % 2 == 0)
			cout << " ";
		cout << " ";
		if (data[k] & 1)
			cout << "1";
		else
			cout << "0";
		if (data[k] & 2)
			cout << "1";
		else
			cout << "0";
		if (data[k] & 4)
			cout << "1";
		else
			cout << "0";
		if (data[k] & 8)
			cout << "1";
		else
			cout << "0";
		if (data[k] & 16)
			cout << "1";
		else
			cout << "0";
		if (data[k] & 32)
			cout << "1";
		else
			cout << "0";
		if (data[k] & 64)
			cout << "1";
		else
			cout << "0";
		if (data[k] & 128)
			cout << "1";
		else
			cout << "0";
	}
	int8 tmp = (k % 4);
	if (!tmp)
		tmp = 4;
	if (tmp <= 3)
		cout << "         ";
	if (tmp <= 2)
		cout << "          ";
	if (tmp <= 1)
		cout << "         ";
	cout << " | " << hex << setw(2) << setfill('0') << (int) data[k-4] << dec;
	if (tmp > 1)
		cout << " " << hex << setw(2) << setfill('0') << (int) data[k-3] << dec;
	if (tmp > 2)
		cout << " " << hex << setw(2) << setfill('0') << (int) data[k-2] << dec;
	if (tmp > 3)
		cout << " " << hex << setw(2) << setfill('0') << (int) data[k-1] << dec;
	cout << endl;
}
