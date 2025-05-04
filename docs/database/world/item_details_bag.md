## Table: `item_details_bag`

**Description:**

Defines `item_details_bag` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `num_slots` (tinyint(3), NOT NULL, DEFAULT 0)
- `weight_reduction` (tinyint(3), NOT NULL, DEFAULT 0)
- `unknown12` (tinyint(3), NOT NULL, DEFAULT 0)
- `backpack` (tinyint(1), NOT NULL, DEFAULT 0)
- `unknown81` (tinyint(3), NOT NULL, DEFAULT 0)
- `unknown69` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `ItemIndex` (`item_id`)
- CONSTRAINT `FK_item_details_bag` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE