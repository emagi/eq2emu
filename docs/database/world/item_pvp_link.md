## Table: `item_pvp_link`

**Description:**

Defines `item_pvp_link` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `base_item` (int(10), NOT NULL)
- `pvp_item` (int(10), NOT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `UK_baseitem` (`base_item`)
- UNIQUE KEY `UK_pvpitem` (`pvp_item`)
- CONSTRAINT `FK__items` FOREIGN KEY (`base_item`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
- CONSTRAINT `FK__items_2` FOREIGN KEY (`pvp_item`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE