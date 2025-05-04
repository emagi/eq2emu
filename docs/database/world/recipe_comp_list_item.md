## Table: `recipe_comp_list_item`

**Description:**

Defines `recipe_comp_list_item` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `comp_list` (int(10), NOT NULL)
- `item_id` (int(10), NOT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `Index 2` (`comp_list`,`item_id`) USING BTREE
- KEY `FK_recipe_comp_list_item_items` (`item_id`)
- CONSTRAINT `FK_recipe_comp_list_item_items` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`)
- CONSTRAINT `FK_recipe_comp_list_item_recipe_comp_list` FOREIGN KEY (`comp_list`) REFERENCES `recipe_comp_list` (`id`) ON DELETE CASCADE ON UPDATE CASCADE