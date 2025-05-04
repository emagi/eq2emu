## Table: `item_details_armor`

**Description:**

Defines `item_details_armor` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `mitigation_low` (int(10), NOT NULL, DEFAULT 0)
- `mitigation_high` (int(10), NOT NULL, DEFAULT 0)
- `absorb` (smallint(6), NOT NULL, DEFAULT 0)
- `unknown` (int(11), NOT NULL, DEFAULT 0)
- `item_score` (int(11), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `NewIndex` (`item_id`)
- CONSTRAINT `FK_item_details_armor` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE