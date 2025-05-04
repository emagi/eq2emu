## Table: `revive_points`

**Description:**

Defines `revive_points` table in the World database.

**Columns:**
- `id` (int(11), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `location_name` (varchar(64), DEFAULT NULL)
- `zone_id` (int(10), NOT NULL, DEFAULT 12)
- `respawn_zone_id` (int(10), NOT NULL, DEFAULT 12)
- `safe_x` (float, NOT NULL, DEFAULT 0)
- `safe_y` (float, NOT NULL, DEFAULT 0)
- `safe_z` (float, NOT NULL, DEFAULT 0)
- `heading` (float, NOT NULL, DEFAULT 0)
- `always_included` (tinyint(1), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `FK_respawn_points` (`respawn_zone_id`)
- KEY `FK_revive_zone` (`zone_id`)
- CONSTRAINT `FK_respawn_points` FOREIGN KEY (`respawn_zone_id`) REFERENCES `zones` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
- CONSTRAINT `FK_revive_zone` FOREIGN KEY (`zone_id`) REFERENCES `zones` (`id`) ON DELETE CASCADE ON UPDATE CASCADE