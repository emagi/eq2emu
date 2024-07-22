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
#ifndef __EQ2_DATABUFFER_
#define __EQ2_DATABUFFER_
#include <string>
#include "../common/types.h"
#include "../common/EQPacket.h"
#include "../common/EQ2_Common_Structs.h"

#ifdef WORLD
	#include "../WorldServer/SpawnLists.h"
#endif

using namespace std;

class DataBuffer{
public:
	bool changed;
	uchar*	getData(){ return (uchar*)buffer.c_str(); }
	int32	getDataSize(){ return buffer.length(); }
	string*	getDataString(){ return &buffer; }
	void	CreateEQ2Color(EQ2_Color& color){
		CreateEQ2Color(&color);
	}
	uchar* GetLoadBuffer(){
		return load_buffer;
	}
	int32 GetLoadPos(){
		return load_pos;
	}
	int32 GetLoadLen(){
		return load_len;
	}
	void SetLoadPos(int32 new_pos){
		load_pos = new_pos;
	}
	void	CreateEQ2Color(EQ2_Color* color){
		int8 rgb[3];
		float* tmp = 0;
		for(int i=0;i<3;i++){
			tmp = (float*)(load_buffer + load_pos);
			rgb[i] = (int8)((*tmp)*255);
			load_pos += sizeof(float);
		}
		color->red = rgb[0];
		color->green = rgb[1];
		color->blue = rgb[2];
	}
	
	template<class Type> void MakeEQ2_Int8(Type& output){
		MakeEQ2_Int8(&output);
	}
	template<class Type> void MakeEQ2_Int8(Type* output){
		float* tmp = (float*)(load_buffer + load_pos);
		if(*tmp < 0)
			*tmp *= -1;
		sint8 result = (sint8)((*tmp)*100);
		memcpy(output, &result, sizeof(sint8));
		load_pos += sizeof(float);
	}
	void	InitializeGetData(){
		get_buffer = (uchar*)buffer.c_str();
		get_len = buffer.length();
		get_pos = 0;
	}
	void	InitializeLoadData(uchar* input, int32 size){
		buffer = string((char*)input, size);
		load_buffer = (uchar*)buffer.c_str();
		load_len = size;
		load_pos = 0;
	}
	template<class String> void LoadDataString(String& output){
		LoadDataString(&output);
	}
	template<class String> void LoadDataString(String* output){
		if((sizeof(output->size) + load_pos) <= load_len){
			memcpy(&output->size, load_buffer + load_pos, sizeof(output->size));
			load_pos += sizeof(output->size);
		}
		if((output->size + load_pos) <= load_len){
			output->data = string((char*)(load_buffer + load_pos), output->size);
			load_pos += output->size;
		}
	}
	template<class Type> void LoadData(Type& output){
		LoadData(&output);
	}
	template<class Type> void LoadData(Type* output, int32 array_size){
		if(array_size<=1){
			LoadData(output);
		}
		else{
			for(int32 i=0;i<array_size;i++)
				LoadData(&output[i]);
		}
	}
	template<class Type> void LoadData(Type* output){
		if((sizeof(Type) + load_pos) <= load_len){
			memcpy(output, load_buffer + load_pos, sizeof(Type));
			load_pos += sizeof(Type);
		}
	}
	template<class Type> void LoadData(Type& output, int32 array_size){
		LoadData(&output, array_size);
	}
	void LoadSkip(int8 bytes){
		load_pos += bytes;
	}
	template<class Type> void LoadSkip(Type& skip){
		LoadSkip(&skip);
	}
	template<class Type> void LoadSkip(Type* skip){
		load_pos += sizeof(Type);
	}
	template<class Type> void GetData(Type* output){
		if((sizeof(Type) + get_pos) <= get_len){
			*output = (Type*)get_buffer;
			get_pos += sizeof(output);
		}
	}
	void AddZeros(int16 num){
		int8* data = new int8[num];
		memset(data, 0, num);
		AddData(*data);
		safe_delete_array(data);
	}
	template<class Type> void StructAddData(Type input, int16 size, string* datastring){
		if(datastring)
			datastring->append((char*)&input, size);
		else
			buffer.append((char*)&input, size);
	}
	template<class Type> void StructAddData(Type input, int32 array_size, int16 size, string* datastring){
		if(array_size>0){
			for(int32 i=0;i<array_size;i++)
				StructAddData(input[i], size, datastring);
		}
		else
			StructAddData(input, size, datastring);
	}
	template<class Type> void AddData(Type input, string* datastring = 0){
		if(!datastring)
			datastring = &buffer;
		datastring->append((char*)&input, sizeof(input));
	}
	template<class Type> void AddData(Type input, int32 array_size, string* datastring = 0){
		if(array_size>0){
			for(int32 i=0;i<array_size;i++)
				AddData(input[i], datastring);
		}
		else
			AddData(input, datastring);
	}
	template<class String> void AddDataString(String* input, string* datastring = 0){
		AddDataString(*input, datastring);
	}
	template<class String> void AddDataString(String input, string* datastring = 0){
		input.size = input.data.length();
		if(!datastring)
			datastring = &buffer;
		datastring->append((char*)&input.size, sizeof(input.size));
		datastring->append(input.data);
	}
	void	AddCharArray(char* array, string* datastring = 0){
		if(!datastring)
			datastring = &buffer;
		datastring->append(array);
	}
	void	AddCharArray(char* array, int16 size, string* datastring = 0){
		if(!datastring)
			datastring = &buffer;
		datastring->append(array, size);
	}
	void	AddData(string data, string* datastring = 0){
		if(!datastring)
			datastring = &buffer;
		datastring->append(data);
	}
	void	Clear() { buffer.clear(); }
private:
	string	buffer;
	uchar*	get_buffer;
	uchar*	load_buffer;
	int32	get_len;
	int32	get_pos;
	int32	load_len;
	int32	load_pos;
};
#endif

