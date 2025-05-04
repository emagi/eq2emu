## Table: `raw_sell_prices`

**Description:**

Defines `raw_sell_prices` table in the World database.

**Columns:**
- `soe_item_id_unsigned` (int(10), NOT NULL)
- `price` (int(10), NOT NULL)
- `status` (int(10), NOT NULL)

**Indexes/Notes:**
- UNIQUE KEY `SoeItemIDUK` (`soe_item_id_unsigned`)