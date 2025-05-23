### Function: IsEpic(Spawn)

**Description:** Checks if the given NPC is flagged as an “Epic” encounter.

**Parameters:**
- `Spawn`: Spawn – The NPC to check.

**Returns:** Boolean – true if the spawn is an Epic tier NPC; false otherwise.

**Example:**

```lua
-- Example AtrebasEtherealBindings.lua (Spells Script)
function precast(Caster, Target)
    -- Does not affect Epic targets
    if IsEpic(Target) then
        return false, 43
    end
    return true
end
```