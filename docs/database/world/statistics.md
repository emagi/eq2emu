## Table: `statistics`

**Description:**

Defines `statistics` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `char_id` (int(10), NOT NULL, DEFAULT 0)
- `guild_id` (int(10), NOT NULL, DEFAULT 0)
- `stat_id` (int(10), NOT NULL, DEFAULT 0)
- `stat_value` (double, NOT NULL, DEFAULT 0)
- `stat_date` (int(11), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `characterIDX` (`char_id`,`guild_id`,`stat_id`)