## Table: `spawn_location_group_associations`

**Description:**

Defines `spawn_location_group_associations` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `group_id1` (int(10), NOT NULL, DEFAULT 0)
- `group_id2` (int(10), NOT NULL, DEFAULT 0)
- `type` (enum('SPAWN_SEPARATELY'), NOT NULL, DEFAULT 'SPAWN_SEPARATELY')

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `GroupAssociationIDX` (`group_id1`,`group_id2`)
- KEY `FK_group_association2` (`group_id2`)
- CONSTRAINT `FK_group_association1` FOREIGN KEY (`group_id1`) REFERENCES `spawn_location_group` (`group_id`) ON DELETE CASCADE ON UPDATE CASCADE
- CONSTRAINT `FK_group_association2` FOREIGN KEY (`group_id2`) REFERENCES `spawn_location_group` (`group_id`) ON DELETE CASCADE ON UPDATE CASCADE