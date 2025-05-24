### Function: GetFactionAmount(player, faction_id)

**Description:**
Gets the amount of faction value the player has with the specified faction_id.

**Parameters:**
- `player` (Spawn) - Spawn reference object `player`.
- `faction_id` (uint32) - Integer value `faction_id`.

**Returns:** SInt32 faction amount of the Player.

**Example:**

```lua
-- From ItemScripts/ForgeryFreeportCitizenshipPapers.lua
function Faction(Item,Player)
    Freeport = GetFactionAmount(Player, 11)
    Freeport_Add = (10000-Freeport)
    Freeport = GetFactionAmount(Player, 12)
    Freeport_Add = (-20000-Freeport)
    Neriak = GetFactionAmount(Player, 13)
    Kelethin = GetFactionAmount(Player, 14)
    Halas = GetFactionAmount(Player, 16)
    Gorowyn = GetFactionAmount(Player, 17)
    alignment = GetAlignment(Player)
 if Freeport <10000 and Freeport >=0 then ChangeFaction(Player, 11, Freeport_Add)
    elseif Freeport <0 then ChangeFaction(Player, 11, (Freeport*-1))
    Faction(Item,Player)    
end
```
