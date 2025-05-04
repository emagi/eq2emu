## Table: `item_details_adornments`

**Description:**

Defines `item_details_adornments` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `duration` (float, NOT NULL, DEFAULT 0)
- `item_types` (bigint(20), NOT NULL, DEFAULT 0)
- `slot_type` (smallint(5), NOT NULL, DEFAULT 0)
- `description` (text, DEFAULT NULL)
- `description2` (text, DEFAULT NULL)
- `unk1` (int(10), NOT NULL, DEFAULT 0)
- `unk2` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `item_id` (`item_id`)
- CONSTRAINT `FK_ida_itemid` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE