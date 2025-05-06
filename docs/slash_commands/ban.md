### Command: /ban player_name [permanent:0/1]

**Handler Macro:** COMMAND_BAN

**Handler Value:** 23

**Required Status:** 10

**Arguments:**
- `arg[0]`: `string player_name`
- `arg[1]`: `int8 permanent`

**Notes:**
- `Permanent` is default 0 which is suspended (Admin Status -1), when set to 1 will be set to banned (Admin Status -2).