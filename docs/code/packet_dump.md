# File: `packet_dump.h`

## Classes

- `ServerPacket`

## Functions

- `void DumpPacketAscii(const uchar* buf, int32 size, int32 cols=16, int32 skip=0);`
- `void DumpPacketHex(const uchar* buf, int32 size, int32 cols=16, int32 skip=0);`
- `void DumpPacketBin(const void* data, int32 len);`
- `void DumpPacket(const uchar* buf, int32 size);`
- `void DumpPacket(const ServerPacket* pack, bool iShowInfo = false);`
- `void DumpPacketBin(const ServerPacket* pack);`
- `void DumpPacketBin(int32 data);`
- `void DumpPacketBin(int16 data);`
- `void DumpPacketBin(int8 data);`

## Notable Comments

- /*
- */
