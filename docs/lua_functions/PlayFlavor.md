### Function: PlayFlavor(spawn, mp3_string, text_string, emote_string, key1, key2, player, language)

**Description:**
Allows Player/NPC to display a text bubble and/or mp3 voiceovers (with usually required key1/key2 values).

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `mp3_string` (string) - String `mp3_string`.
- `text_string` (string) - String `text_string`.
- `emote_string` (string) - String `emote_string`.
- `key1` (int32) - Integer value `key1`.
- `key2` (int32) - Integer value `key2`.
- `player` (Spawn) - Spawn object representing `player`.
- `language` (int32) - Integer value `language`.

**Returns:** None.

**Example:**

```lua
-- From ItemScripts/AcommemorativeFreeportCoin.lua
function examined(Item, Player)
conversation = CreateConversation()
PlayFlavor(Player,"voiceover/english/tullia_domna/fprt_hood04/quests/tulladomna/tulla_x1_initial.mp3","","",309451026,621524268,Player)
--PlayFlavor(Player,"voiceover/english/overlord_lucan_d_lere/fprt_west/lucan_isle_speech.mp3","","",2912329438, 4090300715,Player)

	local choice = MakeRandomInt(1,6)

	if choice == 1 then
		PlayFlavor(Player, "voiceover/english/overlord_lucan_d_lere/fprt_west/lucan_isle_speech_4.mp3", "You show potential, but there are many who seek the auspices of Lucan, and I only have time for champions.", "", 2060818628, 3998142234, Player, 0)
	elseif choice == 2 then
		PlayFlavor(Player, "voiceover/english/overlord_lucan_d_lere/fprt_west/lucan_isle_speech_5.mp3", "Prove yourself, and I shall grant you shelter at the edge of my city, and the chance to earn your place as a proud citizen of Freeport.", "", 4115014723, 2723692261, Player, 0)
	elseif choice == 3 then
		PlayFlavor(Player, "voiceover/english/overlord_lucan_d_lere/fprt_west/lucan_isle_speech_8.mp3", "Together we will restore the glory of ages past, crush the Sons of Zek, and sweep aside the decadent nation of Qeynos!", "", 140890899, 2835297833, Player, 0)
	elseif choice == 4 then
		PlayFlavor(Player, "voiceover/english/overlord_lucan_d_lere/fprt_west/lucan_isle_speech_9.mp3", "With my guidance, you shall gain power and glory as you have never imagined, but should you turn against me, you will find that my wrath is a terrible thing ... Now go!", "", 3855854568, 2247480313, Player, 0)
	elseif choice == 5 then
		PlayFlavor(Player, "voiceover/english/overlord_lucan_d_lere/fprt_west/lucan_isle_speech_7.mp3", "Succeed, and you will share the fortunes of Freeport as we reshape this broken world.", "", 2666628260, 1943756642, Player, 0)
	elseif choice == 6 then
		PlayFlavor(Player, "voiceover/english/overlord_lucan_d_lere/fprt_west/lucan_isle_speech_6.mp3", "Go now, and begin the trials that I have set for you.", "", 1244918730, 586509135, Player, 0)
	end
```
