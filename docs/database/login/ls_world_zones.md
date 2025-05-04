## Table: `ls_world_zones`

**Description:**

Defines `ls_world_zones` table in the Login database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `server_id` (int(10), NOT NULL, DEFAULT 0)
- `zone_id` (int(10), NOT NULL, DEFAULT 0)
- `name` (varchar(128), NOT NULL, DEFAULT '')
- `description` (varchar(128), NOT NULL, DEFAULT '')

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `id` (`id`)
- UNIQUE KEY `IDXServerZone` (`server_id`,`zone_id`)
- KEY `id_2` (`id`)