## Table: `loot_global`

**Description:**

Defines `loot_global` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `type` (enum('Level','Racial','Zone'), NOT NULL, DEFAULT 'Zone')
- `loot_table` (int(10), NOT NULL)
- `value1` (int(10), NOT NULL, DEFAULT 0)
- `value2` (int(10), NOT NULL, DEFAULT 0)
- `value3` (int(10), NOT NULL, DEFAULT 0)
- `value4` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `FKLootTable` (`loot_table`)
- CONSTRAINT `FKLootTable` FOREIGN KEY (`loot_table`) REFERENCES `loottable` (`id`) ON DELETE CASCADE ON UPDATE CASCADE