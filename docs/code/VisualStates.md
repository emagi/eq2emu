# File: `VisualStates.h`

## Classes

- `VisualState`
- `Emote`
- `EmoteVersionRange`
- `VisualStates`

## Functions

- `int GetID() { return id; }`
- `string GetNameString() { return name; }`
- `int32 GetVisualState() { return visual_state; }`
- `string GetNameString() { return name; }`
- `string GetMessageString() { return message; }`
- `string GetTargetedMessageString() { return targeted_message; }`
- `else if (range->GetMinVersion() <= min_version && range->GetMaxVersion() == 0)`
- `else if (range->GetMinVersion() == 0 && max_version <= range->GetMaxVersion())`
- `string GetNameString() { return name; }`
- `void Reset(){`
- `void ClearEmotes(){`
- `void ClearVisualStates(){`
- `void InsertVisualState(VisualState* vs){`
- `void InsertEmoteRange(EmoteVersionRange* emote) {`
- `void InsertSpellVisualRange(EmoteVersionRange* emote, int32 spell_visual_id) {`
- `void ClearSpellVisuals(){`

## Notable Comments

- /*
- */
- // Visual States must use a hash table because of the large amount that exists and the large spacing
- // between their ID's.  String and character arrays could not be used for the first iterator because
- // it would require the same pointer to access it from the hash table, which is obviously not possible
- // since the text is from the client.
- // maximum amount of iterations it will attempt to find a entree
- // if min and max version are both in range
- // if the min version is in range, but max range is 0
- // if min version is 0 and max_version has a cap
