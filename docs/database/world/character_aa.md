## Table: `character_aa`

**Description:**

Defines `character_aa` table in the World database.

**Columns:**
- `id` (bigint(20), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `char_id` (int(10), NOT NULL)
- `template_id` (smallint(5), NOT NULL)
- `tab_id` (tinyint(3), NOT NULL)
- `aa_id` (int(10), NOT NULL)
- `order` (smallint(5), NOT NULL)
- `treeid` (tinyint(3), NOT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `FX_char` (`char_id`)
- CONSTRAINT `FX_char` FOREIGN KEY (`char_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE ON UPDATE CASCADE