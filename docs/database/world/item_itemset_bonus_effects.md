## Table: `item_itemset_bonus_effects`

**Description:**

Defines `item_itemset_bonus_effects` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `set_bonus_id` (int(10), NOT NULL)
- `indent` (tinyint(3), NOT NULL, DEFAULT 0)
- `description` (text, DEFAULT NULL)
- `percentage` (tinyint(3), NOT NULL, DEFAULT 100)
- `effect_order` (tinyint(3), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `UK_setbonusid_effectorder` (`set_bonus_id`,`effect_order`)
- CONSTRAINT `FK_item_itemset_bonus_effects_item_itemset_bonus` FOREIGN KEY (`set_bonus_id`) REFERENCES `item_itemset_bonus` (`id`) ON DELETE CASCADE ON UPDATE CASCADE