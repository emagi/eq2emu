### Function: SetStepFailed(player, quest_id, step)

**Description:**
Set the player's quest step as failed.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `quest_id` (uint32) - Integer value `quest_id`.
- `step` (uint32) - Integer value `step`.

**Returns:** None.

**Example:**

```lua
-- From Quests/Hallmark/freeport_to_qeynos__part_2.lua
function Declined(Quest, QuestGiver, Player)
    FaceTarget(QuestGiver, Player)
	Dialog.New(QuestGiver, Player)   
 	Dialog.AddDialog("I thought I smelled a coward. I want nothing to do with you until you come to your senses.")
	Dialog.AddVoiceover("voiceover/english/watcher_kenjedeau/fprt_sewer02/watcher_kenjedeau005.mp3", 1178065540, 4141402431)
    PlayFlavor(QuestGiver, "", "", "glare", 0, 0, Player)
    Dialog.AddOption("Fine.")	
	Dialog.Start()
	if HasQuest(Player,5889) and GetQuestStep(Player,5889) ==1 then
	    SetStepFailed(Player,5889,1)
	end
```
