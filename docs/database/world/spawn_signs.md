## Table: `spawn_signs`

**Description:**

Defines `spawn_signs` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `spawn_id` (int(10), NOT NULL, DEFAULT 0)
- `type` (enum('Zone','Generic'), NOT NULL, DEFAULT 'Generic')
- `zone_id` (int(10), NOT NULL, DEFAULT 0)
- `widget_id` (int(10), NOT NULL, DEFAULT 0)
- `title` (varchar(250), DEFAULT NULL)
- `widget_x` (float, NOT NULL, DEFAULT 0)
- `widget_y` (float, NOT NULL, DEFAULT 0)
- `widget_z` (float, NOT NULL, DEFAULT 0)
- `icon` (smallint(6), NOT NULL, DEFAULT 0)
- `description` (text, NOT NULL)
- `sign_distance` (float, NOT NULL, DEFAULT 0)
- `zone_x` (float, NOT NULL, DEFAULT 0)
- `zone_y` (float, NOT NULL, DEFAULT 0)
- `zone_z` (float, NOT NULL, DEFAULT 0)
- `zone_heading` (float, NOT NULL, DEFAULT 0)
- `include_heading` (tinyint(3), NOT NULL, DEFAULT 1)
- `include_location` (tinyint(3), NOT NULL, DEFAULT 1)
- `language` (tinyint(3), DEFAULT 0)
- `char_id` (int(10), DEFAULT 0)
- `char_name` (varchar(50), DEFAULT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `SpawnIDX` (`spawn_id`)
- CONSTRAINT `FK_signs_spawn` FOREIGN KEY (`spawn_id`) REFERENCES `spawn` (`id`) ON DELETE CASCADE ON UPDATE CASCADE