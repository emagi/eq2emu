### Function: PlayAnimation(spawn, anim, target, hide_type)

**Description:**
The spawn will play the animation `anim` type specified.  See the Appearance IDs by client in the ReferenceList here https://wiki.eq2emu.com/ReferenceLists
Note that the target and hide_type are optional fields.  Setting the hide_type to 1 will mean only the target receives the update.  Setting to 2 will exclude the target and send to all others.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.
- `anim` (int32) - Integer value `anim`.
- `target` (Spawn) - Spawn objet representing `target`.
- `hide_type` (int32) - Integer value `hide_type`.

**Returns:** None.

**Example:**

```lua
-- From ItemScripts/CrestridersTonic.lua
function cast(Item, Player)
PlayAnimation(Player, 11422) -- No joking its drink animation
end
```
