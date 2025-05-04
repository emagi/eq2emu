## Table: `guild_event_defaults`

**Description:**

Defines `guild_event_defaults` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `event_id` (tinyint(3), NOT NULL)
- `event_name` (varchar(100), NOT NULL, DEFAULT '')
- `retain` (tinyint(1), NOT NULL, DEFAULT 1)
- `broadcast` (tinyint(1), NOT NULL, DEFAULT 1)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `EventNameIDX` (`event_id`,`event_name`)