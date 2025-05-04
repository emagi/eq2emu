## Table: `starting_spells`

**Description:**

Defines `starting_spells` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `race_id` (tinyint(3), NOT NULL, DEFAULT 255)
- `class_id` (tinyint(3), NOT NULL, DEFAULT 255)
- `spell_id` (int(10), NOT NULL, DEFAULT 0)
- `tier` (tinyint(3), NOT NULL, DEFAULT 1)
- `knowledge_slot` (mediumint(9), NOT NULL, DEFAULT -1)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `SpellsIDX` (`race_id`,`class_id`,`spell_id`)
- KEY `FK_starting_spells` (`spell_id`)
- CONSTRAINT `FK_starting_spells` FOREIGN KEY (`spell_id`) REFERENCES `spells` (`id`) ON DELETE CASCADE ON UPDATE CASCADE