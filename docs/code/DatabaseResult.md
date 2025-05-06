# File: `DatabaseResult.h`

## Classes

- `DatabaseResult`

## Functions

- `bool StoreResult(MYSQL_RES* result, uint8 field_count, uint8 row_count);`
- `bool Next();`
- `bool IsNull(unsigned int index);`
- `bool IsNullStr(const char *field_name);`
- `int8 GetInt8(unsigned int index);`
- `int8 GetInt8Str(const char *field_name);`
- `sint8 GetSInt8(unsigned int index);`
- `sint8 GetSInt8Str(const char *field_name);`
- `int16 GetInt16(unsigned int index);`
- `int16 GetInt16Str(const char *field_name);`
- `sint16 GetSInt16(unsigned int index);`
- `sint16 GetSInt16Str(const char *field_name);`
- `int32 GetInt32(unsigned int index);`
- `int32 GetInt32Str(const char *field_name);`
- `sint32 GetSInt32(unsigned int index);`
- `sint32 GetSInt32Str(const char *field_name);`
- `int64 GetInt64(unsigned int index);`
- `int64 GetInt64Str(const char *field_name);`
- `sint64 GetSInt64(unsigned int index);`
- `sint64 GetSInt64Str(const char *field_name);`
- `float GetFloat(unsigned int index);`
- `float GetFloatStr(const char *field_name);`
- `char GetChar(unsigned int index);`
- `char GetCharStr(const char *field_name);`

## Notable Comments

_None detected_
