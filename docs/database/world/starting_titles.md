## Table: `starting_titles`

**Description:**

Defines `starting_titles` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `race_id` (smallint(5), NOT NULL, DEFAULT 255)
- `class_id` (smallint(5), NOT NULL, DEFAULT 255)
- `gender_id` (tinyint(1), NOT NULL, DEFAULT 255)
- `title_id` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `StartTitlesIDX` (`race_id`,`class_id`,`gender_id`,`title_id`)
- KEY `FK_starting_titles` (`title_id`)
- CONSTRAINT `FK_starting_titles` FOREIGN KEY (`title_id`) REFERENCES `titles` (`id`) ON DELETE CASCADE ON UPDATE CASCADE