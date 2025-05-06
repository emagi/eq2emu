# File: `MiscFunctions.h`

## Classes

- `InitWinsock`
- `VersionRange`

## Functions

- `int32	hextoi(char* num);`
- `int64	hextoi64(char* num);`
- `sint32	filesize(FILE* fp);`
- `int32	ResolveIP(const char* hostname, char* errbuf = 0);`
- `void	CoutTimestamp(bool ms = true);`
- `string	loadInt32String(uchar* buffer, int16 buffer_size, int16* pos, EQ2_32BitString* eq_string = NULL);`
- `string	loadInt16String(uchar* buffer, int16 buffer_size, int16* pos, EQ2_16BitString* eq_string = NULL);`
- `string	 loadInt8String(uchar* buffer, int16 buffer_size, int16* pos, EQ2_8BitString* eq_string = NULL);`
- `sint16  storeInt32String(uchar* buffer, int16 buffer_size, string in_str);`
- `sint16  storeInt16String(uchar* buffer, int16 buffer_size, string in_str);`
- `sint16   storeInt8String(uchar* buffer, int16 buffer_size, string in_str);`
- `int		MakeRandomInt(int low, int high);`
- `float	MakeRandomFloat(float low, float high);`
- `float TransformToFloat(sint16 data, int8 bits);`
- `sint16 TransformFromFloat(float data, int8 bits);`
- `int32	GenerateEQ2Color(float r, float g, float b);`
- `int32	GenerateEQ2Color(float* rgb[3]);`
- `void	SetColor(EQ2_Color* color, long data);`
- `int8	MakeInt8(uchar* data, int16* size);`
- `int8	MakeInt8(float* input);`
- `bool	Unpack(int32 srcLen, uchar* data, uchar* dst, int16 dstLen, int16 version = 0, bool reverse = true);`
- `bool	Unpack(uchar* data, uchar* dst, int16 dstLen, int16 version = 0, bool reverse = true);`
- `int32	Pack(uchar* data, uchar* src, int16 srcLen, int16 dstLen, int16 version = 0, bool reverse = true);`
- `void	Reverse(uchar* input, int32 srcLen);`
- `void	Encode(uchar* dst, uchar* src, int16 len);`
- `void	Decode(uchar* dst, uchar* src, int16 len);`
- `string	ToUpper(string input);`
- `string	ToLower(string input);`
- `int32 ParseIntValue(string input);`
- `int64 ParseLongLongValue(string input);`
- `void	MovementDecode(uchar* dst, uchar* newval, uchar* orig, int16 len);`
- `int8 DoOverLoad(int32 val, uchar* data);`
- `int8 CheckOverLoadSize(int32 val);`
- `int32	CountWordsInString(const char* text);`
- `bool IsNumber(const char *num);`
- `void PrintSep(Seperator *sep, const char *name = 0);`
- `string GetDeviceName(string device);`
- `int32 GetDeviceID(string device);`
- `int16 GetItemPacketType(int32 version);`
- `int16 GetOpcodeVersion(int16 version);`
- `void SleepMS(int32 milliseconds);`
- `size_t strlcpy(char *dst, const char *src, size_t size);`
- `float short_to_float(const ushort x);`
- `uint32 float_to_int(const float x);`
- `uint32 as_uint(const float x);`
- `float as_float(const uint32 x);`
- `int64 getCurrentTimestamp();`
- `bool INIReadBool(FILE *f, const char *section, const char *property, bool *out);`
- `bool INIReadInt(FILE *f, const char *section, const char *property, int *out);`
- `void init(T** iVar, T* iSetTo = 0)`
- `int32 GetMinVersion() { return min_version; }`
- `int32 GetMaxVersion() { return max_version; }`

## Notable Comments

- /*
- */
- //int		MakeAnyLenString(char** ret, const char* format, ...);
- //char*	strn0cpy(char* dest, const char* source, int32 size);
- // return value =true if entire string(source) fit, false if it was truncated
- //bool	strn0cpyt(char* dest, const char* source, int32 size);
- //void	CreateEQ2Color(EQ2_Color* color, uchar* data, int16* size);
- ///<summary>Gets the packet type for the given version</summary>
- ///<param name='version'>The client version</param>
- ///<summary>Gets the opcode version_range1 from the clients version</summary>
