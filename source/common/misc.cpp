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
	// VS6 doesn't like the length of STL generated names: disabling
	#pragma warning(disable:4786)
#endif
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <iostream>
#include <zlib.h>
#include <time.h>
#include "misc.h"
#include "types.h"
using namespace std;

#define ENC(c) (((c) & 0x3f) + ' ')
#define DEC(c)	(((c) - ' ') & 0x3f)

map<int,string> DBFieldNames;

#ifndef WIN32
#ifdef FREEBSD
int print_stacktrace()
{
	printf("Insert stack trace here...\n");
	return(0);
}
#else //!WIN32 && !FREEBSD == linux
#include <execinfo.h>
int print_stacktrace()
{
  void *ba[20];
  int n = backtrace (ba, 20);
  if (n != 0)
    {
      char **names = backtrace_symbols (ba, n);
      if (names != NULL)
        {
          int i;
          cerr <<  "called from " << (char*)names[0] << endl;
          for (i = 1; i < n; ++i)
            cerr << "            " << (char*)names[i] << endl;
          free (names);
        }
    }
	return(0);
}
#endif //!FREEBSD
#endif //!WIN32

int Deflate(unsigned char* in_data, int in_length, unsigned char* out_data, int max_out_length)
{
z_stream zstream;
int zerror;
	
	zstream.next_in   = in_data;
	zstream.avail_in  = in_length;
	zstream.zalloc    = Z_NULL;
	zstream.zfree     = Z_NULL;
	zstream.opaque    = Z_NULL;
	deflateInit(&zstream, Z_FINISH);
	zstream.next_out  = out_data;
	zstream.avail_out = max_out_length;
	zerror = deflate(&zstream, Z_FINISH);
	
	if (zerror == Z_STREAM_END)
	{
		deflateEnd(&zstream);
		return zstream.total_out;
	}
	else
	{
		cout << "Error: Deflate: deflate() returned " << zerror << " '";
		if (zstream.msg)
			cout << zstream.msg;
		cout << "'" << endl;
		zerror = deflateEnd(&zstream);
		return 0;
	}
}

int Inflate(unsigned char* indata, int indatalen, unsigned char* outdata, int outdatalen, bool iQuiet)
{
z_stream zstream;
int zerror = 0;
int i;
	
	zstream.next_in		= indata;
	zstream.avail_in	= indatalen;
	zstream.next_out	= outdata;
	zstream.avail_out	= outdatalen;
	zstream.zalloc		= Z_NULL;
	zstream.zfree		= Z_NULL;
	zstream.opaque		= Z_NULL;
	
	i = inflateInit2( &zstream, 15 ); 
	if (i != Z_OK) { 
		return 0;
	}
	
	zerror = inflate( &zstream, Z_FINISH );
	
	if(zerror == Z_STREAM_END) {
		inflateEnd( &zstream );
		return zstream.total_out;
	}
	else {
		if (!iQuiet) {
			cout << "Error: Inflate: inflate() returned " << zerror << " '";
			if (zstream.msg)
				cout << zstream.msg;
			cout << "'" << endl;
		}
		
		if (zerror == Z_DATA_ERROR || zerror == Z_ERRNO)
			return -1;

		if (zerror == Z_MEM_ERROR && zstream.msg == 0)
		{
			return 0;
		}
		
		zerror = inflateEnd( &zstream );
		return 0;
	}
}

void dump_message_column(unsigned char *buffer, unsigned long length, string leader, FILE *to)
{
unsigned long i,j;
unsigned long rows,offset=0;
	rows=(length/16)+1;
	for(i=0;i<rows;i++) {
		fprintf(to, "%s%05ld: ",leader.c_str(),i*16);
		for(j=0;j<16;j++) {
			if(j == 8)
				fprintf(to, "- ");
			if (offset+j<length)
				fprintf(to, "%02x ",*(buffer+offset+j));
			else
				fprintf(to, "   ");
		}
		fprintf(to, "| ");
		for(j=0;j<16;j++,offset++) {
			if (offset<length) {
				char c=*(buffer+offset);
				fprintf(to, "%c",isprint(c) ? c : '.');
			}
		}
		fprintf(to, "\n");
	}
}

string long2ip(unsigned long ip)
{
char temp[16];
union { unsigned long ip; struct { unsigned char a,b,c,d; } octet;} ipoctet;

	ipoctet.ip=ip;
	sprintf(temp,"%d.%d.%d.%d",ipoctet.octet.a,ipoctet.octet.b,ipoctet.octet.c,ipoctet.octet.d);

	return string(temp);
}

string string_from_time(string pattern, time_t now)
{
struct tm *now_tm;
char time_string[51];

	if (!now)
		time(&now);
	now_tm=localtime(&now);

	strftime(time_string,51,pattern.c_str(),now_tm);

	return string(time_string);
}

string timestamp(time_t now)
{
	return string_from_time("[%Y%m%d.%H%M%S] ",now);
}


string pop_arg(string &s, string seps, bool obey_quotes)
{
string ret;
unsigned long i;
bool in_quote=false;

	unsigned long length=s.length();
	for(i=0;i<length;i++) {
		char c=s[i];
		if (c=='"' && obey_quotes) {
			in_quote=!in_quote;
		}
		if (in_quote)
			continue;
		if (seps.find(c)!=0xFFFFFFFF) { 
			break;
		}
	}

	if (i==length) {
		ret=s;
		s="";
	} else {
		ret=s.substr(0,i);
		s.erase(0,i+1);
	}


	return ret;
}

int EQsprintf(char *buffer, const char *pattern, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5, const char *arg6, const char *arg7, const char *arg8, const char *arg9)
{
const char *args[9],*ptr;
char *bptr;
	args[0]=arg1;
	args[1]=arg2;
	args[2]=arg3;
	args[3]=arg4;
	args[4]=arg5;
	args[5]=arg6;
	args[6]=arg7;
	args[7]=arg8;
	args[8]=arg9;
	for(ptr=pattern,bptr=buffer;*ptr;) {
		switch (*ptr) {
			case '%':
				ptr++;
				switch (*ptr) {
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						strcpy(bptr,args[*ptr-'0'-1]);
						bptr+=strlen(args[*ptr-'0'-1]);
						break;
				}
				break;
			default:
				*bptr=*ptr;
				bptr++;
		}
		ptr++;
	}

	*bptr=0;
	return (bptr-buffer);
}

bool alpha_check(unsigned char val){
	if((val >= 0x41 && val <=0x5A) || (val >= 0x61 && val <=0x7A))
		return true;
	else
		return false;
}

unsigned int GetSpellNameCrc(const char* src) {
	if (!src)
		return 0;
	uLong crc = crc32(0L, Z_NULL, 0);
	return crc32(crc, (unsigned const char*)src, strlen(src));
}

int GetItemNameCrc(string item_name){
	const char *src = item_name.c_str();
	uLong crc = crc32(0L, Z_NULL, 0);    
    crc = crc32(crc, (unsigned const char *)src,strlen(src)) + 1;
	return sint32(crc) * -1;
}

unsigned int GetNameCrc(string name) {
	const char* src = name.c_str();
	uLong crc = crc32(0L, Z_NULL, 0);
	crc = crc32(crc, (unsigned const char*)src, strlen(src)) + 1;
	return int32(crc)-1;
}
