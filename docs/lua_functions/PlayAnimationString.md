### Function: PlayAnimationString(spawn, name, opt_target, set_no_target, use_all_spelltargets, ignore_self)

**Description:**
Play an animation by using the string to all Player's or only a specific optional_target (Player).  Review Appearance ID's in the Reference List https://wiki.eq2emu.com/ReferenceLists by client for more details.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `name` (string) - String `name`.
- `opt_target` (int32) - Integer value `opt_target`.
- `set_no_target` (int32) - Integer value `set_no_target`.
- `use_all_spelltargets` (int32) - Integer value `use_all_spelltargets`.
- `ignore_self` (int32) - Integer value `ignore_self`.

**Returns:** None.

**Example:**

```lua
-- From Spells/Commoner/UnholyFear.lua
function cast(Caster, Target)
    local targets = GetSpellTargets()
	for k,v in ipairs(targets) do
        if GetLevel(v) < GetLevel(Caster) then
            PlayAnimationString(v, "cringe", nil, true)
        end
```
