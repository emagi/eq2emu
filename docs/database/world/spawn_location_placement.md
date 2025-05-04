## Table: `spawn_location_placement`

**Description:**

Defines `spawn_location_placement` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `zone_id` (int(10), NOT NULL, DEFAULT 0)
- `spawn_location_id` (int(10), NOT NULL, DEFAULT 0)
- `x` (float, NOT NULL, DEFAULT 0)
- `y` (float, NOT NULL, DEFAULT 0)
- `z` (float, NOT NULL, DEFAULT 0)
- `x_offset` (float, NOT NULL, DEFAULT 0)
- `y_offset` (float, NOT NULL, DEFAULT 0)
- `z_offset` (float, NOT NULL, DEFAULT 0)
- `heading` (float, NOT NULL, DEFAULT 0)
- `pitch` (float, NOT NULL, DEFAULT 0)
- `roll` (float, NOT NULL, DEFAULT 0)
- `respawn` (int(10), NOT NULL, DEFAULT 300)
- `respawn_offset_low` (int(10), NOT NULL, DEFAULT 0)
- `respawn_offset_high` (int(10), NOT NULL, DEFAULT 0)
- `duplicated_spawn` (tinyint(1), NOT NULL, DEFAULT 1)
- `expire_timer` (int(10), NOT NULL, DEFAULT 0)
- `expire_offset` (int(10), NOT NULL, DEFAULT 0)
- `grid_id` (int(10), NOT NULL, DEFAULT 0)
- `processed` (tinyint(1), NOT NULL, DEFAULT 0)
- `instance_id` (int(10), NOT NULL, DEFAULT 0)
- `lvl_override` (int(11), NOT NULL, DEFAULT 0)
- `hp_override` (int(11), NOT NULL, DEFAULT 0)
- `mp_override` (int(11), NOT NULL, DEFAULT 0)
- `str_override` (int(11), NOT NULL, DEFAULT 0)
- `sta_override` (int(11), NOT NULL, DEFAULT 0)
- `wis_override` (int(11), NOT NULL, DEFAULT 0)
- `int_override` (int(11), NOT NULL, DEFAULT 0)
- `agi_override` (int(11), NOT NULL, DEFAULT 0)
- `heat_override` (int(10), NOT NULL, DEFAULT 0)
- `cold_override` (int(10), NOT NULL, DEFAULT 0)
- `magic_override` (int(10), NOT NULL, DEFAULT 0)
- `mental_override` (int(10), NOT NULL, DEFAULT 0)
- `divine_override` (int(10), NOT NULL, DEFAULT 0)
- `disease_override` (int(10), NOT NULL, DEFAULT 0)
- `poison_override` (int(10), NOT NULL, DEFAULT 0)
- `difficulty_override` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `SpawnLocationIDX` (`spawn_location_id`)
- KEY `ZoneIDX` (`zone_id`)
- CONSTRAINT `FK_placement1` FOREIGN KEY (`spawn_location_id`) REFERENCES `spawn_location_name` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
- CONSTRAINT `FK_placement2` FOREIGN KEY (`zone_id`) REFERENCES `zones` (`id`) ON DELETE CASCADE ON UPDATE CASCADE