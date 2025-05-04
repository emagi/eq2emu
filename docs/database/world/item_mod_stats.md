## Table: `item_mod_stats`

**Description:**

Defines `item_mod_stats` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `item_id` (int(10), NOT NULL)
- `type` (tinyint(3), NOT NULL, DEFAULT 0)
- `subtype` (smallint(6), NOT NULL, DEFAULT 0)
- `iValue` (int(11), DEFAULT NULL)
- `fValue` (float, DEFAULT NULL)
- `sValue` (text, DEFAULT NULL)
- `level` (tinyint(3), NOT NULL, DEFAULT 0)
- `stats_order` (tinyint(3), NOT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `UK_setbonusid_statsorder` (`item_id`,`stats_order`) USING BTREE
- CONSTRAINT `FK_item_mod_stats_items` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`) ON UPDATE CASCADE