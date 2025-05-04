## Table: `character_social`

**Description:**

Defines `character_social` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `char_id` (int(10), NOT NULL, DEFAULT 0)
- `name` (varchar(64), NOT NULL, DEFAULT '')
- `type` (enum('FRIEND','IGNORE'), NOT NULL, DEFAULT 'FRIEND')

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `CharNameIdx` (`char_id`,`name`,`type`)
- CONSTRAINT `FK_character_social` FOREIGN KEY (`char_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE ON UPDATE CASCADE