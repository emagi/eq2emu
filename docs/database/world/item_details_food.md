## Table: `item_details_food`

**Description:**

Defines `item_details_food` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `type` (tinyint(3), NOT NULL, DEFAULT 0)
- `level` (smallint(6), NOT NULL, DEFAULT 0)
- `duration` (float, NOT NULL, DEFAULT 0)
- `satiation` (tinyint(3), NOT NULL, DEFAULT 2)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `ItemIndex` (`item_id`)
- CONSTRAINT `FK_item_details_food` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE