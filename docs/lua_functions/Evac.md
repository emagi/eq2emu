### Function: Evac(player, x, y, z, heading)

**Description:**
Evacuates the Player (optional field) or their group (No fields) to a safe spot, typically the zone’s designated evacuation point (e.g., the zone entrance). This mimics the effect of an evac spell.

**Parameters:**
- `player` (Spawn) - Spawn object reference.  The player (usually the caster of the evac or the one whose group to evac).  This is optional, if Player is set Evac is self only.  Otherwise Evac() is for a spell script against all spell targets.
- `x` (float) - Optional X Coordinate (must supply X,Y,Z,Heading).  If for group set Player to nil.
- `y` (float) - Optional Y Coordinate (must supply X,Y,Z,Heading).  If for group set Player to nil.
- `z` (float) - Optional Z Coordinate (must supply X,Y,Z,Heading).  If for group set Player to nil.
- `heading` (float) - Optional Heading Coordinate (must supply X,Y,Z,Heading).  If for group set Player to nil.

**Returns:** None.

**Example:**

```lua
-- From Spells/Fighter/Crusader/Shadowknight/ShadowyElusion.lua
function cast(Caster, Target)
    Evac()
end
```
