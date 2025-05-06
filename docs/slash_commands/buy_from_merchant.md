### Command: /buy_from_merchant item_id quantity

**Handler Macro:** COMMAND_MERCHANT_BUY

**Handler Value:** 87

**Required Status:** 0

**Arguments:**
- `arg[0]`: `int item_id`
- `arg[1]`: `int quantity`

**Notes:**
- If item_id and quantity arguments are not specified, then it displays the merchant buy window.
- If in merchant transaction, will attempt to buy from merchant the item_id and quantity if item is available and sufficient quantity.