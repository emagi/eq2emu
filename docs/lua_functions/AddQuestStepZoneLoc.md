### Function: AddQuestStepZoneLoc(quest, step, description, max_variation, str_taskgroup, icon, dest_x, dest_y, dest_z, dest_zone_id)

**Description:**
Adds a quest step to the Player's journal requiring a new destination location be reached and can also supply a different zone id.  The dest parameters can be repeated for multiple possible locations.

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
- `dest_zone_id` (int32) - Integer value `dest_zone_id`.

**Returns:** None.

**Example:**

```lua
-- From Quests/AlphaTest/stowaway_antonica.lua
function Init(Quest)
	AddQuestStepZoneLoc(Quest, 1, "I must ride to Antonica...", 12, "The Shady Swashbuckler provided me passage to Antonica. He will have the paperwork I need when I get there.", 2285, 395.79, -38.56, 809.38,12)
	AddQuestStepCompleteAction(Quest, 1, "Step1Complete")
end
```
