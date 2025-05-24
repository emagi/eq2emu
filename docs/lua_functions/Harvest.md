### Function: Harvest(player, node)

**Description:**
Forces a harvest action on the specified harvestable object or resource node. When called on a harvestable spawn (like a resource node), it attempts to collect from it as if a player harvested it.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `node` (Spawn) - Spawn object representing `node`.

**Returns:** None (the harvesting results — items or updates — are handled by the system).

**Example:**

```lua
-- From Spells/Commoner/harvest.lua
function cast(Caster, Target)
   	Harvest(Caster, Target)
end
```
