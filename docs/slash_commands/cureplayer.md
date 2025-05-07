### Command: /cureplayer identifier cure_type [spell_type]

**Handler Macro:** COMMAND_CUREPLAYER

**Handler Value:** 531

**Required Status:** 0

**Arguments:**
- `arg[0]`: `string identifier`
- `arg[1]`: `string cure_type`
- `arg[2]`: `string spell_type`

**Notes:**
- Identifies in Player's inventory a potion or spell that can cure the `identifier` (target spawn).  `spell_type` is an optional field.
- /cureplayer [playername|group or raid position][trauma|arcane|noxious|elemental|curse] optional [spell|potion]
- Example: /cureplayer g0 noxious spell - Will attempt to cure yourself of a noxious detriment with only spells and without using potions (even if you have them).