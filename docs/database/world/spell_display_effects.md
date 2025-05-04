## Table: `spell_display_effects`

**Description:**

Defines `spell_display_effects` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `spell_id` (int(10), NOT NULL, DEFAULT 0)
- `tier` (tinyint(3), NOT NULL, DEFAULT 1)
- `percentage` (tinyint(3), DEFAULT 100)
- `description` (text, NOT NULL)
- `bullet` (tinyint(3), NOT NULL, DEFAULT 0)
- `index` (tinyint(3), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `FK_spell_display_effects` (`spell_id`)
- CONSTRAINT `FK_spell_display_effects` FOREIGN KEY (`spell_id`) REFERENCES `spells` (`id`) ON DELETE CASCADE ON UPDATE CASCADE