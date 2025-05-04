## Table: `dialog_play_flavors`

**Description:**

Defines `dialog_play_flavors` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, NOT NULL)
- `npc_id` (int(10), NOT NULL)
- `flavor_id` (int(10), NOT NULL)
- `log_id` (int(10), NOT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `UniqueNpcFlavor` (`npc_id`,`flavor_id`)
- KEY `FK__playflavors_dialog_flavors` (`flavor_id`)
- CONSTRAINT `FK__playflavors_dialog_flavors` FOREIGN KEY (`flavor_id`) REFERENCES `dialog_flavors` (`id`) ON UPDATE CASCADE
- CONSTRAINT `FK__playflavors_dialog_npcs` FOREIGN KEY (`npc_id`) REFERENCES `dialog_npcs` (`id`) ON UPDATE CASCADE