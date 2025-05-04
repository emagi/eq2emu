## Table: `spawn_location_group_chances`

**Description:**

Defines `spawn_location_group_chances` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `group_id` (int(10), NOT NULL, DEFAULT 0)
- `percentage` (float, NOT NULL, DEFAULT 100)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `GroupLocationIDX` (`group_id`)
- CONSTRAINT `FK_group_chances` FOREIGN KEY (`group_id`) REFERENCES `spawn_location_group` (`group_id`) ON DELETE CASCADE ON UPDATE CASCADE