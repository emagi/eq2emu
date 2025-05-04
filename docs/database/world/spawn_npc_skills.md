## Table: `spawn_npc_skills`

**Description:**

Defines `spawn_npc_skills` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `skill_list_id` (int(10), NOT NULL, DEFAULT 0)
- `skill_id` (int(10), NOT NULL, DEFAULT 0)
- `starting_value` (smallint(5), NOT NULL, DEFAULT 25)
- `description` (varchar(64), DEFAULT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `ListSkillIDX` (`skill_list_id`,`skill_id`)
- KEY `FK_spawn_skills` (`skill_id`)
- CONSTRAINT `FK_spawn_skills` FOREIGN KEY (`skill_id`) REFERENCES `skills` (`id`) ON DELETE CASCADE ON UPDATE CASCADE