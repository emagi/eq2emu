## Table: `item_details_house`

**Description:**

Defines `item_details_house` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `rent_reduction` (int(10), NOT NULL, DEFAULT 0)
- `status_rent_reduction` (int(10), NOT NULL, DEFAULT 0)
- `coin_rent_reduction` (float, NOT NULL, DEFAULT 0)
- `house_only` (tinyint(3), NOT NULL, DEFAULT 0)
- `house_location` (tinyint(3), NOT NULL, DEFAULT 0)
- `unk1` (tinyint(3), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `NewIndex` (`item_id`)
- CONSTRAINT `FK_item_details_house` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE