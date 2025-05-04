## Table: `item_details_skill`

**Description:**

Defines `item_details_skill` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `spell_id` (int(10), DEFAULT NULL)
- `spell_tier` (tinyint(3), NOT NULL, DEFAULT 1)
- `soe_spell_crc` (int(10), NOT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `FK_item_details_skill` (`item_id`)
- CONSTRAINT `FK_item_details_skill` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE