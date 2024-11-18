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
#include "../common/Log.h"
#include "MiscFunctions.h"
#include <string.h>
#include <time.h>
#include <math.h>
#include <chrono>
#include <random>

#ifndef WIN32
#include <netinet/in.h>
#include <sys/socket.h>
#endif
#include <iostream>
#include <iomanip>
#ifdef WIN32
	#include <io.h>
#endif
#include "../common/timer.h"
#include "../common/seperator.h"
#include "../common/packet_dump.h"
#include <algorithm>

using namespace std;

#ifndef PATCHER
extern map<int16, int16> EQOpcodeVersions;
#endif

#ifdef WIN32
	#include <WinSock2.h>
	#include <windows.h>

	#define snprintf	_snprintf
	#define vsnprintf	_vsnprintf
	#define strncasecmp	_strnicmp
	#define strcasecmp  _stricmp
#else
	#include <stdlib.h>
	#include <ctype.h>
	#include <stdarg.h>
	#include <sys/types.h>
	#include <sys/time.h>
#ifdef FREEBSD //Timothy Whitman - January 7, 2003
       #include <sys/socket.h>
       #include <netinet/in.h>
 #endif
	#include <sys/stat.h>
	#include <unistd.h>
	#include <netdb.h>
	#include <errno.h>
#endif

void CoutTimestamp(bool ms) {
	time_t rawtime;
	struct tm* gmt_t;
	time(&rawtime);
	gmt_t = gmtime(&rawtime);

    struct timeval read_time;	
    gettimeofday(&read_time,0);

	cout << (gmt_t->tm_year + 1900) << "/" << setw(2) << setfill('0') << (gmt_t->tm_mon + 1) << "/" << setw(2) << setfill('0') << gmt_t->tm_mday << " " << setw(2) << setfill('0') << gmt_t->tm_hour << ":" << setw(2) << setfill('0') << gmt_t->tm_min << ":" << setw(2) << setfill('0') << gmt_t->tm_sec;
	if (ms)
		cout << "." << setw(3) << setfill('0') << (read_time.tv_usec / 1000);
	cout << " GMT";
}

string loadInt32String(uchar* buffer, int16 buffer_size, int16* pos, EQ2_32BitString* eq_string){
	buffer += *pos;
	int32 size = *(int32*)buffer;
	if((size + *pos + sizeof(int16)) > buffer_size){
		cout << "Error in loadInt32String: Corrupt packet.\n";
		return string("");
	}
	buffer += sizeof(int32);
	string ret((char*)buffer, 0, size);
	if(eq_string){
		eq_string->size = size;
		eq_string->data = ret;
	}
	*pos += (size + sizeof(int32));
	return ret;
}
string loadInt16String(uchar* buffer, int16 buffer_size, int16* pos, EQ2_16BitString* eq_string){
	buffer += *pos;
	int16 size = *(int16*)buffer;
	if((size + *pos + sizeof(int16))> buffer_size){
		cout << "Error in loadInt16String: Corrupt packet.\n";
		return string("");
	}
	buffer += sizeof(int16);
	string ret((char*)buffer, 0, size);
	if(eq_string){
		eq_string->size = size;
		eq_string->data = ret;
	}
	*pos += (size + sizeof(int16));
	return ret;
}
string loadInt8String(uchar* buffer, int16 buffer_size, int16* pos, EQ2_8BitString* eq_string){
	buffer += *pos;
	int8 size = *(int8*)buffer;
	if((size + *pos + sizeof(int16)) > buffer_size){
		cout << "Error in loadInt8String: Corrupt packet.\n";
		return string("");
	}
	buffer += sizeof(int8);
	string ret((char*)buffer, 0, size);
	if(eq_string){
		eq_string->size = size;
		eq_string->data = ret;
	}
	*pos += (size + sizeof(int8));
	return ret;
}

sint16 storeInt32String(uchar* buffer, int16 buffer_size, string in_str){
	sint16 string_size = in_str.length();
	if((string_size + sizeof(int32)) > buffer_size)
		return -1;
	memcpy(buffer, &string_size, sizeof(int32));
	buffer += sizeof(int32);
	memcpy(buffer, in_str.c_str(), string_size);
	buffer += string_size;
	return (buffer_size - (string_size + sizeof(int32)));
}
sint16 storeInt16String(uchar* buffer, int16 buffer_size, string in_str){
	sint16 string_size = in_str.length();
	if((string_size + sizeof(int16)) > buffer_size)
		return -1;
	memcpy(buffer, &string_size, sizeof(int16));
	buffer += sizeof(int16);
	memcpy(buffer, in_str.c_str(), string_size);
	buffer += string_size;
	return (buffer_size - (string_size + sizeof(int16)));
}
sint16 storeInt8String(uchar* buffer, int16 buffer_size, string in_str){
	sint16 string_size = in_str.length();
	if((string_size + sizeof(int8)) > buffer_size)
		return -1;
	memcpy(buffer, &string_size, sizeof(int8));
	buffer += sizeof(int8);
	memcpy(buffer, in_str.c_str(), string_size);
	buffer += string_size;
	return (buffer_size - (string_size + sizeof(int8)));
}


sint32 filesize(FILE* fp) {
#ifdef WIN32
	return _filelength(_fileno(fp));
#else
	struct stat file_stat;
	fstat(fileno(fp), &file_stat);
	return (sint32) file_stat.st_size;
#endif
}

int32 ResolveIP(const char* hostname, char* errbuf) {
#ifdef WIN32
	static InitWinsock ws;
#endif
	if (errbuf)
		errbuf[0] = 0;
	if (hostname == 0) {
		if (errbuf)
			snprintf(errbuf, ERRBUF_SIZE, "ResolveIP(): hostname == 0");
		return 0;
	}
    struct sockaddr_in	server_sin;
#ifdef WIN32
	PHOSTENT phostent = NULL;
#else
	struct hostent *phostent = NULL;
#endif
	server_sin.sin_family = AF_INET;
	if ((phostent = gethostbyname(hostname)) == NULL) {
#ifdef WIN32
		if (errbuf)
			snprintf(errbuf, ERRBUF_SIZE, "Unable to get the host name. Error: %i", WSAGetLastError());
#else
		if (errbuf)
			snprintf(errbuf, ERRBUF_SIZE, "Unable to get the host name. Error: %s", strerror(errno));
#endif
		return 0;
	}
#ifdef WIN32
	memcpy ((char FAR *)&(server_sin.sin_addr), phostent->h_addr, phostent->h_length);
#else
	memcpy ((char*)&(server_sin.sin_addr), phostent->h_addr, phostent->h_length);
#endif
	return server_sin.sin_addr.s_addr;
}

#ifdef WIN32
InitWinsock::InitWinsock() {
	WORD version = MAKEWORD (1,1);
	WSADATA wsadata;
	WSAStartup (version, &wsadata);
}

InitWinsock::~InitWinsock() {
	WSACleanup();
}

#endif

#ifndef WIN32
const char * itoa(int value) {
	static char temp[_ITOA_BUFLEN];
	memset(temp, 0, _ITOA_BUFLEN);
	snprintf(temp, _ITOA_BUFLEN,"%d", value);
	return temp;
}


char * itoa(int value, char *result, int base) {
	char *ptr1, *ptr2;
	char c;
	int tmp_value;

	//need a valid base
	if (base < 2 || base > 36) {
		*result = '\0';
		return result;
	}

	ptr1 = ptr2 = result;
	do {
		tmp_value = value;
		value /= base;

		*ptr1++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
	}
	while (value > 0);

	//apply a negative sign if need be
	if (tmp_value < 0)
		*ptr1++ = '-';

	*ptr1-- = '\0';
	while (ptr2 < ptr1) {
		c = *ptr1;
		*ptr1-- = *ptr2;
		*ptr2++ = c;
	}

	return result;
}
#endif

/* 
 * solar: generate a random integer in the range low-high 
 * this should be used instead of the rand()%limit method
 */
int MakeRandomInt(int low, int high)
{
	return (int)MakeRandomFloat((double)low, (double)high + 0.999);
}
int32 hextoi(char* num) {
	int len = strlen(num);
	if (len < 3)
		return 0;

	if (num[0] != '0' || (num[1] != 'x' && num[1] != 'X'))
		return 0;

	int32 ret = 0;
	int mul = 1;
	for (int i=len-1; i>=2; i--) {
		if (num[i] >= 'A' && num[i] <= 'F')
			ret += ((num[i] - 'A') + 10) * mul;
		else if (num[i] >= 'a' && num[i] <= 'f')
			ret += ((num[i] - 'a') + 10) * mul;
		else if (num[i] >= '0' && num[i] <= '9')
			ret += (num[i] - '0') * mul;
		else
			return 0;
		mul *= 16;
	}
	return ret;
}

int64 hextoi64(char* num) {
	int len = strlen(num);
	if (len < 3)
		return 0;

	if (num[0] != '0' || (num[1] != 'x' && num[1] != 'X'))
		return 0;

	int64 ret = 0;
	int mul = 1;
	for (int i=len-1; i>=2; i--) {
		if (num[i] >= 'A' && num[i] <= 'F')
			ret += ((num[i] - 'A') + 10) * mul;
		else if (num[i] >= 'a' && num[i] <= 'f')
			ret += ((num[i] - 'a') + 10) * mul;
		else if (num[i] >= '0' && num[i] <= '9')
			ret += (num[i] - '0') * mul;
		else
			return 0;
		mul *= 16;
	}
	return ret;
}

float MakeRandomFloat(float low, float high) {
    // Handle edge case where range is zero or inverted
	float diff = high - low;
	if(!diff) return low;
	
    if (low == high) return low;
    if (low > high) std::swap(low, high);

    // Use a thread-local random generator for thread safety
    thread_local std::mt19937 generator(std::random_device{}()); // Seed once per thread
    std::uniform_real_distribution<float> distribution(low, high);

    return distribution(generator);
}

int32 GenerateEQ2Color(float* r, float* g, float* b){
	int8 rgb[4] = {0};
	rgb[0] = (int8)((*r)*255);
	rgb[1] = (int8)((*b)*255);
	rgb[2] = (int8)((*g)*255);
	int32 color = 0;
	memcpy(&color, rgb, sizeof(int32));
	return color;
}
int32 GenerateEQ2Color(float* rgb[3]){
		return GenerateEQ2Color(rgb[0], rgb[1], rgb[2]);
}
int8 MakeInt8(float* input){
	float input2 = *input;
	if(input2 < 0)
		input2 *= -1;
	return (int8)(input2*255);
}

vector<string>* SplitString(string str, char delim){
	vector<string>* results = new vector<string>;
	int32 pos;
	while((pos = str.find_first_of(delim))!= str.npos){
		if(pos > 0){
			results->push_back(str.substr(0,pos));
		}
		if(str.length() > pos)
			str = str.substr(pos+1);
		else
			break;
	}
	if(str.length() > 0)
		results->push_back(str);
	return results;
}

bool Unpack(uchar* data, uchar* dst, int16 dstLen, int16 version, bool reverse){
	int32 srcLen = 0;
	memcpy(&srcLen, data, sizeof(int32));
	return Unpack(srcLen, data + 4, dst, dstLen, version, reverse);
}
bool Unpack(int32 srcLen, uchar* data, uchar* dst, int16 dstLen, int16 version, bool reverse) {
//	int32 srcLen = 0;
//	memcpy(&srcLen, data, sizeof(int32));
//	data+=4;
	if(reverse)
		Reverse(data, srcLen);
    int16 pos = 0;
	int16 real_pos = 0;
    while(srcLen && pos < dstLen) {
        if(srcLen >= 0 && !srcLen--) 
			return false;
        int8 code = data[real_pos++];

        if(code >= 128) {
			for(int8 index=0; index<7; index++) {
				if(code & 1) {
					if(pos >= dstLen) 
						return false;
					if(srcLen >= 0 && !srcLen--) 
						return false;
					dst[pos++] = data[real_pos++];
				} else {
					if(pos < dstLen) dst[pos++] = 0;
				}
				code >>= 1;
			}
        } else {
            if(pos + code > dstLen) 
				return false;
            memset(dst+pos, 0, code);
            pos+=code;
        }
    } 
    return srcLen <= 0;
}

int32 Pack(uchar* data, uchar* src, int16 srcLen, int16 dstLen, int16 version, bool reverse) {
	int16	real_pos = 4;
    int32	pos     = 0;
    int32	code    = 0;
    int		codePos = 0;
    int		codeLen = 0;
    int8	zeroLen = 0;
	memset(data,0,dstLen);
	if (version > 1 && version <= 374)
		reverse = false;
    while(pos < srcLen) {
        if(src[pos] || codeLen) {
            if(!codeLen) {
                /*if(zeroLen > 5) {
					data[real_pos++] = zeroLen;
                    zeroLen = 0;
                }
				else if(zeroLen >= 1 && zeroLen<=5){
					for(;zeroLen>0;zeroLen--)
						codeLen++;
				}*/
				if (zeroLen) {
					data[real_pos++] = zeroLen;
					zeroLen = 0;
				}
                codePos = real_pos;
                code    = 0;
                data[real_pos++] = 0;
            }
            if(src[pos]) {
                data[real_pos++] = src[pos];
                code |= 0x80;
            }
            code  >>= 1;
            codeLen++;

            if(codeLen == 7) {
				data[codePos] = int8(0x80 | code);
                codeLen = 0;
            }
        } else {
            if(zeroLen == 0x7F) {
                data[real_pos++] = zeroLen;
                zeroLen = 0;
			}
            zeroLen++;
        }
        pos++;
    }
    if(codeLen) {
        code >>= (7 - codeLen);
        data[codePos] = int8(0x80 | code);
    } else if(zeroLen) {
        data[real_pos++] = zeroLen;
    }
	if(reverse)
		Reverse(data + 4, real_pos - 4);
    int32 dataLen = real_pos - 4;
	memcpy(&data[0], &dataLen, sizeof(int32));
    return dataLen + 4;
}
void	Reverse(uchar* input, int32 srcLen){
	int16 real_pos = 0;
	int16 orig_pos = 0;
	int8 reverse_count = 0;
	while(srcLen > 0 && srcLen < 0xFFFFFFFF){ // XXX it was >=0 before. but i think it was a bug
		int8 code = input[real_pos++];
		srcLen--;
        if(code >= 128) {
			for(int8 index=0; index<7; index++) {
				if(code & 1) {
					if(srcLen >= 0 && !srcLen--) 
						return;
					real_pos++;
					reverse_count++;
				}
				code >>= 1;
			}
        }
		if(reverse_count > 0){
			int8 tmp_data[8] = {0};
			for(int8 i=0;i<reverse_count;i++){
				tmp_data[i] = input[orig_pos + reverse_count-i];
			}
			memcpy(input + orig_pos + 1, tmp_data, reverse_count);
			reverse_count = 0;
		}
		orig_pos = real_pos;
	}
}

void MovementDecode(uchar* dst, uchar* newval, uchar* orig, int16 len){
	int16 pos = len;
    while(pos--) 
		dst[pos] = newval[pos] ^ orig[pos];
}
void Decode(uchar* dst, uchar* src, int16 len) {
    int16 pos = len;
    while(pos--) 
		dst[pos] ^= src[pos];
    memcpy(src, dst, len);
}

void Encode(uchar* dst, uchar* src, int16 len) {
    uchar* data = new uchar[len];
    int16 pos = len;
    while(pos--) 
		data[pos] = int8(src[pos] ^ dst[pos]);
    memcpy(src, dst, len);
    memcpy(dst, data, len);
	safe_delete_array(data);
}

float TransformToFloat(sint16 data, int8 bits) {
	return (float)(data / (float)(1 << bits));
}

sint16 TransformFromFloat(float data, int8 bits) {
	return (sint16)(data * (1 << bits));
}

void	SetColor(EQ2_Color* color, long data){
	memcpy(color, &data, sizeof(EQ2_Color));
}

string ToUpper(string input){
	string ret = input;
	transform(input.begin(), input.end(), ret.begin(), ::toupper);
	return ret;
}
string ToLower(string input){
	string ret = input;
	transform(input.begin(), input.end(), ret.begin(), ::tolower);
	return ret;
}
int32 ParseIntValue(string input){
	int32 ret = 0xFFFFFFFF;
	try{
		if(input.length() > 0){
			ret = atoul(input.c_str());
		}
	}
	catch(...){}
	return ret;
}

int64 ParseLongLongValue(string input){
	int64 ret = 0xFFFFFFFFFFFFFFFF;
	try{
		if(input.length() > 0){
#ifdef WIN32
			ret = _strtoui64(input.c_str(), NULL, 10);
#else
			ret = strtoull(input.c_str(), 0, 10);
#endif
		}
	}
	catch(...){}
	return ret;
}

map<string, string> TranslateBrokerRequest(string request){
	map<string, string> ret;
	string key;
	string value;
	int32 start_pos = 0;
	int32 end_pos = 0;
	int32 pos = request.find("=");
	bool str_val = false;
	while(pos < 0xFFFFFFFF){
		str_val = false;
		key = request.substr(start_pos, pos-start_pos);
		if(request.find("|", pos) == pos+1){
			pos++;
			end_pos = request.find("|", pos+1);
			str_val = true;
		}
		else
			end_pos = request.find(" ", pos);
		if(end_pos < 0xFFFFFFFF){
			value = request.substr(pos+1, end_pos-pos-1);
			start_pos = end_pos+1;
			if(str_val){
				start_pos++;
				ret[key] = ToLower(value);
			}
			else
				ret[key] = value;
			pos = request.find("=", start_pos);
		}
		else{
			value = request.substr(pos+1);
			if(str_val){
				start_pos++;
				ret[key] = ToLower(value);
			}
			else
				ret[key] = value;
			break;
		}
	}
	return ret;
}

int8 CheckOverLoadSize(int32 val){
	int8 ret = 1;
	if(val >= 0xFFFF) //int32
		ret = sizeof(int16) + sizeof(int32);
	else if(val >= 0xFF)
		ret = sizeof(int8) + sizeof(int16);
	return ret;
}

int8 DoOverLoad(int32 val, uchar* data){
	int8 ret = 1;
	if(val >= 0xFFFF){ //int32
		memset(data, 0xFF, sizeof(int16));
		memcpy(data + sizeof(int16), &val, sizeof(int32));
		ret = sizeof(int16) + sizeof(int32);
	}
	else if(val >= 0xFF){ //int16
		memset(data, 0xFF, sizeof(int8));
		memcpy(data + sizeof(int8), &val, sizeof(int16));
		ret = sizeof(int8) + sizeof(int16);
	}
	else
		memcpy(data, &val, sizeof(int8));
	return ret;
}

/* Treats contiguous spaces as one space. */
int32 CountWordsInString(const char* text) {
	int32 words = 0;
	if (text && strlen(text) > 0) {
		bool on_word = false;
		for (int32 i = 0; i < strlen(text); i++) {
			char letter = text[i];
			if (on_word && !((letter >= 48 && letter <= 57) || (letter >= 65 && letter <= 90) || (letter >= 97 && letter <= 122)))
				on_word = false;
			else if (!on_word && ((letter >= 48 && letter <= 57) || (letter >= 65 && letter <= 90) || (letter >= 97 && letter <= 122))){
				on_word = true;
				words++;
			}
		}
	}
	return words;
}

bool IsNumber(const char *num) {
	size_t len, i;

	if (!num)
		return false;

	len = strlen(num);
	if (len == 0)
		return false;

	for (i = 0; i < len; i++) {
		if (!isdigit(num[i]))
			return false;
	}

	return true;
}

void PrintSep(Seperator *sep, const char *name) {
	int32 i = 0;

	LogWrite(MISC__DEBUG, 0, "Misc", "Printing sep %s", name ? name : "No Name");
	if (!sep)
		LogWrite(MISC__DEBUG, 0, "Misc", "\tSep is null");
	else {
		while (sep->arg[i] && strlen(sep->arg[i]) > 0) {
			LogWrite(MISC__DEBUG, 0, "Misc", "\t%i => %s", i, sep->arg[i]);
			i++;
		}
	}
}

#define INI_IGNORE(c) (c == '\n' || c == '\r' || c == '#')

static bool INIGoToSection(FILE *f, const char *section) {
	size_t size = strlen(section) + 3;
	char line[256], *buf, *tmp;
	bool found = false;

	if ((buf = (char *)malloc(size)) == NULL) {
		fprintf(stderr, "%s: %u: Unable to allocate %zu bytes\n", __FUNCTION__, __LINE__, size);
		return false;
	}

	sprintf(buf, "[%s]", section);

	while (fgets(line, sizeof(line), f) != NULL) {
		if (INI_IGNORE(line[0]))
			continue;

		if (line[0] == '[') {
			if ((tmp = strstr(line, "\n")) != NULL)
				*tmp = '\0';
			if ((tmp = strstr(line, "\r")) != NULL)
				*tmp = '\0';

			if (strcasecmp(buf, line) == 0) {
				found = true;
				break;
			}
		}
	}

	free(buf);
	return found;
}

static char * INIFindValue(FILE *f, const char *section, const char *property) {
	char line[256], *key, *val;

	if (section != NULL && !INIGoToSection(f, section))
		return NULL;

	while (fgets(line, sizeof(line), f) != NULL) {
		if (INI_IGNORE(line[0]))
			continue;

		if (section != NULL && line[0] == '[')
			return NULL;

		if ((key = strtok(line, "=")) == NULL)
			continue;

		if (strcasecmp(key, property) == 0) {
			val = strtok(NULL, "\n\r");

			if (val == NULL)
				return NULL;

			return strdup(val);
		}
	}

	return NULL;
}

bool INIReadInt(FILE *f, const char *section, const char *property, int *out) {
	char *value;

	rewind(f);

	if ((value = INIFindValue(f, section, property)) == NULL)
		return false;

	if (!IsNumber(value)) {
		free(value);
		return false;
	}

	*out = atoi(value);
	free(value);

	return true;
}

bool INIReadBool(FILE *f, const char *section, const char *property, bool *out) {
	char *value;

	rewind(f);

	if ((value = INIFindValue(f, section, property)) == NULL)
		return false;

	*out = (strcasecmp(value, "1") == 0 || strcasecmp(value, "true") == 0 || strcasecmp(value, "on") == 0 || strcasecmp(value, "yes") == 0);
	free(value);

	return true;

}
string GetDeviceName(string device) {
	if (device == "chemistry_table") 
		device = "Chemistry Table";
	else if (device == "work_desk")
		device = "Engraved Desk";
	else if (device == "forge") 
		device = "Forge";
	else if (device == "stove and keg")
		device = "Stove & Keg";
	else if (device == "sewing_table") 
		device = "Sewing Table & Mannequin";
	else if (device == "woodworking_table") 
		device = "Woodworking Table";
	else if (device == "work_bench") 
		device = "Work Bench";
	else if (device == "crafting_intro_anvil")
		device = "Mender's Anvil";
	return device;
}

int32 GetDeviceID(string device) {
	if (device == "Chemistry Table") 
		return 3;
	else if (device == "Engraved Desk")
		return 4;
	else if (device == "Forge") 
		return 2;
	else if (device == "Stove & Keg")
		return 7;
	else if (device == "Sewing Table & Mannequin") 
		return 1;
	else if (device == "Woodworking Table") 
		return 6;
	else if (device == "Work Bench") 
		return 5;
	else if (device == "Mender's Anvil")
		return 0xFFFFFFFF;
	return 0;
}
int16 GetItemPacketType(int32 version) {
	int16 item_version;
	if (version >= 64707)
		item_version = 0x5CFE;
	else if (version >= 63119)
		item_version = 0x56FE;
	else if (version >= 60024)
		item_version = 0x51FE;
	else if (version >= 57107)
		item_version = 0x4CFE;
	else if (version >= 57048)
		item_version = 0x48FE;
	else if (version >= 1199)
		item_version = 0x44FE;
	else if (version >= 1195)
		item_version = 0x40FE;
	else if (version >= 1193)
		item_version = 0x3FFE;
	else if (version >= 1190)
		item_version = 0x3EFE;
	else if (version >= 1188)
		item_version = 0x3DFE;
	else if (version >= 1096)
		item_version = 0x35FE;
	else if (version >= 1027)
		item_version = 0x31FE;
	else if (version >= 1008)
		item_version = 0x2CFE;
	else if (version >= 927)
		item_version = 0x23FE;
	else if (version >= 893)
		item_version = 0x22FE;
	else if (version >= 860)
		item_version = 0x20FE;
	else if (version > 546)
		item_version = 0x1CFE;
	else
		item_version = 0;

	return item_version;
}

#ifndef PATCHER
int16 GetOpcodeVersion(int16 version) {
	int16 ret = version;
	int16 version1 = 0;
	int16 version2 = 0;
	map<int16, int16>::iterator itr;
	for (itr = EQOpcodeVersions.begin(); itr != EQOpcodeVersions.end(); itr++) {
		version1 = itr->first;
		version2 = itr->second;
		if (version >= version1 && version <= version2) {
			ret = version1;
			break;
		}	
	}

	return ret;
}
#endif

void SleepMS(int32 milliseconds) {
#if defined(_WIN32)
	Sleep(milliseconds);
#else
	usleep(milliseconds * 1000);
#endif
}

size_t
strlcpy(char *dst, const char *src, size_t size) {
	char *d = dst;
	const char *s = src;
	size_t n = size;

	if (n != 0 && --n != 0) {
		do {
			if ((*d++ = *s++) == 0)
				break;
		} while (--n != 0);
	}

	if (n == 0) {
		if (size != 0)
			*d = '\0';
		while (*s++)
			;
	}

	return(s - src - 1);
}

float short_to_float(const ushort x) { // IEEE-754 16-bit floating-point format (without infinity): 1-5-10, exp-15, +-131008.0, +-6.1035156E-5, +-5.9604645E-8, 3.311 digits
	const uint32 e = (x & 0x7C00) >> 10; // exponent
	const uint32 m = (x & 0x03FF) << 13; // mantissa
	const uint32 v = as_uint((float)m) >> 23; // evil log2 bit hack to count leading zeros in denormalized format
	return as_float((x & 0x8000) << 16 | (e != 0) * ((e + 112) << 23 | m) | ((e == 0) & (m != 0)) * ((v - 37) << 23 | ((m << (150 - v)) & 0x007FE000))); // sign : normalized : denormalized
}

uint32 float_to_int(const float x) { // IEEE-754 16-bit floating-point format (without infinity): 1-5-10, exp-15, +-131008.0, +-6.1035156E-5, +-5.9604645E-8, 3.311 digits
	const uint32 b = as_uint(x) + 0x00001000; // round-to-nearest-even: add last bit after truncated mantissa
	const uint32 e = (b & 0x7F800000) >> 23; // exponent
	const uint32 m = b & 0x007FFFFF; // mantissa; in line below: 0x007FF000 = 0x00800000-0x00001000 = decimal indicator flag - initial rounding
	return (b & 0x80000000) >> 16 | (e > 112)* ((((e - 112) << 10) & 0x7C00) | m >> 13) | ((e < 113) & (e > 101))* ((((0x007FF000 + m) >> (125 - e)) + 1) >> 1) | (e > 143) * 0x7FFF; // sign : normalized : denormalized : saturate
}

uint32 as_uint(const float x) {
	return *(uint32*)&x;
}

float as_float(const uint32 x) {
	return *(float*)&x;
}

// Function to get the current timestamp in milliseconds
int64 getCurrentTimestamp() {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return duration.count();
}

std::tuple<int, int, int, int> convertTimestampDuration(int64 total_seconds) {
	std::chrono::milliseconds duration(total_seconds);
    // Convert to days, hours, minutes, and seconds
    auto days = std::chrono::duration_cast<std::chrono::duration<int, std::ratio<86400000>>>(duration);
    duration -= days;

    auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
    duration -= hours;

    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
    duration -= minutes;

    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);

    // Return the result as a tuple
    return std::make_tuple(days.count(), hours.count(), minutes.count(), seconds.count());
}