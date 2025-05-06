### Command: /bot settings [action] [value]

**Handler Macro:** COMMAND_BOT_SETTINGS

**Handler Value:** 506

**Required Status:** 0

**Arguments:**
- `arg[0]`: `string action`
- `arg[1]`: `int8 value`

**Notes:**
- `action` can be helm, hood, cloak, taunt
- `value` can be 1 to enable, 0 to disable.
- /bot settings `[helm/hood/cloak/taunt] [0/1]` - Turn setting on (1) or off(0)
