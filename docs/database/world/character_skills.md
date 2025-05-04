## Table: `character_skills`

**Description:**

Defines `character_skills` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `char_id` (int(10), NOT NULL, DEFAULT 0)
- `skill_id` (int(10), NOT NULL, DEFAULT 0)
- `current_val` (smallint(5), NOT NULL, DEFAULT 1)
- `max_val` (smallint(5), NOT NULL, DEFAULT 1)
- `progress` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `CharSkillIDX` (`char_id`,`skill_id`)
- KEY `FK_skills_character` (`skill_id`)
- CONSTRAINT `FK_character_skills` FOREIGN KEY (`char_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
- CONSTRAINT `FK_skills_character` FOREIGN KEY (`skill_id`) REFERENCES `skills` (`id`) ON DELETE CASCADE ON UPDATE CASCADE