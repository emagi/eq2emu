## Table: `character_factions`

**Description:**

Defines `character_factions` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `char_id` (int(10), NOT NULL, DEFAULT 0)
- `faction_id` (int(10), NOT NULL, DEFAULT 0)
- `faction_level` (mediumint(9), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `NewIndex` (`char_id`,`faction_id`)
- KEY `FK_factions` (`faction_id`)
- CONSTRAINT `FK_character_factions` FOREIGN KEY (`char_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
- CONSTRAINT `FK_factions` FOREIGN KEY (`faction_id`) REFERENCES `factions` (`id`) ON DELETE CASCADE ON UPDATE CASCADE