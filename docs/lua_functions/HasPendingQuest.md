### Function: HasPendingQuest(player, quest_id)

**Description:**
Identifies if the Player has a pending quest acceptance window for the quest_id specified.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `quest_id` (uint32) - Integer value `quest_id`.

**Returns:** None.

**Example:**

```lua
-- From SpawnScripts/ElddarGrove/NaturalistTummyfill.lua

function hailed(NPC, Spawn)
    local playerLevel = GetLevel(Spawn)
    local quests = nil

    -- Determine the quests based on the player's level (20-29)
    if playerLevel >= 20 and playerLevel <= 24 then
        quests = questsByLevel["20-24"]
    elseif playerLevel >= 25 and playerLevel <= 29 then
        quests = questsByLevel["25-29"]
    end
	
    if quests then
        -- Check if the player already has an active quest
        for _, questID in ipairs(quests) do
            if HasQuest(Spawn, questID) then
                -- If the player has an active quest, inform them and exit
                Say(NPC, "You're already on a task. Finish it first before taking another.")
                return
            elseif HasPendingQuest(Spawn, questID) then
                -- If the player has a pending quest, do nothing and exit
                return
            end
        end
        -- If no quest was found, offer a new one
        local newQuestID = quests[math.random(#quests)]
        OfferQuest(NPC, Spawn, newQuestID)
        Say(NPC, "I have a task for someone of your experience. Will you take it?")
    else
        Say(NPC, "I have no tasks suitable for your level. You may need to seek another challenge.")
    end
end
```
