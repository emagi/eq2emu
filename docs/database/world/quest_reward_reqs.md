## Table: `quest_reward_reqs`

**Description:**

Defines `quest_reward_reqs` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `list_id` (int(10), NOT NULL, DEFAULT 0)
- `class_req` (tinyint(3), NOT NULL, DEFAULT 0)
- `race_req` (tinyint(3), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id