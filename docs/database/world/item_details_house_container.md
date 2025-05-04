## Table: `item_details_house_container`

**Description:**

Defines `item_details_house_container` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `num_slots` (smallint(5), NOT NULL, DEFAULT 0)
- `allowed_types` (bigint(20), NOT NULL, DEFAULT 0)
- `unknown12` (int(10), NOT NULL, DEFAULT 0)
- `unknown13` (tinyint(3), NOT NULL, DEFAULT 0)
- `broker_commission` (smallint(5), NOT NULL, DEFAULT 0)
- `fence_commission` (smallint(5), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `HouseContainerIDX` (`item_id`)
- CONSTRAINT `FK_item_details_house_container` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE