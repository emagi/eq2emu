## Table: `merchant_inventory`

**Description:**

Defines `merchant_inventory` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `inventory_id` (int(10), NOT NULL, DEFAULT 0)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `quantity` (smallint(5), NOT NULL, DEFAULT 65535)
- `price_item_id` (int(10), NOT NULL, DEFAULT 0)
- `price_item_qty` (smallint(5), NOT NULL, DEFAULT 0)
- `price_item2_id` (int(10), NOT NULL, DEFAULT 0)
- `price_item2_qty` (smallint(5), NOT NULL, DEFAULT 0)
- `price_status` (int(10), NOT NULL, DEFAULT 0)
- `price_coins` (int(10), NOT NULL, DEFAULT 0)
- `price_stationcash` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `InventoryIDX` (`inventory_id`)
- KEY `FK_merchant_inventory2` (`item_id`)
- CONSTRAINT `FK_merchant_inventory1` FOREIGN KEY (`inventory_id`) REFERENCES `merchants` (`inventory_id`) ON DELETE CASCADE ON UPDATE CASCADE
- CONSTRAINT `FK_merchant_inventory2` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE