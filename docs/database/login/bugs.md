## Table: `bugs`

**Description:**

Defines `bugs` table in the Login database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `Status` (enum('New','Invalid','Fixed','Server, NOT NULL, DEFAULT 'New')
- `world_id` (int(10), NOT NULL, DEFAULT 0)
- `account_id` (int(10), NOT NULL, DEFAULT 0)
- `player` (varchar(64), NOT NULL, DEFAULT ' ')
- `category` (varchar(64), NOT NULL, DEFAULT ' ')
- `subcategory` (varchar(64), NOT NULL, DEFAULT ' ')
- `causes_crash` (varchar(64), NOT NULL, DEFAULT '')
- `reproducible` (varchar(64), NOT NULL, DEFAULT '')
- `summary` (varchar(128), NOT NULL, DEFAULT ' ')
- `description` (text, NOT NULL)
- `version` (varchar(32), NOT NULL, DEFAULT '')
- `spawn_name` (varchar(64), NOT NULL, DEFAULT 'N/A')
- `spawn_id` (int(10), NOT NULL, DEFAULT 0)
- `bug_datetime` (timestamp, NOT NULL, DEFAULT current_timestamp())
- `zone_id` (int(10), NOT NULL, DEFAULT 0)
- `assign_to_forum_id` (int(10), DEFAULT 0)
- `fixed_by_forum_id` (int(10), NOT NULL, DEFAULT 0)
- `forum_id` (int(10), NOT NULL, DEFAULT 0)
- `post_id` (int(10), NOT NULL, DEFAULT 0)
- `priority` (tinyint(3), NOT NULL, DEFAULT 0)
- `bug_updated` (int(10), NOT NULL, DEFAULT 0)
- `bug_type` (tinyint(1), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id