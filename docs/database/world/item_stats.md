## Table: `item_stats`

**Description:**

Defines `item_stats` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `type` (tinyint(3), NOT NULL, DEFAULT 0)
- `subtype` (tinyint(3), NOT NULL, DEFAULT 0)
- `value` (float, NOT NULL, DEFAULT 0)
- `text` (varchar(250), DEFAULT NULL)
- `description` (text, DEFAULT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `FK_item_stats` (`item_id`)
- CONSTRAINT `FK_item_stats` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE