### Function: ChangeFaction(spawn, faction_id, increase_or_decrease)

**Description:**

Changes the faction on the Spawn for the faction_id by increasing or decreasing the value.  Decrease would be a negative value.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `faction_id` (uint32) - Integer value `faction_id`.
- `increase_or_decrease` (sint32) - Integer value `increase`.

**Returns:** True boolean if successful updating the faction value.

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
