# File: `MutexHelper.h`

## Classes

- `IsPointer`
- `Locker`
- `IsPointer`
- `DeleteData`
- `HandleDeletes`

## Functions

- `void lock(){`
- `void unlock(){`
- `void SetData(int type, KeyT key, ValueT value, unsigned int time){`
- `void DeleteKey(){`
- `void DeleteValue(){`
- `int GetType(){`
- `void AddPendingDelete(T value, unsigned int time){`
- `void CheckDeletes(bool force = false){`

## Notable Comments

- /*
- */
- //	pthread_mutex_destroy(&CSMutex);
