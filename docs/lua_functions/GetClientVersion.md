### Function: GetClientVersion(Spawn)

**Description:**
Retrieves the game client version of the specified player (useful if the server supports multiple client versions). This can determine differences in available features or UI.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.

**Returns:** UInt32 value of the client version, Isle of Refuge = 373, Desert of Flames (not CD, January 2006) = 546, Kingdom of Sky (Feb 2006 DVD) = 561

**Example:**

```lua
-- From ItemScripts/AcommemorativeQeynosCoin.lua
function examined(Item, Player)
choice = MakeRandomInt(0,100)
if choice >=2 then
    if GetClientVersion(Player) >546 then
    conversation = CreateConversation()
    PlayFlavor(Player, "voiceover/english/queen_antonia_bayle/qey_north/antonia_isle_speech_1.mp3", "", "", 499186274, 1744595600, Player)
    -- PlayFlavor(Player,"voiceover/english/tullia_domna/fprt_hood04/quests/tulladomna/tulla_x1_initial.mp3","","",309451026,621524268,Player)
    --	PlayFlavor(Player,"voiceover/english/queen_antonia_bayle/qey_north/antonia_isle_speech.mp3","","", 2297205435, 1273418227,Player)
    AddConversationOption(conversation, "\"Many among you...\"", "visage03")
    AddConversationOption(conversation, "Put the coin away.", "CloseItemConversation")
    StartDialogConversation(conversation, 2, Item, Player, "As you clutch the coin in your hand, you hear a voice magically speaking in your mind.                                                                                                                                      \"Good traveler, you have seen much in your journey, and now you seek refuge in our humble City of Qeynos. As ruler and servant of the good people of Qeynos, I, Antonia Bayle, welcome you.\"")
    else
    conversation = CreateConversation()
    PlayFlavor(Player,"voiceover/english/queen_antonia_bayle/qey_north/antonia_isle_speech.mp3","","", 2297205435, 1273418227,Player)
    AddConversationOption(conversation, "Put the coin away.", "CloseItemConversation")
    StartDialogConversation(conversation, 2, Item, Player, "As you clutch the coin in your hand, you hear a voice magically speaking in your mind.")    
end
```
