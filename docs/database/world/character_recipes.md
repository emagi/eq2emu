## Table: `character_recipes`

**Description:**

Defines `character_recipes` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `char_id` (int(10), NOT NULL, DEFAULT 0)
- `recipe_id` (int(10), NOT NULL, DEFAULT 0)
- `highest_stage` (tinyint(3), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `fk_characterrecipes_charid` (`char_id`)
- KEY `fk_characterrecipes_recipeid` (`recipe_id`)
- CONSTRAINT `fk_characterrecipes_charid` FOREIGN KEY (`char_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE ON UPDATE CASCADE