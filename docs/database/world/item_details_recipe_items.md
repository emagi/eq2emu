## Table: `item_details_recipe_items`

**Description:**

Defines `item_details_recipe_items` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `recipe_id` (int(10), NOT NULL, DEFAULT 0)
- `name` (varchar(255), NOT NULL, DEFAULT '')
- `icon` (smallint(5), NOT NULL, DEFAULT 0)
- `soe_recipe_crc` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `FK_item_details_recipe_items` (`recipe_id`)
- CONSTRAINT `item_details_recipe_items_ibfk_1` FOREIGN KEY (`recipe_id`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE