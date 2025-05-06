# File: `MutexMap.h`

## Classes

- `MutexMap`
- `iterator`

## Functions

- `bool HasNext(){`
- `bool Next(){`
- `return Next();`
- `int count(KeyT key, bool include_pending = false){`
- `void clear(bool delete_all = false){`
- `void deleteData(KeyT key, int8 type, int32 erase_time = 0){`
- `void erase(KeyT key, bool erase_key = false, bool erase_value = false, int32 erase_time = 0){`
- `iterator begin(){`
- `return iterator(this);`
- `void Put(KeyT key, ValueT value){`
- `void AddAccess(){`
- `void RemoveAccess(){`
- `void SetChanging(){`
- `void SetNotChanging(){`
- `void update(bool force = false){`

## Notable Comments

- /*
- */
