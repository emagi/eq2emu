# File: `Log.h`

## Classes

- `LogTypeStatus`

## Functions

- `void LogStart();`
- `void LogStop();`
- `int8 GetLoggerLevel(LogType type);`
- `void LogWrite(LogType type, int8 log_level, const char *cat_text, const char *fmt, ...);`
- `void ColorizeLog(int color, char *date, const char *display_name, const char *category, string buffer);`
- `bool LogParseConfigs();`

## Notable Comments

- /*
- */
