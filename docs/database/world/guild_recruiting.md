## Table: `guild_recruiting`

**Description:**

Defines `guild_recruiting` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `guild_id` (int(10), NOT NULL, DEFAULT 0)
- `short_desc` (varchar(250), NOT NULL, DEFAULT '')
- `full_desc` (text, DEFAULT NULL)
- `min_level` (tinyint(3), NOT NULL, DEFAULT 0)
- `play_style` (tinyint(3), NOT NULL, DEFAULT 0)
- `looking_for` (tinyint(3), NOT NULL, DEFAULT 0)
- `descriptive_tag1` (tinyint(3), NOT NULL, DEFAULT 0)
- `descriptive_tag2` (tinyint(3), NOT NULL, DEFAULT 0)
- `descriptive_tag3` (tinyint(3), NOT NULL, DEFAULT 0)
- `descriptive_tag4` (tinyint(3), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `GuildIDX` (`guild_id`)
- CONSTRAINT `FK_guild_recruiting` FOREIGN KEY (`guild_id`) REFERENCES `guilds` (`id`) ON DELETE CASCADE ON UPDATE CASCADE