## Table: `guild_colors`

**Description:**

Defines `guild_colors` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `guild_id` (int(10), NOT NULL, DEFAULT 0)
- `color_type` (enum('Emblem','Edge','Pattern'), DEFAULT NULL)
- `red` (float, NOT NULL, DEFAULT 0)
- `green` (float, NOT NULL, DEFAULT 0)
- `blue` (float, NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `GuildColorIDX` (`guild_id`,`color_type`)
- CONSTRAINT `FK_guild_colors` FOREIGN KEY (`guild_id`) REFERENCES `guilds` (`id`) ON DELETE CASCADE ON UPDATE CASCADE