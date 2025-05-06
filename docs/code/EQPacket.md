# File: `EQPacket.h`

## Classes

- `OpcodeManager`
- `EQStream`
- `EQPacket`
- `EQApplicationPacket`
- `EQProtocolPacket`
- `EQ2Packet`
- `EQApplicationPacket`

## Functions

- `void DumpRawHeader(uint16 seq=0xffff, FILE *to = stdout) const;`
- `void DumpRawHeaderNoTime(uint16 seq=0xffff, FILE *to = stdout) const;`
- `void DumpRaw(FILE *to = stdout) const;`
- `void setVersion(int16 new_version){ version = new_version; }`
- `void setSrcInfo(uint32 sip, uint16 sport) { src_ip=sip; src_port=sport; }`
- `void setDstInfo(uint32 dip, uint16 dport) { dst_ip=dip; dst_port=dport; }`
- `void setTimeInfo(uint32 ts_sec, uint32 ts_usec) { timestamp.tv_sec=ts_sec; timestamp.tv_usec=ts_usec; }`
- `void copyInfo(const EQPacket *p) { src_ip=p->src_ip; src_port=p->src_port;  dst_ip=p->dst_ip; dst_port=p->dst_port; timestamp.tv_sec=p->timestamp.tv_sec; timestamp.tv_usec=p->timestamp.tv_usec; }`
- `uint32 Size() const { return size+2; }`
- `uint16 GetRawOpcode() const { return(opcode); }`
- `void SetProtocolOpcode(int16 new_opcode){`
- `bool combine(const EQProtocolPacket *rhs);`
- `uint32 serialize (unsigned char *dest, int8 offset = 0) const;`
- `bool AppCombine(EQ2Packet* rhs);`
- `int8 PreparePacket(int16 MaxLen);`
- `bool combine(const EQApplicationPacket *rhs);`
- `uint32 serialize (unsigned char *dest) const;`
- `uint32 Size() const { return size+app_opcode_size; }`
- `void SetOpcodeSize(uint8 s) { app_opcode_size=s; }`
- `void SetOpcode(EmuOpcode op);`
- `void DumpPacketHex(const EQApplicationPacket* app);`
- `void DumpPacket(const EQProtocolPacket* app);`
- `void DumpPacketAscii(const EQApplicationPacket* app);`
- `void DumpPacket(const EQApplicationPacket* app, bool iShowInfo = false);`
- `void DumpPacketBin(const EQApplicationPacket* app);`

## Notable Comments

- /*
- */
- //no reason to have this method in zone or world
- //bool dont_combine;
- //caching version of get
- //this is just a cache so we dont look it up several times on Get()
- //this constructor should only be used by EQProtocolPacket, as it
- //assumes the first two bytes of buf are the opcode.
