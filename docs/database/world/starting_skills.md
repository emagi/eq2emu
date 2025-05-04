## Table: `starting_skills`

**Description:**

Defines `starting_skills` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `class_id` (tinyint(3), NOT NULL, DEFAULT 255)
- `race_id` (tinyint(3), NOT NULL, DEFAULT 255)
- `skill_id` (int(10), NOT NULL, DEFAULT 0)
- `current_val` (smallint(5), NOT NULL, DEFAULT 1)
- `max_val` (smallint(5), NOT NULL, DEFAULT 1)
- `progress` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `RaceClassIDX` (`race_id`,`class_id`,`skill_id`)
- KEY `FK_starting_skills` (`skill_id`)
- CONSTRAINT `FK_starting_skills` FOREIGN KEY (`skill_id`) REFERENCES `skills` (`id`) ON UPDATE CASCADE