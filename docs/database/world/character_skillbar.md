## Table: `character_skillbar`

**Description:**

Defines `character_skillbar` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `char_id` (int(10), NOT NULL, DEFAULT 0)
- `type` (tinyint(3), NOT NULL, DEFAULT 1)
- `hotbar` (int(10), NOT NULL, DEFAULT 0)
- `spell_id` (int(10), NOT NULL, DEFAULT 0)
- `tier` (tinyint(3), NOT NULL, DEFAULT 1)
- `slot` (int(10), NOT NULL, DEFAULT 0)
- `text_val` (varchar(255), NOT NULL, DEFAULT 'Unused')

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `NewIndex` (`hotbar`,`char_id`,`slot`)
- KEY `FK_character_skillbar` (`char_id`)
- CONSTRAINT `FK_character_skillbar` FOREIGN KEY (`char_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE ON UPDATE CASCADE