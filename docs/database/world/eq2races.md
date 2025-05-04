## Table: `eq2races`

**Description:**

Defines `eq2races` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `race` (int(10), NOT NULL, DEFAULT 0)
- `race_type` (int(10), NOT NULL, DEFAULT 0)
- `name` (varchar(64), DEFAULT NULL)
- `is_player_race` (tinyint(3), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id