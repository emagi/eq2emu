### Command: /book read item_index

**Handler Macro:** COMMAND_READ

**Handler Value:** 463

**Required Status:** 0

**Arguments:**
- `arg[0]`: `string action`
- `arg[1]`: `int32 item_index`

**Notes:**
- The handler is assigned to COMMAND_READ, but the /book command is called since that is what the handler is assigned to.
- First argument is `action`, we support the value `read`.  Second argument is the item index in player's item list.