## Table: `character_buyback`

**Description:**

Defines `character_buyback` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `char_id` (int(10), NOT NULL, DEFAULT 0)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `quantity` (smallint(5), NOT NULL, DEFAULT 1)
- `price` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `FK_character_buyback` (`char_id`)
- CONSTRAINT `FK_character_buyback` FOREIGN KEY (`char_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE ON UPDATE CASCADE