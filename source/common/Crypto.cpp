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
#include "Crypto.h"
#include <iostream>
#include "../common/packet_dump.h"

using namespace std;
void test();
int64 Crypto::RSADecrypt(uchar* text, int16 size){
	int64 ret = 0;
	uchar* buffer = new uchar[8];
	for(int i=7;i>=0;i--)
		buffer[7-i] = text[i];
	memcpy(&ret, buffer, 8);
	safe_delete_array(buffer);
	return ret;
}

void Crypto::RC4Decrypt(uchar* text, int32 size){
    MCrypto.lock();
	client->Cypher(text, size);
    MCrypto.unlock();
}

void Crypto::RC4Encrypt(uchar* text, int32 size){
    MCrypto.lock();
	server->Cypher(text, size);
    MCrypto.unlock();
}

