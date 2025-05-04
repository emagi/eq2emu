## Table: `item_details_bauble`

**Description:**

Defines `item_details_bauble` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `cast` (smallint(6), NOT NULL, DEFAULT 100)
- `recovery` (smallint(6), NOT NULL, DEFAULT 0)
- `duration` (int(10), NOT NULL, DEFAULT 0)
- `recast` (float, NOT NULL, DEFAULT 1)
- `display_slot_optional` (tinyint(1), NOT NULL, DEFAULT 0)
- `display_cast_time` (tinyint(1), NOT NULL, DEFAULT 0)
- `display_bauble_type` (tinyint(1), NOT NULL, DEFAULT 0)
- `effect_radius` (float, NOT NULL, DEFAULT 0)
- `max_aoe_targets` (int(11), NOT NULL, DEFAULT 0)
- `display_until_cancelled` (tinyint(3), NOT NULL, DEFAULT 0)
- `item_score` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `ItemBaubleIDX` (`item_id`)
- CONSTRAINT `FK_item_details_bauble` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE