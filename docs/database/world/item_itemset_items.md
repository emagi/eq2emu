## Table: `item_itemset_items`

**Description:**

Defines `item_itemset_items` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `set_id` (int(10), NOT NULL)
- `item_id` (int(10), DEFAULT NULL)
- `item_name` (varchar(255), NOT NULL, DEFAULT '')
- `index` (int(10), NOT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `FK_item_itemset_items_item_itemsets` (`set_id`)
- KEY `ItemNameIDX` (`item_name`)
- KEY `FK_item_itemset_items_items` (`item_id`)
- CONSTRAINT `FK_item_itemset_items_item_itemsets` FOREIGN KEY (`set_id`) REFERENCES `item_itemsets` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
- CONSTRAINT `FK_item_itemset_items_items` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`)