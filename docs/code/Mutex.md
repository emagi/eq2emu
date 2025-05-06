# File: `Mutex.h`

## Classes

- `CriticalSection`
- `Mutex`
- `LockMutex`

## Functions

- `void lock();`
- `void unlock();`
- `bool trylock();`
- `void lock();`
- `void unlock();`
- `bool trylock();`
- `void readlock(const char* function = 0, int32 line = 0);`
- `void releasereadlock(const char* function = 0, int32 line = 0);`
- `bool tryreadlock(const char* function = 0);`
- `void writelock(const char* function = 0, int32 line = 0);`
- `void releasewritelock(const char* function = 0, int32 line = 0);`
- `bool trywritelock(const char* function = 0);`
- `void waitReaders(const char* function = 0, int32 line = 0);`
- `void SetName(string in_name);`
- `void unlock();`
- `void lock();`

## Notable Comments

- /*
- */
