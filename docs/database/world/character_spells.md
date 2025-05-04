## Table: `character_spells`

**Description:**

Defines `character_spells` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `char_id` (int(10), NOT NULL, DEFAULT 0)
- `spell_id` (int(10), NOT NULL, DEFAULT 0)
- `tier` (tinyint(3), NOT NULL, DEFAULT 1)
- `knowledge_slot` (mediumint(9), NOT NULL, DEFAULT -1)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `NewIndex` (`char_id`,`spell_id`,`tier`)
- KEY `FK_char_spells` (`spell_id`)
- CONSTRAINT `FK_char_spells` FOREIGN KEY (`spell_id`) REFERENCES `spells` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
- CONSTRAINT `FK_character_spells` FOREIGN KEY (`char_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE ON UPDATE CASCADE