## Table: `guild_ranks_defaults`

**Description:**

Defines `guild_ranks_defaults` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `rank_id` (int(10), NOT NULL, DEFAULT 0)
- `rank_name` (varchar(250), NOT NULL)
- `permission1` (int(10), NOT NULL, DEFAULT 0)
- `permission2` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `RankNameIDX` (`rank_name`)