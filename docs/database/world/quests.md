## Table: `quests`

**Description:**

Defines `quests` table in the World database.

**Columns:**
- `quest_id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `name` (varchar(64), DEFAULT NULL)
- `type` (varchar(64), DEFAULT NULL)
- `zone` (varchar(64), DEFAULT NULL)
- `level` (tinyint(3), NOT NULL, DEFAULT 0)
- `enc_level` (tinyint(3), NOT NULL, DEFAULT 0)
- `description` (text, DEFAULT NULL)
- `spawn_id` (int(10), NOT NULL, DEFAULT 0)
- `completed_text` (text, DEFAULT NULL)
- `lua_script` (varchar(255), DEFAULT NULL)
- `shareable_flag` (int(10), NOT NULL, DEFAULT 0)
- `deleteable` (tinyint(3), NOT NULL, DEFAULT 1)
- `status_to_earn_min` (int(10), NOT NULL, DEFAULT 0)
- `status_to_earn_max` (int(10), NOT NULL, DEFAULT 0)
- `hide_reward` (tinyint(3), NOT NULL, DEFAULT 0)

**Primary Keys:**
- quest_id