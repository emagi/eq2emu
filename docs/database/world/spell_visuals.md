## Table: `spell_visuals`

**Description:**

Defines `spell_visuals` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `name` (varchar(128), DEFAULT NULL)
- `alternate_spell_visual` (varchar(128), DEFAULT '')
- `spell_visual_id` (int(10), NOT NULL, DEFAULT 0)
- `min_version_range` (int(10), NOT NULL, DEFAULT 0)
- `max_version_range` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id