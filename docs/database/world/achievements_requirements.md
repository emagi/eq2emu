## Table: `achievements_requirements`

**Description:**

Defines `achievements_requirements` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `achievement_id` (int(10), NOT NULL, DEFAULT 0)
- `name` (varchar(250), DEFAULT NULL)
- `qty_req` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `AchievementNameIDX` (`name`)
- KEY `FK_achievement_requirements` (`achievement_id`)
- CONSTRAINT `FK_achievement_requirements` FOREIGN KEY (`achievement_id`) REFERENCES `achievements` (`achievement_id`) ON UPDATE CASCADE