## Table: `item_details_book`

**Description:**

Defines `item_details_book` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `language` (tinyint(3), NOT NULL, DEFAULT 0)
- `author` (varchar(255), NOT NULL, DEFAULT '')
- `title` (varchar(255), NOT NULL, DEFAULT '')

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `ItemBookIDX` (`item_id`)
- CONSTRAINT `FK_item_details_book` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE