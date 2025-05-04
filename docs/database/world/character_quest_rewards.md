## Table: `character_quest_rewards`

**Description:**

Defines `character_quest_rewards` table in the World database.

**Columns:**
- `char_id` (int(10), NOT NULL, DEFAULT 0)
- `quest_id` (int(10), NOT NULL, DEFAULT 0)
- `indexed` (int(10), NOT NULL, DEFAULT 0)
- `is_temporary` (tinyint(3), NOT NULL, DEFAULT 0)
- `is_collection` (tinyint(3), NOT NULL, DEFAULT 0)
- `has_displayed` (tinyint(3), NOT NULL, DEFAULT 0)
- `tmp_coin` (bigint(20), NOT NULL, DEFAULT 0)
- `tmp_status` (int(10), NOT NULL, DEFAULT 0)
- `description` (text, NOT NULL, DEFAULT '')