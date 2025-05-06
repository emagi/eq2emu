# File: `Factions.h`

## Classes

- `Faction`
- `MasterFactionList`
- `PlayerFaction`

## Functions

- `void Clear() {`
- `sint32 GetDefaultFactionValue(int32 faction_id){`
- `void AddFaction(Faction* faction){`
- `sint32 GetIncreaseAmount(int32 faction_id){`
- `sint32 GetDecreaseAmount(int32 faction_id){`
- `int32 GetFactionCount(){`
- `void AddHostileFaction(int32 faction_id, int32 hostile_faction_id){`
- `void AddFriendlyFaction(int32 faction_id, int32 friendly_faction_id){`
- `sint32		GetMaxValue(sint8 con);`
- `sint32		GetMinValue(sint8 con);`
- `sint32		GetFactionValue(int32 faction_id);`
- `bool		ShouldIncrease(int32 faction_id);`
- `bool		ShouldDecrease(int32 faction_id);`
- `bool		IncreaseFaction(int32 faction_id, int32 amount = 0);`
- `bool		DecreaseFaction(int32 faction_id, int32 amount = 0);`
- `bool		SetFactionValue(int32 faction_id, sint32 value);`
- `sint8		GetCon(int32 faction_id);`
- `int8		GetPercent(int32 faction_id);`
- `bool		ShouldAttack(int32 faction_id);`

## Notable Comments

- /*
- */
