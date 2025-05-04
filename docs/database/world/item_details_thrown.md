## Table: `item_details_thrown`

**Description:**

Defines `item_details_thrown` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `range_bonus` (int(11), NOT NULL, DEFAULT 0)
- `damage_bonus` (int(11), NOT NULL, DEFAULT 0)
- `hit_bonus` (float, NOT NULL, DEFAULT 0)
- `damage_type` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `NewIndex` (`item_id`)
- CONSTRAINT `FK_item_details_thrown` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE