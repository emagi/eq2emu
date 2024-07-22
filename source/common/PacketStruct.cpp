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
#include <iostream>
#include <exception>
#include "PacketStruct.h"
#include "ConfigReader.h"
#include "../common/debug.h"
#include "MiscFunctions.h"
#include "Log.h"

#ifdef WORLD
#include "../WorldServer/Items/Items.h"
#include "../WorldServer/Player.h"
#include "../WorldServer/World.h"
#endif

extern ConfigReader configReader;
using namespace std;

DataStruct::DataStruct() {
	item_size = 0;
	type = 0;
	type2 = 0;
	length = 1;
	if_flag_set = false;
	if_flag_not_set = false;
	if_set = false;
	if_not_set = false;
	if_not_equals = false;
	if_equals = false;
	is_set = false;
	optional = false;
	oversized = 0;
	oversized_byte = 0;
	add = false;
	addType = 0;
	maxArraySize = 0;
	default_value = 0;
}
DataStruct::DataStruct(DataStruct* data_struct) {
	type = data_struct->GetType();
	type2 = data_struct->GetType2();
	length = data_struct->GetLength();
	name = data_struct->GetName();
	array_size_variable = data_struct->array_size_variable;
	default_value = data_struct->default_value;
	add = true;
	addType = type;
	oversized = data_struct->GetOversized();
	oversized_byte = data_struct->GetOversizedByte();
	maxArraySize = data_struct->GetMaxArraySize();
	if_flag_set = false;
	if_flag_not_set = false;
	if_set = false;
	if_not_set = false;
	if_not_equals = false;
	if_equals = false;
	optional = false;
	if (data_struct->GetIfSet())
		SetIfSetVariable(data_struct->GetIfSetVariable());
	if (data_struct->GetIfNotSet())
		SetIfNotSetVariable(data_struct->GetIfNotSetVariable());
	if (data_struct->GetIfNotEquals())
		SetIfNotEqualsVariable(data_struct->GetIfNotEqualsVariable());
	if (data_struct->GetIfEquals())
		SetIfEqualsVariable(data_struct->GetIfEqualsVariable());
	if (data_struct->GetIfFlagSet())
		SetIfFlagSetVariable(data_struct->GetIfFlagSetVariable());
	if (data_struct->GetIfFlagNotSet())
		SetIfFlagNotSetVariable(data_struct->GetIfFlagNotSetVariable());
	item_size = 0;
	is_set = false;
}
DataStruct::DataStruct(const char* new_name, const char* new_type, int32 new_length, const char* new_type2) {
	name = string(new_name);
	type = 0;
	type2 = 0;
	SetType(new_type, &type);
	if (new_type2)
		SetType(new_type2, &type2);
	length = new_length;
	add = true;
	addType = type;
	if_set = false;
	if_not_set = false;
	is_set = false;
	if_not_equals = false;
	item_size = 0;
}
const char* DataStruct::GetArraySizeVariable() {
	return array_size_variable.c_str();
}
void DataStruct::SetArraySizeVariable(const char* new_name) {
	array_size_variable = string(new_name);
}
void DataStruct::SetType(const char* new_type, int8* output_type) {
	if (strlen(new_type) > 3 && strncasecmp("int", new_type, 3) == 0) {
		if (strncasecmp("int8", new_type, 4) == 0)
			*output_type = DATA_STRUCT_INT8;
		else if (strncasecmp("int16", new_type, 5) == 0)
			*output_type = DATA_STRUCT_INT16;
		else if (strncasecmp("int32", new_type, 5) == 0)
			*output_type = DATA_STRUCT_INT32;
		else if (strncasecmp("int64", new_type, 5) == 0)
			*output_type = DATA_STRUCT_INT64;
	}
	else if (strlen(new_type) > 4 && strncasecmp("sint", new_type, 4) == 0) {
		if (strncasecmp("sint8", new_type, 5) == 0)
			*output_type = DATA_STRUCT_SINT8;
		else if (strncasecmp("sint16", new_type, 6) == 0)
			*output_type = DATA_STRUCT_SINT16;
		else if (strncasecmp("sint32", new_type, 6) == 0)
			*output_type = DATA_STRUCT_SINT32;
		else if (strncasecmp("sint64", new_type, 6) == 0)
			*output_type = DATA_STRUCT_SINT64;
	}
	else if (strlen(new_type) == 4 && strncasecmp("char", new_type, 4) == 0)
		*output_type = DATA_STRUCT_CHAR;
	else if (strlen(new_type) == 5 && strncasecmp("float", new_type, 5) == 0)
		*output_type = DATA_STRUCT_FLOAT;
	else if (strlen(new_type) == 6 && strncasecmp("double", new_type, 6) == 0)
		*output_type = DATA_STRUCT_DOUBLE;
	else if (strlen(new_type) >= 5 && strncasecmp("EQ2_", new_type, 4) == 0) {
		if (strncasecmp("EQ2_8", new_type, 5) == 0)
			*output_type = DATA_STRUCT_EQ2_8BIT_STRING;
		else if (strncasecmp("EQ2_16", new_type, 6) == 0)
			*output_type = DATA_STRUCT_EQ2_16BIT_STRING;
		else if (strncasecmp("EQ2_32", new_type, 6) == 0)
			*output_type = DATA_STRUCT_EQ2_32BIT_STRING;
		else if (strncasecmp("EQ2_E", new_type, 5) == 0)
			*output_type = DATA_STRUCT_EQUIPMENT;
		else if (strncasecmp("EQ2_C", new_type, 5) == 0)
			*output_type = DATA_STRUCT_COLOR;
		else if (strncasecmp("EQ2_I", new_type, 5) == 0)
			*output_type = DATA_STRUCT_ITEM;
	}
	else if (strlen(new_type) >= 5) {
		if (strncasecmp("Array", new_type, 5) == 0)
			*output_type = DATA_STRUCT_ARRAY;
	}
	else
		LogWrite(PACKET__ERROR, 0, "Packet", "Invalid Type: %s", new_type);
}
DataStruct::DataStruct(const char* new_name, int32 new_length) {
	name = string(new_name);
	length = new_length;
	if_set = false;
	if_not_set = false;
	is_set = false;
	item_size = 0;
}
DataStruct::DataStruct(const char* new_name, int8 new_type, int32 new_length, int8 new_type2) {
	name = string(new_name);
	type = new_type;
	length = new_length;
	type2 = new_type2;
	addType = type;
	if_set = false;
	if_not_set = false;
	is_set = false;
	item_size = 0;
}
void DataStruct::SetType(int8 new_type) {
	type = new_type;
	addType = type;
}
void DataStruct::SetMaxArraySize(int8 size) {
	maxArraySize = size;
}
void DataStruct::SetOversized(int8 val) {
	oversized = val;
}
void DataStruct::SetDefaultValue(int8 new_val) {
	default_value = new_val;
}
void DataStruct::SetName(const char* new_name) {
	name = string(new_name);
}
void DataStruct::SetLength(int32 new_length) {
	length = new_length;
}
void DataStruct::SetOversizedByte(int8 val) {
	oversized_byte = val;
}
void DataStruct::SetItemSize(int32 val) {
	item_size = val;
	if(item_size)
		is_set = true;
	else
		is_set = false;
}
void DataStruct::SetIfEqualsVariable(const char* variable) {
	if (variable) {
		if_equals = true;
		if_equals_variable = string(variable);
	}
	else
		if_equals = false;
}
void DataStruct::SetIfNotEqualsVariable(const char* variable) {
	if (variable) {
		if_not_equals = true;
		if_not_equals_variable = string(variable);
	}
	else
		if_not_equals = false;
}
void DataStruct::SetIfFlagNotSetVariable(const char* variable) {
	if (variable) {
		if_flag_not_set = true;
		if_flag_not_set_variable = string(variable);
	}
	else
		if_flag_not_set = false;
}
void DataStruct::SetIfFlagSetVariable(const char* variable) {
	if (variable) {
		if_flag_set = true;
		if_flag_set_variable = string(variable);
	}
	else
		if_flag_set = false;
}
void DataStruct::SetIfSetVariable(const char* variable) {
	if (variable) {
		if_set = true;
		if_set_variable = string(variable);
	}
	else
		if_set = false;
}
void DataStruct::SetIfNotSetVariable(const char* variable) {
	if (variable) {
		if_not_set = true;
		if_not_set_variable = string(variable);
	}
	else
		if_not_set = false;
}
void DataStruct::SetIsSet(bool val) {
	is_set = val;
}
bool DataStruct::IsSet() {
	return is_set;
}
void DataStruct::SetIsOptional(bool val) {
	optional = val;
}
bool DataStruct::IsOptional() {
	return optional;
}
int32 DataStruct::GetItemSize() {
	return item_size;
}
bool DataStruct::GetIfSet() {
	return if_set;
}
const char* DataStruct::GetIfSetVariable() {
	if (if_set_variable.length() > 0)
		return if_set_variable.c_str();
	return 0;
}
bool DataStruct::GetIfNotSet() {
	return if_not_set;
}
const char* DataStruct::GetIfNotSetVariable() {
	if (if_not_set_variable.length() > 0)
		return if_not_set_variable.c_str();
	return 0;
}
bool DataStruct::GetIfEquals() {
	return if_equals;
}
const char* DataStruct::GetIfEqualsVariable() {
	if (if_equals_variable.length() > 0)
		return if_equals_variable.c_str();
	return 0;
}
bool DataStruct::GetIfNotEquals() {
	return if_not_equals;
}
const char* DataStruct::GetIfNotEqualsVariable() {
	if (if_not_equals_variable.length() > 0)
		return if_not_equals_variable.c_str();
	return 0;
}
bool DataStruct::GetIfFlagSet() {
	return if_flag_set;
}
const char* DataStruct::GetIfFlagSetVariable() {
	if (if_flag_set_variable.length() > 0)
		return if_flag_set_variable.c_str();
	return 0;
}
bool DataStruct::GetIfFlagNotSet() {
	return if_flag_not_set;
}
const char* DataStruct::GetIfFlagNotSetVariable() {
	if (if_flag_not_set_variable.length() > 0)
		return if_flag_not_set_variable.c_str();
	return 0;
}
int8 DataStruct::GetDefaultValue() {
	return default_value;
}
int8 DataStruct::GetType() {
	return type;
}
int8 DataStruct::GetType2() {
	return type2;
}
const char* DataStruct::GetName() {
	return name.c_str();
}
int8 DataStruct::GetOversized() {
	return oversized;
}
int8 DataStruct::GetOversizedByte() {
	return oversized_byte;
}
int8 DataStruct::GetMaxArraySize() {
	return maxArraySize;
}
int32 DataStruct::GetLength() {
	return length;
}
string DataStruct::GetStringName() {
	return name;
}
bool DataStruct::AddToStruct() {
	return add;
}
void DataStruct::SetAddToStruct(bool val) {
	add = val;
}
int8 DataStruct::GetAddType() {
	return addType;
}
void DataStruct::SetAddType(int8 new_type) {
	addType = new_type;
}
string DataStruct::AppendVariable(string orig, const char* val) {
	if (!val)
		return orig;
	if(orig.length() == 0)
		return string(val);
	if (orig.find(",") < 0xFFFFFFFF) { //has more than one already
		string valstr = string(val);
		vector<string>* varnames = SplitString(orig, ',');
		if (varnames) {
			for (int32 i = 0; i < varnames->size(); i++) {
				if (valstr.compare(varnames->at(i)) == 0) {
					return orig; //already in the variable, no need to append
				}
			}
			safe_delete(varnames);
		}		
	}
	return orig.append(",").append(val);
}
int32 DataStruct::GetDataSizeInBytes() {
	int32 ret = 0;
	switch (type) {
	case DATA_STRUCT_INT8:
		ret = length * sizeof(int8);
		break;
	case DATA_STRUCT_INT16:
		ret = length * sizeof(int16);
		break;
	case DATA_STRUCT_INT32:
		ret = length * sizeof(int32);
		break;
	case DATA_STRUCT_INT64:
		ret = length * sizeof(int64);
		break;
	case DATA_STRUCT_SINT8:
		ret = length * sizeof(sint8);
		break;
	case DATA_STRUCT_SINT16:
		ret = length * sizeof(sint16);
		break;
	case DATA_STRUCT_SINT32:
		ret = length * sizeof(sint32);
		break;
	case DATA_STRUCT_SINT64:
		ret = length * sizeof(sint64);
		break;
	case DATA_STRUCT_FLOAT:
		ret = length * sizeof(float);
		break;
	case DATA_STRUCT_DOUBLE:
		ret = length * sizeof(double);
		break;
	case DATA_STRUCT_ARRAY:
		// Array elements won't have a size so get out now to avoid the warning.
		break;
	default:
		LogWrite(PACKET__WARNING, 0, "DataStruct", "Tried retrieving a data size from an unsupported data struct type in GetDataSizeInBytes()");
		break;
	}
	return ret;
}

PacketStruct::PacketStruct(PacketStruct* packet, int16 in_client_version) {
	parent = packet->parent;
	client_version = in_client_version;
	vector<DataStruct*>::iterator itr2;
	name = packet->name;

	for (itr2 = packet->structs.begin(); itr2 != packet->structs.end(); itr2++) {
		add(new DataStruct(*itr2));
	}
	vector<string>::iterator itr;
	for (itr = packet->flags.begin(); itr != packet->flags.end(); itr++) {
		AddFlag((*itr).c_str());
	}
	sub_packet = false;
	opcode = packet->opcode;
	version = packet->version;
	opcode_type = packet->opcode_type;
	sub_packet_size = 1;


	addPacketArrays(packet);
}

PacketStruct::PacketStruct() {
	parent = 0;
	opcode = OP_Unknown;
	opcode_type = string("");
}

PacketStruct::PacketStruct(PacketStruct* packet, bool sub) {
	vector<DataStruct*>::iterator itr2;

	for (itr2 = packet->structs.begin(); itr2 != packet->structs.end(); itr2++) {
		add(new DataStruct(*itr2));
	}
	vector<string>::iterator itr;
	for (itr = packet->flags.begin(); itr != packet->flags.end(); itr++) {
		AddFlag((*itr).c_str());
	}
	sub_packet = sub;
	opcode = packet->opcode;
	version = packet->version;
	opcode_type = packet->opcode_type;
	name = packet->name;
	sub_packet_size = 0;
	parent = 0;
}
PacketStruct::~PacketStruct() {
	deleteDataStructs(&structs);
	deleteDataStructs(&orig_structs);
	deletePacketArrays(this);
	struct_map.clear();
	struct_data.clear();
	flags.clear();
}

void PacketStruct::deleteDataStructs(vector<DataStruct*>* data_structs) {
	if ( !data_structs || data_structs->size() == 0 )
	return;

	DataStruct* data = 0;
	vector<DataStruct*>::iterator itr;
	for (itr = data_structs->begin(); itr != data_structs->end(); itr++) {
		data = *itr;
		void* ptr = GetStructPointer(data);

		// stop the struct_data from growing with old data/ptr info, memory leaking and eventual buffer overflow (crash)
		map<DataStruct*, void*>::iterator datastr = struct_data.find(data);
		if (datastr != struct_data.end())
			struct_data.erase(datastr);

		switch (data->GetType()) {
		case DATA_STRUCT_EQ2_8BIT_STRING: {
			EQ2_8BitString* real_ptr = (EQ2_8BitString*)ptr;
			safe_delete(real_ptr);
			break;
		}
		case DATA_STRUCT_EQ2_16BIT_STRING: {
			EQ2_16BitString* real_ptr = (EQ2_16BitString*)ptr;
			safe_delete(real_ptr);
			break;
		}
		case DATA_STRUCT_EQ2_32BIT_STRING: {
			EQ2_32BitString* real_ptr = (EQ2_32BitString*)ptr;
			safe_delete(real_ptr);
			break;
		}
		case DATA_STRUCT_EQUIPMENT: {
			EQ2_EquipmentItem* real_ptr = (EQ2_EquipmentItem*)ptr;
			safe_delete_array(real_ptr);
			break;
		}
		case DATA_STRUCT_DOUBLE: {
			double* real_ptr = (double*)ptr;
			safe_delete_array(real_ptr);
			break;
		}
		case DATA_STRUCT_FLOAT: {
			float* real_ptr = (float*)ptr;
			safe_delete_array(real_ptr);
			break;
		}
		case DATA_STRUCT_INT8: {
			int8* real_ptr = (int8*)ptr;
			safe_delete_array(real_ptr);
			break;
		}
		case DATA_STRUCT_INT16: {
			int16* real_ptr = (int16*)ptr;
			safe_delete_array(real_ptr);
			break;
		}
		case DATA_STRUCT_INT32: {
			int32* real_ptr = (int32*)ptr;
			safe_delete_array(real_ptr);
			break;
		}
		case DATA_STRUCT_INT64: {
			int64* real_ptr = (int64*)ptr;
			safe_delete_array(real_ptr);
			break;
		}
		case DATA_STRUCT_SINT8: {
			sint8* real_ptr = (sint8*)ptr;
			safe_delete_array(real_ptr);
			break;
		}
		case DATA_STRUCT_SINT16: {
			sint16* real_ptr = (sint16*)ptr;
			safe_delete_array(real_ptr);
			break;
		}
		case DATA_STRUCT_SINT32: {
			sint32* real_ptr = (sint32*)ptr;
			safe_delete_array(real_ptr);
			break;
		}
		case DATA_STRUCT_SINT64: {
			sint64* real_ptr = (sint64*)ptr;
			safe_delete_array(real_ptr);
			break;
		}
		case DATA_STRUCT_ITEM: {
			uchar* real_ptr = (uchar*)ptr;
			safe_delete_array(real_ptr);
			break;
		}
		case DATA_STRUCT_CHAR: {
			char* real_ptr = (char*)ptr;
			safe_delete_array(real_ptr);
			break;
		}
		case DATA_STRUCT_COLOR: {
			EQ2_Color* real_ptr = (EQ2_Color*)ptr;
			safe_delete_array(real_ptr);
			break;
		}
		}
		ptr = 0;
		safe_delete(data);
	}
}
void PacketStruct::deletePacketArrays(PacketStruct* packet) {
	if (!packet)
		return;
	vector<PacketStruct*>::iterator itr;

	for (itr = packet->arrays.begin(); itr != packet->arrays.end(); itr++)
		safe_delete(*itr);
	packet->arrays.clear();

	for (itr = packet->orig_packets.begin(); itr != packet->orig_packets.end(); itr++)
		safe_delete(*itr);
	packet->orig_packets.clear();
}

void PacketStruct::renameSubstructArray(const char* substruct, int32 index) {
	vector<PacketStruct*>::iterator itr;
	char tmp[10] = { 0 };
	sprintf(tmp, "%i", index);
	for (itr = arrays.begin(); itr != arrays.end(); itr++) {
		(*itr)->SetName(string(substruct).append("_").append((*itr)->GetName()).append("_").append(tmp).c_str());
	}
}

void PacketStruct::addPacketArrays(PacketStruct* packet) {
	if (!packet)
		return;
	vector<PacketStruct*>::iterator itr;

	for (itr = packet->arrays.begin(); itr != packet->arrays.end(); itr++) {
		PacketStruct* tmp = new PacketStruct(*itr, true);
		tmp->addPacketArrays(*itr);
		add(tmp);
	}
}

bool PacketStruct::IsStringValueType(string in_name, int32 index) {
	DataStruct* data = findStruct(in_name.c_str(), index);
	switch (data->GetType()) {
	case DATA_STRUCT_CHAR:
	case DATA_STRUCT_EQ2_8BIT_STRING:
	case DATA_STRUCT_EQ2_16BIT_STRING:
	case DATA_STRUCT_EQ2_32BIT_STRING:
		return true;
	}
	return false;
}

bool PacketStruct::IsColorValueType(string in_name, int32 index) {
	DataStruct* data = findStruct(in_name.c_str(), index);
	if (data->GetType() == DATA_STRUCT_COLOR)
		return true;
	else
		return false;
}
void PacketStruct::setColor(DataStruct* data, int8 red, int8 green, int8 blue, int32 index = 0) {
	if (data && data->GetType() == DATA_STRUCT_COLOR) {
		EQ2_Color* color = (EQ2_Color*)GetStructPointer(data);
		color[index].blue = blue;
		color[index].red = red;
		color[index].green = green;
		if (blue > 0 || green > 0 || red > 0)
			data->SetIsSet(true);
	}
}
void PacketStruct::setEquipment(DataStruct* data, int16 type, int8 c_red, int8 c_blue, int8 c_green, int8 h_red, int8 h_blue, int8 h_green, int32 index) {
	if (data) {
		EQ2_EquipmentItem* equipment = (EQ2_EquipmentItem*)GetStructPointer(data);
		EQ2_Color* color = (EQ2_Color*)&equipment[index].color;
		EQ2_Color* highlight = (EQ2_Color*)&equipment[index].highlight;
		equipment[index].type = type;
		color->blue = c_blue;
		color->red = c_red;
		color->green = c_green;
		highlight->blue = h_blue;
		highlight->red = h_red;
		highlight->green = h_green;
		if (c_red > 0 || c_blue > 0 || c_green > 0 || h_red > 0 || h_blue > 0 || h_green > 0)
			data->SetIsSet(true);
	}
}
void PacketStruct::add(PacketStruct* packet_struct) {
	packet_struct->parent = this;
	arrays.push_back(packet_struct);
}
const char* PacketStruct::GetOpcodeType() {
	return opcode_type.c_str();
}

void PacketStruct::SetOpcodeType(const char* in_type) {
	if (in_type)
		opcode_type = string(in_type);
	else
		opcode_type = string("");
}

