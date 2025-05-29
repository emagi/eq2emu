### Function: ProcHate(Source, Target, HateAmount)

**Description:** Directly modifies hate by a given amount as a result of a proc or effect. This will increase (or decrease, if negative) the hate that the source has toward the target or vice versa (depending on internal implementation; likely it adds hate from Source towards Target).

**Parameters:**

`Source`: Spawn – The entity generating hate.
`Target`: Spawn – The entity receiving hate (being hated more).
`HateAmount`: Int32 – The amount of hate to add (or remove if negative).

**Returns:** None.

**Example:**

```lua
-- Example usage (increase hate of an NPC toward the player as a taunt proc)
ProcHate(Player, NPC, 500)
```