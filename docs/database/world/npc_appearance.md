## Table: `npc_appearance`

**Description:**

Defines `npc_appearance` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `spawn_id` (int(10), NOT NULL, DEFAULT 0)
- `signed_value` (tinyint(4), NOT NULL, DEFAULT 0)
- `type` (varchar(32), NOT NULL, DEFAULT ' ')
- `red` (smallint(6), NOT NULL, DEFAULT 0)
- `green` (smallint(6), NOT NULL, DEFAULT 0)
- `blue` (smallint(6), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `SpawnIDX` (`spawn_id`)
- CONSTRAINT `FK_npc_appearance` FOREIGN KEY (`spawn_id`) REFERENCES `spawn` (`id`) ON DELETE CASCADE ON UPDATE CASCADE