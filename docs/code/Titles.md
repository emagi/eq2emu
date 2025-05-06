# File: `Titles.h`

## Classes

- `Title`
- `MasterTitlesList`
- `PlayerTitlesList`

## Functions

- `void			SetID(int32 id) {this->id = id;}`
- `void			SetName(const char *name) {strncpy(this->name, name, sizeof(this->name));}`
- `void			SetPrefix(int8 prefix) {this->prefix = prefix;}`
- `void			SetSaveNeeded(bool save_needed) {this->save_needed = save_needed;}`
- `sint32			GetID() {return id;}`
- `int8			GetPrefix() {return prefix;}`
- `bool			GetSaveNeeded() {return save_needed;}`
- `void Clear();`
- `int32 Size();`
- `void AddTitle(Title* title);`
- `void Add(Title* title);`
- `int32 Size();`
- `void ReleaseReadLock() { MPlayerTitleMutex.releasereadlock(); }`

## Notable Comments

- /*
- */
