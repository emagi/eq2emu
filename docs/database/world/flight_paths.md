## Table: `flight_paths`

**Description:**

Defines `flight_paths` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `zone_id` (int(10), NOT NULL)
- `name` (char(255), NOT NULL)
- `speed` (float, NOT NULL, DEFAULT 0)
- `flying` (tinyint(3), NOT NULL, DEFAULT 1)
- `early_dismount` (tinyint(3), NOT NULL, DEFAULT 1)

**Primary Keys:**
- id