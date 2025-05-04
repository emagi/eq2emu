## Table: `recipe_components`

**Description:**

Defines `recipe_components` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `recipe_id` (int(10), NOT NULL, DEFAULT 0)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `slot_id` (tinyint(3), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id