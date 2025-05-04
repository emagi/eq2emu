## Table: `item_details_range`

**Description:**

Defines `item_details_range` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `dmg_low` (smallint(5), NOT NULL, DEFAULT 0)
- `dmg_high` (smallint(5), NOT NULL, DEFAULT 0)
- `dmg_mastery_low` (smallint(5), NOT NULL, DEFAULT 0)
- `dmg_mastery_high` (smallint(5), NOT NULL, DEFAULT 0)
- `dmg_base_low` (smallint(5), NOT NULL, DEFAULT 0)
- `dmg_base_high` (smallint(5), NOT NULL, DEFAULT 0)
- `delay` (tinyint(3), NOT NULL, DEFAULT 0)
- `range_low` (smallint(5), NOT NULL, DEFAULT 0)
- `range_high` (smallint(5), NOT NULL, DEFAULT 0)
- `damage_rating` (float, NOT NULL, DEFAULT 0)
- `damage_type` (tinyint(3), NOT NULL, DEFAULT 0)
- `item_score` (int(10), DEFAULT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `NewIndex` (`item_id`)
- CONSTRAINT `FK_item_details_range` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE