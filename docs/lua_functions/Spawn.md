### Function: Spawn(zone, spawn_id, x, y, z, heading)

**Description:**
Create's a new Spawn from the spawn database table based on the spawn_id (id parameter in the database).

**Parameters:**
- `zone` (Zone) - Zone object representing `zone`.
- `spawn_id` (uint32) - Integer value `spawn_id`.
- `x` (int32) - Integer value `x`.
- `y` (int32) - Integer value `y`.
- `z` (int32) - Integer value `z`.
- `heading` (int32) - Integer value `heading`.

**Returns:** None.

**Example:**

```lua
-- From Quests/QueensColony/the_source_of_evil.lua
function step3_complete_talkedToSorcerer(Quest, QuestGiver, Player)
	UpdateQuestStepDescription(Quest, 3, "I spoke with Sorcerer Oofala.")
	UpdateQuestTaskGroupDescription(Quest, 3, "I spoke with Sorcerer Oofala.")
    --Spawn(GetZone(Player), 2530142, false, GetX(Player), GetY(Player), GetZ(Player))
    --Spawn the Dark Blademaster once you approach his location only if the player has a quest (zonescript)
    --x = 154.66    y = 1.38397    z = -178.158    heading = 349.984
	AddQuestStepKill(Quest, 4, "Kill the Dark Blademaster near the fog on Sapswill Hill.", 1, 100, "Oofala explained that by removing the totems, the evil has been disturbed and produced a champion to fight back. I will need to confront the Dark Blademaster. He has emerged from a tent on Sapswill Hill.", 611, 2530032) 
	AddQuestStepCompleteAction(Quest, 4, "step4_complete_killedBlademaster")
end
```
