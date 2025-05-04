## Table: `emotes`

**Description:**

Defines `emotes` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `name` (varchar(128), DEFAULT NULL)
- `visual_state_id` (int(10), NOT NULL, DEFAULT 0)
- `message` (varchar(255), DEFAULT NULL)
- `targeted_message` (varchar(255), DEFAULT NULL)
- `self_message` (varchar(255), DEFAULT NULL)
- `min_version_range` (int(10), NOT NULL, DEFAULT 0)
- `max_version_range` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id