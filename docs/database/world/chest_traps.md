## Table: `chest_traps`

**Description:**

Defines `chest_traps` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `applicable_zone_id` (int(10), NOT NULL, DEFAULT 0)
- `chest_min_difficulty` (int(10), NOT NULL, DEFAULT 0)
- `chest_max_difficulty` (int(10), NOT NULL, DEFAULT 0)
- `spell_id` (int(10), NOT NULL, DEFAULT 0)
- `spell_tier` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id