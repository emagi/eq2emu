## Table: `guild_points_history`

**Description:**

Defines `guild_points_history` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `guild_id` (int(10), NOT NULL, DEFAULT 0)
- `char_id` (int(10), NOT NULL, DEFAULT 0)
- `points_date` (int(10), NOT NULL, DEFAULT 0)
- `modified_by` (varchar(64), NOT NULL, DEFAULT '')
- `comment` (text, DEFAULT NULL)
- `points` (float, NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `GuildCharIDX` (`guild_id`,`char_id`)
- KEY `FK_char_points_history` (`char_id`)
- CONSTRAINT `FK_char_points_history` FOREIGN KEY (`char_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
- CONSTRAINT `FK_guild_points_history` FOREIGN KEY (`guild_id`) REFERENCES `guilds` (`id`) ON DELETE CASCADE ON UPDATE CASCADE