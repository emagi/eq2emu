# File: `debug.h`

## Classes

- `EQEMuLog`
- `PerformanceMonitor`

## Functions

- `void CatchSignal(int);`
- `typedef void (* msgCallbackBuf)(LogIDs id, const char *buf, int8 size, int32 count);`
- `typedef void (* msgCallbackFmt)(LogIDs id, const char *fmt, va_list ap);`
- `void SetAllCallbacks(msgCallbackFmt proc);`
- `void SetAllCallbacks(msgCallbackBuf proc);`
- `void SetCallback(LogIDs id, msgCallbackFmt proc);`
- `void SetCallback(LogIDs id, msgCallbackBuf proc);`
- `bool writebuf(LogIDs id, const char *buf, int8 size, int32 count);`
- `bool write(LogIDs id, const char *fmt, ...);`
- `bool Dump(LogIDs id, int8* data, int32 size, int32 cols=16, int32 skip=0);`
- `bool open(LogIDs id);`
- `bool writeNTS(LogIDs id, bool dofile, const char *fmt, ...); // no error checking, assumes is open, no locking, no timestamp, no newline`

## Notable Comments

- /*
- */
- // Debug Levels
- /*
- */
- //#ifndef _CRTDBG_MAP_ALLOC
- //#endif
- // VS6 doesn't like the length of STL generated names: disabling
- //these are callbacks called for each
- /* LogStatus: bitwise variable
