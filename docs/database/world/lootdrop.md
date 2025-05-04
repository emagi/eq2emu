## Table: `lootdrop`

**Description:**

Defines `lootdrop` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `loot_table_id` (int(10), NOT NULL, DEFAULT 0)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `item_charges` (smallint(5), NOT NULL, DEFAULT 1)
- `equip_item` (tinyint(3), NOT NULL, DEFAULT 0)
- `probability` (float, NOT NULL, DEFAULT 25)
- `no_drop_quest_completed` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `NewIndex` (`loot_table_id`)
- KEY `FK_lootdrop` (`item_id`)
- CONSTRAINT `FK_loottable` FOREIGN KEY (`loot_table_id`) REFERENCES `loottable` (`id`) ON DELETE CASCADE ON UPDATE CASCADE