## Table: `bot_equipment`

**Description:**

Defines `bot_equipment` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `bot_id` (int(10), NOT NULL)
- `slot` (tinyint(3), NOT NULL)
- `item_id` (int(11), NOT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `bot_id_slot` (`bot_id`,`slot`)
- CONSTRAINT `FK__bots` FOREIGN KEY (`bot_id`) REFERENCES `bots` (`id`) ON DELETE CASCADE ON UPDATE CASCADE