### Function: AddQuestStepLocation(quest, step, description, max_variation, str_taskgroup, icon, dest_x, dest_y, dest_z)

**Description:**
Adds a quest step to the player's journal requiring a location be reached for the complete action to be performed.  The dest_x, dest_y, dest_z can be repeated in sequence to add multiple potential locations to reach.

**Parameters:**
- `quest` (Quest) - Quest object representing `quest`.
- `step` (int32) - Integer value `step`.
- `description` (int32) - Integer value `description`.
- `max_variation` (int32) - Integer value `max_variation`.
- `str_taskgroup` (string) - String `str_taskgroup`.
- `icon` (int32) - Integer value `icon`.
- `dest_x` (float) - Float value `dest_x`.
- `dest_y` (float) - Float value `dest_y`.
- `dest_z` (float) - Float value `dest_z`.

**Returns:** None.

**Example:**

```lua
-- From Quests/Antonica/history_of_the_ayrdal_part_i.lua
function Init(Quest)
	AddQuestStepLocation(Quest, 1, "I need to visit Glade of the Coven.", 10, "I would like to visit the Glade of the Coven in Antonica.", 11, 160, -24, 441)
	AddQuestStepCompleteAction(Quest, 1, "Step1Complete")
end
```
