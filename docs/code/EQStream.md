# File: `EQStream.h`

## Classes

- `SessionRequest`
- `SessionResponse`
- `ClientSessionStats`
- `ServerSessionStats`
- `OpcodeManager`
- `EQStreamFactory`
- `EQStream`

## Functions

- `char GetChar(uchar in);`
- `void WriteToFile(char* pFormat, ...);`
- `void WritePackets(const char* opcodeName, uchar* data, int32 size, bool outgoing);`
- `void WritePackets(EQ2Packet* app, bool outgoing);`
- `void SetLastSeqSent(uint32);`
- `long GetMaxAckReceived();`
- `long GetNextAckToSend();`
- `long GetLastAckSent();`
- `void SetMaxAckReceived(uint32 seq);`
- `void SetNextAckToSend(uint32);`
- `void SetLastAckSent(uint32);`
- `bool CheckCombineQueue();`
- `int8 EQ2_Compress(EQ2Packet* app, int8 offset = 3);`
- `int16	GetClientVersion(){ return client_version; }`
- `void	SetClientVersion(int16 version){ client_version = version; }`
- `void	ResetSessionAttempts() { reconnectAttempt = 0; }`
- `bool	HasSessionAttempts() { return reconnectAttempt>0; }`
- `void init(bool resetSession = true);`
- `void SetMaxLen(uint32 length) { MaxLen=length; }`
- `int8 getTimeoutDelays(){ return timeout_delays; }`
- `void addTimeoutDelay(){ timeout_delays++; }`
- `void EQ2QueuePacket(EQ2Packet* app, bool attempted_combine = false);`
- `void PreparePacket(EQ2Packet* app, int8 offset = 0);`
- `void UnPreparePacket(EQ2Packet* app);`
- `void EncryptPacket(EQ2Packet* app, int8 compress_offset, int8 offset);`
- `void FlushCombinedPacket();`
- `void SendPacket(EQApplicationPacket *p);`
- `void QueuePacket(EQProtocolPacket *p);`
- `void SendPacket(EQProtocolPacket *p);`
- `void NonSequencedPush(EQProtocolPacket *p);`
- `void SequencedPush(EQProtocolPacket *p);`
- `void CheckResend(int eq_fd);`
- `void AckPackets(uint16 seq);`
- `void Write(int eq_fd);`
- `void SetActive(bool val) { streamactive = val; }`
- `void WritePacket(int fd,EQProtocolPacket *p);`
- `void EncryptPacket(uchar* data, int16 size);`
- `uint32 GetKey() { return Key; }`
- `void SetKey(uint32 k) { Key=k; }`
- `void SetSession(uint32 s) { Session=s; }`
- `void SetLastPacketTime(uint32 t) {LastPacket=t;}`
- `void Process(const unsigned char *data, const uint32 length);`
- `void ProcessPacket(EQProtocolPacket *p, EQProtocolPacket* lastp=NULL);`
- `bool ProcessEmbeddedPacket(uchar* pBuffer, uint16 length, int8 opcode = OP_Packet);`
- `bool HandleEmbeddedPacket(EQProtocolPacket *p, int16 offset = 2, int16 length = 0);`
- `void SendSessionResponse();`
- `void SendSessionRequest();`
- `void SendDisconnect(bool setstate = true);`
- `void SendAck(uint16 seq);`
- `void SendOutOfOrderAck(uint16 seq);`
- `bool CheckTimeout(uint32 now, uint32 timeout=30) { return  (LastPacket && (now-LastPacket) > timeout); }`
- `bool Stale(uint32 now, uint32 timeout=30) { return  (LastPacket && (now-LastPacket) > timeout); }`
- `void InboundQueuePush(EQApplicationPacket *p);`
- `void InboundQueueClear();`
- `void OutboundQueueClear();`
- `bool HasOutgoingData();`
- `void SendKeyRequest();`
- `int16 processRSAKey(EQProtocolPacket *p, uint16 subpacket_length = 0);`
- `void RemoveData() { InboundQueueClear(); OutboundQueueClear(); if (CombinedAppPacket) delete CombinedAppPacket; }`
- `void Close() { SendDisconnect(); }`
- `bool CheckActive() { return (GetState()==ESTABLISHED); }`
- `bool CheckClosed() { return GetState()==CLOSED; }`
- `void SetOpcodeSize(uint8 s) { app_opcode_size = s; }`
- `void SetStreamType(EQStreamType t);`
- `void ProcessQueue();`
- `void Decay();`
- `void AdjustRates(uint32 average_delta);`

## Notable Comments

- /*
- */
- //Deltas are in ms, representing round trip times
- /*000*/	uint16 RequestID;
- /*002*/	uint32 last_local_delta;
- /*006*/	uint32 average_delta;
- /*010*/	uint32 low_delta;
- /*014*/	uint32 high_delta;
- /*018*/	uint32 last_remote_delta;
- /*022*/	uint64 packets_sent;
