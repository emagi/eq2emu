## Table: `instance_spawns_removed`

**Description:**

Defines `instance_spawns_removed` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `instance_id` (int(10), NOT NULL, DEFAULT 0)
- `spawn_type` (smallint(5), NOT NULL, DEFAULT 0)
- `spawn_location_entry_id` (int(10), NOT NULL, DEFAULT 0)
- `respawn_time` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `InstanceIDX` (`instance_id`)
- KEY `SpawnIDX` (`spawn_location_entry_id`)