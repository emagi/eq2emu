# File: `Appearances.h`

## Classes

- `Appearance`
- `Appearances`

## Functions

- `int32 GetID() { return id; }`
- `int16 GetMinClientVersion() { return min_client; }`
- `string GetNameString() { return name; }`
- `void Reset(){`
- `void ClearAppearances(){`
- `void InsertAppearance(Appearance* a){`

## Notable Comments

- /*
- */
- // Appearances must use a hash table because of the large amount that exists and the large spacing
- // between their ID's.  String and character arrays could not be used for the first iterator because
- // it would require the same pointer to access it from the hash table, which is obviously not possible
- // since the text is from the client.
- // maximum amount of iterations it will attempt to find a entree
- // JA: someday add the min_client_version to the map to determine which appearance_id to set per client version
