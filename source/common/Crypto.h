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
#ifndef _CRYPTO_H
#define _CRYPTO_H
#include <string>
#include <mutex>
#include "RC4.h"
#include "../common/types.h"

using namespace std;
class Crypto {
public:
	~Crypto(){ safe_delete(client); safe_delete(server); }
	Crypto() { rc4_key = 0; encrypted = false; client = 0; server = 0; };

	static int64 RSADecrypt(uchar* text, int16 size);
	void RC4Encrypt(uchar* text, int32 size);
	void RC4Decrypt(uchar* text, int32 size);
	int64 getRC4Key() { return rc4_key; }
	void setRC4Key(int64 key) { 
		rc4_key = key;
		if(key > 0){
			encrypted = true; 
			client = new RC4(~key);
			server = new RC4(key); 
			uchar temp[20];
			client->Cypher(temp, 20);
			server->Cypher(temp, 20);
		}
		else{
			encrypted = false;
			safe_delete(client);
			safe_delete(server);
		}
	}
	bool isEncrypted(){ return encrypted; }
	void setEncrypted(bool in_val){ encrypted = in_val; }


private:
	RC4* server;
	RC4* client;
	bool encrypted;
	int64 rc4_key;
	mutex MCrypto;
};

#endif

