### Function: UpdateQuestZone(quest, zone)

**Description:**
Update the quest's zone setting.

**Parameters:**
- `quest` (Quest) - Quest object representing `quest`.
- `zone` (Zone) - Zone object representing `zone`.

**Returns:** None.

**Example:**

```lua
-- From Quests/Antonica/a_foul_wind.lua
function Init(Quest)
	SetQuestFeatherColor(Quest, 3)
	SetQuestRepeatable(Quest)
	UpdateQuestZone(Quest,"Firemyst Gully: A Foul Wind")
	AddQuestStepZoneLoc(Quest, 1, "I need to seek out Firemyst Gully in the center of Antonica.", 10, "I need to seek out Lord Nalin at Firemyst Gully in eastern Antonica.", 2183, -1188.04, -13.62, 706.57,14)
	AddQuestStepCompleteAction(Quest, 1, "QuestComplete")
end
```
