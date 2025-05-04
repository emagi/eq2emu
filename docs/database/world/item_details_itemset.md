## Table: `item_details_itemset`

**Description:**

Defines `item_details_itemset` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `itemset_item_id` (int(10), NOT NULL, DEFAULT 0)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `item_icon` (smallint(5), NOT NULL, DEFAULT 0)
- `item_stack_size` (smallint(5), NOT NULL, DEFAULT 0)
- `item_list_color` (int(10), NOT NULL, DEFAULT 0)
- `language_type` (tinyint(3), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `ItemsetIDX` (`itemset_item_id`,`item_id`,`language_type`)
- CONSTRAINT `FK_item_details_itemset` FOREIGN KEY (`itemset_item_id`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE