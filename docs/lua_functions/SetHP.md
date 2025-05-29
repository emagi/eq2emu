### Function: SetHP(Spawn, CurrentHP)

**Description:** Sets the Spawn's Current HP, if the CurrentHP value is higher than the Spawn's Total HP it will override the Total HP as well.

**Parameters:**

`Spawn`: Spawn – The Spawn whoms HP should be changed.
`CurrentHP`: SInt32 - New HP value for the spawn.

**Returns:** None.

**Example:**

```lua
-- Example set Spawn HP to 1000
SetHP(Spawn, 1000)
```