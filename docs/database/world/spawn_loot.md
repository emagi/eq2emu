## Table: `spawn_loot`

**Description:**

Defines `spawn_loot` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `spawn_id` (int(10), NOT NULL, DEFAULT 0)
- `loottable_id` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `NewIndex` (`loottable_id`,`spawn_id`)
- KEY `FK_loot_spawn` (`spawn_id`)
- CONSTRAINT `FK_loot_spawn` FOREIGN KEY (`spawn_id`) REFERENCES `spawn` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
- CONSTRAINT `FK_spawnloot_loottable` FOREIGN KEY (`loottable_id`) REFERENCES `loottable` (`id`) ON DELETE CASCADE ON UPDATE CASCADE