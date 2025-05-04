## Table: `login_char_colors`

**Description:**

Defines `login_char_colors` table in the Login database.

**Columns:**
- `id` (bigint(20), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `login_characters_id` (int(10), NOT NULL)
- `signed_value` (tinyint(4), NOT NULL, DEFAULT 0)
- `type` (varchar(32), NOT NULL)
- `red` (smallint(6), NOT NULL, DEFAULT 0)
- `green` (smallint(6), NOT NULL, DEFAULT 0)
- `blue` (smallint(6), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `id` (`id`)
- KEY `id_2` (`id`)
- KEY `CharID` (`login_characters_id`)