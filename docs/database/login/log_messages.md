## Table: `log_messages`

**Description:**

Defines `log_messages` table in the Login database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `log_time` (int(10), NOT NULL, DEFAULT 0)
- `log_type` (text, NOT NULL, DEFAULT '')
- `message` (text, NOT NULL)
- `account` (varchar(64), NOT NULL, DEFAULT '')
- `client_data_version` (int(10), NOT NULL, DEFAULT 0)
- `log_entry_archived` (tinyint(1), NOT NULL, DEFAULT 0)
- `type` (text, DEFAULT NULL)
- `name` (text, NOT NULL)
- `version` (text, DEFAULT NULL)

**Primary Keys:**
- id