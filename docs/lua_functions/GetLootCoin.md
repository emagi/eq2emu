### Function: GetLootCoin(spawn)

**Description:**
Get the loot coin in copper assigned to the spawn.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** UInt32 in copper of the spawn's lootable coin.

**Example:**

```lua
-- From SpawnScripts/GMHall/TwoFace.lua
function Yes1(NPC, Spawn)
	FaceTarget(NPC, Spawn)
	conversation = CreateConversation()
	coins = 5000
	local poolCoins = RemoveCoin(Spawn, coins)
	--[[This little section will pool coins but will only last until player logs out =(
	local npcCoins = GetLootCoin(NPC)
	Say(NPC, "I have " .. npcCoins .. " coins. And I just stole from you.")
	--]]
	if(poolCoins) then
		--[[local totalCoins = npcCoins + coins
		SetLootCoin(NPC, totalCoins)--]]
		PlaySound(NPC, "voiceover/english/voice_emotes/thank/thank_2_1054.mp3", GetX(NPC), GetY(NPC), GetZ(NPC))
		Say(NPC, "Thank you, let's begin!")
		randpick = math.random(1, 2)
		AddConversationOption(conversation, "Heads!", "Heads1")
		AddConversationOption(conversation, "Tails!", "Tails1")
		StartConversation(conversation, NPC, Spawn, GetName(Spawn) .. ", I'm going to flip a coin, call it in the air...")
	else
		Say(NPC, "I'm sorry but you don't have enough money, begone.")
		PlaySound(NPC, "sounds/combat/impact/leather/impact_metal_to_leather04.wav", GetX(NPC), GetY(NPC), GetZ(NPC))
	end
```
