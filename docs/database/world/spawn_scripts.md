## Table: `spawn_scripts`

**Description:**

Defines `spawn_scripts` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `spawn_id` (int(10), NOT NULL, DEFAULT 0)
- `spawnentry_id` (int(10), NOT NULL, DEFAULT 0)
- `spawn_location_id` (int(10), NOT NULL, DEFAULT 0)
- `lua_script` (varchar(255), NOT NULL, DEFAULT ' ')

**Primary Keys:**
- id