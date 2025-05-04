## Table: `collection_details`

**Description:**

Defines `collection_details` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `collection_id` (int(10), NOT NULL, DEFAULT 0)
- `item_id` (int(11), NOT NULL, DEFAULT 0)
- `item_index` (tinyint(3), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `FK_collection_details` (`collection_id`)
- CONSTRAINT `FK_collection_details` FOREIGN KEY (`collection_id`) REFERENCES `collections` (`id`) ON DELETE CASCADE ON UPDATE CASCADE