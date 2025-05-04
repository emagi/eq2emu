## Table: `item_details_armorset`

**Description:**

Defines `item_details_armorset` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `soe_id` (int(11), NOT NULL, DEFAULT 0)
- `armorset_item_id` (int(10), NOT NULL, DEFAULT 0)
- `item_crc` (int(10), NOT NULL, DEFAULT 0)
- `item_name` (varchar(255), NOT NULL, DEFAULT 'N/A')
- `item_icon` (smallint(5), NOT NULL, DEFAULT 0)
- `unknown_piece` (int(10), NOT NULL, DEFAULT 0)
- `language_type` (tinyint(3), NOT NULL, DEFAULT 1)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `ArmorSetIDX` (`item_id`,`soe_id`)
- CONSTRAINT `FK_item_details_armorset` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE