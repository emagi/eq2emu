## Table: `character_achievements_items`

**Description:**

Defines `character_achievements_items` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `char_id` (int(10), NOT NULL, DEFAULT 0)
- `achievement_id` (int(10), NOT NULL, DEFAULT 0)
- `items` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id