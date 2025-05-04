## Table: `merchant_multipliers`

**Description:**

Defines `merchant_multipliers` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `merchant_id` (int(10), NOT NULL, DEFAULT 0 COMMENT 'Testing') → Testing
- `low_buy_multiplier` (float, NOT NULL, DEFAULT 1)
- `high_buy_multiplier` (float, NOT NULL, DEFAULT 10)
- `low_sell_multiplier` (float, NOT NULL, DEFAULT 1)
- `high_sell_multiplier` (float, NOT NULL, DEFAULT 10)
- `multiplier_faction_id` (int(10), NOT NULL, DEFAULT 0)
- `min_faction` (int(11), NOT NULL, DEFAULT -20000)
- `max_faction` (int(11), NOT NULL, DEFAULT 50000)

**Primary Keys:**
- id