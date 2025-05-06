# File: `packet_functions.h`

## Classes

_None detected_

## Functions

- `int32 roll(int32 in, int8 bits);`
- `int64 roll(int64 in, int8 bits);`
- `int32 rorl(int32 in, int8 bits);`
- `int64 rorl(int64 in, int8 bits);`
- `void EncryptProfilePacket(EQApplicationPacket* app);`
- `void EncryptProfilePacket(uchar* pBuffer, int32 size);`
- `void EncryptZoneSpawnPacket(EQApplicationPacket* app);`
- `void EncryptZoneSpawnPacket(uchar* pBuffer, int32 size);`
- `int DeflatePacket(unsigned char* in_data, int in_length, unsigned char* out_data, int max_out_length);`
- `uint32 InflatePacket(uchar* indata, uint32 indatalen, uchar* outdata, uint32 outdatalen, bool iQuiet = false);`
- `uint32 GenerateCRC(int32 b, int32 bufsize, uchar *buf);`
- `uint32 GenerateCRCRecipe(uint32 b, void* buf, uint32 bufsize);`

## Notable Comments

- /*
- */
- //void EncryptSpawnPacket(EQApplicationPacket* app);
- //void EncryptSpawnPacket(uchar* pBuffer, int32 size);
