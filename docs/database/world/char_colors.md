## Table: `char_colors`

**Description:**

Defines `char_colors` table in the World database.

**Columns:**
- `id` (bigint(20), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `char_id` (int(10), NOT NULL)
- `signed_value` (tinyint(4), NOT NULL, DEFAULT 0)
- `type` (varchar(32), NOT NULL)
- `red` (smallint(6), NOT NULL, DEFAULT 0)
- `green` (smallint(6), NOT NULL, DEFAULT 0)
- `blue` (smallint(6), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `FK_char_colors` (`char_id`)
- CONSTRAINT `FK_char_colors` FOREIGN KEY (`char_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE ON UPDATE CASCADE