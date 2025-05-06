# File: `string_util.h`

## Classes

_None detected_

## Functions

- `std::vector<std::string> split(std::string str_to_split, char delimiter);`
- `std::string implode(std::string glue, std::vector<std::string> src);`
- `return ltrim(rtrim(str, chars), chars);`
- `std::string implode(const std::string &glue, const std::pair<char, char> &encapsulation, const std::vector<T> &src)`
- `std::string output(oss.str());`
- `std::vector<std::string> join_pair(const std::string &glue, const std::pair<char, char> &encapsulation, const std::vector<std::pair<T1, T2>> &src)`
- `std::vector<std::string> join_tuple(const std::string &glue, const std::pair<char, char> &encapsulation, const std::vector<std::tuple<T1, T2, T3, T4>> &src)`
- `std::vector<std::string> SplitString(const std::string &s, char delim);`
- `std::string EscapeString(const char *src, size_t sz);`
- `std::string EscapeString(const std::string &s);`
- `bool StringIsNumber(const std::string &s);`
- `void ToLowerString(std::string &s);`
- `void ToUpperString(std::string &s);`
- `std::string JoinString(const std::vector<std::string>& ar, const std::string &delim);`
- `void find_replace(std::string& string_subject, const std::string& search_string, const std::string& replace_string);`
- `void ParseAccountString(const std::string &s, std::string &account, std::string &loginserver);`
- `bool atobool(const char* iBool);`
- `bool isAlphaNumeric(const char *text);`
- `bool strn0cpyt(char* dest, const char* source, uint32 size);`
- `int MakeAnyLenString(char** ret, const char* format, ...);`
- `uint32 AppendAnyLenString(char** ret, uint32* bufsize, uint32* strlen, const char* format, ...);`
- `uint32 hextoi(const char* num);`
- `uint64 hextoi64(const char* num);`
- `void MakeLowerString(const char *source, char *target);`
- `void RemoveApostrophes(std::string &s);`

## Notable Comments

- /*
- * Copyright 2013 Facebook, Inc.
- *
- * Licensed under the Apache License, Version 2.0 (the "License");
- * you may not use this file except in compliance with the License.
- * You may obtain a copy of the License at
- *
- *   http://www.apache.org/licenses/LICENSE-2.0
- *
- * Unless required by applicable law or agreed to in writing, software
