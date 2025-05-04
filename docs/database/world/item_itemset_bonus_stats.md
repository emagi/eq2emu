## Table: `item_itemset_bonus_stats`

**Description:**

Defines `item_itemset_bonus_stats` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `set_bonus_id` (int(10), NOT NULL)
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
- UNIQUE KEY `UK_setbonusid_statsorder` (`set_bonus_id`,`stats_order`)
- CONSTRAINT `FK_item_itemset_bonus_stats_item_itemset_bonus` FOREIGN KEY (`set_bonus_id`) REFERENCES `item_itemset_bonus` (`id`) ON DELETE CASCADE ON UPDATE CASCADE