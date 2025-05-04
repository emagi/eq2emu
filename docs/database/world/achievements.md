## Table: `achievements`

**Description:**

Defines `achievements` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `achievement_id` (int(10), NOT NULL, DEFAULT 0)
- `title` (varchar(50), NOT NULL)
- `uncompleted_text` (varchar(255), DEFAULT NULL)
- `completed_text` (varchar(255), DEFAULT NULL)
- `category` (varchar(50), DEFAULT NULL)
- `expansion` (varchar(50), DEFAULT NULL)
- `icon` (smallint(5), NOT NULL, DEFAULT 0)
- `point_value` (int(10), NOT NULL, DEFAULT 0)
- `qty_req` (int(10), NOT NULL, DEFAULT 0)
- `hide_achievement` (tinyint(1), NOT NULL, DEFAULT 0)
- `unknown3a` (int(10), NOT NULL, DEFAULT 0)
- `unknown3b` (int(10), NOT NULL, DEFAULT 0)
- `max_version` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `AchievementTitleIDX` (`achievement_id`,`title`)