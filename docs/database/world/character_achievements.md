## Table: `character_achievements`

**Description:**

Defines `character_achievements` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `char_id` (int(10), NOT NULL, DEFAULT 0)
- `achievement_id` (int(10), NOT NULL, DEFAULT 0)
- `completed_date` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `FK_character_achievements` (`char_id`)
- CONSTRAINT `FK_character_achievements` FOREIGN KEY (`char_id`) REFERENCES `characters` (`id`) ON UPDATE CASCADE