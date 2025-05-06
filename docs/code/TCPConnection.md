# File: `TCPConnection.h`

## Classes

- `TCPServer`
- `TCPConnection`
- `TCPNetPacket_Struct`
- `TCPServer`

## Functions

- `void TCPServerLoop(void* tmp);`
- `void TCPConnectionLoop(void* tmp);`
- `bool			Connect(char* irAddress, int16 irPort, char* errbuf = 0);`
- `bool			Connect(int32 irIP, int16 irPort, char* errbuf = 0);`
- `void			AsyncConnect(char* irAddress, int16 irPort);`
- `void			AsyncConnect(int32 irIP, int16 irPort);`
- `bool			Send(const uchar* data, sint32 size);`
- `eTCPMode		GetMode()			{ return TCPMode; }`
- `void			Free();		// Inform TCPServer that this connection object is no longer referanced`
- `bool			GetEcho();`
- `void			SetEcho(bool iValue);`
- `void			SetState(int8 iState);`
- `bool			CheckNetActive();`
- `bool			RunLoop();`
- `bool	GetAsyncConnect();`
- `bool	SetAsyncConnect(bool iValue);`
- `void	OutQueuePush(ServerPacket* pack);`
- `void	RemoveRelay(TCPConnection* relay, bool iSendRelayDisconnect);`
- `void	ProcessNetworkLayerPacket(ServerPacket* pack);`
- `void	SendNetErrorPacket(const char* reason = 0);`
- `bool SendData(char* errbuf = 0);`
- `bool RecvData(char* errbuf = 0);`
- `bool ProcessReceivedData(char* errbuf = 0);`
- `bool ProcessReceivedDataAsPackets(char* errbuf = 0);`
- `bool ProcessReceivedDataAsOldPackets(char* errbuf = 0);`
- `void ClearBuffers();`
- `void	LineOutQueuePush(char* line);`
- `void	InModeQueuePush(TCPNetPacket_Struct* tnps);`
- `bool	ServerSendQueuePop(uchar** data, sint32* size);`
- `void	ServerSendQueuePushEnd(const uchar* data, sint32 size);`
- `void	ServerSendQueuePushEnd(uchar** data, sint32 size);`
- `void	ServerSendQueuePushFront(uchar* data, sint32 size);`
- `bool	Open(int16 iPort = 0, char* errbuf = 0);			// opens the port`
- `void	Close();						// closes the port`
- `bool	IsOpen();`
- `void	SendPacket(ServerPacket* pack);`
- `void	SendPacket(TCPConnection::TCPNetPacket_Struct** tnps);`
- `void	Process();`
- `bool	RunLoop();`
- `void			AddConnection(TCPConnection* con);`
- `void	ListenNewConnections();`
- `void	CheckInQueue();`

## Notable Comments

- /*
- */
- /*
- */
- // Functions for outgoing connections
