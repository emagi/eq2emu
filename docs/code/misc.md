# File: `misc.h`

## Classes

_None detected_

## Functions

- `void Unprotect(string &s, char what);`
- `void Protect(string &s, char what);`
- `bool ItemParse(const char *data, int length, map<int,map<int,string> > &items, int id_pos, int name_pos, int max_field, int level=0);`
- `int Tokenize(string s, map<int,string> & tokens, char delim='|');`
- `void LoadItemDBFieldNames();`
- `void encode_length(unsigned long length, char *out);`
- `void decode(char *in, char *out);`
- `void encode_chunk(char *in, int len, char *out);`
- `void decode_chunk(char *in, char *out);`
- `int Deflate(unsigned char* in_data, int in_length, unsigned char* out_data, int max_out_length);`
- `int Inflate(unsigned char* indata, int indatalen, unsigned char* outdata, int outdatalen, bool iQuiet=true);`
- `int print_stacktrace();`
- `bool alpha_check(unsigned char val);`
- `void dump_message_column(unsigned char *buffer, unsigned long length, string leader="", FILE *to = stdout);`
- `string string_from_time(string pattern, time_t now=0);`
- `string timestamp(time_t now=0);`
- `string long2ip(unsigned long ip);`
- `string pop_arg(string &s, string seps, bool obey_quotes);`
- `int EQsprintf(char *buffer, const char *pattern, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5, const char *arg6, const char *arg7, const char *arg8, const char *arg9);`
- `int GetItemNameCrc(string item_name);`

## Notable Comments

- /*
- */
