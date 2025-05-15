### Command: /house_deposit spawn_id deposit_copper deposit_status

**Handler Macro:** COMMAND_HOUSE_DEPOSIT

**Handler Value:** 518

**Required Status:** 0

**Arguments:**
- `arg[0]`: `int spawn_id`
- `arg[1]`: `string deposit_copper`
- `arg[2]`: `string deposit_status`

**Notes:**
- If outside an instance the spawn_id needs to be set to provide the correct Spawn/Door providing the House.  Otherwise inside the player instance the spawn_id is optional (provide 0 for arg[0]).  Deposit_copper will only allow player inventory coin, not bank coin.