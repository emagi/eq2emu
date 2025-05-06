# File: `Languages.h`

## Classes

- `Language`
- `MasterLanguagesList`
- `PlayerLanguagesList`

## Functions

- `void		SetID(int32 id) {this->id = id;}`
- `void		SetName(const char *name) {strncpy(this->name, name, sizeof(this->name));}`
- `void		SetSaveNeeded(bool save_needed) {this->save_needed = save_needed;}`
- `int32		GetID() {return id;}`
- `bool		GetSaveNeeded() {return save_needed;}`
- `void	Clear();`
- `int32	Size();`
- `void	AddLanguage(Language* language);`
- `void Clear();`
- `void Add(Language* language);`

## Notable Comments

- /*
- */
