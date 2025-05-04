## Table: `character_macros`

**Description:**

Defines `character_macros` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `char_id` (int(10), NOT NULL, DEFAULT 0)
- `macro_number` (tinyint(3), NOT NULL, DEFAULT 0)
- `macro_icon` (smallint(5), NOT NULL, DEFAULT 0)
- `macro_name` (varchar(64), DEFAULT NULL)
- `macro_text` (text, DEFAULT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `FK_character_macros` (`char_id`)
- CONSTRAINT `FK_character_macros` FOREIGN KEY (`char_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE ON UPDATE CASCADE