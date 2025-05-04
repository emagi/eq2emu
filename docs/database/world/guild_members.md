## Table: `guild_members`

**Description:**

Defines `guild_members` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `guild_id` (int(10), NOT NULL, DEFAULT 0)
- `char_id` (int(10), NOT NULL, DEFAULT 0)
- `recruiter_id` (int(10), NOT NULL, DEFAULT 0)
- `guild_status` (int(10), NOT NULL, DEFAULT 0)
- `points` (float, NOT NULL, DEFAULT 0)
- `rank_id` (tinyint(3), NOT NULL, DEFAULT 7)
- `member_flags` (tinyint(3), NOT NULL, DEFAULT 0)
- `join_date` (int(10), NOT NULL, DEFAULT 0)
- `note` (varchar(250), DEFAULT NULL)
- `officer_note` (varchar(250), DEFAULT NULL)
- `recruiting_message` (varchar(512), DEFAULT NULL)
- `recruiter_picture_data` (blob, DEFAULT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `GuildCharIDX` (`guild_id`,`char_id`)
- KEY `FK_guild_members2` (`char_id`)
- CONSTRAINT `FK_guild_members1` FOREIGN KEY (`guild_id`) REFERENCES `guilds` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
- CONSTRAINT `FK_guild_members2` FOREIGN KEY (`char_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE ON UPDATE CASCADE