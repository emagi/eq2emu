## Table: `spawn_widgets`

**Description:**

Defines `spawn_widgets` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `spawn_id` (int(10), NOT NULL)
- `widget_id` (int(10), NOT NULL, DEFAULT 0)
- `widget_x` (float, NOT NULL, DEFAULT 0)
- `widget_y` (float, NOT NULL, DEFAULT 0)
- `widget_z` (float, NOT NULL, DEFAULT 0)
- `include_heading` (tinyint(3), NOT NULL, DEFAULT 1)
- `include_location` (tinyint(3), NOT NULL, DEFAULT 1)
- `icon` (tinyint(3), NOT NULL, DEFAULT 4)
- `type` (enum('Generic','Door','Lift'), NOT NULL, DEFAULT 'Generic')
- `open_heading` (float, NOT NULL, DEFAULT -1)
- `closed_heading` (float, NOT NULL, DEFAULT -1)
- `open_x` (float, NOT NULL, DEFAULT 0)
- `open_y` (float, NOT NULL, DEFAULT 0)
- `open_z` (float, NOT NULL, DEFAULT 0)
- `action_spawn_id` (int(10), NOT NULL, DEFAULT 0)
- `open_sound_file` (varchar(255), NOT NULL, DEFAULT '0')
- `close_sound_file` (varchar(255), NOT NULL, DEFAULT '0')
- `open_duration` (smallint(5), NOT NULL, DEFAULT 10)
- `close_x` (float, NOT NULL, DEFAULT 0)
- `close_y` (float, NOT NULL, DEFAULT 0)
- `close_z` (float, NOT NULL, DEFAULT 0)
- `linked_spawn_id` (int(10), NOT NULL, DEFAULT 0)
- `house_id` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `SpawnIDX` (`spawn_id`)
- KEY `WidgetIDX` (`widget_id`)
- CONSTRAINT `FK_widgets_spawn` FOREIGN KEY (`spawn_id`) REFERENCES `spawn` (`id`) ON DELETE CASCADE ON UPDATE CASCADE