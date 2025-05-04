## Table: `spawn_ground`

**Description:**

Defines `spawn_ground` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `spawn_id` (int(10), NOT NULL, DEFAULT 0)
- `number_harvests` (tinyint(3), NOT NULL, DEFAULT 3)
- `num_attempts_per_harvest` (tinyint(3), NOT NULL, DEFAULT 1)
- `groundspawn_id` (int(10), NOT NULL, DEFAULT 0)
- `collection_skill` (enum('Unused','Mining','Gathering','Fishing','Trapping','Foresting','Collecting'), NOT NULL, DEFAULT 'Unused')
- `randomize_heading` (tinyint(3), DEFAULT 1)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `SpawnIdx` (`spawn_id`)
- KEY `GroundIDX` (`groundspawn_id`)
- CONSTRAINT `FK_groundspawn` FOREIGN KEY (`spawn_id`) REFERENCES `spawn` (`id`) ON DELETE CASCADE ON UPDATE CASCADE