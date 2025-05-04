## Table: `guild_event_filters`

**Description:**

Defines `guild_event_filters` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `guild_id` (int(10), NOT NULL, DEFAULT 0)
- `event_id` (int(10), NOT NULL, DEFAULT 0)
- `retain` (tinyint(1), NOT NULL, DEFAULT 1)
- `broadcast` (tinyint(1), NOT NULL, DEFAULT 1)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `FilterEventIDX` (`guild_id`,`event_id`)
- CONSTRAINT `FK_guild_event_filters` FOREIGN KEY (`guild_id`) REFERENCES `guilds` (`id`) ON DELETE CASCADE ON UPDATE CASCADE