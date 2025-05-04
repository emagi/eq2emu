## Table: `locations`

**Description:**

Defines `locations` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `zone_id` (int(10), NOT NULL, DEFAULT 0)
- `grid_id` (int(10), NOT NULL, DEFAULT 0)
- `name` (varchar(64), DEFAULT NULL)
- `include_y` (tinyint(3), NOT NULL, DEFAULT 0)
- `discovery` (tinyint(1), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `ZoneIDX` (`zone_id`)
- KEY `GridIDX` (`grid_id`)
- CONSTRAINT `FK_locations` FOREIGN KEY (`zone_id`) REFERENCES `zones` (`id`) ON DELETE CASCADE ON UPDATE CASCADE