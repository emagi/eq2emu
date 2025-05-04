## Table: `login_worldstats`

**Description:**

Defines `login_worldstats` table in the Login database.

**Columns:**
- `id` (int(11), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `world_id` (int(11), NOT NULL, DEFAULT 0)
- `world_status` (int(11), NOT NULL, DEFAULT -1)
- `current_players` (int(11), NOT NULL, DEFAULT 0)
- `current_zones` (int(11), NOT NULL, DEFAULT 0)
- `connected_time` (timestamp, NOT NULL, DEFAULT current_timestamp())
- `last_update` (timestamp, NOT NULL, DEFAULT '0000-00-00 00:00:00')
- `world_max_level` (tinyint(3), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `NewIndex` (`world_id`)