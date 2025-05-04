## Table: `faction_alliances`

**Description:**

Defines `faction_alliances` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `faction_id` (int(10), NOT NULL, DEFAULT 0)
- `friend_faction` (int(10), NOT NULL, DEFAULT 0)
- `hostile_faction` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `FactionIDX` (`faction_id`,`friend_faction`,`hostile_faction`)
- CONSTRAINT `FK_faction_alliances` FOREIGN KEY (`faction_id`) REFERENCES `factions` (`id`) ON UPDATE CASCADE