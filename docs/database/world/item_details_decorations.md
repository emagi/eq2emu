## Table: `item_details_decorations`

**Description:**

Defines `item_details_decorations` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `decoration_name` (text, NOT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `DecorationIDX` (`item_id`)
- CONSTRAINT `FK_item_details_decorations` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`) ON UPDATE CASCADE