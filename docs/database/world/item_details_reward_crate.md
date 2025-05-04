## Table: `item_details_reward_crate`

**Description:**

Defines `item_details_reward_crate` table in the World database.

**Columns:**
- `id` (int(11), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `item_id` (int(10), NOT NULL)
- `unk1` (tinyint(3), NOT NULL, DEFAULT 0)
- `unk2` (tinyint(3), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `FK_item_details_reward_crate_items` (`item_id`)
- CONSTRAINT `FK_item_details_reward_crate_items` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE