## Table: `spawn_location_group`

**Description:**

Defines `spawn_location_group` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `group_id` (int(10), NOT NULL, DEFAULT 0)
- `placement_id` (int(10), NOT NULL, DEFAULT 0)
- `name` (varchar(200), NOT NULL, DEFAULT '')

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `GroupPlacementIDX` (`group_id`,`placement_id`)
- KEY `FK_group_placement` (`placement_id`)
- CONSTRAINT `FK_group_placement` FOREIGN KEY (`placement_id`) REFERENCES `spawn_location_placement` (`id`) ON DELETE CASCADE ON UPDATE CASCADE