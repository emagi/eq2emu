## Table: `account`

**Description:**

Defines `account` table in the Login database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `name` (varchar(64), NOT NULL, DEFAULT '')
- `passwd` (varchar(512), NOT NULL, DEFAULT '')
- `ip_address` (varchar(50), NOT NULL, DEFAULT '0')
- `email_address` (varchar(50), NOT NULL, DEFAULT 'Unknown')
- `created_date` (int(10), NOT NULL, DEFAULT 0)
- `key1` (varchar(64), NOT NULL, DEFAULT '0')
- `last_update` (timestamp, NOT NULL, DEFAULT current_timestamp() ON UPDATE current_timestamp())
- `hack_count` (tinyint(3), NOT NULL, DEFAULT 0)
- `last_client_version` (smallint(5), NOT NULL, DEFAULT 0)
- `account_enabled` (tinyint(1), NOT NULL, DEFAULT 1)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `Name` (`name`)