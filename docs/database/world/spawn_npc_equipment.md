## Table: `spawn_npc_equipment`

**Description:**

Defines `spawn_npc_equipment` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `equipment_list_id` (int(10), NOT NULL, DEFAULT 0)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `description` (varchar(64), DEFAULT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `ListItemIDX` (`equipment_list_id`,`item_id`)
- KEY `FK_spawn_npc_equipment` (`item_id`)
- CONSTRAINT `FK_spawn_npc_equipment` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE