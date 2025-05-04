## Table: `recipe_secondary_comp`

**Description:**

Defines `recipe_secondary_comp` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `recipe_id` (int(10), NOT NULL)
- `index` (int(10), NOT NULL)
- `comp_list` (int(10), NOT NULL)
- `qty` (int(10), NOT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `UNQRecipeIndex` (`recipe_id`,`index`)
- KEY `FK_recipe_secondary_comp_recipe_comp_list` (`comp_list`)
- CONSTRAINT `FK_recipe_secondary_comp_recipe` FOREIGN KEY (`recipe_id`) REFERENCES `recipe` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
- CONSTRAINT `FK_recipe_secondary_comp_recipe_comp_list` FOREIGN KEY (`comp_list`) REFERENCES `recipe_comp_list` (`id`)