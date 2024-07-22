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
#include "Log.h"
#include "xmlParser.h"
#include "types.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include "../WorldServer/World.h"
#include "../WorldServer/client.h"
#include "../WorldServer/zoneserver.h"

extern ZoneList zone_list;

#ifdef _WIN32
	#include <process.h>
	#ifndef snprintf
		#define snprintf sprintf_s
	#endif
#include <WinSock2.h>
	#include <Windows.h>
#else
#endif

#define LOG_CATEGORY(category) #category,
const char *log_category_names[NUMBER_OF_LOG_CATEGORIES] = {
	#include "LogTypes.h"
};

#define LOG_TYPE(category, type, level, color, enabled, logfile, console, client, str) { level, color, enabled, logfile, console, client, LOG_ ##category, #category "__" #type, ( strlen(str)>0 ) ? str : #category "__" #type },
static LogTypeStatus real_log_type_info[NUMBER_OF_LOG_TYPES+1] =
{
	#include "LogTypes.h"
	{ 0, 0, false, false, false, false, NUMBER_OF_LOG_CATEGORIES, "BAD TYPE", "Bad Name" } /* dummy trailing record */
};

LogTypeStatus *log_type_info = real_log_type_info;

//make these rules?
#define LOG_CYCLE		100		//milliseconds between each batch of log writes
#define LOGS_PER_CYCLE	50		//amount of logs to write per cycle

#define LOG_DIR	"logs"

#if defined LOGIN
#define EXE_NAME	"login"
#elif defined WORLD
#define EXE_NAME	"world"
#elif defined PARSER
#define EXE_NAME	"parser"
#elif defined PATCHER
#define EXE_NAME	"patcher"
#else
#define EXE_NAME	"whatprogyourunning"
#endif

#define DATE_MAX		8
#define LOG_NAME_MAX		32

struct logq_t {
	LogType log_type;
	char date[DATE_MAX + 1];
	char name[LOG_NAME_MAX + 1];
	char *text;
	struct logq_t *next;
	struct logq_t *prev;
};

//doubly linked list of logs
static logq_t head;
static logq_t tail;
static int num_logqs = 0;
static Mutex mlogqs;

//loop until....
static bool looping = false;

//because our code has LogWrite's before main(), make sure any of those do the
//call to LogStart if it hasn't been called already...
static bool start_called = false;

static void SetConsoleColor(int color) {
#ifdef _WIN32
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);

	if (handle == NULL || handle == INVALID_HANDLE_VALUE)
		return;
#endif

	switch (color) {
		case FOREGROUND_WHITE:
		case FOREGROUND_WHITE_BOLD:
		case FOREGROUND_RED:
		case FOREGROUND_RED_BOLD:
		case FOREGROUND_GREEN:
		case FOREGROUND_GREEN_BOLD:
		case FOREGROUND_BLUE:
		case FOREGROUND_BLUE_BOLD:
		case FOREGROUND_YELLOW:
		case FOREGROUND_YELLOW_BOLD:
		case FOREGROUND_CYAN:
		case FOREGROUND_CYAN_BOLD:
		case FOREGROUND_MAGENTA:
		case FOREGROUND_MAGENTA_BOLD:
#ifdef _WIN32
			SetConsoleTextAttribute(handle, color);
#else
			printf("\033[%i;%i;40m", color > 100 ? 1 : 0, color > 100 ? color - 100 : color);
#endif
			break;
		default:
#ifdef _WIN32
			SetConsoleTextAttribute(handle, FOREGROUND_WHITE_BOLD);
#else
			printf("\033[0;37;40m");
#endif
			break;
	}
}

static FILE * OpenLogFile() {
	char file[FILENAME_MAX + 1];
	struct stat st;
	struct tm *tm;
	time_t now;
	FILE *f;

	now = time(NULL);
	tm = localtime(&now);

	//make sure the logs directory exists
	if (stat(LOG_DIR, &st) != 0) {
#ifdef _WIN32
		if (!CreateDirectory(LOG_DIR, NULL)) {
			fprintf(stderr, "Unable to create directory '%s'\n", LOG_DIR);
			return stderr;
		}
#else
		if (mkdir(LOG_DIR, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
			fprintf(stderr, "Unable to create direcotry '%s': %s\n", LOG_DIR, strerror(errno));
			return stderr;
		}
#endif
	}

#ifdef NO_PIDLOG
	snprintf(file, FILENAME_MAX, LOG_DIR"/%04i-%02i-%02i_eq2" EXE_NAME ".log", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
#else
	snprintf(file, FILENAME_MAX, LOG_DIR"/%04i-%02i-%02i_eq2" EXE_NAME "_%04i.log", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, getpid());
#endif

	if ((f = fopen(file, "a")) == NULL) {
		fprintf(stderr, "Could not open '%s' for writing: %s\n", file, strerror(errno));
		return stderr;
	}

	return f;
}

static void WriteQueuedLogs(int count) {
	logq_t pending_head, pending_tail, *logq, *tmp;
	int i = 0;
	FILE *f;

	pending_head.next = &pending_tail;
	pending_tail.prev = &pending_head;

	//loop through our queued logs and store at most `count` logs into a temporary list
	//since io functions are expensive, we'll write from a temporary list so we don't hold the
	//write lock of the main list for a long period of time
	mlogqs.writelock();
	logq = head.next;

	while (head.next != &tail) {
		//first remove the log from the master list
		logq = head.next;
		logq->next->prev = &head;
		head.next = logq->next;

		//now insert it into the temporary list
		tmp = pending_tail.prev;
		tmp->next = logq;
		logq->prev = tmp;
		logq->next = &pending_tail;
		pending_tail.prev = logq;
		--num_logqs;

		logq = logq->next;

		//if we have a limit, check it
		if (count > 0 && ++i == count)
			break;	
	}

	//if we have no logs to write, we're done
	if ((logq = pending_head.next) == &pending_tail)
	{
		mlogqs.releasewritelock();
		return;
	}

	while (logq != &pending_tail) {
		if (log_type_info[logq->log_type].console) {
			SetConsoleColor(FOREGROUND_WHITE_BOLD);
			printf("%s ", logq->date);
			SetConsoleColor(log_type_info[logq->log_type].color);
			printf("%s ", log_type_info[logq->log_type].display_name);
			SetConsoleColor(FOREGROUND_WHITE_BOLD);
			printf("%-10s: ", logq->name);
			SetConsoleColor(log_type_info[logq->log_type].color);
			printf("%s\n", logq->text);
			SetConsoleColor(-1);
			fflush(stdout);
		}

		if (log_type_info[logq->log_type].logfile) {
			f = OpenLogFile();

			if (f != stderr || (f == stderr && !log_type_info[logq->log_type].console)) {
				fprintf(f, "%s %s %s: %s\n", logq->date, log_type_info[logq->log_type].display_name, logq->name, logq->text);
				fflush(f);
				if (f != stderr)
					fclose(f);
			}
		}

#if defined WORLD
		if (log_type_info[logq->log_type].client) {
			// eventually output logging to the client who "subscribed" to the logger
			// in-game, they type:
			//		/logsys add WORLD__DEBUG 5
			// to watch world debug loggers of level 5 or less
		}
#endif

		tmp = logq;
		logq = logq->next;

		mlogqs.releasewritelock();

		free(tmp->text);
		free(tmp);
	}
}

ThreadReturnType LogLoop(void *args) {
	while (looping) {
		WriteQueuedLogs(LOGS_PER_CYCLE);
		Sleep(LOG_CYCLE);
	}

	THREAD_RETURN(NULL);
}

void LogStart() {
	if (start_called)
		return;

	//initialize the doubly linked list
	head.prev = NULL;
	head.next = &tail;
	tail.prev = &head;
	tail.next = NULL;

	mlogqs.SetName("logqueue");
	looping = true;

#ifdef _WIN32
	_beginthread(LogLoop, 0, NULL);
#else
	pthread_t thread;
	pthread_create(&thread, NULL, LogLoop, NULL);
	pthread_detach(thread);
#endif

	start_called = true;
}

void LogStop() {
	looping = false;
	WriteQueuedLogs(-1);
	start_called = false;
}

static void LogQueueAdd(LogType log_type, char *text, int len, const char *cat_text = NULL) {
	logq_t *logq;
	struct tm *tm;
	time_t now;

	if ((logq = (logq_t *)calloc(1, sizeof(logq_t))) == NULL) {
		free(text);
		fprintf(stderr, "%s: %u: Unable to allocate %zu bytes\n", __FUNCTION__, __LINE__, sizeof(logq_t));
		return;
	}

	if ((logq->text = (char *)calloc(len + 1, sizeof(char))) == NULL) {
		free(text);
		free(logq);
		fprintf(stderr, "%s: %u: Unable to allocate %i bytes\n", __FUNCTION__, __LINE__, len + 1);
		return;
	}

	now = time(NULL);
	tm = localtime(&now);

	logq->log_type = log_type;
	snprintf(logq->date, DATE_MAX + 1, "%02i:%02i:%02i", tm->tm_hour, tm->tm_min, tm->tm_sec);
	strncpy(logq->name, cat_text == NULL || cat_text[0] == '\0' ? log_type_info[log_type].name : cat_text, LOG_NAME_MAX);
	strncpy(logq->text, text, len);
	free(text);

	if (!start_called)
		LogStart();

	//insert at the end
	mlogqs.writelock();
	tail.prev->next = logq;
	logq->prev = tail.prev;
	logq->next = &tail;
	tail.prev = logq;
	++num_logqs;
	mlogqs.releasewritelock();
}

int8 GetLoggerLevel(LogType type)
{
	return log_type_info[type].level;
}

// JA: horrific hack for Parser, since queued logging keeps crashing between parses.
#ifndef PARSER
void LogWrite(LogType type, int8 log_level, const char *cat_text, const char *fmt, ...) {
	int count, size = 64;
	char *buf;
	va_list ap;

	// if there is no formatting, or the logger is DISABLED
	// or the log_level param exceeds the minimum allowed value, abort logwrite
	if (!log_type_info[type].enabled || (log_level > 0 && log_type_info[type].level < log_level))
		return;

	while (true) {
		if ((buf = (char *)malloc(size)) == NULL) {
			fprintf(stderr, "%s: %i: Unable to allocate %i bytes\n", __FUNCTION__, __LINE__, size);
			return;
		}
		
		va_start(ap, fmt);
		count = vsnprintf(buf, size, fmt, ap);
		va_end(ap);

		if (count > -1 && count < size)
			break;

		free(buf);
		if (count > 1)
			size = count + 1;
		else
			size *= 2;
	}

	LogQueueAdd(type, buf, count, cat_text);
}
#else
void LogWrite(LogType type, int8 log_level, const char *cat_text, const char *format, ...) 
{
	// if there is no formatting, or the logger is DISABLED
	// or the log_level param exceeds the minimum allowed value, abort logwrite
	if ( !format || !log_type_info[type].enabled || (log_level > 0 && log_type_info[type].level < log_level) )
		return;

    time_t clock;
    struct tm *tm;

	char buffer[LOG_BUFFER_SIZE], date[32];
	va_list args;
	FILE *f;
	size_t cat_text_len = 0;

	memset(buffer, 0, sizeof(buffer));
	memset(date, 0, sizeof(date));

	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer) - 1, format, args);
	va_end(args);

    time(&clock);
    tm = localtime(&clock);
	snprintf(date, sizeof(date)-1, "%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
	// DateString(date, sizeof(date));

	cat_text_len = strlen(cat_text);
	//if( strlen(cat_text) == 0 ) // cat_text was blank
	//	cat_text = (char*)log_type_info[type].name;

	/* write to the log file? */
	if (log_type_info[type].logfile) 
	{
		char exename[200] = "";

	#ifdef LOGIN
		snprintf(exename, sizeof(exename), "login");
	#endif
	#ifdef WORLD
		snprintf(exename, sizeof(exename), "world");
	#endif
	#ifdef PARSER
		snprintf(exename, sizeof(exename), "parser");
	#endif
	#ifdef PATCHER
		snprintf(exename, sizeof(exename), "patcher");
	#endif

		char filename[200], log_header[200] = "";

	#ifndef NO_PIDLOG
		snprintf(filename, sizeof(filename)-1, "logs/%04d-%02d-%02d_eq2%s_%04i.log", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, exename, getpid());
	#else
		snprintf(filename, sizeof(filename)-1, "logs/%04d-%02d-%02d_eq2%s.log", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, exename);
	#endif

		f=fopen(filename, "r");
		if( !f )
			snprintf(log_header, sizeof(log_header), "===[ New log '%s' started ]===\n\n", filename);
		else
		fclose (f);

		f = fopen(filename, "a");
		if (f) {
			if( strlen(log_header) > 0 )
				fprintf(f, "%s\n", log_header);
			fprintf(f, "%s %s %s: %s\n", date, log_type_info[type].display_name, cat_text, buffer);
			fclose(f);
		}
	}
	
	/* write to the console? */
	if (log_type_info[type].console)
	{
	#ifdef _WIN32
			ColorizeLog(log_type_info[type].color, date, log_type_info[type].display_name, cat_text_len == 0 ? log_type_info[type].name : cat_text, (string)buffer);
	#else
				printf("%s %s %s: %s\n", date, log_type_info[type].display_name, cat_text_len == 0 ? log_type_info[type].name : cat_text, buffer);
	#endif
	}
}

void
ColorizeLog(int color, char *date, const char *display_name, const char *category, string buffer)
{
	#ifdef _WIN32
		HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
		if (console == INVALID_HANDLE_VALUE) {
			printf("%s %s %s: %s\n", date, display_name, category, buffer);
			return;
		}
		printf("%s ", date);
		SetConsoleTextAttribute(console, color);
		printf("%s ", display_name);
		SetConsoleTextAttribute(console, FOREGROUND_WHITE_BOLD);
		printf("%s: ", category);
		SetConsoleTextAttribute(console, color);
		printf("%s\n", buffer.c_str());
		SetConsoleTextAttribute(console, FOREGROUND_WHITE);
	#endif
}

#endif

LogTypeStatus *
GetLogTypeStatus(const char *category, const char *type) {
	char combined[256];
	int i;

	memset(combined, 0, sizeof(combined));
	snprintf(combined, sizeof(combined) - 1, "%s__%s", category, type);

	for (i = 0; i < NUMBER_OF_LOG_TYPES; i++) {
		if (strcasecmp(log_type_info[i].name, combined) == 0)
			return &log_type_info[i];
	}

	return &log_type_info[NUMBER_OF_LOG_TYPES];
}

void
ProcessLogConfig(XMLNode node) {
	int i;
	const char *category, *type, *level, *color, *enabled, *logs;
	LogTypeStatus *lfs;
	XMLNode child;

	category = node.getAttribute("Category");
	if (!category) {
		LogWrite(MISC__WARNING, 0, "Misc", "Error parsing log config. Config missing a Category");
		return;
	}

	for (i = 0; i < node.nChildNode("ConfigType"); i++) {
		child = node.getChildNode("ConfigType", i);
		type = child.getAttribute("Type");

		if (!type) {
			LogWrite(MISC__WARNING, 0, "Misc", "Error parsing log config. Config missing a Type");
			continue;
		}

		lfs = GetLogTypeStatus(category, type);
		level = child.getAttribute("Level");
		enabled = child.getAttribute("Enabled");
		color = child.getAttribute("Color");
		logs = child.getAttribute("Logs");

		if (!logs) {
			LogWrite(MISC__WARNING, 0, "Misc", "Error parsing log config. Config missing 'Logs' attribute to specify which log(s) to write to");
			continue;
		}
		if (!IsNumber(logs)) {
			LogWrite(MISC__WARNING, 0, "Misc", "Error parsing log config. Attribute 'Logs' must be a number. See LogTypes.h for the valid types.");
			continue;
		}

		if (enabled) {
			if (!strcasecmp("true", enabled) || !strcasecmp("on", enabled))
				lfs->enabled = true;
			else if (!strcasecmp("false", enabled) || !strcasecmp("off", enabled))
				lfs->enabled = false;
			else
				LogWrite(MISC__WARNING, 0, "Misc", "Error parsing log config. Log setting 'Enabled' has invalid value '%s'. 'true'/'on' or 'false'/'off' are valid values", enabled);
		}

		if (IsNumber(level))
			lfs->level = atoi(level);
		else
			lfs->level = 0;

		if (color) {
			if (IsNumber(color))
				lfs->color = atoi(color);
			else if (!strcasecmp("White", color))
				lfs->color = FOREGROUND_WHITE;
			else if (!strcasecmp("Green", color))
				lfs->color = FOREGROUND_GREEN;
			else if (!strcasecmp("Yellow", color))
				lfs->color = FOREGROUND_YELLOW;
			else if (!strcasecmp("Red", color))
				lfs->color = FOREGROUND_RED;
			else if (!strcasecmp("Blue", color))
				lfs->color = FOREGROUND_BLUE;
			else if (!strcasecmp("Cyan", color))
				lfs->color = FOREGROUND_CYAN;
			else if (!strcasecmp("Magenta", color))
				lfs->color = FOREGROUND_MAGENTA;
			else if (!strcasecmp("WhiteBold", color))
				lfs->color = FOREGROUND_WHITE_BOLD;
			else if (!strcasecmp("GreenBold", color))
				lfs->color = FOREGROUND_GREEN_BOLD;
			else if (!strcasecmp("YellowBold", color))
				lfs->color = FOREGROUND_YELLOW_BOLD;
			else if (!strcasecmp("RedBold", color))
				lfs->color = FOREGROUND_RED_BOLD;
			else if (!strcasecmp("BlueBold", color))
				lfs->color = FOREGROUND_BLUE_BOLD;
			else if (!strcasecmp("CyanBold", color))
				lfs->color = FOREGROUND_CYAN_BOLD;
			else if (!strcasecmp("MagentaBold", color))
				lfs->color = FOREGROUND_MAGENTA_BOLD;
			else
				LogWrite(MISC__WARNING, 0, "Misc", "Error parsing log config. Log setting 'Color' has invalid value '%s'", color);
		}

		// JA: something was wrong here, lfs->logfile or console always was true, even if bit was off. Will ask Scatman about it someday.
		lfs->logfile	= (atoi(logs) & LOG_LOGFILE);
		lfs->console	= (atoi(logs) & LOG_CONSOLE);
		lfs->client		= (atoi(logs) & LOG_CLIENT);
	}
}

bool
LogParseConfigs() {
	XMLNode main_node;
	int i;

	main_node = XMLNode::openFileHelper("log_config.xml", "EQ2EmuLogConfigs");
	if (main_node.isEmpty()) {
		LogWrite(MISC__WARNING, 0, "Misc", "Unable to parse the file 'log_config.xml' or it does not exist. Default values will be used");
		return false;
	}

	for (i = 0; i < main_node.nChildNode("LogConfig"); i++)
		ProcessLogConfig(main_node.getChildNode("LogConfig", i));

	return true;
}
