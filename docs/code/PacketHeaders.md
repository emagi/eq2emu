# File: `PacketHeaders.h`

## Classes

- `CharSelectProfile`
- `LS_CharSelectList`
- `LS_DeleteCharacterRequest`

## Functions

- `void				SaveData(int16 in_version);`
- `void				Data();`
- `void					addChar(uchar* data, int16 size);`
- `void					loadData(int32 account, vector<CharSelectProfile*> charlist, int16 version);`
- `void			loadData(EQApplicationPacket* packet);`

## Notable Comments

- /*
- */
