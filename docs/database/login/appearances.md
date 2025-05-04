## Table: `appearances`

**Description:**

Defines `appearances` table in the Login database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `appearance_id` (int(10), NOT NULL)
- `name` (varchar(250), NOT NULL)
- `min_client_version` (smallint(5), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `AppIDX` (`appearance_id`)