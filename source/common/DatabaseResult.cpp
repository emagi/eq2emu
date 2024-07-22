/*  
    EQ2Emulator:  Everquest II Server Emulator
    Copyright (C) 2007  EQ2EMulator Development Team (http://www.eq2emulator.net)

    This file is part of EQ2Emulator.

    EQ2Emulator is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    EQ2Emulator is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with EQ2Emulator.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdlib.h>
#include <string.h>
#include "Log.h"
#include "DatabaseResult.h"

//enforced by MySQL...couldn't find a #define in their headers though
#define FIELD_NAME_MAX	64

//return this instead of NULL for certain functions to prevent crashes from coding errors
static const char *empty_str = "";

DatabaseResult::DatabaseResult(): field_map(), result(0), num_fields(0), row(0) {
}

DatabaseResult::~DatabaseResult() {
	unsigned int i;

	if (result != NULL)
		mysql_free_result(result);

	if (field_map.size()) {
		field_map.clear();
	}
}

bool DatabaseResult::StoreResult(MYSQL_RES* res, uint8 field_count, uint8 row_count) {

	//clear any previously stored result
	if (result != NULL)
		mysql_free_result(result);

	//clear any field names from a previous result
	if (field_map.size()) {
		field_map.clear();
	}

	result = res;
	num_rows = row_count;	
	num_fields = field_count;

	// No rows or fields then we don't care
	if (!num_rows || !num_fields) {
		mysql_free_result(res);
		result = NULL;
		return false;
	}

	
	const MYSQL_FIELD* fields = mysql_fetch_fields(result);

	for (uint8 i = 0; i < num_fields; ++i) {
		field_map.emplace(std::make_pair(std::string_view(fields[i].name), i));
	}
	
	return true;
}

const char * DatabaseResult::GetFieldValue(unsigned int index) {
	if (index >= num_fields) {
		LogWrite(DATABASE__ERROR, 0, "Database Result", "Attempt to access field at index %u but there %s only %u field%s", index, num_fields == 1 ? "is" : "are", num_fields, num_fields == 1 ? "" : "s");
		return NULL;
	}

	return row[index];
}

const char * DatabaseResult::GetFieldValueStr(const char *field_name) {
	const auto& map_iterator = field_map.find(std::string_view(field_name));
	if (map_iterator != field_map.end()) {
		return row[map_iterator->second];
	}

	LogWrite(DATABASE__ERROR, 0, "Database Result", "Unknown field name '%s'", field_name);
	return NULL;
}

bool DatabaseResult::Next() {
	return (result != NULL && (row = mysql_fetch_row(result)) != NULL);
}

bool DatabaseResult::IsNull(unsigned int index) {
	const char *value = GetFieldValue(index);
	return value == NULL;
}

bool DatabaseResult::IsNullStr(const char *field_name) {
	const char *value = GetFieldValueStr(field_name);
	return value == NULL;
}

int8 DatabaseResult::GetInt8(unsigned int index) {
	const char *value = GetFieldValue(index);
	return value == NULL ? 0 : atoi(value);
}

int8 DatabaseResult::GetInt8Str(const char *field_name) {
	const char *value = GetFieldValueStr(field_name);
	return value == NULL ? 0 : atoi(value);
}

sint8 DatabaseResult::GetSInt8(unsigned int index) {
	const char *value = GetFieldValue(index);
	return value == NULL ? 0 : atoi(value);
}

sint8 DatabaseResult::GetSInt8Str(const char *field_name) {
	const char *value = GetFieldValueStr(field_name);
	return value == NULL ? 0 : atoi(value);
}

int16 DatabaseResult::GetInt16(unsigned int index) {
	const char *value = GetFieldValue(index);
	return value == NULL ? 0 : atoi(value);
}

int16 DatabaseResult::GetInt16Str(const char *field_name) {
	const char *value = GetFieldValueStr(field_name);
	return value == NULL ? 0 : atoi(value);
}

sint16 DatabaseResult::GetSInt16(unsigned int index) {
	const char *value = GetFieldValue(index);
	return value == NULL ? 0 : atoi(value);
}

sint16 DatabaseResult::GetSInt16Str(const char *field_name) {
	const char *value = GetFieldValueStr(field_name);
	return value == NULL ? 0 : atoi(value);
}

int32 DatabaseResult::GetInt32(unsigned int index) {
	const char *value = GetFieldValue(index);
	return value == NULL ? 0U : strtoul(value, NULL, 10);
}

int32 DatabaseResult::GetInt32Str(const char *field_name) {
	const char *value = GetFieldValueStr(field_name);
	return value == NULL ? 0U : strtoul(value, NULL, 10);
}

sint32 DatabaseResult::GetSInt32(unsigned int index) {
	const char *value = GetFieldValue(index);
	return value == NULL ? 0 : atoi(value);
}

sint32 DatabaseResult::GetSInt32Str(const char *field_name) {
	const char *value = GetFieldValueStr(field_name);
	return value == NULL ? 0 : atoi(value);
}

uint64 DatabaseResult::GetInt64(unsigned int index) {
	const char *value = GetFieldValue(index);
#ifdef _WIN32
	return value == NULL ? 0UL : _strtoui64(value, NULL, 10);
#else
	return value == NULL ? 0UL : strtoull(value, NULL, 10);
#endif
}

uint64 DatabaseResult::GetInt64Str(const char *field_name) {
	const char *value = GetFieldValueStr(field_name);
#ifdef _WIN32
	return value == NULL ? 0UL : _strtoui64(value, NULL, 10);
#else
	return value == NULL ? 0UL : strtoull(value, NULL, 10);
#endif
}

sint64 DatabaseResult::GetSInt64(unsigned int index) {
	const char *value = GetFieldValue(index);
#ifdef _WIN32
	return value == NULL ? 0L : _strtoi64(value, NULL, 10);
#else
	return value == NULL ? 0L : strtoll(value, NULL, 10);
#endif
}

sint64 DatabaseResult::GetSInt64Str(const char *field_name) {
	const char *value = GetFieldValueStr(field_name);
#ifdef _WIN32
	return value == NULL ? 0L : _strtoi64(value, NULL, 10);
#else
	return value == NULL ? 0L : strtoll(value, NULL, 10);
#endif
}

float DatabaseResult::GetFloat(unsigned int index) {
	const char *value = GetFieldValue(index);
	return value == NULL ? 0.0F : atof(value);
}

float DatabaseResult::GetFloatStr(const char *field_name) {
	const char *value = GetFieldValueStr(field_name);
	return value == NULL ? 0.0F : atof(value);
}

char DatabaseResult::GetChar(unsigned int index) {
	const char *value = GetFieldValue(index);
	return value == NULL ? '\0' : value[0];
}

char DatabaseResult::GetCharStr(const char *field_name) {
	const char *value = GetFieldValueStr(field_name);
	return value == NULL ? '\0' : value[0];
}

const char * DatabaseResult::GetString(unsigned int index) {
	const char *value = GetFieldValue(index);
	return value == NULL ? empty_str : value;
}

const char * DatabaseResult::GetStringStr(const char *field_name) {
	const char *value = GetFieldValueStr(field_name);
	return value == NULL ? empty_str : value;
}
