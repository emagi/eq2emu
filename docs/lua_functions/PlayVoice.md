### Function: PlayVoice(spawn, mp3_string, key1, key2, player)

**Description:**
Spawn plays a mp3 voiceover with no text bubble.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `mp3_string` (string) - String `mp3_string`.
- `key1` (int32) - Integer value `key1`.
- `key2` (int32) - Integer value `key2`.
- `player` (Spawn) - Spawn object representing `player`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/Antonica/TheSageofAges.lua
function RandomGreeting(NPC, Spawn)
	local choice = MakeRandomInt(1,3)

	if choice == 1 then
		PlayVoice(NPC, "voiceover/english/voice_emotes/greetings/greetings_2_1022.mp3", 0, 0, Spawn)
	elseif choice == 2 then
		PlayVoice(NPC, "voiceover/english/voice_emotes/greetings/greetings_3_1022.mp3", 0, 0, Spawn)
	elseif choice == 3 then
		PlayVoice(NPC, "voiceover/english/voice_emotes/greetings/greetings_1_1022.mp3", 0, 0, Spawn)
	end
```
