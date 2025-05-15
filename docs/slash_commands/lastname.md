### Command: /lastname name

**Handler Macro:** COMMAND_LASTNAME

**Handler Value:** 28

**Required Status:** 0

**Arguments:**
- `arg[0]`: `string name`

**Notes:**
- Sets lastname of your character to `name`.  Must pass the Zone RuleSet and/or Global RuleSet for minimum level [R_Player, MinLastNameLevel] = 20 (default), minimum last name character length [R_Player, MinLastNameLength] = 4 (default) and maximum last name character length [R_Player, MaxLastNameLength] = 20 (default).