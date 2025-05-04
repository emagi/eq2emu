## Table: `item_details_marketplace`

**Description:**

Defines `item_details_marketplace` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `marketplace_item_id` (int(10), NOT NULL, DEFAULT 0)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `item_icon` (smallint(5), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `MarketplaceIDX` (`marketplace_item_id`,`item_id`)
- CONSTRAINT `FK_item_details_marketplace` FOREIGN KEY (`marketplace_item_id`) REFERENCES `items` (`id`) ON UPDATE CASCADE