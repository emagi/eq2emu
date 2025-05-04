## Table: `character_items`

**Description:**

Defines `character_items` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `type` (enum('EQUIPPED','NOT-EQUIPPED','APPEARANCE','DELETED'), NOT NULL, DEFAULT 'NOT-EQUIPPED')
- `account_id` (int(10), NOT NULL, DEFAULT 0)
- `char_id` (int(10), NOT NULL, DEFAULT 0)
- `bag_id` (int(11), NOT NULL, DEFAULT 0)
- `slot` (int(11), NOT NULL, DEFAULT 0)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `creator` (varchar(64), NOT NULL, DEFAULT '')
- `condition_` (tinyint(3), NOT NULL, DEFAULT 100)
- `attuned` (tinyint(3), NOT NULL, DEFAULT 0)
- `count` (smallint(5), NOT NULL, DEFAULT 1)
- `max_sell_value` (int(10), NOT NULL, DEFAULT 0)
- `login_checksum` (int(10), NOT NULL, DEFAULT 0)
- `adorn0` (int(10), NOT NULL, DEFAULT 0)
- `adorn1` (int(10), NOT NULL, DEFAULT 0)
- `adorn2` (int(10), NOT NULL, DEFAULT 0)
- `adorn1_time` (int(10), NOT NULL, DEFAULT 0)
- `adorn3` (int(10), NOT NULL, DEFAULT 0)
- `adorn4` (int(10), NOT NULL, DEFAULT 0)
- `adorn5` (int(10), NOT NULL, DEFAULT 0)
- `adorn6` (int(10), NOT NULL, DEFAULT 0)
- `adorn7` (int(10), NOT NULL, DEFAULT 0)
- `adorn8` (int(10), NOT NULL, DEFAULT 0)
- `adorn9` (int(10), NOT NULL, DEFAULT 0)
- `adorn10` (int(10), NOT NULL, DEFAULT 0)
- `no_sale` (tinyint(1), NOT NULL, DEFAULT 0)
- `last_saved` (timestamp, NOT NULL, DEFAULT current_timestamp() ON UPDATE current_timestamp())
- `created` (timestamp, NOT NULL, DEFAULT current_timestamp())
- `equip_slot` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `NewIndex` (`char_id`,`type`,`bag_id`,`slot`)
- KEY `FK_items` (`item_id`)
- CONSTRAINT `FK_character_items` FOREIGN KEY (`char_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
- CONSTRAINT `FK_items` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE