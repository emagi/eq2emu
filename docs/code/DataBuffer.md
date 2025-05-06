# File: `DataBuffer.h`

## Classes

- `DataBuffer`

## Functions

- `int32	getDataSize(){ return buffer.length(); }`
- `void	CreateEQ2Color(EQ2_Color& color){`
- `int32 GetLoadPos(){`
- `int32 GetLoadLen(){`
- `void SetLoadPos(int32 new_pos){`
- `void	CreateEQ2Color(EQ2_Color* color){`
- `void	InitializeGetData(){`
- `void	InitializeLoadData(uchar* input, int32 size){`
- `void LoadSkip(int8 bytes){`
- `void AddZeros(int16 num){`
- `void	AddCharArray(char* array, string* datastring = 0){`
- `void	AddCharArray(char* array, int16 size, string* datastring = 0){`
- `void	AddData(string data, string* datastring = 0){`
- `void	Clear() { buffer.clear(); }`

## Notable Comments

- /*
- */
- *tmp *= -1;
- *output = (Type*)get_buffer;
