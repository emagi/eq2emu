### Function: GetAlignment(Player)

**Description:**
Returns the alignment of the player character – typically Good, Neutral, or Evil in EQ2. Alignment often affects starting city and some quest options.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

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
