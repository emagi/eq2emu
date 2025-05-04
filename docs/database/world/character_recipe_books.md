## Table: `character_recipe_books`

**Description:**

Defines `character_recipe_books` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `char_id` (int(10), NOT NULL, DEFAULT 0)
- `recipebook_id` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `fk_characterrecipebook_charid` (`char_id`)
- KEY `fk_characterrecipebook_recipebookid` (`recipebook_id`)