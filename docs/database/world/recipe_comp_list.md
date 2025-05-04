## Table: `recipe_comp_list`

**Description:**

Defines `recipe_comp_list` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `name` (tinytext, NOT NULL)
- `bEmpty` (tinyint(1), NOT NULL, DEFAULT 1)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `IDXName` (`name`(100)) USING BTREE