## Table: `item_mod_strings`

**Description:**

Defines `item_mod_strings` table in the World database.

**Columns:**
- `id` (int(11), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `item_id` (int(10), NOT NULL)
- `mod` (varchar(255), NOT NULL, DEFAULT '')
- `description` (text, NOT NULL)
- `unk1` (tinyint(3), NOT NULL, DEFAULT 0)
- `unk2` (tinyint(3), NOT NULL, DEFAULT 0)
- `index` (tinyint(3), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `UK_itemid_index` (`index`,`item_id`) USING BTREE
- KEY `FK_item_mod_strings_items` (`item_id`)
- CONSTRAINT `FK_item_mod_strings_items` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`) ON UPDATE CASCADE