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
#ifndef LOG_H_
#define LOG_H_

#include <string.h>
#include "../WorldServer/client.h"

#define LOG_BUFFER_SIZE	4096

#define LOG_CATEGORY(category) LOG_ ##category ,
enum LogCategory
{
    #include "LogTypes.h"
    NUMBER_OF_LOG_CATEGORIES
};

#define LOG_TYPE(category, type, level, color, enabled, logfile, console, client, str) category##__##type ,
enum LogType
{
    #include "LogTypes.h"
    NUMBER_OF_LOG_TYPES
};

extern const char* log_category_names[NUMBER_OF_LOG_CATEGORIES];

struct LogTypeStatus
{
	int8 level;
	int color;
    bool enabled;
	bool logfile;
	bool console;
	bool client;
    LogCategory category;
    const char *name;
    const char *display_name;
};

extern LogTypeStatus* log_type_info;

void LogStart();
void LogStop();
int8 GetLoggerLevel(LogType type);
void LogWrite(LogType type, int8 log_level, const char *cat_text, const char *fmt, ...);
#ifdef PARSER
	void ColorizeLog(int color, char *date, const char *display_name, const char *category, string buffer);
#endif

bool LogParseConfigs();

#endif
