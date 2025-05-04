## Table: `item_details_recipe`

**Description:**

Defines `item_details_recipe` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `recipe_id` (int(10), NOT NULL, DEFAULT 0)
- `max_uses` (smallint(5), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id