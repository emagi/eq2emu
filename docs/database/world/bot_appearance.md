## Table: `bot_appearance`

**Description:**

Defines `bot_appearance` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `bot_id` (int(10), NOT NULL)
- `signed_value` (tinyint(4), NOT NULL, DEFAULT 0)
- `type` (varchar(32), NOT NULL)
- `red` (smallint(6), NOT NULL, DEFAULT 0)
- `green` (smallint(6), NOT NULL, DEFAULT 0)
- `blue` (smallint(6), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `bot_id_type` (`bot_id`,`type`)
- CONSTRAINT `FK_bot_id` FOREIGN KEY (`bot_id`) REFERENCES `bots` (`id`) ON DELETE CASCADE ON UPDATE CASCADE