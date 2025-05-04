## Table: `item_itemset_bonus`

**Description:**

Defines `item_itemset_bonus` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `set_id` (int(10), NOT NULL)
- `index` (int(10), NOT NULL)
- `num_items_needed` (tinyint(3), NOT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `UK_setid_index` (`set_id`,`index`)
- CONSTRAINT `FK__item_itemsets` FOREIGN KEY (`set_id`) REFERENCES `item_itemsets` (`id`) ON DELETE CASCADE ON UPDATE CASCADE