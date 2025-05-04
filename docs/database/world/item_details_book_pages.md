## Table: `item_details_book_pages`

**Description:**

Defines `item_details_book_pages` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `page` (tinyint(3), NOT NULL, DEFAULT 0)
- `page_text` (text, NOT NULL)
- `page_text_valign` (tinyint(3), NOT NULL, DEFAULT 0)
- `page_text_halign` (tinyint(3), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `ItemBookIDX` (`item_id`)