void PacketStruct::setDataType(DataStruct* data_struct, sint8 data, int32 index) {
	if (data_struct) {
		sint8* ptr = (sint8*)GetStructPointer(data_struct);
		ptr[index] = data;
		if (data != 0 && data != -1)
			data_struct->SetIsSet(true);
	}
}
void PacketStruct::setDataType(DataStruct* data_struct, sint16 data, int32 index) {
	if (data_struct) {
		sint16* ptr = (sint16*)GetStructPointer(data_struct);
		ptr[index] = data;
		if (data != 0 && data != -1)
			data_struct->SetIsSet(true);
	}
}
void PacketStruct::setDataType(DataStruct* data_struct, sint32 data, int32 index) {
	if (data_struct) {
		sint32* ptr = (sint32*)GetStructPointer(data_struct);
		ptr[index] = data;
		if (data != 0 && data != -1)
			data_struct->SetIsSet(true);
	}
}
void PacketStruct::setDataType(DataStruct* data_struct, sint64 data, int32 index) {
	if (data_struct) {
		sint64* ptr = (sint64*)GetStructPointer(data_struct);
		ptr[index] = data;
		if (data > 0)
			data_struct->SetIsSet(true);
	}
}
void PacketStruct::setDataType(DataStruct* data_struct, char data, int32 index) {
	if (data_struct) {
		char* ptr = (char*)GetStructPointer(data_struct);
		ptr[index] = data;
		if (data > 0)
			data_struct->SetIsSet(true);
	}
}
void PacketStruct::setDataType(DataStruct* data_struct, int8 data, int32 index) {
	if (data_struct) {
		int8* ptr = (int8*)GetStructPointer(data_struct);
		ptr[index] = data;
		if (data > 0)
			data_struct->SetIsSet(true);
	}
}
void PacketStruct::setDataType(DataStruct* data_struct, int16 data, int32 index) {
	if (data_struct) {
		int16* ptr = (int16*)GetStructPointer(data_struct);
		ptr[index] = data;
		if (data > 0)
			data_struct->SetIsSet(true);
	}
}
void PacketStruct::setDataType(DataStruct* data_struct, int32 data, int32 index) {
	if (data_struct) {
		int32* ptr = (int32*)GetStructPointer(data_struct);
		ptr[index] = data;
		if (data > 0)
			data_struct->SetIsSet(true);
	}
}
void PacketStruct::setDataType(DataStruct* data_struct, int64 data, int32 index) {
	if (data_struct) {
		int64* ptr = (int64*)GetStructPointer(data_struct);
		ptr[index] = data;
		if (data > 0)
			data_struct->SetIsSet(true);
	}
}
void PacketStruct::setDataType(DataStruct* data_struct, float data, int32 index) {
	if (data_struct) {
		float* ptr = (float*)GetStructPointer(data_struct);
		ptr[index] = data;
		if (data > 0)
			data_struct->SetIsSet(true);
	}
}
void PacketStruct::setDataType(DataStruct* data_struct, double data, int32 index) {
	if (data_struct) {
		double* ptr = (double*)GetStructPointer(data_struct);
		ptr[index] = data;
		if (data > 0)
			data_struct->SetIsSet(true);
	}
}
void PacketStruct::setData(DataStruct* data_struct, EQ2_8BitString* input_string, int32 index, bool use_second_type) {
	if (data_struct) {
		EQ2_8BitString* tmp = (EQ2_8BitString*)GetStructPointer(data_struct);
		tmp->data = input_string->data;
		tmp->size = input_string->data.length();
		if (input_string->data.length() > 0)
			data_struct->SetIsSet(true);
	}
}
void PacketStruct::setData(DataStruct* data_struct, EQ2_16BitString* input_string, int32 index, bool use_second_type) {
	if (data_struct) {
		EQ2_16BitString* tmp = (EQ2_16BitString*)GetStructPointer(data_struct);
		tmp->data = input_string->data;
		tmp->size = input_string->data.length();
		if (input_string->data.length() > 0)
			data_struct->SetIsSet(true);
	}
}
void PacketStruct::setData(DataStruct* data_struct, EQ2_32BitString* input_string, int32 index, bool use_second_type) {
	if (data_struct) {
		EQ2_32BitString* tmp = (EQ2_32BitString*)GetStructPointer(data_struct);
		tmp->data = input_string->data;
		tmp->size = input_string->data.length();
		if (input_string->data.length() > 0)
			data_struct->SetIsSet(true);
	}
}
void PacketStruct::add(DataStruct* data) {
	structs.push_back(data);
	struct_map[data->GetStringName()] = data;
	switch (data->GetType()) {
	case DATA_STRUCT_INT8: {
		struct_data[data] = new int8[data->GetLength()];
		int8* ptr = (int8*)GetStructPointer(data);
		if (data->GetLength() > 1) {
			int8 default_val = data->GetDefaultValue();
			memset(ptr, default_val, data->GetLength() * sizeof(int8));
		}
		else
			ptr[0] = 0;
		break;
	}
	case DATA_STRUCT_INT16: {
		struct_data[data] = new int16[data->GetLength()];
		int16* ptr = (int16*)GetStructPointer(data);
		if (data->GetLength() > 1) {
			int8 default_val = data->GetDefaultValue();
			memset(ptr, default_val, data->GetLength() * sizeof(int16));
		}
		else
			ptr[0] = 0;
		break;
	}
	case DATA_STRUCT_INT32: {
		struct_data[data] = new int32[data->GetLength()];
		int32* ptr = (int32*)GetStructPointer(data);
		if (data->GetLength() > 1) {
			int8 default_val = data->GetDefaultValue();
			memset(ptr, default_val, data->GetLength() * sizeof(int32));
		}
		else
			ptr[0] = 0;
		break;
	}
	case DATA_STRUCT_INT64: {
		struct_data[data] = new int64[data->GetLength()];
		int64* ptr = (int64*)GetStructPointer(data);
		if (data->GetLength() > 1) {
			int8 default_val = data->GetDefaultValue();
			memset(ptr, default_val, data->GetLength() * sizeof(int64));
		}
		else
			ptr[0] = 0;
		break;
	}
	case DATA_STRUCT_SINT8: {
		struct_data[data] = new sint8[data->GetLength()];
		sint8* ptr = (sint8*)GetStructPointer(data);
		if (data->GetLength() > 1)
			memset(ptr, 0, data->GetLength() * sizeof(sint8));
		else
			ptr[0] = 0;
		break;
	}
	case DATA_STRUCT_SINT16: {
		struct_data[data] = new sint16[data->GetLength()];
		sint16* ptr = (sint16*)GetStructPointer(data);
		if (data->GetLength() > 1)
			memset(ptr, 0, data->GetLength() * sizeof(sint16));
		else
			ptr[0] = 0;
		break;
	}
	case DATA_STRUCT_SINT32: {
		struct_data[data] = new sint32[data->GetLength()];
		sint32* ptr = (sint32*)GetStructPointer(data);
		if (data->GetLength() > 1)
			memset(ptr, 0, data->GetLength() * sizeof(sint32));
		else
			ptr[0] = 0;
		break;
	}
	case DATA_STRUCT_SINT64: {
		struct_data[data] = new sint64[data->GetLength()];
		sint64* ptr = (sint64*)GetStructPointer(data);
		if (data->GetLength() > 1)
			memset(ptr, 0, data->GetLength() * sizeof(sint64));
		else
			ptr[0] = 0;
		break;
	}
	case DATA_STRUCT_CHAR: {
		struct_data[data] = new char[data->GetLength()];
		char* ptr = (char*)GetStructPointer(data);
		if (data->GetLength() > 1)
			memset(ptr, 0, data->GetLength() * sizeof(char));
		else
			ptr[0] = 0;
		break;
	}
	case DATA_STRUCT_FLOAT: {
		struct_data[data] = new float[data->GetLength()];
		float* ptr = (float*)GetStructPointer(data);
		if (data->GetLength() > 1)
			memset(ptr, 0, data->GetLength() * sizeof(float));
		else
			ptr[0] = 0;
		break;
	}
	case DATA_STRUCT_DOUBLE: {
		struct_data[data] = new double[data->GetLength()];
		double* ptr = (double*)GetStructPointer(data);
		if (data->GetLength() > 1)
			memset(ptr, 0, data->GetLength() * sizeof(double));
		else
			ptr[0] = 0;
		break;
	}
	case DATA_STRUCT_ARRAY: {
		data->SetLength(0);
		break;
	}
	case DATA_STRUCT_EQ2_8BIT_STRING: {
		string name2 = data->GetStringName();
		for (int32 i = 1; i < data->GetLength(); i++) {
			DataStruct* new_data = new DataStruct(data);
			char blah[10] = { 0 };
			sprintf(blah, "%i", i);
			name2.append("_").append(blah);
			new_data->SetName(name2.c_str());
			new_data->SetLength(1);
			EQ2_8BitString* tmp = new EQ2_8BitString;
			tmp->size = 0;
			struct_data[new_data] = tmp;
			structs.push_back(new_data);
		}
		data->SetLength(1);
		EQ2_8BitString* tmp = new EQ2_8BitString;
		tmp->size = 0;
		struct_data[data] = tmp;
		break;
	}
	case DATA_STRUCT_EQ2_16BIT_STRING: {
		string name2 = data->GetStringName();
		for (int32 i = 1; i < data->GetLength(); i++) {
			DataStruct* new_data = new DataStruct(data);
			char blah[10] = { 0 };
			sprintf(blah, "%i", i);
			name2.append("_").append(blah);
			new_data->SetName(name2.c_str());
			new_data->SetLength(1);
			EQ2_16BitString* tmp = new EQ2_16BitString;
			tmp->size = 0;
			struct_data[new_data] = tmp;
			structs.push_back(new_data);
		}
		data->SetLength(1);
		EQ2_16BitString* tmp = new EQ2_16BitString;
		tmp->size = 0;
		struct_data[data] = tmp;
		break;
	}
	case DATA_STRUCT_EQ2_32BIT_STRING: {
		string name2 = data->GetStringName();
		for (int32 i = 1; i < data->GetLength(); i++) {
			DataStruct* new_data = new DataStruct(data);
			char blah[10] = { 0 };
			sprintf(blah, "%i", i);
			name2.append("_").append(blah);
			new_data->SetName(name2.c_str());
			new_data->SetLength(1);
			EQ2_32BitString* tmp = new EQ2_32BitString;
			tmp->size = 0;
			struct_data[new_data] = tmp;
			structs.push_back(new_data);
		}
		data->SetLength(1);
		EQ2_32BitString* tmp = new EQ2_32BitString;
		tmp->size = 0;
		struct_data[data] = tmp;
		break;
	}
	case DATA_STRUCT_COLOR: {
		struct_data[data] = new EQ2_Color[data->GetLength()];
		EQ2_Color* ptr = (EQ2_Color*)GetStructPointer(data);
		for (int16 i = 0; i < data->GetLength(); i++) {
			ptr[i].red = 0;
			ptr[i].blue = 0;
			ptr[i].green = 0;
		}
		break;
	}
	case DATA_STRUCT_EQUIPMENT: {
		struct_data[data] = new EQ2_EquipmentItem[data->GetLength()];
		EQ2_EquipmentItem* ptr = (EQ2_EquipmentItem*)GetStructPointer(data);
		for (int16 i = 0; i < data->GetLength(); i++) {
			memset(&ptr[i].color, 0, sizeof(ptr[i].color));
			memset(&ptr[i].highlight, 0, sizeof(ptr[i].highlight));
			ptr[i].type = 0;
		}
		break;
	}
	case DATA_STRUCT_ITEM: {
		struct_data[data] = new uchar[10000];
		char* ptr = (char*)GetStructPointer(data);
		memset(ptr, 0, 10000);
		break;
	}
	}
}
void PacketStruct::remove(DataStruct* data) {
	vector<DataStruct*>::iterator itr;
	for (itr = structs.begin(); itr != structs.end(); itr++) {
		if (data == (*itr)) {
			structs.erase(itr);
			safe_delete(data);
			return;
		}
	}
}
DataStruct* PacketStruct::findStruct(const char* name, int32 index) {
	return findStruct(name, index, index);
}

DataStruct* PacketStruct::findStruct(const char* name, int32 index1, int32 index2) {
	DataStruct* data = 0;

	if (struct_map.count(string(name)) > 0) {
		data = struct_map[string(name)];
		if (data && index2 < data->GetLength())
			return data;
	}
	vector<DataStruct*>::iterator itr;

	PacketStruct* packet = 0;
	vector<PacketStruct*>::iterator itr2;
	string name2 = string(name);
	if (index1 < 0xFFFF) {
		char blah[10] = { 0 };
		sprintf(blah, "_%i", index1);
		name2.append(blah);
	}
	if (struct_map.count(name2) > 0) {
		data = struct_map[name2];
		if (data && index2 < data->GetLength())
			return data;
	}
	for (itr2 = arrays.begin(); itr2 != arrays.end(); itr2++) {
		packet = *itr2;
		data = packet->findStruct(name, index1, index2);
		if (data != 0)
			return data;
	}
	return 0;
}
void PacketStruct::remove(const char* name) {
	DataStruct* data = 0;
	vector<DataStruct*>::iterator itr;
	for (itr = structs.begin(); itr != structs.end(); itr++) {
		data = *itr;
		if (strcmp(name, data->GetName()) == 0) {
			structs.erase(itr);
			safe_delete(data);
			return;
		}
	}
}
string* PacketStruct::serializeString() {
	serializePacket();
	return getDataString();
}
void PacketStruct::setSmallString(DataStruct* data_struct, const char* text, int32 index) {
	EQ2_8BitString* string_data = new EQ2_8BitString;
	string_data->data = string(text);
	string_data->size = string_data->data.length();
	setData(data_struct, string_data, index);
	safe_delete(string_data);
}
void PacketStruct::setMediumString(DataStruct* data_struct, const char* text, int32 index) {
	EQ2_16BitString* string_data = new EQ2_16BitString;
	string_data->data = string(text);
	string_data->size = string_data->data.length();
	setData(data_struct, string_data, index);
	safe_delete(string_data);
}
void PacketStruct::setLargeString(DataStruct* data_struct, const char* text, int32 index) {
	EQ2_32BitString* string_data = new EQ2_32BitString;
	string_data->data = string(text);
	string_data->size = string_data->data.length();
	setData(data_struct, string_data, index);
	safe_delete(string_data);
}
void PacketStruct::setSmallStringByName(const char* name, const char* text, int32 index) {
	setSmallString(findStruct(name, index), text, index);
}
void PacketStruct::setMediumStringByName(const char* name, const char* text, int32 index) {
	setMediumString(findStruct(name, index), text, index);
}
void PacketStruct::setLargeStringByName(const char* name, const char* text, int32 index) {
	setLargeString(findStruct(name, index), text, index);
}

bool PacketStruct::GetVariableIsSet(const char* name) {
	DataStruct* ds2 = findStruct(name, 0);
	if (!ds2 || !ds2->IsSet())
		return false;
	return true;
}

bool PacketStruct::GetVariableIsNotSet(const char* name) {
	DataStruct* ds2 = findStruct(name, 0);
	if (ds2 && ds2->IsSet())
		return false;
	return true;
}

bool PacketStruct::CheckFlagExists(const char* name) {
	vector<string>::iterator itr;
	for (itr = flags.begin(); itr != flags.end(); itr++) {
		if (*itr == string(name))
			return true;
	}
	return false;
}

void PacketStruct::AddFlag(const char* name) {
	if (flags.size() > 0) {
		vector<string>::iterator itr;
		for (itr = flags.begin(); itr != flags.end(); itr++) {
			if (*itr == string(name))
				return;
		}
	}
	flags.push_back(string(name));
}

bool PacketStruct::LoadPacketData(uchar* data, int32 data_len, bool create_color) {
	loadedSuccessfully = true;
	DataStruct* data_struct = 0;
	try {
		InitializeLoadData(data, data_len);
		vector<DataStruct*>::iterator itr;

		for (itr = structs.begin(); itr != structs.end(); itr++) {
			data_struct = *itr;
			if (!data_struct->AddToStruct())
				continue;
			
			if (data_struct->GetIfSet() && data_struct->GetIfSetVariable()) {
				string varname = string(data_struct->GetIfSetVariable());
				if (varname.find(",") < 0xFFFFFFFF) {
					vector<string>* varnames = SplitString(varname, ',');
					if (varnames) {
						bool should_continue = true;
						for (int32 i = 0; i < varnames->size(); i++) {
							if (GetVariableIsSet(varnames->at(i).c_str())) {
								should_continue = false;
								break;
							}
						}
						safe_delete(varnames);
						if (should_continue)
							continue;
					}
				}
				else {
					// Check to see if the variable contains %i, if it does assume we are in an array
					// and get the current index from the end of the data struct's name
					char name[250] = { 0 };
					if (varname.find("%i") < 0xFFFFFFFF) {
						vector<string>* varnames = SplitString(data_struct->GetName(), '_');
						int index = atoi(varnames->at(varnames->size() - 1).c_str());
						sprintf(name, varname.c_str(), index);
					}
					else
						strcpy(name, varname.c_str());

					if (!GetVariableIsSet(name))
						continue;
				}
			}
			if (data_struct->GetIfNotSet() && data_struct->GetIfNotSetVariable()) {
				string varname = string(data_struct->GetIfNotSetVariable());
				if (varname.find(",") < 0xFFFFFFFF) {
					vector<string>* varnames = SplitString(varname, ',');
					if (varnames) {
						bool should_continue = false;
						for (int32 i = 0; i < varnames->size(); i++) {
							if (!GetVariableIsNotSet(varnames->at(i).c_str())) {
								should_continue = true;
								break;
							}
						}
						safe_delete(varnames);
						if (should_continue)
							continue;
					}
				}
				else {
					// Check to see if the variable contains %i, if it does assume we are in an array
					// and get the current index from the end of the data struct's name
					char name[250] = { 0 };
					if (varname.find("%i") < 0xFFFFFFFF) {
						vector<string>* varnames = SplitString(data_struct->GetName(), '_');
						int index = atoi(varnames->at(varnames->size() - 1).c_str());
						sprintf(name, varname.c_str(), index);
					}
					else
						strcpy(name, varname.c_str());

					if (!GetVariableIsNotSet(name))
						continue;
				}
			}
			// Quick implementaion of IfVariableNotEquals
			// probably not what it was intended for as it currently just checks to see if the given variable equals 1
			// should probably change it so you can define what the variable should or shouldn't equal
			//
			// ie: IfVariableNotEquals="stat_type_%i=1"
			// would be a check to make sure that stat_type_%i does not equal 1 and if it does exclude this element
			if (data_struct->GetIfNotEquals() && data_struct->GetIfNotEqualsVariable()) {
				// Get the variable name
				string varname = string(data_struct->GetIfNotEqualsVariable());
				char name[250] = { 0 };
				// Check to see if the variable has %i in the name, if it does assume we are in an array and get the current
				// index and replace it
				if (varname.find("%i") < 0xFFFFFFFF) {
					// Get the current index by getting the number at the end of the name
					vector<string>* varnames = SplitString(data_struct->GetName(), '_');
					int index = atoi(varnames->at(varnames->size() - 1).c_str());

					string substr = "stat_type";
					if (strncmp(varname.c_str(), substr.c_str(), strlen(substr.c_str())) == 0) {
						// adorn_stat_subtype  18 chars
						string temp = varname.substr(12);
						char temp2[20] = { 0 };
						int index2 = atoi(temp.c_str());
						itoa(index2, temp2, 10);
						varname = varname.substr(0, 12).append(temp2).append("_%i");
					}
					sprintf(name, varname.c_str(), index);
					safe_delete(varnames);
				}
				else
					strcpy(name, varname.c_str());

				// Get the data for the variable
				int16 value = 0;
				DataStruct* data_struct2 = findStruct(name, 0);
				value = getType_int16(data_struct2);
				// Hack for items as it is the only struct that currently uses IfVariableNotEquals
				if (value == 1)
					continue;
			}
			// copy and paste of the code above for IfEquals
			if (data_struct->GetIfEquals() && data_struct->GetIfEqualsVariable()) {
				// Get the variable name
				string varname = string(data_struct->GetIfEqualsVariable());
				char name[250] = { 0 };
				// Check to see if the variable has %i in the name, if it does assume we are in an array and get the current
				// index and replace it
				if (varname.find("%i") < 0xFFFFFFFF) {
					// Get the current index by getting the number at the end of the name
					vector<string>* varnames = SplitString(data_struct->GetName(), '_');
					int index = atoi(varnames->at(varnames->size() - 1).c_str());

					string substr = "stat_type";
					if (strncmp(varname.c_str(), substr.c_str(), strlen(substr.c_str())) == 0) {
						// adorn_stat_subtype  18 chars
						string temp = varname.substr(12);
						char temp2[20] = { 0 };
						int index2 = atoi(temp.c_str());
						itoa(index2, temp2, 10);
						varname = varname.substr(0, 12).append(temp2).append("_%i");
					}
					sprintf(name, varname.c_str(), index);
					safe_delete(varnames);
				}
				else
					strcpy(name, varname.c_str());

				// Get the data for the variable
				int16 value = 0;
				DataStruct* data_struct2 = findStruct(name, 0);
				value = getType_int16(data_struct2);
				// Hack for items as it is the only struct that currently uses IfVariableNotEquals
				if (value != 1)
					continue;
			}

			// The following is tailored to items as they are the only structs that use type2
			// if more type2's are needed outside of the item stats array we need to think up an element
			// to determine when to use type2 over type. 
			// Added checks for set stats and adorn stats - Zcoretri
			bool useType2 = false;
			if (data_struct->GetType2() > 0) {
				int16 type = 0;
				char name[250] = { 0 };
				vector<string>* varnames = SplitString(data_struct->GetName(), '_');
				string struct_name = data_struct->GetName();
				if (struct_name.find("set") < 0xFFFFFFFF) {
					string tmp = "set_stat_type";
					struct_name.replace(0, 9, tmp);
					sprintf(name, "%s", struct_name.c_str());
				}
				else if (struct_name.find("adorn") < 0xFFFFFFFF) {
					string tmp = "adorn_stat_type";
					struct_name.replace(0, 9, tmp);
					sprintf(name, "%s", struct_name.c_str());
				}
				else {
					// set name to stat_type_# (where # is the current index of the array we are in)
					sprintf(name, "%s_%s", "stat_type", varnames->at(varnames->size() - 1).c_str());
				}
				// Look up the value for stat_type
				DataStruct* data_struct2 = findStruct(name, 0);
				type = getType_int16(data_struct2);
				// If stat_type == 6 we use a float, else we use sint16
				if (type != 6 && type != 7)
					useType2 = true;
				safe_delete(varnames);
			}
			if (!StructLoadData(data_struct, GetStructPointer(data_struct), data_struct->GetLength(), useType2, create_color))
			{
				loadedSuccessfully = false;
				break;
			}
		}
	}
	catch (...) {
		loadedSuccessfully = false;
	}
	return loadedSuccessfully;
}
bool PacketStruct::StructLoadData(DataStruct* data_struct, void* data, int32 len, bool useType2, bool create_color) {
	int8 type = 0;
	if (useType2) {
		type = data_struct->GetType2();
		// Need to change the data the struct expects to type2
		data_struct->SetType(type);
		LogWrite(PACKET__DEBUG, 7, "Items", "Using type2 = %i", type);
	}
	else
		type = data_struct->GetType();

	switch (type) {
	case DATA_STRUCT_INT8:
		LoadData((int8*)data, len);
		data_struct->SetIsSet(*((int8*)data) > 0);
		break;
	case DATA_STRUCT_INT16:
		if (data_struct->GetOversized() > 0) {
			LoadData((int8*)data, len);
			if (getType_int16(data_struct) == data_struct->GetOversizedByte()) {
				LoadData((int16*)data, len);
			}
		}
		else {
			LoadData((int16*)data, len);
		}
		data_struct->SetIsSet(*((int16*)data) > 0);
		break;
	case DATA_STRUCT_INT32:
		if (data_struct->GetOversized() > 0) {
			LoadData((int8*)data, len);
			if (getType_int32(data_struct) == data_struct->GetOversizedByte()) {
				LoadData((int32*)data, len);
			}
			else
				LoadData((int8*)data, len);
		}
		else {
			LoadData((int32*)data, len);
		}
		data_struct->SetIsSet(*((int32*)data) > 0);
		break;
	case DATA_STRUCT_INT64:
		LoadData((int64*)data, len);
		data_struct->SetIsSet(*((int64*)data) > 0);
		break;
	case DATA_STRUCT_SINT8:
		LoadData((sint8*)data, len);
		data_struct->SetIsSet(*((sint8*)data) > 0);
		break;
	case DATA_STRUCT_SINT16:
		if (data_struct->GetOversized() > 0) {
			LoadData((sint8*)data, len);
			sint8 val = (sint8)getType_sint16(data_struct);
			if (val < 0) //necessary because when using memcpy from a smaller data type to a larger one, the sign is lost
				setData(data_struct, val, 0);
			if (getType_sint16(data_struct) == data_struct->GetOversizedByte())
				LoadData((sint16*)data, len);
		}
		else
			LoadData((sint16*)data, len);
		data_struct->SetIsSet(*((sint16*)data) > 0);
		break;
	case DATA_STRUCT_SINT32:
		LoadData((sint32*)data, len);
		data_struct->SetIsSet(*((sint32*)data) > 0);
		break;
	case DATA_STRUCT_SINT64:
		LoadData((sint64*)data, len);
		data_struct->SetIsSet(*((sint64*)data) > 0);
		break;
	case DATA_STRUCT_CHAR:
		LoadData((char*)data, len);
		data_struct->SetIsSet(true);
		break;
	case DATA_STRUCT_FLOAT:
		LoadData((float*)data, len);
		data_struct->SetIsSet(*((float*)data) > 0);
		break;
	case DATA_STRUCT_DOUBLE:
		LoadData((double*)data, len);
		data_struct->SetIsSet(*((double*)data) > 0);
		break;
	case DATA_STRUCT_EQ2_8BIT_STRING: {
		LoadDataString((EQ2_8BitString*)data);
		data_struct->SetIsSet(((EQ2_8BitString*)data)->data.length() > 0);
		break;
	}
	case DATA_STRUCT_EQ2_16BIT_STRING: {
		LoadDataString((EQ2_16BitString*)data);
		data_struct->SetIsSet(((EQ2_16BitString*)data)->data.length() > 0);
		break;
	}
	case DATA_STRUCT_EQ2_32BIT_STRING: {
		LoadDataString((EQ2_32BitString*)data);
		data_struct->SetIsSet(((EQ2_32BitString*)data)->data.length() > 0);
		break;
	}
	case DATA_STRUCT_COLOR: {
			// lets not do this again, DoF behaves differently than AoM, DoF is not compatible with CreateEQ2Color
			//if (strcmp(GetName(), "CreateCharacter") == 0 || strcmp(GetName(), "WS_SubmitCharCust") == 0)
		if(create_color)
			CreateEQ2Color((EQ2_Color*)data);
		else
			LoadData((EQ2_Color*)data, len);
		break;
	}
	case DATA_STRUCT_EQUIPMENT: {
		LoadData((EQ2_EquipmentItem*)data);
		break;
	}
	case DATA_STRUCT_ARRAY: {
		int32 size = GetArraySize(data_struct, 0);
		if (size > 0xFFFF || size > GetLoadLen()-GetLoadPos()) {
			LogWrite(PACKET__WARNING, 1, "Packet", "Possible corrupt packet while loading struct array, orig array size: %u in struct name %s, data name %s, load_len %u, load_pos %u", size, GetName(), (data_struct && data_struct->GetName()) ? data_struct->GetName() : "??", GetLoadLen(), GetLoadPos());
			return false;
		}
		PacketStruct* ps = GetPacketStructByName(data_struct->GetName());
		if (ps && ps->GetSubPacketSize() != size) {
			if (data_struct->GetMaxArraySize() > 0 && size > data_struct->GetMaxArraySize())
				size = data_struct->GetMaxArraySize();
			ps->reAddAll(size);
		}
		if (ps && size > 0) {
			//for(int i=0;i<size && (GetLoadLen()-GetLoadPos()) > 0;i++){
			if(ps->LoadPacketData(GetLoadBuffer() + GetLoadPos(), GetLoadLen() - GetLoadPos(), create_color)) {
				SetLoadPos(GetLoadPos() + ps->GetLoadPos());
			}
			//}
		}
		break;
	}
	default: {
		data_struct->SetIsSet(false);
	}
	}

	return true;
}
PacketStruct* PacketStruct::GetPacketStructByName(const char* name) {
	PacketStruct* ps = 0;
	vector<PacketStruct*>::iterator itr;
	for (itr = arrays.begin(); itr != arrays.end(); itr++) {
		ps = *itr;
		if (strcmp(ps->GetName(), name) == 0)
			return ps;
		ps = ps->GetPacketStructByName(name);
		if (ps)
			return ps;
	}
	return 0;
}

void PacketStruct::reAddAll(int32 length) {
	vector<DataStruct*>::iterator itr;
	DataStruct* ds = 0;
	PacketStruct* ps = 0;
	vector<PacketStruct*>::iterator packet_itr;
	if (orig_structs.size() == 0)
		orig_structs = structs;
	else
		deleteDataStructs(&structs);
	structs.clear();

	if (orig_packets.size() == 0 && arrays.size() > 0)
		orig_packets = arrays;
	else {
		for (packet_itr = arrays.begin(); packet_itr != arrays.end(); packet_itr++) {
			ps = *packet_itr;
			safe_delete(ps);
		}
	}
	arrays.clear();

	for (int16 i = 0; i < length; i++) {
		for (packet_itr = orig_packets.begin(); packet_itr != orig_packets.end(); packet_itr++) {
			ps = *packet_itr;
			PacketStruct* new_packet = new PacketStruct(ps, true);
			char tmp[20] = { 0 };
			sprintf(tmp, "_%i", i);
			string name = string(new_packet->GetName());
			name.append(tmp);
			new_packet->SetName(name.c_str());
			add(new_packet);
		}
		for (itr = orig_structs.begin(); itr != orig_structs.end(); itr++) {
			ds = *itr;
			DataStruct* new_data = new DataStruct(ds);
			char tmp[20] = { 0 };
			sprintf(tmp, "_%i", i);
			string name = new_data->GetStringName();
			if (IsSubPacket() && parent->IsSubPacket()) {
				string parent_name = string(GetName());
				try {
					if (parent_name.rfind("_") < 0xFFFFFFFF)
						sprintf(tmp, "%i_%i", atoi(parent_name.substr(parent_name.rfind("_") + 1).c_str()), i);
				}
				catch (...) {
					sprintf(tmp, "_%i", i);
				}
			}
			name.append(tmp);
			new_data->SetName(name.c_str());
			if (new_data->GetType() == DATA_STRUCT_ARRAY) {
				string old_size_arr = string(new_data->GetArraySizeVariable());
				new_data->SetArraySizeVariable(old_size_arr.append(tmp).c_str());
			}
			add(new_data);
		}
	}
	sub_packet_size = length;
}
int32 PacketStruct::GetArraySize(DataStruct* data_struct, int32 index) {
	if (data_struct) {
		const char* name = data_struct->GetArraySizeVariable();
		return GetArraySize(name, index);
	}
	return 0;
}
int32 PacketStruct::GetArraySize(const char* name, int32 index) {
	int32 ret = 0;
	DataStruct* ds = findStruct(name, index);
	if (ds) {
		if (ds->GetType() == DATA_STRUCT_INT8) {
			int8* tmp = (int8*)GetStructPointer(ds);
			ret = *tmp;
		}
		else if (ds->GetType() == DATA_STRUCT_INT16) {
			int16* tmp = (int16*)GetStructPointer(ds);
			ret = *tmp;
		}
		else if (ds->GetType() == DATA_STRUCT_INT32) {
			int32* tmp = (int32*)GetStructPointer(ds);
			ret = *tmp;
		}
		else if (ds->GetType() == DATA_STRUCT_INT64) {
			int64* tmp = (int64*)GetStructPointer(ds);
			ret = *tmp;
		}
	}
	return ret;
}

void PacketStruct::UpdateArrayByArrayLength(DataStruct* data_struct, int32 index, int32 size) {
	if (data_struct) {
		PacketStruct* packet = 0;
		DataStruct* data = 0;
		vector<DataStruct*>::iterator itr;

		for (itr = structs.begin(); itr != structs.end(); itr++) {
			data = *itr;
			if (strcmp(data->GetArraySizeVariable(), data_struct->GetName()) == 0) {
				packet = GetPacketStructByName(data->GetName());
				if (packet)
					packet->reAddAll(size);
				return;
			}
		}
		vector<PacketStruct*>::iterator itr2;
		for (itr2 = arrays.begin(); itr2 != arrays.end(); itr2++) {
			packet = *itr2;
			packet->UpdateArrayByArrayLength(data_struct, index, size);
		}
	}
}

void PacketStruct::UpdateArrayByArrayLengthName(const char* name, int32 index, int32 size) {
	UpdateArrayByArrayLength(findStruct(name, index), index, size);
}
int32 PacketStruct::GetArraySizeByName(const char* name, int32 index) {
	DataStruct* ds1 = findStruct(name, index);
	return GetArraySize(ds1, index);
}

int16 PacketStruct::GetOpcodeValue(int16 client_version) {
	int16 opcode = 0xFFFF;
	bool client_cmd = false;
	int16 OpcodeVersion = 0;
#ifndef LOGIN
	if (GetOpcode() == OP_ClientCmdMsg && strlen(GetOpcodeType()) > 0 && !IsSubPacket())
		client_cmd = true;
#endif
	if (client_cmd) {
		EmuOpcode sub_opcode = EQOpcodeManager[0]->NameSearch(GetOpcodeType());
		if (sub_opcode != OP_Unknown) { //numbers should be used at OpcodeTypes, define them!
			OpcodeVersion = GetOpcodeVersion(client_version);
			if (EQOpcodeManager.count(OpcodeVersion) > 0) {
				opcode = EQOpcodeManager[OpcodeVersion]->EmuToEQ(sub_opcode);
				if (opcode == 0xCDCD) {
					LogWrite(PACKET__ERROR, 0, "Packet", "Could not find valid opcode for opcode: %s and client_version: %i", EQOpcodeManager[OpcodeVersion]->EmuToName(sub_opcode), client_version);
				}
			}
		}		
	}
	else {
		OpcodeVersion = GetOpcodeVersion(client_version);
		if (EQOpcodeManager.count(OpcodeVersion) > 0) {
			opcode = EQOpcodeManager[OpcodeVersion]->EmuToEQ(GetOpcode());
			if (opcode == 0xCDCD) {
				LogWrite(PACKET__ERROR, 0, "Packet", "Could not find valid opcode for opcode: %s and client_version: %i", EQOpcodeManager[OpcodeVersion]->EmuToName(GetOpcode()), client_version);
			}
		}
	}
#ifndef LOGIN
	if(opcode == 0)
		opcode = 0xFFFF;
#endif	
	return opcode;
}

void PacketStruct::serializePacket(bool clear) {
	if (clear)
		Clear();
	bool client_cmd = false;
	string client_data;
#ifndef LOGIN
	if (GetOpcode() == OP_ClientCmdMsg && strlen(GetOpcodeType()) > 0 && !IsSubPacket())
		client_cmd = true;
#endif
	DataStruct* data = 0;
	vector<DataStruct*>::iterator itr;
	for (itr = structs.begin(); itr != structs.end(); itr++) {
		data = *itr;
		if (data->IsOptional())//this would be false if the datastruct WAS optional, but was set
			continue;
		if (data->AddToStruct()) {
			if (data->GetIfFlagNotSet() && CheckFlagExists(data->GetIfFlagNotSetVariable()))
				continue;
			if (data->GetIfFlagSet() && !CheckFlagExists(data->GetIfFlagSetVariable()))
				continue;
			if (data->GetIfSet() && data->GetIfSetVariable()) {
				string varname = string(data->GetIfSetVariable());
				if (varname.find(",") < 0xFFFFFFFF) {
					vector<string>* varnames = SplitString(varname, ',');
					if (varnames) {
						bool should_continue = true;
						for (int32 i = 0; i < varnames->size(); i++) {
							if (GetVariableIsSet(varnames->at(i).c_str())) {
								should_continue = false;
								break;
							}
						}
						safe_delete(varnames);
						if (should_continue)
							continue;
					}
				}
				else {
					if (!GetVariableIsSet(varname.c_str()))
						continue;
				}
			}
			if (data->GetIfNotSet() && data->GetIfNotSetVariable()) {
				string varname = string(data->GetIfNotSetVariable());
				if (varname.find(",") < 0xFFFFFFFF) {
					vector<string>* varnames = SplitString(varname, ',');
					if (varnames) {
						bool should_continue = false;
						for (int32 i = 0; i < varnames->size(); i++) {
							if (!GetVariableIsNotSet(varnames->at(i).c_str())) {
								should_continue = true;
								break;
							}
						}
						safe_delete(varnames);
						if (should_continue)
							continue;
					}
				}
				else {
					// Check to see if the variable contains %i, if it does assume we are in an array
					// and get the current index from the end of the data struct's name
					char name[250] = { 0 };
					if (varname.find("%i") < 0xFFFFFFFF) {
						vector<string>* varnames = SplitString(varname, '_');
						vector<string>* indexes = SplitString(data->GetName(), '_');
						int index = 0;
						if (indexes->size() > 0)
							index = atoi(indexes->at(indexes->size() - 1).c_str());

						sprintf(name, varname.c_str(), index);
					}
					else
						strcpy(name, varname.c_str());
					if (!GetVariableIsNotSet(name))
						continue;
				}
			}
			// Quick implementaion of IfVariableNotEquals
			// probably not what it was intended for as it currently just checks to see if the given variable equals 1
			// should probably change it so you can define what the variable should or shouldn't equal
			//
			// ie: IfVariableNotEquals="stat_type_%i=1"
			// would be a check to make sure that stat_type_%i does not equal 1 and if it does exclude this element
			if (data->GetIfNotEquals() && data->GetIfNotEqualsVariable()) {
				// Get the variable name
				string varname = string(data->GetIfNotEqualsVariable());
				char name[250] = { 0 };
				// Check to see if the variable has %i in the name, if it does assume we are in an array and get the current
				// index and replace it
				if (varname.find("%i") < 0xFFFFFFFF) {
					// Get the current index by getting the number at the end of the name
					vector<string>* varnames = SplitString(data->GetName(), '_');
					int index = atoi(varnames->at(varnames->size() - 1).c_str());

					string substr = "stat_type";
					if (strncmp(varname.c_str(), substr.c_str(), strlen(substr.c_str())) == 0) {
						// adorn_stat_subtype  18 chars
						string temp = varname.substr(12);
						char temp2[20] = { 0 };
						int index2 = atoi(temp.c_str());
						itoa(index2, temp2, 10);
						varname = varname.substr(0, 12).append(temp2).append("_%i");
					}
					sprintf(name, varname.c_str(), index);
					safe_delete(varnames);
				}
				else
					strcpy(name, varname.c_str());

				// Get the data for the variable
				int16 value = 0;
				DataStruct* data_struct2 = findStruct(name, 0);
				value = getType_int16(data_struct2);
				// Hack for items as it is the only struct that currently uses IfVariableNotEquals
				if (value == 1)
					continue;
			}
			// copy and paste of the code above for IfEquals
			if (data->GetIfEquals() && data->GetIfEqualsVariable()) {
				// Get the variable name
				string varname = string(data->GetIfEqualsVariable());
				char name[250] = { 0 };
				// Check to see if the variable has %i in the name, if it does assume we are in an array and get the current
				// index and replace it
				if (varname.find("%i") < 0xFFFFFFFF) {
					// Get the current index by getting the number at the end of the name
					vector<string>* varnames = SplitString(data->GetName(), '_');
					int index = 0;
					if (varnames)
						index = atoi(varnames->at(varnames->size() - 1).c_str());

					string substr = "stat_type";
					if (strncmp(varname.c_str(), substr.c_str(), strlen(substr.c_str())) == 0) {
						// adorn_stat_subtype  18 chars
						string temp = varname.substr(12);
						char temp2[20] = { 0 };
						int index2 = atoi(temp.c_str());
						itoa(index2, temp2, 10);
						varname = varname.substr(0, 12).append(temp2).append("_%i");
					}
					sprintf(name, varname.c_str(), index);
					safe_delete(varnames);
				}
				else
					strcpy(name, varname.c_str());

				// Get the data for the variable
				int16 value = 0;
				DataStruct* data_struct2 = findStruct(name, 0);
				value = getType_int16(data_struct2);
				// Hack for items as it is the only struct that currently uses IfVariableNotEquals
				if (value != 1)
					continue;
			}
			if (client_cmd)
				AddSerializedData(data, 0, &client_data);
			else
				AddSerializedData(data);
		}
	}
#ifndef LOGIN
	if (client_cmd) {
		int16 opcode_val = GetOpcodeValue(client_version);
		Clear();
		int32 size = client_data.length() + 3; //gotta add the opcode and oversized
		int8 oversized = 0xFF;
		int16 OpcodeVersion = GetOpcodeVersion(client_version);
		if (opcode_val == EQOpcodeManager[OpcodeVersion]->EmuToEQ(OP_EqExamineInfoCmd) && client_version > 561)
			size += (size - 9);
		if (client_version <= 374) {
			if (size >= 255) {
				StructAddData(oversized, sizeof(int8), 0);
				StructAddData(size, sizeof(int16), 0);
			}
			else {
				StructAddData(size, sizeof(int8), 0);
			}
			StructAddData(oversized, sizeof(int8), 0);
			StructAddData(opcode_val, sizeof(int16), 0);
		}
		else {
			StructAddData(size, sizeof(int32), 0);
			StructAddData(oversized, sizeof(int8), 0);
			StructAddData(opcode_val, sizeof(int16), 0);
		}
		AddData(client_data);
	}
#endif
}
int32 PacketStruct::GetTotalPacketSize() {
	int32 retSize = 0;
	DataStruct* data = 0;
	vector<DataStruct*>::iterator itr;
	EQ2_8BitString* tmp1 = 0;
	EQ2_16BitString* tmp2 = 0;
	EQ2_32BitString* tmp3 = 0;
	for (itr = structs.begin(); itr != structs.end(); itr++) {
		data = *itr;
		switch (data->GetType()) {
		case DATA_STRUCT_INT8:
		case DATA_STRUCT_SINT8:
		case DATA_STRUCT_CHAR:
			retSize += (1 * data->GetLength());
			break;
		case DATA_STRUCT_SINT16:
		case DATA_STRUCT_INT16:
			retSize += (2 * data->GetLength());
			break;
		case DATA_STRUCT_INT32:
		case DATA_STRUCT_SINT32:
		case DATA_STRUCT_FLOAT:
		case DATA_STRUCT_DOUBLE:
			retSize += (4 * data->GetLength());
			break;
		case DATA_STRUCT_SINT64:
		case DATA_STRUCT_INT64:
			retSize += (8 * data->GetLength());
			break;
		case DATA_STRUCT_EQ2_8BIT_STRING:
			tmp1 = (EQ2_8BitString*)GetStructPointer(data);
			if (tmp1) {
				for (int16 i = 0; i < data->GetLength(); i++)
					retSize += tmp1[i].data.length();
			}
			retSize += (1 * data->GetLength());
			break;
		case DATA_STRUCT_EQ2_16BIT_STRING: {
			tmp2 = (EQ2_16BitString*)GetStructPointer(data);
			if (tmp2) {
				for (int16 i = 0; i < data->GetLength(); i++)
					retSize += tmp2[i].data.length();
			}
			retSize += (2 * data->GetLength());
			break;
		}
		case DATA_STRUCT_EQ2_32BIT_STRING: {
			tmp3 = (EQ2_32BitString*)GetStructPointer(data);
			if (tmp3) {
				for (int16 i = 0; i < data->GetLength(); i++)
					retSize += tmp3[i].data.length();
			}
			retSize += (4 * data->GetLength());
			break;
		}
		case DATA_STRUCT_ARRAY: {
			int32 size = GetArraySize(data, 0);
			PacketStruct* ps = GetPacketStructByName(data->GetName());
			if (ps && ps->GetSubPacketSize() != size) {
				ps->reAddAll(size);
			}
			if (ps && size > 0)
				retSize += ps->GetTotalPacketSize();
			break;
		}
		case DATA_STRUCT_COLOR: {
			retSize += ((sizeof(int8) * 3) * data->GetLength());
			break;
		}
		case DATA_STRUCT_EQUIPMENT: {
			retSize += ((((sizeof(int8) * 3) * 2) + sizeof(int16)) * data->GetLength());
			break;
		}
		}
	}
	return retSize;
}

void PacketStruct::AddSerializedData(DataStruct* data, int32 index, string* datastring) {
	switch (data->GetAddType()) {
	case DATA_STRUCT_INT8:
		StructAddData((int8*)GetStructPointer(data), data->GetLength(), sizeof(int8), datastring);
		break;
	case DATA_STRUCT_INT16:
		if (data->GetOversized() > 0) {
			if (*((int16*)GetStructPointer(data)) >= data->GetOversized()) {
				StructAddData(data->GetOversizedByte(), sizeof(int8), datastring);
				StructAddData((int16*)GetStructPointer(data), data->GetLength(), sizeof(int16), datastring);
			}
			else
				StructAddData((int8*)GetStructPointer(data), data->GetLength(), sizeof(int8), datastring);
		}
		else
			StructAddData((int16*)GetStructPointer(data), data->GetLength(), sizeof(int16), datastring);
		break;
	case DATA_STRUCT_INT32:
		if (data->GetOversized() > 0) {
			if (*((int32*)GetStructPointer(data)) >= data->GetOversized()) {
				StructAddData(data->GetOversizedByte(), sizeof(int8), datastring);
				StructAddData((int32*)GetStructPointer(data), data->GetLength(), sizeof(int32), datastring);
			}
			else
				StructAddData((int16*)GetStructPointer(data), data->GetLength(), sizeof(int16), datastring);
		}
		else
			StructAddData((int32*)GetStructPointer(data), data->GetLength(), sizeof(int32), datastring);
		break;
	case DATA_STRUCT_INT64:
		StructAddData((int64*)GetStructPointer(data), data->GetLength(), sizeof(int64), datastring);
		break;
	case DATA_STRUCT_SINT8:
		StructAddData((sint8*)GetStructPointer(data), data->GetLength(), sizeof(sint8), datastring);
		break;
	case DATA_STRUCT_SINT16:
		if (data->GetOversized() > 0) {
			sint16 val = *((sint16*)GetStructPointer(data));
			if (val >= data->GetOversized() || val <= (data->GetOversized() * -1)) {
				StructAddData(data->GetOversizedByte(), sizeof(sint8), datastring);
				StructAddData((sint16*)GetStructPointer(data), data->GetLength(), sizeof(sint16), datastring);
			}
			else
				StructAddData((sint8*)GetStructPointer(data), data->GetLength(), sizeof(sint8), datastring);
		}
		else
			StructAddData((sint16*)GetStructPointer(data), data->GetLength(), sizeof(sint16), datastring);
		break;
	case DATA_STRUCT_SINT32:
		StructAddData((sint32*)GetStructPointer(data), data->GetLength(), sizeof(sint32), datastring);
		break;
	case DATA_STRUCT_SINT64:
		StructAddData((sint64*)GetStructPointer(data), data->GetLength(), sizeof(sint64), datastring);
		break;
	case DATA_STRUCT_CHAR:
		StructAddData((char*)GetStructPointer(data), data->GetLength(), sizeof(char), datastring);
		break;
	case DATA_STRUCT_FLOAT:
		StructAddData((float*)GetStructPointer(data), data->GetLength(), sizeof(float), datastring);
		break;
	case DATA_STRUCT_DOUBLE:
		StructAddData((double*)GetStructPointer(data), data->GetLength(), sizeof(double), datastring);
		break;
	case DATA_STRUCT_EQ2_8BIT_STRING: {
		for (int16 i = 0; i < data->GetLength(); i++) {
			EQ2_8BitString* ds = (EQ2_8BitString*)GetStructPointer(data);
			AddDataString(ds[i], datastring);
		}
		break;
	}
	case DATA_STRUCT_EQ2_16BIT_STRING: {
		for (int16 i = 0; i < data->GetLength(); i++) {
			EQ2_16BitString* ds = (EQ2_16BitString*)GetStructPointer(data);
			AddDataString(ds[i], datastring);
		}
		break;
	}
	case DATA_STRUCT_EQ2_32BIT_STRING: {
		for (int16 i = 0; i < data->GetLength(); i++) {
			EQ2_32BitString* ds = (EQ2_32BitString*)GetStructPointer(data);
			AddDataString(ds[i], datastring);
		}
		break;
	}
	case DATA_STRUCT_ARRAY: {
		int32 size = GetArraySize(data, 0);
		PacketStruct* ps = GetPacketStructByName(data->GetName());
		if (ps && ps->GetSubPacketSize() != size) {
			ps->reAddAll(size);
		}
		if (ps && size > 0) {
			ps->serializePacket();
			string data = *(ps->getDataString());
			AddData(data, datastring);
		}
		break;
	}
	case DATA_STRUCT_COLOR: {
		StructAddData((EQ2_Color*)GetStructPointer(data), data->GetLength(), sizeof(EQ2_Color), datastring);
		break;
	}
	case DATA_STRUCT_EQUIPMENT: {
		StructAddData((EQ2_EquipmentItem*)GetStructPointer(data), data->GetLength(), sizeof(EQ2_EquipmentItem), datastring);
		break;
	}
	case DATA_STRUCT_ITEM: {
		//DumpPacket((uchar*)GetStructPointer(data), data->GetItemSize());
		AddCharArray((char*)GetStructPointer(data), data->GetItemSize(), datastring);
		break;
	}
	}
}

int8 PacketStruct::getType_int8_ByName(const char* name, int32 index, bool force) {
	DataStruct* data_struct = findStruct(name, index);
	return getType_int8(data_struct, index, force);
}

int16 PacketStruct::getType_int16_ByName(const char* name, int32 index, bool force) {
	DataStruct* data_struct = findStruct(name, index);
	return getType_int16(data_struct, index, force);
}

int32 PacketStruct::getType_int32_ByName(const char* name, int32 index, bool force) {
	DataStruct* data_struct = findStruct(name, index);
	return getType_int32(data_struct, index, force);
}

int64 PacketStruct::getType_int64_ByName(const char* name, int32 index, bool force) {
	DataStruct* data_struct = findStruct(name, index);
	return getType_int64(data_struct, index, force);
}

sint8 PacketStruct::getType_sint8_ByName(const char* name, int32 index, bool force) {
	DataStruct* data_struct = findStruct(name, index);
	return getType_sint8(data_struct, index, force);
}

sint16 PacketStruct::getType_sint16_ByName(const char* name, int32 index, bool force) {
	DataStruct* data_struct = findStruct(name, index);
	return getType_sint16(data_struct, index, force);
}

sint32 PacketStruct::getType_sint32_ByName(const char* name, int32 index, bool force) {
	DataStruct* data_struct = findStruct(name, index);
	return getType_sint32(data_struct, index, force);
}

sint64 PacketStruct::getType_sint64_ByName(const char* name, int32 index, bool force) {
	DataStruct* data_struct = findStruct(name, index);
	return getType_sint64(data_struct, index, force);
}

float PacketStruct::getType_float_ByName(const char* name, int32 index, bool force) {
	DataStruct* data_struct = findStruct(name, index);
	return getType_float(data_struct, index, force);
}

double PacketStruct::getType_double_ByName(const char* name, int32 index, bool force) {
	DataStruct* data_struct = findStruct(name, index);
	return getType_double(data_struct, index, force);
}

char PacketStruct::getType_char_ByName(const char* name, int32 index, bool force) {
	DataStruct* data_struct = findStruct(name, index);
	return getType_char(data_struct, index, force);
}

EQ2_8BitString PacketStruct::getType_EQ2_8BitString_ByName(const char* name, int32 index, bool force) {
	DataStruct* data_struct = findStruct(name, index);
	return getType_EQ2_8BitString(data_struct, index, force);
}

EQ2_16BitString	PacketStruct::getType_EQ2_16BitString_ByName(const char* name, int32 index, bool force) {
	DataStruct* data_struct = findStruct(name, index);
	return getType_EQ2_16BitString(data_struct, index, force);
}

EQ2_32BitString	PacketStruct::getType_EQ2_32BitString_ByName(const char* name, int32 index, bool force) {
	DataStruct* data_struct = findStruct(name, index);
	return getType_EQ2_32BitString(data_struct, index, force);
}

EQ2_Color PacketStruct::getType_EQ2_Color_ByName(const char* name, int32 index, bool force) {
	DataStruct* data_struct = findStruct(name, index);
	return getType_EQ2_Color(data_struct, index, force);
}

EQ2_EquipmentItem PacketStruct::getType_EQ2_EquipmentItem_ByName(const char* name, int32 index, bool force) {
	DataStruct* data_struct = findStruct(name, index);
	return getType_EQ2_EquipmentItem(data_struct, index, force);
}

int8 PacketStruct::getType_int8(DataStruct* data_struct, int32 index, bool force) {
	if (data_struct && ((data_struct->GetType() == DATA_STRUCT_INT8) || force)) {
		int8* ptr = (int8*)GetStructPointer(data_struct);
		return ptr[index];
	}
	return 0;
}
int16 PacketStruct::getType_int16(DataStruct* data_struct, int32 index, bool force) {
	if (data_struct && ((data_struct->GetType() == DATA_STRUCT_INT16) || force)) {
		int16* ptr = (int16*)GetStructPointer(data_struct);
		return ptr[index];
	}
	return 0;
}
int32 PacketStruct::getType_int32(DataStruct* data_struct, int32 index, bool force) {
	if (data_struct && ((data_struct->GetType() == DATA_STRUCT_INT32) || force)) {
		int32* ptr = (int32*)GetStructPointer(data_struct);
		return ptr[index];
	}
	return 0;
}
int64 PacketStruct::getType_int64(DataStruct* data_struct, int32 index, bool force) {
	if (data_struct && ((data_struct->GetType() == DATA_STRUCT_INT64) || force)) {
		int64* ptr = (int64*)GetStructPointer(data_struct);
		return ptr[index];
	}
	return 0;
}
sint8 PacketStruct::getType_sint8(DataStruct* data_struct, int32 index, bool force) {
	if (data_struct && ((data_struct->GetType() == DATA_STRUCT_SINT8) || force)) {
		sint8* ptr = (sint8*)GetStructPointer(data_struct);
		return ptr[index];
	}
	return 0;
}
sint16 PacketStruct::getType_sint16(DataStruct* data_struct, int32 index, bool force) {
	if (data_struct && ((data_struct->GetType() == DATA_STRUCT_SINT16) || force)) {
		sint16* ptr = (sint16*)GetStructPointer(data_struct);
		return ptr[index];
	}
	return 0;
}
sint32 PacketStruct::getType_sint32(DataStruct* data_struct, int32 index, bool force) {
	if (data_struct && ((data_struct->GetType() == DATA_STRUCT_SINT32) || force)) {
		sint32* ptr = (sint32*)GetStructPointer(data_struct);
		return ptr[index];
	}
	return 0;
}
sint64 PacketStruct::getType_sint64(DataStruct* data_struct, int32 index, bool force) {
	if (data_struct && ((data_struct->GetType() == DATA_STRUCT_SINT64) || force)) {
		sint64* ptr = (sint64*)GetStructPointer(data_struct);
		return ptr[index];
	}
	return 0;
}
float PacketStruct::getType_float(DataStruct* data_struct, int32 index, bool force) {
	if (data_struct && ((data_struct->GetType() == DATA_STRUCT_FLOAT) || force)) {
		float* ptr = (float*)GetStructPointer(data_struct);
		return ptr[index];
	}
	return 0;
}
double PacketStruct::getType_double(DataStruct* data_struct, int32 index, bool force) {
	if (data_struct && ((data_struct->GetType() == DATA_STRUCT_DOUBLE) || force)) {
		double* ptr = (double*)GetStructPointer(data_struct);
		return ptr[index];
	}
	return 0;
}
char PacketStruct::getType_char(DataStruct* data_struct, int32 index, bool force) {
	if (data_struct && ((data_struct->GetType() == DATA_STRUCT_CHAR) || force)) {
		char* ptr = (char*)GetStructPointer(data_struct);
		return ptr[index];
	}
	return 0;
}
EQ2_8BitString PacketStruct::getType_EQ2_8BitString(DataStruct* data_struct, int32 index, bool force) {
	if (data_struct && ((data_struct->GetType() == DATA_STRUCT_EQ2_8BIT_STRING) || force)) {
		EQ2_8BitString* ptr = (EQ2_8BitString*)GetStructPointer(data_struct);
		return ptr[index];
	}
	EQ2_8BitString ret;
	ret.size = 0;
	return ret;
}
EQ2_16BitString PacketStruct::getType_EQ2_16BitString(DataStruct* data_struct, int32 index, bool force) {
	if (data_struct && ((data_struct->GetType() == DATA_STRUCT_EQ2_16BIT_STRING) || force)) {
		EQ2_16BitString* ptr = (EQ2_16BitString*)GetStructPointer(data_struct);
		return ptr[index];
	}
	EQ2_16BitString ret;
	ret.size = 0;
	return ret;
}
EQ2_32BitString PacketStruct::getType_EQ2_32BitString(DataStruct* data_struct, int32 index, bool force) {
	if (data_struct && ((data_struct->GetType() == DATA_STRUCT_EQ2_32BIT_STRING) || force)) {
		EQ2_32BitString* ptr = (EQ2_32BitString*)GetStructPointer(data_struct);
		return ptr[index];
	}
	EQ2_32BitString ret;
	ret.size = 0;
	return ret;
}
EQ2_Color PacketStruct::getType_EQ2_Color(DataStruct* data_struct, int32 index, bool force) {
	if (data_struct && ((data_struct->GetType() == DATA_STRUCT_COLOR) || force)) {
		EQ2_Color* ptr = (EQ2_Color*)GetStructPointer(data_struct);
		return ptr[index];
	}
	EQ2_Color ret;
	ret.blue = 0;
	ret.red = 0;
	ret.green = 0;
	return ret;
}
EQ2_EquipmentItem PacketStruct::getType_EQ2_EquipmentItem(DataStruct* data_struct, int32 index, bool force) {
	if (data_struct && ((data_struct->GetType() == DATA_STRUCT_EQUIPMENT) || force)) {
		EQ2_EquipmentItem* ptr = (EQ2_EquipmentItem*)GetStructPointer(data_struct);
		return ptr[index];
	}
	EQ2_EquipmentItem ret;
	ret.color.blue = 0;
	ret.color.red = 0;
	ret.color.green = 0;
	ret.highlight.blue = 0;
	ret.highlight.red = 0;
	ret.highlight.green = 0;
	ret.type = 0;
	return ret;
}

bool PacketStruct::SetOpcode(const char* new_opcode) {
	opcode = EQOpcodeManager[0]->NameSearch(new_opcode);
	if (opcode == OP_Unknown) {
#ifndef MINILOGIN
		LogWrite(PACKET__ERROR, 0, "Packet", "Warning: PacketStruct '%s' uses an unknown opcode named '%s', this struct cannot be serialized directly.", GetName(), new_opcode);
#endif
		return false;
	}
	return true;
}

EQ2Packet* PacketStruct::serialize() {
	serializePacket();
	
	if (GetOpcode() != OP_Unknown)
		return new EQ2Packet(GetOpcode(), getData(), getDataSize());
	else {
#ifndef MINILOGIN
		LogWrite(PACKET__ERROR, 0, "Packet", "Warning: PacketStruct '%s' uses an unknown opcode and cannot be serialized directly.", GetName());
#endif
		return 0;
	}
}

EQ2Packet* PacketStruct::serializeCountPacket(int16 version, int8 offset, uchar* orig_packet, uchar* xor_packet) {
	string* packet_data = serializeString();
	uchar* data = (uchar*)packet_data->c_str();
	int32 size = packet_data->size();
	uchar* packed_data = new uchar[size + 1000]; // this size + 20 is poorly defined, depending on the packet data, we could use a additional length of 1K+
	memset(packed_data, 0, size + 1000);
	if (orig_packet && xor_packet) {
		memcpy(xor_packet, data + 6, size - 6 - offset);
		Encode(xor_packet, orig_packet, size - 6 - offset);
		size = Pack(packed_data, xor_packet, size - 6 - offset, size + 1000, version);
	}
	else
		size = Pack(packed_data, data + 6, packet_data->size() - 6 - offset, packet_data->size() + 1000, version);
	uchar* combined = new uchar[size + sizeof(int16) + offset];
	memset(combined, 0, size + sizeof(int16) + offset);
	uchar* ptr = combined;
	memcpy(ptr, data, sizeof(int16));
	ptr += sizeof(int16);
	memcpy(ptr, packed_data, size);
	if (offset > 0) {
		ptr += size;
		uchar* ptr2 = data;
		ptr2 += packet_data->size() - offset;
		memcpy(ptr, ptr2, offset);
	}
	EQ2Packet* app = new EQ2Packet(GetOpcode(), combined, size + sizeof(int16) + offset);
	safe_delete_array(packed_data);
	safe_delete_array(combined);
	return app;
}

bool PacketStruct::IsSubPacket() {
	return sub_packet;
}
void PacketStruct::IsSubPacket(bool new_val) {
	sub_packet = new_val;
}
int32 PacketStruct::GetSubPacketSize() {
	return sub_packet_size;
}
void PacketStruct::SetSubPacketSize(int32 new_size) {
	sub_packet_size = new_size;
}
void* PacketStruct::GetStructPointer(DataStruct* data_struct, bool erase) {
	try {
		map<DataStruct*, void*>::iterator tmpitr = struct_data.find(data_struct);
		if (tmpitr != struct_data.end()) {
			if (erase)
				struct_data.erase(data_struct);
			return tmpitr->second;
		}
		else {
			PacketStruct* packet = 0;
			vector<PacketStruct*>::iterator itr2;
			for (itr2 = arrays.begin(); itr2 != arrays.end(); itr2++) {
				packet = *itr2;
				if (packet) {
					void* tmp = packet->GetStructPointer(data_struct, erase);
					if (tmp != 0)
						return tmp;
				}
			}
		}
	}
	catch (...) {
		cout << "Caught Exception...\n";
	}
	return 0;
}

vector<DataStruct*> PacketStruct::GetDataStructs() {
	vector<DataStruct*> ret;
	DataStruct* ds = 0;
	vector<DataStruct*>::iterator itr;
	for (itr = structs.begin(); itr != structs.end(); itr++) {
		ds = *itr;
		if (ds->GetType() == DATA_STRUCT_ARRAY) {
			int32 size = GetArraySize(ds, 0);
			PacketStruct* ps = GetPacketStructByName(ds->GetName());
			if (ps && ps->GetSubPacketSize() != size) {
				ps->reAddAll(size);
			}
			if (ps) {
				vector<DataStruct*> ret2 = ps->GetDataStructs();
				vector<DataStruct*>::iterator itr2;
				for (itr2 = ret2.begin(); itr2 != ret2.end(); itr2++) {
					ret.push_back(*itr2);
				}
			}
		}
		else if (ds->GetLength() == 0 && ds->GetType() == DATA_STRUCT_ARRAY) {
			int32 size = GetArraySize(ds, 0);
			PacketStruct* ps = GetPacketStructByName(ds->GetName());
			if (ps && ps->GetSubPacketSize() != size) {
				ps->reAddAll(size);
			}
			if (ps) {
				vector<DataStruct*> ret2 = ps->GetDataStructs();
				vector<DataStruct*>::iterator itr2;
				for (itr2 = ret2.begin(); itr2 != ret2.end(); itr2++) {
					ret.push_back(*itr2);
				}
			}
		}
		else
			ret.push_back(ds);
	}
	return ret;
}

void PacketStruct::PrintPacket() {
	DataStruct* ds = 0;
	vector<DataStruct*>::iterator itr;
	for (itr = structs.begin(); itr != structs.end(); itr++) {
		ds = *itr;
		if (!ds->AddToStruct())
			continue;
		for (int16 i = 0; i < ds->GetLength(); i++) {
			cout << "Name: " << ds->GetName() << "  \tIndex:  " << i << " \tType:  ";
			switch (ds->GetType()) {
			case DATA_STRUCT_INT8:
				printf("int8\t\tData:  %i", getType_int8_ByName(ds->GetName(), i));
				break;
			case DATA_STRUCT_INT16:
				printf("int16\t\tData:  %i", getType_int16_ByName(ds->GetName(), i));
				break;
			case DATA_STRUCT_INT32:
				printf("int32\t\tData:  %u", getType_int32_ByName(ds->GetName(), i));
				break;
			case DATA_STRUCT_INT64:
				printf("int64\t\tData:  %llu", getType_int64_ByName(ds->GetName(), i));
				break;
			case DATA_STRUCT_SINT8:
				printf("sint8\t\tData: %i", getType_sint8_ByName(ds->GetName(), i));
				break;
			case DATA_STRUCT_SINT16:
				printf("sint16\t\tData:  %i", getType_sint16_ByName(ds->GetName(), i));
				break;
			case DATA_STRUCT_SINT32:
				printf("sint32\t\tData:  %i", getType_sint32_ByName(ds->GetName(), i));
				break;
			case DATA_STRUCT_SINT64:
				printf("sint64\t\tData:  %lli", getType_sint64_ByName(ds->GetName(), i));
				break;
			case DATA_STRUCT_CHAR:
				printf("char\t\tData:  %c", getType_char_ByName(ds->GetName(), i));
				break;
			case DATA_STRUCT_FLOAT:
				printf("float\t\tData:  %f", getType_float_ByName(ds->GetName(), i));
				break;
			case DATA_STRUCT_DOUBLE:
				printf("double\t\tData:  %f", getType_double_ByName(ds->GetName(), i));
				break;
			case DATA_STRUCT_EQ2_8BIT_STRING:
				printf("EQ2_8BitString\tData:  %s", getType_EQ2_8BitString_ByName(ds->GetName(), i).data.c_str());
				break;
			case DATA_STRUCT_EQ2_16BIT_STRING:
				printf("EQ2_16BitString\tData:  %s", getType_EQ2_16BitString_ByName(ds->GetName(), i).data.c_str());
				break;
			case DATA_STRUCT_EQ2_32BIT_STRING: {
				printf("EQ2_32BitString\tData:  %s", getType_EQ2_32BitString_ByName(ds->GetName(), i).data.c_str());
				break;
			}
			case DATA_STRUCT_ITEM: {
				if (ds->GetItemSize() > 0) {
					DumpPacket((uchar*)GetStructPointer(ds), ds->GetItemSize());
				}
				break;
			}
			case DATA_STRUCT_ARRAY: {
				int32 size = GetArraySize(ds, 0);
				PacketStruct* ps = GetPacketStructByName(ds->GetName());
				if (ps && ps->GetSubPacketSize() != size) {
					ps->reAddAll(size);
				}
				if (ps) {
					cout << "Array:\tData:  \n";
					ps->PrintPacket();
				}
				break;
			}
			case DATA_STRUCT_COLOR: {
				cout.unsetf(ios_base::dec);
				cout.setf(ios_base::hex);
				printf("EQ2_Color\tData: ");
				EQ2_Color tmp = getType_EQ2_Color_ByName(ds->GetName(), i);
				printf("R: %i", tmp.red);
				printf(", G: %i", tmp.green);
				printf(", B: %i", tmp.blue);
				break;
			}
			case DATA_STRUCT_EQUIPMENT: {
				cout.unsetf(ios_base::dec);
				cout.setf(ios_base::hex);
				printf("EQ2_EquipmentItem\tData: ");
				EQ2_EquipmentItem tmp = getType_EQ2_EquipmentItem_ByName(ds->GetName(), i);
				printf("type: ");
				printf(" ,color R: %i", tmp.color.red);
				printf(" ,color G: %i", tmp.color.green);
				printf(" ,color B: %i", tmp.color.blue);
				printf(" ,hl R: %i", tmp.highlight.red);
				printf(" ,hl G: %i", tmp.highlight.green);
				printf(" ,hl B: %i", tmp.highlight.blue);
				break;
			}
			}
			cout << endl;
		}
		if (ds->GetLength() == 0 && ds->GetType() == DATA_STRUCT_ARRAY) {
			int32 size = GetArraySize(ds, 0);
			PacketStruct* ps = GetPacketStructByName(ds->GetName());
			if (ps && ps->GetSubPacketSize() != size) {
				ps->reAddAll(size);
			}
			if (ps) {
				cout << "Array:\tData: \n";
				ps->PrintPacket();
			}
		}
	}
}

void PacketStruct::LoadFromPacketStruct(PacketStruct* packet, char* substruct_name) {
	vector<DataStruct*>::iterator itr;
	DataStruct* ds = 0;
	char name[512];

	//scatman (1/30/2012): these declarations are here to get rid of a linux compile error "taking address of temporary"
	EQ2_8BitString str8;
	EQ2_16BitString str16;
	EQ2_32BitString str32;
	EQ2_EquipmentItem equip;

	for (itr = structs.begin(); itr != structs.end(); itr++) {
		ds = *itr;
		for (int16 i = 0; i < ds->GetLength(); i++) {
			memset(name, 0, sizeof(name));

			if (substruct_name)
				snprintf(name, sizeof(name), "%s_%s_0", substruct_name, ds->GetName());
			else
				strncpy(name, ds->GetName(), sizeof(name));
			name[sizeof(name) - 1] = '\0';

			switch (ds->GetType()) {
			case DATA_STRUCT_INT8:
				setData(ds, packet->getType_int8_ByName(name, i), i);
				break;
			case DATA_STRUCT_SINT8:
				setData(ds, packet->getType_sint8_ByName(name, i), i);
				break;
			case DATA_STRUCT_CHAR:
				setData(ds, packet->getType_char_ByName(name, i), i);
				break;
			case DATA_STRUCT_SINT16:
				setData(ds, packet->getType_sint16_ByName(name, i), i);
				break;
			case DATA_STRUCT_INT16:
				setData(ds, packet->getType_int16_ByName(name, i), i);
				break;
			case DATA_STRUCT_INT32:
				setData(ds, packet->getType_int32_ByName(name, i), i);
				break;
			case DATA_STRUCT_INT64:
				setData(ds, packet->getType_int64_ByName(name, i), i);
				break;
			case DATA_STRUCT_SINT32:
				setData(ds, packet->getType_sint32_ByName(name, i), i);
				break;
			case DATA_STRUCT_SINT64:
				setData(ds, packet->getType_sint64_ByName(name, i), i);
				break;
			case DATA_STRUCT_FLOAT:
				setData(ds, packet->getType_float_ByName(name, i), i);
				break;
			case DATA_STRUCT_DOUBLE:
				setData(ds, packet->getType_double_ByName(name, i), i);
				break;
			case DATA_STRUCT_EQ2_8BIT_STRING:
				str8 = packet->getType_EQ2_8BitString_ByName(name, i);
				setData(ds, &str8, i);
				break;
			case DATA_STRUCT_EQ2_16BIT_STRING:
				str16 = packet->getType_EQ2_16BitString_ByName(name, i);
				setData(ds, &str16, i);
				break;
			case DATA_STRUCT_EQ2_32BIT_STRING:
				str32 = packet->getType_EQ2_32BitString_ByName(name, i);
				setData(ds, &str32, i);
				break;
			case DATA_STRUCT_ARRAY: {
				int32 size = GetArraySize(ds, 0);
				PacketStruct* ps = GetPacketStructByName(ds->GetName());
				if (ps && size > 0)
					ps->LoadFromPacketStruct(packet, substruct_name);
				break;
			}
			case DATA_STRUCT_COLOR:
				setColor(ds, packet->getType_EQ2_Color_ByName(name, i), i);
				break;
			case DATA_STRUCT_EQUIPMENT:
				equip = packet->getType_EQ2_EquipmentItem_ByName(name, i);
				setEquipmentByName(ds->GetName(), &equip, i);
				break;
			default:
				break;
			}
		}
	}
}

#ifdef WORLD
void PacketStruct::setItem(DataStruct* ds, Item* item, Player* player, int32 index, sint8 offset, bool loot_item, bool make_empty_item_packet, bool inspect) {
	if (!ds)
		return;
	uchar* ptr = (uchar*)GetStructPointer(ds);
	
	if(!item) {
		if(make_empty_item_packet) {
			if(client_version <= 373) {
				// for player inspection this will offset the parts of the packet that have no items
				uchar bogusItemBuffer[] = {0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x64,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
				int sizeOfArray = sizeof(bogusItemBuffer) / sizeof(bogusItemBuffer[0]);
				ds->SetItemSize(sizeOfArray);
				memcpy(ptr, bogusItemBuffer, sizeOfArray);	
			}
			else if(client_version <= 561) {
				// for player inspection this will offset the parts of the packet that have no items
				uchar bogusItemBuffer[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x8C,0x5A,0xF1,0xD2,0x8C,0x5A,0xF1,0xD2,0x01,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00};
				int sizeOfArray = sizeof(bogusItemBuffer) / sizeof(bogusItemBuffer[0]);
				ds->SetItemSize(sizeOfArray);
				memcpy(ptr, bogusItemBuffer, sizeOfArray);	
			}
			else {
				// for player inspection this will offset the parts of the packet that have no items
				uchar bogusItemBuffer[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00 /*0x68 was item flags*/,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x64,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
				int sizeOfArray = sizeof(bogusItemBuffer) / sizeof(bogusItemBuffer[0]);
				ds->SetItemSize(sizeOfArray);
				memcpy(ptr, bogusItemBuffer, sizeOfArray);
			}
		}
		return;
	}
	PacketStruct* packet = item->PrepareItem(client_version, false, loot_item, inspect);
	if (packet) {
		int16 item_version = GetItemPacketType(packet->GetVersion());
		// newer clients can handle the item structure without the loot_item flag set to true, older clients like DoF need a smaller subpacket of item
		item->serialize(packet, true, player, item_version, 0, (packet->GetVersion() <= 561) ? loot_item : false, inspect);

		string* generic_string_data = packet->serializeString();
		int32 size = generic_string_data->length(); // had to set to 81
		int32 actual_length = size;

		if(client_version <= 373 && make_empty_item_packet && inspect) {
			size += 2; // end padding for the specialized item (used for player inspection)
		}
		else if(offset > 12) {
			size += 6; // end padding for the specialized item (used for player inspection)
		}
		//DumpPacket((uchar*)generic_string_data->c_str(), size);
		if (size <= 13)
		{
			ds->SetIsSet(false);
			return;
		}
		size -= (9 + offset);
		if (client_version > 561 && item->IsBag() == false && item->IsBauble() == false && item->IsFood() == false && (offset == 0 || offset == -1 || offset == 2))
			size = (size * 2) - 5;
		uchar* out_data = new uchar[size + 1];
		memset(out_data, 0, size+1);
		uchar* out_ptr = out_data;
		memcpy(out_ptr, (uchar*)generic_string_data->c_str() + (9 + offset), actual_length - (9 + offset));
		//DumpPacket((uchar*)generic_string_data->c_str() + (9 + offset), size);
		//without these it will prompt for your character name
		if (offset == 0 || offset == -1 || offset == 2) {
			if (client_version <= 561 && item->details.count > 0)
				out_data[0] = item->details.count;
			else
				out_data[0] = 1;
		}
		//
		out_ptr += generic_string_data->length() - (10 + offset);
		if (client_version > 561 && item->IsBag() == false && item->IsBauble() == false && item->IsFood() == false && (offset == 0 || offset == -1 || offset == 2)) {
			out_data[4] = 0x80;
			memcpy(out_ptr, (uchar*)generic_string_data->c_str() + (13 + offset), generic_string_data->length() - (13 + offset));
		}
		ds->SetItemSize(size);
		memcpy(ptr, out_data, size);
		safe_delete_array(out_data);
		delete packet;
	}
	//DumpPacket(ptr2, ds->GetItemSize());
}
void PacketStruct::setItemByName(const char* name, Item* item, Player* player, int32 index, sint8 offset, bool loot_item, bool make_empty_item_packet, bool inspect) {
	setItem(findStruct(name, index), item, player, index, offset, loot_item, make_empty_item_packet, inspect);
}
void PacketStruct::setItemArrayDataByName(const char* name, Item* item, Player* player, int32 index1, int32 index2, sint8 offset, bool loot_item, bool make_empty_item_packet, bool inspect) {
	char tmp[10] = { 0 };
	sprintf(tmp, "_%i", index1);
	string name2 = string(name).append(tmp);
	setItem(findStruct(name2.c_str(), index1, index2), item, player, index2, offset, loot_item, make_empty_item_packet, inspect);
}

void PacketStruct::ResetData() {
	vector<DataStruct*>::iterator itr;
	for (itr = structs.begin(); itr != structs.end(); itr++) {
		DataStruct* ds = *itr;
		void* ptr = GetStructPointer(ds);
		if (!ptr)
			continue;
		switch (ds->GetType())
		{
		case DATA_STRUCT_EQ2_8BIT_STRING: {
			EQ2_8BitString* real_ptr = (EQ2_8BitString*)ptr;
			real_ptr->size = 0;
			real_ptr->data.clear();
			break;
		}
		case DATA_STRUCT_EQ2_16BIT_STRING: {
			EQ2_16BitString* real_ptr = (EQ2_16BitString*)ptr;
			real_ptr->size = 0;
			real_ptr->data.clear();
			break;
		}
		case DATA_STRUCT_EQ2_32BIT_STRING: {
			EQ2_32BitString* real_ptr = (EQ2_32BitString*)ptr;
			real_ptr->size = 0;
			real_ptr->data.clear();
			break;
		}
		case DATA_STRUCT_EQUIPMENT: {
			memset(ptr, 0, sizeof(EQ2_EquipmentItem) * ds->GetLength());
			break;
		}
		case DATA_STRUCT_DOUBLE: {
			memset(ptr, 0, sizeof(double) * ds->GetLength());
			break;
		}
		case DATA_STRUCT_FLOAT: {
			memset(ptr, 0, sizeof(float) * ds->GetLength());
			break;
		}
		case DATA_STRUCT_INT8: {
			memset(ptr, 0, sizeof(int8) * ds->GetLength());
			break;
		}
		case DATA_STRUCT_INT16: {
			memset(ptr, 0, sizeof(int16) * ds->GetLength());
			break;
		}
		case DATA_STRUCT_INT32: {
			memset(ptr, 0, sizeof(int32) * ds->GetLength());
			break;
		}
		case DATA_STRUCT_INT64: {
			memset(ptr, 0, sizeof(int64) * ds->GetLength());
			break;
		}
		case DATA_STRUCT_SINT8: {
			memset(ptr, 0, sizeof(sint8) * ds->GetLength());
			break;
		}
		case DATA_STRUCT_SINT16: {
			memset(ptr, 0, sizeof(sint16) * ds->GetLength());
			break;
		}
		case DATA_STRUCT_SINT32: {
			memset(ptr, 0, sizeof(sint32) * ds->GetLength());
			break;
		}
		case DATA_STRUCT_SINT64: {
			memset(ptr, 0, sizeof(sint64) * ds->GetLength());
			break;
		}
		case DATA_STRUCT_ITEM: {
			memset(ptr, 0, 10000 * ds->GetLength());
			break;
		}
		case DATA_STRUCT_CHAR: {
			memset(ptr, 0, sizeof(char) * ds->GetLength());
			break;
		}
		case DATA_STRUCT_COLOR: {
			memset(ptr, 0, sizeof(EQ2_Color) * ds->GetLength());
			break;
		}
		}
	}
	vector<PacketStruct*>::iterator itr2;
	for (itr2 = arrays.begin(); itr2 != arrays.end(); itr2++)
		(*itr2)->ResetData();
}
#endif
