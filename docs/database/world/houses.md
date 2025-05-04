## Table: `houses`

**Description:**

Defines `houses` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `name` (char(64), NOT NULL, DEFAULT '0')
- `cost_coins` (bigint(20), NOT NULL, DEFAULT 0)
- `cost_status` (int(10), NOT NULL, DEFAULT 0)
- `upkeep_coins` (bigint(20), NOT NULL, DEFAULT 0)
- `upkeep_status` (int(10), NOT NULL, DEFAULT 0)
- `vault_slots` (tinyint(3), NOT NULL, DEFAULT 0)
- `alignment` (tinyint(3), NOT NULL, DEFAULT 0)
- `guild_level` (tinyint(3), NOT NULL, DEFAULT 0)
- `zone_id` (int(10), NOT NULL, DEFAULT 0)
- `exit_zone_id` (int(10), NOT NULL, DEFAULT 0)
- `exit_x` (float, NOT NULL, DEFAULT 0)
- `exit_y` (float, NOT NULL, DEFAULT 0)
- `exit_z` (float, NOT NULL, DEFAULT 0)
- `exit_heading` (float, NOT NULL, DEFAULT 0)

**Primary Keys:**
- id