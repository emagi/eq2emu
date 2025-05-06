### Command: /buyback_from_merchant id quantity

**Handler Macro:** COMMAND_BUYBACK

**Handler Value:** 91

**Required Status:** 0

**Arguments:**
- `arg[0]`: `int64 id`
- `arg[1]`: `int quantity`

**Notes:**
- Used to buyback a previously sold item with a merchant.  `id` is a combination of the item_id and unique_id.  First 4 bytes is the item id, second 4 bytes is the unique id.