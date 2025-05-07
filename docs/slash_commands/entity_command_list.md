### Command: /entity_command list name

**Handler Macro:** COMMAND_ENTITYCOMMAND_LIST

**Handler Value:** 242

**Required Status:** 200

**Arguments:**
- `arg[0]`: `string name`

**Notes:**
- Lists the zone entity commands that wild card match to the `name` provided.
- Result is multiline each line: "Command Text: hail, Command List ID: #, Distance: 5.0".  Distance is the max distance allowed to use the command, List ID is from the database entity_commands table command_list_id parameter.