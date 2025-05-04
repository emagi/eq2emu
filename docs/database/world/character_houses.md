## Table: `character_houses`

**Description:**

Defines `character_houses` table in the World database.

**Columns:**
- `id` (bigint(20), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `char_id` (int(10), NOT NULL, DEFAULT 0)
- `house_id` (int(10), NOT NULL, DEFAULT 0)
- `instance_id` (int(10), NOT NULL, DEFAULT 0)
- `upkeep_due` (int(10), NOT NULL, DEFAULT 0)
- `escrow_coins` (bigint(20), NOT NULL, DEFAULT 0)
- `escrow_status` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `FK_char` (`char_id`)
- CONSTRAINT `FK_char` FOREIGN KEY (`char_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE ON UPDATE CASCADE