## Table: `item_effects`

**Description:**

Defines `item_effects` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `effect` (varchar(512), NOT NULL, DEFAULT 'Unknown')
- `percentage` (tinyint(3), NOT NULL, DEFAULT 100)
- `bullet` (tinyint(3), NOT NULL, DEFAULT 0)
- `index` (int(11), NOT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `UK_itemid_index` (`item_id`,`index`)
- KEY `FK_item_effects` (`item_id`)
- KEY `EffectIDX` (`effect`)
- CONSTRAINT `FK_item_effects` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE