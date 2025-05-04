## Table: `item_details_reward_crate_item`

**Description:**

Defines `item_details_reward_crate_item` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `crate_item_id` (int(10), NOT NULL, DEFAULT 0)
- `reward_item_id` (int(10), DEFAULT NULL)
- `soe_item_id` (int(10), NOT NULL, DEFAULT 0)
- `soe_item_crc` (int(10), NOT NULL)
- `icon` (smallint(5), NOT NULL, DEFAULT 0)
- `stack_size` (int(10), NOT NULL, DEFAULT 0)
- `name_color` (int(10), NOT NULL, DEFAULT 0)
- `name` (text, DEFAULT NULL)
- `language_type` (tinyint(3), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `FK_item_details_itemset` (`crate_item_id`)
- KEY `FK_item_details_reward_crate_item_items` (`reward_item_id`)
- CONSTRAINT `FK_item_details_reward_crate_item_items` FOREIGN KEY (`reward_item_id`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
- CONSTRAINT `FK_item_details_reward_crate_item_items_crateid` FOREIGN KEY (`crate_item_id`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE