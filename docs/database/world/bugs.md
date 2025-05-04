## Table: `bugs`

**Description:**

Defines `bugs` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `account_id` (int(10), NOT NULL, DEFAULT 0)
- `player` (varchar(64), NOT NULL, DEFAULT ' ')
- `category` (varchar(64), NOT NULL, DEFAULT ' ')
- `subcategory` (varchar(64), NOT NULL, DEFAULT ' ')
- `causes_crash` (varchar(64), NOT NULL, DEFAULT ' ')
- `reproducible` (varchar(64), NOT NULL, DEFAULT ' ')
- `summary` (varchar(128), NOT NULL, DEFAULT ' ')
- `description` (text, NOT NULL)
- `version` (varchar(32), NOT NULL, DEFAULT ' ')
- `spawn_name` (varchar(64), NOT NULL, DEFAULT 'N/A')
- `spawn_id` (int(10), NOT NULL, DEFAULT 0)
- `bug_datetime` (timestamp, NOT NULL, DEFAULT current_timestamp())
- `zone_id` (int(10), NOT NULL, DEFAULT 0)
- `copied` (int(10), NOT NULL, DEFAULT 0)
- `dbversion` (int(10), NOT NULL, DEFAULT 0)
- `worldversion` (varchar(64), NOT NULL, DEFAULT '')

**Primary Keys:**
- id