## Table: `guild_ranks`

**Description:**

Defines `guild_ranks` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `guild_id` (int(10), NOT NULL, DEFAULT 0)
- `rank_id` (tinyint(3), NOT NULL, DEFAULT 0)
- `rank_name` (varchar(64), NOT NULL)
- `permission1` (int(10), NOT NULL, DEFAULT 0)
- `permission2` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `GuildRankIDX` (`guild_id`,`rank_id`)
- CONSTRAINT `FK_guild_ranks` FOREIGN KEY (`guild_id`) REFERENCES `guilds` (`id`) ON DELETE CASCADE ON UPDATE CASCADE