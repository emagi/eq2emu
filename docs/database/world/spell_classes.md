## Table: `spell_classes`

**Description:**

Defines `spell_classes` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `spell_id` (int(10), NOT NULL, DEFAULT 0)
- `adventure_class_id` (tinyint(3), NOT NULL, DEFAULT 255)
- `tradeskill_class_id` (tinyint(3), NOT NULL, DEFAULT 255)
- `level` (tinyint(3), NOT NULL, DEFAULT 1)
- `classic_level` (float, NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `spell_id` (`spell_id`,`adventure_class_id`)
- KEY `FK_spell_classes` (`spell_id`)
- CONSTRAINT `FK_spell_classes` FOREIGN KEY (`spell_id`) REFERENCES `spells` (`id`) ON DELETE CASCADE ON UPDATE CASCADE