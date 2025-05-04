## Table: `channels`

**Description:**

Defines `channels` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `name` (varchar(128), NOT NULL)
- `password` (varchar(128), DEFAULT NULL)
- `level_restriction` (mediumint(8), NOT NULL, DEFAULT 0)
- `classes` (bigint(20), NOT NULL, DEFAULT 0)
- `races` (bigint(20), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `uk_channels_name` (`name`)