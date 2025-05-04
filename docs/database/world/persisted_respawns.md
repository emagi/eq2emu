## Table: `persisted_respawns`

**Description:**

Defines `persisted_respawns` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `spawn_location_entry_id` (int(10), NOT NULL, DEFAULT 0)
- `spawn_type` (smallint(5), NOT NULL, DEFAULT 0)
- `zone_id` (int(10), NOT NULL, DEFAULT 0)
- `respawn_time` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `idx_zone_spawn` (`zone_id`,`spawn_location_entry_id`)