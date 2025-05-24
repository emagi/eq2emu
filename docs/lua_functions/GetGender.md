### Function: GetGender(spawn)

**Description:**
Gets the gender of the current spawn.  0 = Female, 1 = Male

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** None.

**Example:**

```lua
-- From Quests/BeggarsCourt/an_errand_for_the_queen.lua
function Accepted(Quest, QuestGiver, Player)
	FaceTarget(QuestGiver, Player)
	Dialog.New(QuestGiver, Player)
	Dialog.AddDialog("Well, you're late, good man.  I've been puffing up my cheeks and snorting loudly, hoping you'd find me.  My entourage deserted me, and now you must execute them.  Go find these ogres and kill them; they hide in the Sprawl and call themselves Giantslayer Bashers.  Now, go child.  Queenly blessings to you!")
	Dialog.AddVoiceover("voiceover/english/tullia_domna/fprt_hood04/quests/tulladomna/tulla_x1_accept.mp3", 2208976682, 3386849948)
	PlayFlavor(QuestGiver, "", "", "scold", 0, 0, Player, 0)
	Dialog.AddOption("At once, my 'Queen'.")
	if GetGender(Player)== 2 then
	Dialog.AddOption("I'm not a man, but ...err, Yes, my 'Queen'.")
	end    
```
