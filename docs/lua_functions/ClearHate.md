### Function: ClearHate(NPC, Hated)

**Description:** Removes the `Hated` target on the NPC's hate list.

**Parameters:**

`NPC`: Spawn – The NPC whose hate list should be cleared.
`Hated`: Spawn – The hated target that is to be removed from the hate list.

**Returns:** None.

**Example:**

```lua
-- Example usage (an NPC stops attacking/hating Target)
ClearHate(NPC, Target)
```