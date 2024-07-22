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
#include "ConfigReader.h"
#include "Log.h"

ConfigReader::~ConfigReader(){
	MStructs.lock();
	DestroyStructs();
	MStructs.unlock();
}
PacketStruct* ConfigReader::getStructByVersion(const char* name, int16 version){
	PacketStruct* packet = 0;
	PacketStruct* newpacket = 0;
	MStructs.lock();
	vector<PacketStruct*>* struct_versions = structs[string(name)];
    if(struct_versions){
            vector<PacketStruct*>::iterator iter;
            for(iter = struct_versions->begin(); iter != struct_versions->end(); iter++){
                    packet = *iter;
					if(packet && packet->GetVersion() == version){
                            newpacket = new PacketStruct(packet, version);
							break;
					}
            }
    }
	MStructs.unlock();
	if(!newpacket)
		LogWrite(PACKET__ERROR, 0, "Packet", "Could not find struct named '%s' with version: %i", name, version);
    return newpacket;
}
void ConfigReader::ReloadStructs(){
	MStructs.lock();
	DestroyStructs();
	for(int32 i=0;i<load_files.size();i++)
		processXML_Elements(load_files[i].c_str());
	MStructs.unlock();
}
void ConfigReader::DestroyStructs(){
	map<string, vector<PacketStruct*>*>::iterator struct_iterator;
	for(struct_iterator=structs.begin();struct_iterator!=structs.end();struct_iterator++) {
		vector<PacketStruct*>* versions = struct_iterator->second;
		vector<PacketStruct*>::iterator version_iter;
		if(versions){
			for(version_iter = versions->begin(); version_iter != versions->end(); version_iter++){
				safe_delete(*version_iter);
			}
		}
		safe_delete(versions);
	}
	structs.clear();
}
PacketStruct* ConfigReader::getStruct(const char* name, int16 version){
	PacketStruct* latest_version = 0;
	PacketStruct* new_latest_version = 0;
	MStructs.lock();
	vector<PacketStruct*>* struct_versions = structs[string(name)];
	if(struct_versions){
		vector<PacketStruct*>::iterator iter;
		for(iter = struct_versions->begin(); iter != struct_versions->end(); iter++){
			if((*iter)->GetVersion() <= version && (!latest_version || (*iter)->GetVersion() > latest_version->GetVersion()))
				latest_version = *iter;
		}		
		if (latest_version) {
			if (latest_version->GetOpcode() != OP_Unknown && (latest_version->GetOpcodeValue(version) == 0xFFFF || latest_version->GetOpcodeValue(version)==0xCDCD)) {
				LogWrite(PACKET__ERROR, 0, "Packet", "Could not find valid opcode for Packet Struct '%s' and client version %d", latest_version->GetName(), version);
			}
			else if(strlen(latest_version->GetOpcodeType()) == 0 || latest_version->GetOpcode() != OP_Unknown)
				new_latest_version = new PacketStruct(latest_version, version);
		}
			
	}
	MStructs.unlock();
	if(!new_latest_version && !latest_version)
		LogWrite(PACKET__ERROR, 0, "Packet", "Could not find struct named '%s'", name);
	return new_latest_version;
}
int16 ConfigReader::GetStructVersion(const char* name, int16 version){
	MStructs.lock();
	vector<PacketStruct*>* struct_versions = structs[string(name)];
	int16 ret = 0;
	if(struct_versions){
		vector<PacketStruct*>::iterator iter;
		PacketStruct* latest_version = 0;
		for(iter = struct_versions->begin(); iter != struct_versions->end(); iter++){
			if(!latest_version || ( (*iter)->GetVersion() > latest_version->GetVersion() && (*iter)->GetVersion() <= version) )
				latest_version = *iter;
		}
		if(latest_version)
			ret = latest_version->GetVersion();
	}
	MStructs.unlock();
	return ret;
}
void ConfigReader::addStruct(const char* name, int16 version, PacketStruct* new_struct){
	string strname(name);
	vector<PacketStruct*>* struct_versions = structs[strname];
	if(struct_versions)
		struct_versions->push_back(new_struct);
	else{
		struct_versions = new vector<PacketStruct*>;
		struct_versions->push_back(new_struct);
		structs[strname] = struct_versions;
	}
}
bool ConfigReader::LoadFile(const char* name){
	load_files.push_back(name);
	return processXML_Elements(name);
}
bool ConfigReader::processXML_Elements(const char* fileName){
	XMLNode xMainNode=XMLNode::openFileHelper(fileName,"EQ2Emulator");
	if(xMainNode.isEmpty())
		return false;
	for(int i=0;i<xMainNode.nChildNode("Struct");i++){
		const char* struct_name = xMainNode.getChildNode("Struct", i).getAttribute("Name");
		const char* str_version = xMainNode.getChildNode("Struct", i).getAttribute("ClientVersion");
		const char* opcode_name = xMainNode.getChildNode("Struct", i).getAttribute("OpcodeName");
		const char* opcode_type = xMainNode.getChildNode("Struct", i).getAttribute("OpcodeType");
		if(!struct_name || !str_version)
		{
			LogWrite(MISC__WARNING, 0, "Misc", "Ignoring invalid struct, all structs must include at least a Name and ClientVersion!");
			continue;
		}
		int16 version = 1;
		try
		{
			version = atoi(str_version);
		}
		catch(...)
		{
			LogWrite(MISC__WARNING, 0, "Misc", "Ignoring invalid version for struct named '%s': '%s'", struct_name, str_version);
			continue;
		}
		PacketStruct* new_struct = new PacketStruct();
		new_struct->SetName(struct_name);
		if(opcode_type)
			new_struct->SetOpcodeType(opcode_type);
		if(opcode_name){
			if(!new_struct->SetOpcode(opcode_name)){
				safe_delete(new_struct);
				continue;
			}	
		}
		new_struct->SetVersion(version);
		loadDataStruct(new_struct, xMainNode.getChildNode("Struct", i));
		addStruct(struct_name, version, new_struct);
	}
	return true;
}
void ConfigReader::loadDataStruct(PacketStruct* packet, XMLNode parentNode, bool array_packet){
	for(int x=0;x<parentNode.nChildNode();x++){
			const char* name = parentNode.getChildNode("Data", x).getAttribute("ElementName");
			const char* type = parentNode.getChildNode("Data", x).getAttribute("Type");
			const char* size = parentNode.getChildNode("Data", x).getAttribute("Size");
			const char* type2 = parentNode.getChildNode("Data", x).getAttribute("Type2");
			const char* array_size = parentNode.getChildNode("Data", x).getAttribute("ArraySizeVariable");
			const char* max_array = parentNode.getChildNode("Data", x).getAttribute("MaxArraySize");
			const char* substruct = parentNode.getChildNode("Data", x).getAttribute("Substruct");
			const char* default_value = parentNode.getChildNode("Data", x).getAttribute("DefaultByteValue");
			const char* oversized = parentNode.getChildNode("Data", x).getAttribute("OversizedValue");
			const char* oversized_byte = parentNode.getChildNode("Data", x).getAttribute("OversizedByte");
			const char* if_variable = parentNode.getChildNode("Data", x).getAttribute("IfVariableSet");
			const char* if_not_variable = parentNode.getChildNode("Data", x).getAttribute("IfVariableNotSet");
			const char* if_equals_variable = parentNode.getChildNode("Data", x).getAttribute("IfVariableEquals");
			const char* if_not_equals_variable = parentNode.getChildNode("Data", x).getAttribute("IfVariableNotEquals");
			const char* optional = parentNode.getChildNode("Data", x).getAttribute("Optional");
			const char* if_flag_not_set_variable = parentNode.getChildNode("Data", x).getAttribute("IfFlagNotSet");
			const char* if_flag_set_variable = parentNode.getChildNode("Data", x).getAttribute("IfFlagSet");

			//const char* type2criteria = parentNode.getChildNode("Data", x).getAttribute("Type2Criteria");	// JA: LE added to PacketAnalyzer for Items parsing - 12.2012
			//const char* criteria = parentNode.getChildNode("Data", x).getAttribute("Criteria");				// JA: LE added to PacketAnalyzer for Items parsing - 12.2012

			int8 max_array_size = 0;
			try{
				if(max_array)
					max_array_size = atoi(max_array);
			}
			catch(...){}

			int16 num_size = 1;
			try{
				if(size)
					num_size = atoi(size);
			}
			catch(...){}
			int8 byte_val = 0;
			try{
				if(default_value)
					byte_val = atoi(default_value);
			}
			catch(...){}
			if(substruct && name){
				PacketStruct* substruct_packet = getStruct(substruct, packet->GetVersion());
				if(substruct_packet){
					vector<DataStruct*>::iterator itr;
					vector<DataStruct*>* structs = substruct_packet->getStructs();
					DataStruct* ds = 0;
					int i = 0;
					char tmp[12] = {0};
					for(i=0;i<num_size;i++){
						snprintf(tmp, sizeof(tmp)-1, "%i", i);
						for(itr=structs->begin();itr!=structs->end();itr++) {
							ds = *itr;
							string new_name;
							if(array_packet)
								new_name = string(name).append("_").append(ds->GetStringName());
							else
								new_name = string(name).append("_").append(ds->GetStringName()).append("_").append(tmp);

							DataStruct* ds2 = new DataStruct(new_name.c_str(), ds->GetType(),ds->GetLength(), ds->GetType2());

							if(!array_packet && strlen(ds->GetArraySizeVariable()) > 1)
									ds2->SetArraySizeVariable(string(name).append("_").append(ds->GetArraySizeVariable()).append("_").append(tmp).c_str());
							ds2->SetOversized(ds->GetOversized());
							ds2->SetOversizedByte(ds->GetOversizedByte());
							ds2->SetDefaultValue(ds->GetDefaultValue());
							ds2->SetMaxArraySize(ds->GetMaxArraySize());
							ds2->SetIfSetVariable(ds->GetIfSetVariable() ? ds->GetIfSetVariable() : if_variable);
							ds2->SetIfNotSetVariable(ds->GetIfSetVariable() ? ds->GetIfNotSetVariable() : if_not_variable);
							ds2->SetIfNotEqualsVariable(ds->GetIfNotEqualsVariable());
							ds2->SetIfFlagNotSetVariable(ds->GetIfFlagNotSetVariable());
							ds2->SetIfFlagSetVariable(ds->GetIfFlagSetVariable());
							ds2->SetIsOptional(ds->IsOptional());
							ds2->AddIfSetVariable(if_variable); //add this if the modifier is on the piece that is including the substruct
							ds2->AddIfNotSetVariable(if_not_variable); //add this if the modifier is on the piece that is including the substruct
							packet->add(ds2);
						}
					}
					if(!array_packet){
						i--;
						substruct_packet->renameSubstructArray(name, i);
						//ds2->SetArraySizeVariable((char*)string(name).append("_").append(ds->GetArraySizeVariable()).append("_").append(tmp).c_str());
						packet->addPacketArrays(substruct_packet);
					}
					
					safe_delete(substruct_packet);
				}
				continue;
			}
			else if(type && strncasecmp(type,"Array", 5)==0 && array_size){
				PacketStruct* new_packet = new PacketStruct;
				new_packet->SetName(name);
				new_packet->IsSubPacket(true);
				new_packet->SetVersion(packet->GetVersion());
				loadDataStruct(new_packet, parentNode.getChildNode("Data", x), true);
				packet->add(new_packet);
			}
			if(!name || !type)
			{
				LogWrite(MISC__WARNING, 0, "Misc", "Ignoring invalid Data Element, all elements must include at least an ElementName and Type!");
				LogWrite(MISC__WARNING, 0, "Misc", "\tStruct: '%s', version: %i", parentNode.getAttribute("Name"), parentNode.getAttribute("ClientVersion"));
				continue;
			}
			DataStruct* ds = new DataStruct(name, type, num_size, type2);
			int8 oversized_value = 0;
			int8 oversized_byte_value = 255;
			if(oversized){
				try{
					oversized_value = atoi(oversized);
				}
				catch(...){}
			}
			if(oversized_byte){
					try{
					oversized_byte_value = atoi(oversized_byte);
				}
				catch(...){}
			}
			ds->SetOversizedByte(oversized_byte_value);
			ds->SetOversized(oversized_value);
			ds->SetMaxArraySize(max_array_size);
			if(array_size)
				ds->SetArraySizeVariable(array_size);
			ds->SetDefaultValue(byte_val);
			ds->SetIfSetVariable(if_variable);
			ds->SetIfNotSetVariable(if_not_variable);
			ds->SetIfEqualsVariable(if_equals_variable);
			ds->SetIfNotEqualsVariable(if_not_equals_variable);
			ds->SetIfFlagNotSetVariable(if_flag_not_set_variable);
			ds->SetIfFlagSetVariable(if_flag_set_variable);
			if (optional && strlen(optional) > 0 && (strcmp("true", optional) == 0 || strcmp("TRUE", optional) == 0 || strcmp("True", optional) == 0))
				ds->SetIsOptional(true);
			packet->add(ds);
		}
}

