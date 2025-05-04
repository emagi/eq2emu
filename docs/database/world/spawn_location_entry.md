## Table: `spawn_location_entry`

**Description:**

Defines `spawn_location_entry` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `spawn_id` (int(10), NOT NULL, DEFAULT 0)
- `spawn_location_id` (int(10), NOT NULL, DEFAULT 0)
- `spawnpercentage` (float, NOT NULL, DEFAULT 0)
- `condition` (tinyint(3), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `SpawnLocationIDX` (`spawn_location_id`)
- KEY `SpawnIDX` (`spawn_id`)
- CONSTRAINT `FK_spawn_entry1` FOREIGN KEY (`spawn_location_id`) REFERENCES `spawn_location_name` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
- CONSTRAINT `FK_spawn_entry2` FOREIGN KEY (`spawn_id`) REFERENCES `spawn` (`id`) ON DELETE CASCADE ON UPDATE CASCADE