## Table: `character_pictures`

**Description:**

Defines `character_pictures` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `char_id` (int(10), NOT NULL, DEFAULT 0)
- `pic_type` (tinyint(3), NOT NULL, DEFAULT 0)
- `picture` (mediumtext, NOT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `character_pic_id` (`char_id`,`pic_type`)