## Table: `login_versions`

**Description:**

Defines `login_versions` table in the Login database.

**Columns:**
- `id` (smallint(5), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `version` (varchar(30), NOT NULL, DEFAULT '')

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `NewIndex` (`version`)