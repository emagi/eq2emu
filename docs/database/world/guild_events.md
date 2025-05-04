## Table: `guild_events`

**Description:**

Defines `guild_events` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `guild_id` (int(10), NOT NULL, DEFAULT 0)
- `event_id` (bigint(20), NOT NULL, DEFAULT 0)
- `event_date` (int(10), NOT NULL, DEFAULT 0)
- `event_type` (smallint(5), NOT NULL, DEFAULT 0)
- `description` (text, NOT NULL)
- `display` (tinyint(1), NOT NULL, DEFAULT 0)
- `locked` (tinyint(1), NOT NULL, DEFAULT 0)
- `archived` (tinyint(1), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `GuildIDX` (`guild_id`,`event_id`)
- KEY `EventDateIDX` (`event_date`)
- CONSTRAINT `FK_guild_events` FOREIGN KEY (`guild_id`) REFERENCES `guilds` (`id`) ON DELETE CASCADE ON UPDATE CASCADE