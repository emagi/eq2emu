### Function: GetCurrentHP(spawn)

**Description:**
Returns the current HP of the Spawn, this can be represented as GetHP(spawn) as well.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** SInt32 current hp value of the spawn.

**Example:**

```lua
-- From SpawnScripts/GreaterFaydark/grobins.lua
function healthchanged(NPC)

	if GetCurrentHP(NPC) <= (GetMaxHP(NPC) / 2) then
		choice = math.random(1,4)
		if choice == 1 then
			PlayFlavor(NPC, "voiceover/english/exp03_combatvo/goblin_greater_faydark/ft/_exp03/goblin/goblin_greater_faydark_battle_25d9a433.mp3", "Grum! Grum! ", "", 1460066353, 1003945639, Spawn)
		elseif choice == 2 then
			PlayFlavor(NPC, "voiceover/english/exp03_combatvo/goblin_greater_faydark/ft/_exp03/goblin/goblin_greater_faydark_battle_4e5ee4ae.mp3", "Smash the squishies.", "", 3016834030, 2330929155, Spawn)
		elseif choice == 3 then
			PlayFlavor(NPC, "voiceover/english/exp03_combatvo/goblin_greater_faydark/ft/_exp03/goblin/goblin_greater_faydark_battle_603b0f3b.mp3", "Run away from the mines!", "", 861506750, 2339330363, Spawn)
		elseif choice == 4 then
			PlayFlavor(NPC, "voiceover/english/exp03_combatvo/goblin_greater_faydark/ft/_exp03/goblin/goblin_greater_faydark_battle_cf61b767.mp3", "Groblin's go!", "", 1309387887, 223459313, Spawn)
		else
		-- say nothing
		end
```
