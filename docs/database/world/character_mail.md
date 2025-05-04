## Table: `character_mail`

**Description:**

Defines `character_mail` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `player_to_id` (int(10), NOT NULL, DEFAULT 0)
- `player_from` (varchar(64), NOT NULL, DEFAULT '')
- `subject` (varchar(255), NOT NULL, DEFAULT '')
- `mail_body` (text, DEFAULT NULL)
- `already_read` (tinyint(1), NOT NULL, DEFAULT 0)
- `mail_type` (tinyint(3), NOT NULL, DEFAULT 0)
- `coin_copper` (int(10), NOT NULL, DEFAULT 0)
- `coin_silver` (int(10), NOT NULL, DEFAULT 0)
- `coin_gold` (int(10), NOT NULL, DEFAULT 0)
- `coin_plat` (int(10), NOT NULL, DEFAULT 0)
- `stack` (smallint(5), NOT NULL, DEFAULT 0)
- `postage_cost` (int(10), NOT NULL, DEFAULT 0)
- `attachment_cost` (int(10), NOT NULL, DEFAULT 0)
- `char_item_id` (int(10), NOT NULL, DEFAULT 0)
- `time_sent` (int(10), NOT NULL, DEFAULT 0)
- `expire_time` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `FK_character_mail` (`player_to_id`)
- CONSTRAINT `FK_character_mail` FOREIGN KEY (`player_to_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE ON UPDATE CASCADE