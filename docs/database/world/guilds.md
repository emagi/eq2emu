## Table: `guilds`

**Description:**

Defines `guilds` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `name` (varchar(64), NOT NULL, DEFAULT '')
- `motd` (text, DEFAULT NULL)
- `level` (tinyint(3), NOT NULL, DEFAULT 1)
- `xp` (bigint(20), NOT NULL, DEFAULT 0)
- `xp_needed` (bigint(20), NOT NULL, DEFAULT 0)
- `formed_on` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `GuildNameIDX` (`name`)