## Table: `instances`

**Description:**

Defines `instances` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `zone_id` (int(10), NOT NULL, DEFAULT 0)
- `player_minlevel` (int(10), NOT NULL, DEFAULT 0)
- `player_maxlevel` (int(10), NOT NULL, DEFAULT 0)
- `player_avglevel` (int(10), NOT NULL, DEFAULT 0)
- `player_firstlevel` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `ZoneIDX` (`zone_id`)
- CONSTRAINT `FK_instances` FOREIGN KEY (`zone_id`) REFERENCES `zones` (`id`) ON DELETE CASCADE ON UPDATE CASCADE