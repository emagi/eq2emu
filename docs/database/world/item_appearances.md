## Table: `item_appearances`

**Description:**

Defines `item_appearances` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `equip_type` (int(10), NOT NULL, DEFAULT 0)
- `red` (tinyint(3), NOT NULL, DEFAULT 0)
- `green` (tinyint(3), NOT NULL, DEFAULT 0)
- `blue` (tinyint(3), NOT NULL, DEFAULT 0)
- `highlight_red` (tinyint(3), NOT NULL, DEFAULT 0)
- `highlight_green` (tinyint(3), NOT NULL, DEFAULT 0)
- `highlight_blue` (tinyint(3), NOT NULL, DEFAULT 0)
- `appearance_type` (smallint(5), NOT NULL, DEFAULT 0)
- `slot` (smallint(5), NOT NULL, DEFAULT 0)
- `house_placement_type` (int(10), NOT NULL, DEFAULT 0)
- `vis_state` (int(10), NOT NULL, DEFAULT 4294967295)
- `vis_state2` (int(10), NOT NULL, DEFAULT 4294967295)
- `mount_type` (int(10), NOT NULL, DEFAULT 4294967295)
- `heraldry` (binary(7), NOT NULL, DEFAULT '0\0\0\0\0\0\0')
- `reforging_decoration` (int(10), NOT NULL, DEFAULT 0)
- `bWeaponUnk` (tinyint(1), NOT NULL, DEFAULT 0)
- `b2hWeapon` (tinyint(1), NOT NULL, DEFAULT 0)
- `bUnknown` (tinyint(1), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `FK_new_appearances_items` (`item_id`)
- CONSTRAINT `FK_new_appearances_items` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE