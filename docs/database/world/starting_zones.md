## Table: `starting_zones`

**Description:**

Defines `starting_zones` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `class_id` (tinyint(3), NOT NULL, DEFAULT 255)
- `race_id` (tinyint(3), NOT NULL, DEFAULT 255)
- `choice` (tinyint(3), NOT NULL, DEFAULT 255)
- `zone_id` (int(11), NOT NULL, DEFAULT 253)
- `is_instance` (tinyint(1), NOT NULL, DEFAULT 0)
- `min_version` (int(10), NOT NULL, DEFAULT 0)
- `max_version` (int(10), NOT NULL, DEFAULT 0)
- `x` (float, NOT NULL, DEFAULT -999999)
- `y` (float, NOT NULL, DEFAULT -999999)
- `z` (float, NOT NULL, DEFAULT -999999)
- `heading` (float, NOT NULL, DEFAULT -999999)
- `deity` (int(11), NOT NULL, DEFAULT 255)
- `ruleflag` (int(11), NOT NULL, DEFAULT 0)
- `description` (text, DEFAULT '')
- `start_alignment` (tinyint(3), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id