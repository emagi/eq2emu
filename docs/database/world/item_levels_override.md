## Table: `item_levels_override`

**Description:**

Defines `item_levels_override` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `adventure_class_id` (tinyint(3), NOT NULL, DEFAULT 0)
- `tradeskill_class_id` (tinyint(3), NOT NULL, DEFAULT 0)
- `level` (smallint(5), NOT NULL, DEFAULT 1)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `ItemClassIDX` (`item_id`,`adventure_class_id`,`tradeskill_class_id`)
- CONSTRAINT `FK_item_levels_override` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE