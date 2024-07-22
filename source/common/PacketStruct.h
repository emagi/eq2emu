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
#ifndef __EQ2_PACKETSTRUCT__
#define __EQ2_PACKETSTRUCT__
#include "types.h"
#include "DataBuffer.h"
#include "opcodemgr.h"

#include <vector>
#include <map>
#ifdef WORLD
class Item;
class Player;
#endif
extern map<int16, OpcodeManager*>EQOpcodeManager;
using namespace std;

#define DATA_STRUCT_NONE				0
#define DATA_STRUCT_INT8				1
#define DATA_STRUCT_INT16				2
#define DATA_STRUCT_INT32				3
#define DATA_STRUCT_INT64				4
#define DATA_STRUCT_FLOAT				5
#define DATA_STRUCT_DOUBLE				6
#define DATA_STRUCT_COLOR				7
#define DATA_STRUCT_SINT8				8
#define DATA_STRUCT_SINT16				9
#define DATA_STRUCT_SINT32				10
#define DATA_STRUCT_CHAR				11
#define DATA_STRUCT_EQ2_8BIT_STRING		12
#define DATA_STRUCT_EQ2_16BIT_STRING	13
#define DATA_STRUCT_EQ2_32BIT_STRING	14
#define DATA_STRUCT_EQUIPMENT			15
#define DATA_STRUCT_ARRAY				16
#define DATA_STRUCT_ITEM				17
#define DATA_STRUCT_SINT64				18

class DataStruct {
public:
	DataStruct();
	DataStruct(DataStruct* data_struct);
	DataStruct(const char* new_name, int8 new_type, int32 new_length = 1, int8 new_type2 = DATA_STRUCT_NONE);
	DataStruct(const char* new_name, const char* new_type, int32 new_length = 1, const char* new_type2 = 0);
	DataStruct(const char* new_name, int32 new_length);
	void	SetType(const char* new_type, int8* output_type);
	void	SetType(int8 new_type);
	void	SetName(const char* new_name);
	void	SetLength(int32 new_length);
	void	SetArraySizeVariable(const char* new_name);
	void	SetDefaultValue(int8 new_val);
	void	SetMaxArraySize(int8 size);
	void	SetOversized(int8 val);
	void	SetOversizedByte(int8 val);
	void	SetAddToStruct(bool val);
	void	SetAddType(int8 new_type);
	void	SetPackedIndex(int8 new_index);
	void	SetPackedSizeVariable(const char* new_name);
	void	SetPacked(const char* value);
	void	SetItemSize(int32 val);
	void	SetIfSetVariable(const char* variable);
	void	SetIfNotSetVariable(const char* variable);
	void	SetIfEqualsVariable(const char* variable);
	void	SetIfNotEqualsVariable(const char* variable);
	void	SetIfFlagSetVariable(const char* variable);
	void	SetIfFlagNotSetVariable(const char* variable);
	void	SetIsSet(bool val);
	void	SetIsOptional(bool val);

	int8	GetPackedIndex();
	const char* GetPackedSizeVariable();
	const char* GetArraySizeVariable();
	int8	GetDefaultValue();
	int8	GetOversized();
	int8	GetOversizedByte();
	int8	GetMaxArraySize();
	int8	GetType();
	int8	GetType2();
	const char* GetName();
	string	GetStringName();
	int32	GetLength();
	bool	AddToStruct();
	int8	GetAddType();
	int32	GetItemSize();
	bool	GetIfSet();
	const char* GetIfSetVariable();
	bool	GetIfNotSet();
	const char* GetIfNotSetVariable();
	bool	GetIfEquals();
	const char* GetIfEqualsVariable();
	bool	GetIfNotEquals();
	const char* GetIfNotEqualsVariable();
	bool	GetIfFlagSet();
	const char* GetIfFlagSetVariable();
	bool	GetIfFlagNotSet();
	const char* GetIfFlagNotSetVariable();
	bool	IsSet();
	bool	IsOptional();
	int32 GetDataSizeInBytes();
	string	AppendVariable(string orig, const char* val);
	void	AddIfSetVariable(const char* val) {
		if (val) {
			if_set_variable = AppendVariable(if_set_variable, val);
			is_set = true;
		}
	}
	void	AddIfNotSetVariable(const char* val) {
		if (val) {
			if_not_set_variable = AppendVariable(if_not_set_variable, val);
			if_not_set = true;
		}
	}

private:
	bool	is_set;
	bool	if_not_set;
	bool	optional;
	bool	if_set;
	bool	if_not_equals;
	bool	if_equals;
	bool	if_flag_set;
	bool	if_flag_not_set;
	string	if_flag_not_set_variable;
	string	if_flag_set_variable;
	string	if_not_equals_variable;
	string	if_equals_variable;
	string	if_not_set_variable;
	string	if_set_variable;
	int8	oversized;
	int8	oversized_byte;
	bool	add;
	int8	addType;
	int8	maxArraySize;
	string	array_size_variable;
	string	name;
	int8	type;
	int8	default_value;
	int8	type2;
	int32	length;
	int32	item_size;
};
class PacketStruct : public DataBuffer {
public:
	PacketStruct();
	PacketStruct(PacketStruct* packet, bool sub);
	PacketStruct(PacketStruct* packet, int16 in_client_version);
	~PacketStruct();
	void add(DataStruct* data);
	void reAddAll(int32 length);
	void add(PacketStruct* packet_struct);
	void addPacketArrays(PacketStruct* packet);
	void deletePacketArrays(PacketStruct* packet);
	void deleteDataStructs(vector<DataStruct*>* data_structs);
	void setSmallStringByName(const char* name, const char* text, int32 index = 0);
	void setMediumStringByName(const char* name, const char* text, int32 index = 0);
	void setLargeStringByName(const char* name, const char* text, int32 index = 0);
	void setSmallString(DataStruct* data_struct, const char* text, int32 index = 0);
	void setMediumString(DataStruct* data_struct, const char* text, int32 index = 0);
	void setLargeString(DataStruct* data_struct, const char* text, int32 index = 0);
	void renameSubstructArray(const char* substruct, int32 index);
	template<class Data> void setSubstructSubstructDataByName(const char* substruct_name1, const char* substruct_name2, const char* name, Data data, int32 substruct_index1 = 0, int32 substruct_index2 = 0, int32 index = 0) {
		char tmp[15] = { 0 };
		sprintf(tmp, "_%i_%i", substruct_index1, substruct_index2);
		string name2 = string(substruct_name1).append("_").append(substruct_name2).append("_").append(name).append(tmp);
		setData(findStruct(name2.c_str(), index), data, index);
	}
	template<class Data> void setSubstructDataByName(const char* substruct_name, const char* name, Data data, int32 substruct_index = 0, int32 index = 0) {
		char tmp[10] = { 0 };
		sprintf(tmp, "_%i", substruct_index);
		string name2 = string(substruct_name).append("_").append(name).append(tmp);
		setData(findStruct(name2.c_str(), index), data, index);
	}
	template<class Data> void setSubstructColorByName(const char* substruct_name, const char* name, Data data, int32 substruct_index = 0, int32 index = 0) {
		char tmp[10] = { 0 };
		sprintf(tmp, "_%i", substruct_index);
		string name2 = string(substruct_name).append("_").append(name).append(tmp);
		setColor(findStruct(name2.c_str(), index), data, index);
	}
	template<class Data> void setSubstructArrayDataByName(const char* substruct_name, const char* name, Data data, int32 index = 0, int32 substruct_index = 0) {
		char tmp[10] = { 0 };
		sprintf(tmp, "_%i", substruct_index);
		string name2 = string(substruct_name).append("_").append(name).append(tmp);
		setData(findStruct(name2.c_str(), substruct_index, index), data, index);
	}
	template<class Data> void setSubstructArrayColorByName(const char* substruct_name, const char* name, Data data, int32 substruct_index = 0, int32 index = 0) {
		char tmp[10] = { 0 };
		sprintf(tmp, "_%i", substruct_index);
		string name2 = string(substruct_name).append("_").append(name).append(tmp);
		setColor(findStruct(name2.c_str(), index, substruct_index), data, index);
	}
	template<class Data> void setDataByName(const char* name, Data data, int32 index = 0, bool use_second_type = false) {
		setData(findStruct(name, index), data, index, use_second_type);
	}
	template<class Data> void setDataByName(const char* name, Data* data, int32 index = 0, bool use_second_type = false) {
		setData(findStruct(name, index), data, index, use_second_type);
	}
	template<class Data> void setSubArrayDataByName(const char* name, Data data, int32 index1 = 0, int32 index2 = 0, int32 index3 = 0) {
		char tmp[20] = { 0 };
		sprintf(tmp, "%i_%i", index1, index2);
		string name2 = string(name).append(tmp);
		setData(findStruct(name2.c_str(), index2, index3), data, index3);
	}
	template<class Data> void setArrayDataByName(const char* name, Data data, int32 index1 = 0, int32 index2 = 0, bool use_second_type = false) {
		char tmp[10] = { 0 };
		sprintf(tmp, "_%i", index1);
		string name2 = string(name).append(tmp);
		setData(findStruct(name2.c_str(), index1, index2), data, index2, use_second_type);
	}
	void setArrayAddToPacketByName(const char* name, bool new_val, int32 index1 = 0, int32 index2 = 0) {
		char tmp[10] = { 0 };
		sprintf(tmp, "_%i", index1);
		string name2 = string(name).append(tmp);
		DataStruct* data = findStruct(name2.c_str(), index2);
		if (data)
			data->SetAddToStruct(new_val);
	}
	void setAddToPacketByName(const char* name, bool new_val, int32 index = 0) {
		DataStruct* data = findStruct(name, index);
		if (data)
			data->SetAddToStruct(new_val);
	}
	void setAddTypePacketByName(const char* name, int8 new_val, int32 index = 0) {
		DataStruct* data = findStruct(name, index);
		if (data)
			data->SetAddType(new_val);
	}
	const char* GetOpcodeType();
	bool	IsSubPacket();
	void	IsSubPacket(bool new_val);
	int32	GetSubPacketSize();
	void	SetSubPacketSize(int32 new_size);
	void	SetOpcodeType(const char* opcodeType);
	int32	GetArraySizeByName(const char* name, int32 index);
	int32	GetArraySize(DataStruct* data_struct, int32 index);
	int32	GetArraySize(const char* name, int32 index);
	void	LoadFromPacketStruct(PacketStruct* packet, char* substruct_name = 0);
	bool	GetVariableIsSet(const char* name);
	bool	GetVariableIsNotSet(const char* name);

	int8	getType_int8_ByName(const char* name, int32 index = 0, bool force = false);
	int16	getType_int16_ByName(const char* name, int32 index = 0, bool force = false);
	int32	getType_int32_ByName(const char* name, int32 index = 0, bool force = false);
	int64	getType_int64_ByName(const char* name, int32 index = 0, bool force = false);
	sint8	getType_sint8_ByName(const char* name, int32 index = 0, bool force = false);
	sint16	getType_sint16_ByName(const char* name, int32 index = 0, bool force = false);
	sint32	getType_sint32_ByName(const char* name, int32 index = 0, bool force = false);
	sint64	getType_sint64_ByName(const char* name, int32 index = 0, bool force = false);
	float	getType_float_ByName(const char* name, int32 index = 0, bool force = false);
	double	getType_double_ByName(const char* name, int32 index = 0, bool force = false);
	char	getType_char_ByName(const char* name, int32 index = 0, bool force = false);
	EQ2_8BitString	getType_EQ2_8BitString_ByName(const char* name, int32 index = 0, bool force = false);
	EQ2_16BitString	getType_EQ2_16BitString_ByName(const char* name, int32 index = 0, bool force = false);
	EQ2_32BitString	getType_EQ2_32BitString_ByName(const char* name, int32 index = 0, bool force = false);
	EQ2_Color getType_EQ2_Color_ByName(const char* name, int32 index = 0, bool force = false);
	EQ2_EquipmentItem getType_EQ2_EquipmentItem_ByName(const char* name, int32 index = 0, bool force = false);

	int8	getType_int8(DataStruct* data_struct, int32 index = 0, bool force = false);
	int16	getType_int16(DataStruct* data_struct, int32 index = 0, bool force = false);
	int32	getType_int32(DataStruct* data_struct, int32 index = 0, bool force = false);
	int64	getType_int64(DataStruct* data_struct, int32 index = 0, bool force = false);
	sint8	getType_sint8(DataStruct* data_struct, int32 index = 0, bool force = false);
	sint16	getType_sint16(DataStruct* data_struct, int32 index = 0, bool force = false);
	sint32	getType_sint32(DataStruct* data_struct, int32 index = 0, bool force = false);
	sint64	getType_sint64(DataStruct* data_struct, int32 index = 0, bool force = false);
	float	getType_float(DataStruct* data_struct, int32 index = 0, bool force = false);
	double	getType_double(DataStruct* data_struct, int32 index = 0, bool force = false);
	char	getType_char(DataStruct* data_struct, int32 index = 0, bool force = false);
	EQ2_8BitString	getType_EQ2_8BitString(DataStruct* data_struct, int32 index = 0, bool force = false);
	EQ2_16BitString	getType_EQ2_16BitString(DataStruct* data_struct, int32 index = 0, bool force = false);
	EQ2_32BitString	getType_EQ2_32BitString(DataStruct* data_struct, int32 index = 0, bool force = false);
	EQ2_Color getType_EQ2_Color(DataStruct* data_struct, int32 index = 0, bool force = false);
	EQ2_EquipmentItem getType_EQ2_EquipmentItem(DataStruct* data_struct, int32 index = 0, bool force = false);

	void setDataType(DataStruct* data_struct, char data, int32 index);
	void setDataType(DataStruct* data_struct, int8 data, int32 index);
	void setDataType(DataStruct* data_struct, int16 data, int32 index);
	void setDataType(DataStruct* data_struct, int32 data, int32 index);
	void setDataType(DataStruct* data_struct, int64 data, int32 index);
	void setDataType(DataStruct* data_struct, sint8 data, int32 index);
	void setDataType(DataStruct* data_struct, sint16 data, int32 index);
	void setDataType(DataStruct* data_struct, sint32 data, int32 index);
	void setDataType(DataStruct* data_struct, sint64 data, int32 index);
	void setDataType(DataStruct* data_struct, float data, int32 index);
	void setDataType(DataStruct* data_struct, double data, int32 index);
	void setData(DataStruct* data_struct, EQ2_8BitString* input_string, int32 index, bool use_second_type = false);
	void setData(DataStruct* data_struct, EQ2_16BitString* input_string, int32 index, bool use_second_type = false);
	void setData(DataStruct* data_struct, EQ2_32BitString* input_string, int32 index, bool use_second_type = false);

	template<class Data> void setData(DataStruct* data_struct, Data* data, int32 index, bool use_second_type = false) {
		if (!data_struct)
			return;
		data_struct->SetIsOptional(false);
		int8 type_to_use = (use_second_type) ? data_struct->GetType2() : data_struct->GetType();
		if (type_to_use >= DATA_STRUCT_EQ2_8BIT_STRING && type_to_use <= DATA_STRUCT_EQ2_32BIT_STRING) {
			if (type_to_use == DATA_STRUCT_EQ2_8BIT_STRING) {
				setSmallString(data_struct, data, index);
			}
			else if (type_to_use == DATA_STRUCT_EQ2_16BIT_STRING) {
				setMediumString(data_struct, data, index);
			}
			else {
				setLargeString(data_struct, data, index);
			}
		}
		else {
			if (data_struct && index == 0 && data_struct->GetLength() > 1) {
				if (type_to_use == DATA_STRUCT_CHAR) {
					for (int32 i = 0; data && i < data_struct->GetLength() && i < strlen(data); i++)
						setData(data_struct, data[i], i);
				}
				else {
					for (int32 i = 0; i < data_struct->GetLength(); i++)
						setData(data_struct, data[i], i);
				}
			}
			else
				setData(data_struct, *data, index);
		}
	}
	template<class Data> void setData(DataStruct* data_struct, Data data, int32 index, bool use_second_type = false) {
		if (data_struct && index < data_struct->GetLength()) {
			data_struct->SetIsOptional(false);
			int8 type_to_use = (use_second_type) ? data_struct->GetType2() : data_struct->GetType();
			if (use_second_type) {
				// Need to figure out why type2 always seems to be 205
				// since only items use type2 for now just hardcoded the value needed (BAD!!!)
				//type_to_use = DATA_STRUCT_SINT16; // 9;
				data_struct->SetType(type_to_use);
			}
			switch (type_to_use) {
			case DATA_STRUCT_INT8:
				setDataType(data_struct, (int8)data, index);
				break;
			case DATA_STRUCT_INT16:
				setDataType(data_struct, (int16)data, index);
				break;
			case DATA_STRUCT_INT32:
				setDataType(data_struct, (int32)data, index);
				break;
			case DATA_STRUCT_INT64:
				setDataType(data_struct, (int64)data, index);
				break;
			case DATA_STRUCT_SINT8:
				setDataType(data_struct, (sint8)data, index);
				break;
			case DATA_STRUCT_SINT16:
				setDataType(data_struct, (sint16)data, index);
				break;
			case DATA_STRUCT_SINT32:
				setDataType(data_struct, (sint32)data, index);
				break;
			case DATA_STRUCT_SINT64:
				setDataType(data_struct, (sint64)data, index);
				break;
			case DATA_STRUCT_CHAR:
				setDataType(data_struct, (char)data, index);
				break;
			case DATA_STRUCT_FLOAT:
				setDataType(data_struct, (float)data, index);
				break;
			case DATA_STRUCT_DOUBLE:
				setDataType(data_struct, (double)data, index);
				break;
			case DATA_STRUCT_COLOR:
				setColor(data_struct, *((EQ2_Color*)&data), index);
				break;
			case DATA_STRUCT_EQUIPMENT:
				setEquipmentByName(data_struct, *((EQ2_EquipmentItem*)&data), index);
				break;
			case DATA_STRUCT_ITEM:
				break;
			}
		}
	}

	template<class Data> void setSubArrayLengthByName(const char* name, Data data, int32 index1 = 0, int32 index2 = 0) {
		char tmp[10] = { 0 };
		sprintf(tmp, "_%i", index1);
		string name2 = string(name).append(tmp);
		DataStruct* data_struct = findStruct(name2.c_str(), index2);
		setData(data_struct, data, index2);
		UpdateArrayByArrayLength(data_struct, index2, data);
	}
	template<class Data> void setArrayLengthByName(const char* name, Data data, int32 index = 0) {
		DataStruct* data_struct = findStruct(name, index);
		setData(data_struct, data, index);
		UpdateArrayByArrayLength(data_struct, index, data);
	}
	template<class Data> void setSubstructArrayLengthByName(const char* substruct_name, const char* name, Data data, int32 substruct_index = 0, int32 index = 0) {
		char tmp[10] = { 0 };
		sprintf(tmp, "_%i", substruct_index);
		string name2 = string(substruct_name).append("_").append(name).append(tmp);

		DataStruct* data_struct = findStruct(name2.c_str(), index);
		setData(data_struct, data, index);
		UpdateArrayByArrayLength(data_struct, index, data);
	}
	void UpdateArrayByArrayLengthName(const char* name, int32 index, int32 size);
	void UpdateArrayByArrayLength(DataStruct* data_struct, int32 index, int32 size);
	bool StructLoadData(DataStruct* data_struct, void* data, int32 len, bool useType2 = false, bool create_color = false);
	bool LoadPacketData(uchar* data, int32 data_len, bool create_color = false);
	bool CheckFlagExists(const char* name);

	void setColorByName(const char* name, EQ2_Color* data, int32 index = 0) {
		if (data)
			setColorByName(name, data->red, data->green, data->blue, index);
	}
	void setColorByName(const char* name, EQ2_Color data, int32 index = 0) {
		setColorByName(name, data.red, data.green, data.blue, index);
	}
	void setColor(DataStruct* data_struct, EQ2_Color data, int32 index = 0) {
		if (data_struct) {
			EQ2_Color* ptr = (EQ2_Color*)struct_data[data_struct];
			ptr[index] = data;
		}
	}
	void setColorByName(const char* name, int8 red, int8 green, int8 blue, int32 index = 0) {
		setColor(findStruct(name, index), red, green, blue, index);
	}
	void setColor(DataStruct* data, int8 red, int8 green, int8 blue, int32 index);
	void setEquipmentByName(DataStruct* data_struct, EQ2_EquipmentItem data, int32 index = 0) {
		if (data_struct) {
			EQ2_EquipmentItem* ptr = (EQ2_EquipmentItem*)struct_data[data_struct];
			ptr[index] = data;
		}
	}
#ifdef WORLD	
	void setItem(DataStruct* ds, Item* item, Player* player, int32 index, sint8 offset = 0, bool loot_item = false, bool make_empty_item_packet = false, bool inspect = false);
	void setItemByName(const char* name, Item* item, Player* player, int32 index = 0, sint8 offset = 0, bool loot_item = false, bool make_empty_item_packet = false, bool inspect = false);
	void setItemArrayDataByName(const char* name, Item* item, Player* player, int32 index1 = 0, int32 index2 = 0, sint8 offset = 0, bool loot_item = false, bool make_empty_item_packet = false, bool inspect = false);
#endif
	void setEquipmentByName(const char* name, EQ2_EquipmentItem data, int32 index = 0) {
		setEquipmentByName(findStruct(name, index), data, index);
	}
	void setEquipmentByName(const char* name, EQ2_EquipmentItem* data, int32 size) {
		DataStruct* data_struct = findStruct(name, 0);
		if (data_struct) {
			for (int32 i = 0; i < size; i++)
				setEquipmentByName(data_struct, data[i], i);
		}
	}
	void setEquipmentByName(const char* name, int32 type, int8 c_red, int8 c_blue, int8 c_green, int8 h_red, int8 h_blue, int8 h_green, int32 index = 0) {
		setEquipment(findStruct(name, index), type, c_red, c_blue, c_green, h_red, h_blue, h_green, index);
	}
	void setEquipment(DataStruct* data, int16 type, int8 c_red, int8 c_blue, int8 c_green, int8 h_red, int8 h_blue, int8 h_green, int32 index);
	void remove(DataStruct* data);
	vector<DataStruct*>* getStructs() { return &structs; }
	DataStruct* findStruct(const char* name, int32 index);
	DataStruct* findStruct(const char* name, int32 index1, int32 index2);
	void remove(const char* name);
	void remove(int32 position);
	void serializePacket(bool clear = true);

	void AddSerializedData(DataStruct* data, int32 index = 0, string* datastring = 0);
	EQ2Packet* serialize();
	EQ2Packet* serializeCountPacket(int16 version, int8 offset = 0, uchar* orig_packet = 0, uchar* xor_packet = 0);
	string* serializeString();
	int32 GetVersion() { return version; }
	void SetVersion(int32 in_version) { version = in_version; }
	bool SetOpcode(const char* new_opcode);
	EmuOpcode GetOpcode() { return opcode; }
	int16 GetOpcodeValue(int16 client_version);
	const char* GetName() { return name.c_str(); }
	void SetName(const char* in_name) { name = string(in_name); }
	bool LoadedSuccessfully() { return loadedSuccessfully; }
	bool IsStringValueType(string in_name, int32 index);
	bool IsColorValueType(string in_name, int32 index);
	int32 GetTotalPacketSize();
	PacketStruct* GetPacketStructByName(const char* name);
	void* GetStructPointer(DataStruct* data_struct, bool erase = false);
	void PrintPacket();
	string GetSQLQuery(const char* table_name);
	vector<DataStruct*> GetDataStructs();
	void AddPackedData();
	void ResetData();
	void AddFlag(const char* name);

private:
	PacketStruct* parent;
	int32 sub_packet_size;
	string	opcode_type;
	bool sub_packet;
	bool loadedSuccessfully;
	string name;
	EmuOpcode opcode;
	int16 version;
	int16 client_version;
	vector<PacketStruct*> arrays;
	vector<string> flags;
	map<DataStruct*, void*> struct_data;
	map<int8, string> packed_data;
	map<string, DataStruct*> struct_map;
	vector<DataStruct*> structs;
	vector<DataStruct*> orig_structs;
	vector<PacketStruct*> orig_packets;
};
#endif