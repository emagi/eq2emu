### Function: ApplySpellVisual(target, spell_visual)

**Description:**

Apply a spell visual from the spell visuals in the ReferenceList https://wiki.eq2emu.com/ReferenceLists by client version.  Older clients like KoS/DoF translate newer spell visuals to old via the spell_visuals table alternate_spell_visual.

**Parameters:**
- `target` (Spawn) - Spawn object representing `target`.
- `spell_visual` (uint32) - Integer value `spell_visual`.



**Returns:** None.

**Example:**

```lua
-- From ItemScripts/BardCertificationPapers.lua
    ApplySpellVisual(Player, 324)
```
