## Table: `spell_traits`

**Description:**

Defines `spell_traits` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `spell_id` (int(10), NOT NULL, DEFAULT 0)
- `level` (tinyint(3), NOT NULL, DEFAULT 0)
- `class_req` (tinyint(3), NOT NULL, DEFAULT 255)
- `race_req` (tinyint(3), NOT NULL, DEFAULT 255)
- `isTrait` (tinyint(1), NOT NULL, DEFAULT 0)
- `isInate` (tinyint(1), NOT NULL, DEFAULT 0)
- `isFocusEffect` (tinyint(1), NOT NULL, DEFAULT 0)
- `isTraining` (tinyint(1), NOT NULL, DEFAULT 0)
- `tier` (tinyint(3), NOT NULL, DEFAULT 0)
- `group` (tinyint(3), NOT NULL, DEFAULT 0)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `is_active` (tinyint(1), NOT NULL, DEFAULT 1)
- `insert_id` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `FK_spell_traits` (`spell_id`)
- CONSTRAINT `FK_spell_traits` FOREIGN KEY (`spell_id`) REFERENCES `spells` (`id`) ON DELETE CASCADE ON UPDATE CASCADE