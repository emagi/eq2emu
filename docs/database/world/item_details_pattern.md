## Table: `item_details_pattern`

**Description:**

Defines `item_details_pattern` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `pattern_item_id` (int(10), NOT NULL, DEFAULT 0)
- `item_icon` (smallint(5), NOT NULL, DEFAULT 0)
- `item_name` (varchar(250), DEFAULT '')

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `PatternIDX` (`pattern_item_id`)
- KEY `FK_item_details_pattern` (`item_id`)
- CONSTRAINT `FK_item_details_pattern` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`) ON UPDATE CASCADE