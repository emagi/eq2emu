## Table: `table_versions`

**Description:**

Defines `table_versions` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `name` (varchar(64), NOT NULL, DEFAULT '')
- `version` (int(10), NOT NULL, DEFAULT 0)
- `download_version` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `UniqueName` (`name`)