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
#ifndef _MISC_H

#define _MISC_H
#include <stdio.h>
#include <string>
#include <map>

using namespace std;

#define ITEMFIELDCOUNT 116

void Unprotect(string &s, char what);

void Protect(string &s, char what);

bool ItemParse(const char *data, int length, map<int,map<int,string> > &items, int id_pos, int name_pos, int max_field, int level=0);

int Tokenize(string s, map<int,string> & tokens, char delim='|');

void LoadItemDBFieldNames();

void encode_length(unsigned long length, char *out);
unsigned long decode_length(char *in);
unsigned long  encode(char *in, unsigned long length, char *out);
void decode(char *in, char *out);
void encode_chunk(char *in, int len, char *out);
void decode_chunk(char *in, char *out);

int Deflate(unsigned char* in_data, int in_length, unsigned char* out_data, int max_out_length);
int Inflate(unsigned char* indata, int indatalen, unsigned char* outdata, int outdatalen, bool iQuiet=true);
#ifndef WIN32
int print_stacktrace();
#endif

bool alpha_check(unsigned char val);

void dump_message_column(unsigned char *buffer, unsigned long length, string leader="", FILE *to = stdout);
string string_from_time(string pattern, time_t now=0);
string timestamp(time_t now=0);
string long2ip(unsigned long ip);
string pop_arg(string &s, string seps, bool obey_quotes);
int EQsprintf(char *buffer, const char *pattern, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5, const char *arg6, const char *arg7, const char *arg8, const char *arg9);
unsigned int GetSpellNameCrc(const char* src);
int GetItemNameCrc(string item_name);
unsigned int GetNameCrc(string name);
#endif
