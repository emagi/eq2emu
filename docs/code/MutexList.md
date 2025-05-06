# File: `MutexList.h`

## Classes

- `MutexList`
- `iterator`

## Functions

- `bool HasNext(){`
- `bool Next(){`
- `return Next();`
- `void SetChanging(){`
- `void SetNotChanging(){`
- `void AddAccess(){`
- `void RemoveAccess(){`
- `iterator begin(){`
- `return iterator(this);`
- `void clear(bool erase_all = false){`
- `bool PendingContains(T key){`
- `return count(key); //only occurs whenever we change to changing state at the same time as a reading state`
- `void RemoveData(T key, int32 erase_time = 0){`
- `void Remove(T key, bool erase = false, int32 erase_time = 0){`
- `void Add(T key){`
- `bool update(bool force = false){`

## Notable Comments

- /*
- */
- /*if(list.has_pending_data)
- //if(access_count > 5)
- //	cout << "Possible error.\n";
