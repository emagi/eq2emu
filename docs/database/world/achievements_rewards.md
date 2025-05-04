## Table: `achievements_rewards`

**Description:**

Defines `achievements_rewards` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `achievement_id` (int(10), NOT NULL, DEFAULT 0)
- `reward` (varchar(250), DEFAULT '"')

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `AchievementRewardIDX` (`achievement_id`,`reward`)
- CONSTRAINT `FK_achievement_rewards` FOREIGN KEY (`achievement_id`) REFERENCES `achievements` (`achievement_id`) ON UPDATE CASCADE