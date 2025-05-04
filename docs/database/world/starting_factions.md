## Table: `starting_factions`

**Description:**

Defines `starting_factions` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `faction_id` (int(10), NOT NULL, DEFAULT 0)
- `starting_city` (tinyint(3), NOT NULL, DEFAULT 0)
- `value` (mediumint(9), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `NewIndex` (`faction_id`,`starting_city`)
- CONSTRAINT `FK_starting_factions` FOREIGN KEY (`faction_id`) REFERENCES `factions` (`id`) ON DELETE CASCADE ON UPDATE CASCADE