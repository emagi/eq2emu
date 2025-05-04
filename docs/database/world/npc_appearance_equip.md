## Table: `npc_appearance_equip`

**Description:**

Defines `npc_appearance_equip` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `spawn_id` (int(10), NOT NULL, DEFAULT 0)
- `slot_id` (tinyint(3), NOT NULL, DEFAULT 0)
- `equip_type` (int(10), NOT NULL, DEFAULT 0)
- `red` (tinyint(3), NOT NULL, DEFAULT 0)
- `green` (tinyint(3), NOT NULL, DEFAULT 0)
- `blue` (tinyint(3), NOT NULL, DEFAULT 0)
- `highlight_red` (tinyint(3), NOT NULL, DEFAULT 0)
- `highlight_green` (tinyint(3), NOT NULL, DEFAULT 0)
- `highlight_blue` (tinyint(3), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `NewIndex` (`spawn_id`,`slot_id`)
- CONSTRAINT `FK_npc_equipment` FOREIGN KEY (`spawn_id`) REFERENCES `spawn` (`id`) ON DELETE CASCADE ON UPDATE CASCADE