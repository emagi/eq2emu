### Command: /buy_from_broker item_id quantity

**Handler Macro:** COMMAND_BUY_FROM_BROKER

**Handler Value:** 95

**Required Status:** 0

**Arguments:**
- `arg[0]`: `int item_id`
- `arg[1]`: `int quantity`

**Notes:**
- /buy_from_broker is not fully implemented, this just acts much like summon item.  Supplies the player inventory with an item of `item_id` and `quantity`.