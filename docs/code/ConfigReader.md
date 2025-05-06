# File: `ConfigReader.h`

## Classes

- `ConfigReader`

## Functions

- `void addStruct(const char* name, int16 version, PacketStruct* new_struct);`
- `void loadDataStruct(PacketStruct* packet, XMLNode parentNode, bool array_packet = false);`
- `bool processXML_Elements(const char* fileName);`
- `int16 GetStructVersion(const char* name, int16 version);`
- `void DestroyStructs();`
- `void ReloadStructs();`
- `bool LoadFile(const char* name);`

## Notable Comments

- /*
- */
- //vector<PacketStruct*> structs;
