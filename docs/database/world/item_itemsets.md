## Table: `item_itemsets`

**Description:**

Defines `item_itemsets` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `set_name` (varchar(255), NOT NULL)
- `bPvpDesc` (tinyint(4), NOT NULL)
- `itemLevel` (smallint(6), NOT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `UK_ItemSet_IDX` (`bPvpDesc`,`set_name`,`itemLevel`) USING BTREE
- KEY `SetNameIDX` (`set_name`)