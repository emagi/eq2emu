## Table: `item_details_shield`

**Description:**

Defines `item_details_shield` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `mitigation_low` (smallint(5), NOT NULL, DEFAULT 0)
- `mitigation_high` (smallint(5), NOT NULL, DEFAULT 0)
- `item_score` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `NewIndex` (`item_id`)
- CONSTRAINT `FK_item_details_shield` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE