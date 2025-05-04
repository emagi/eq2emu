## Table: `character_history`

**Description:**

Defines `character_history` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `char_id` (int(10), NOT NULL, DEFAULT 0)
- `type` (enum('None','Death','Discovery','XP'), NOT NULL, DEFAULT 'None')
- `subtype` (enum('None','Adventure','Tradeskill','Quest','AA','Item','Location'), NOT NULL, DEFAULT 'None')
- `value` (int(10), NOT NULL, DEFAULT 0)
- `value2` (int(10), NOT NULL, DEFAULT 0)
- `location` (varchar(200), DEFAULT '')
- `event_id` (int(10), NOT NULL, DEFAULT 0)
- `event_date` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `char_id` (`char_id`,`type`,`subtype`,`value`)
- KEY `CharHistoryIDX` (`char_id`,`type`,`subtype`)
- CONSTRAINT `FK_character_history` FOREIGN KEY (`char_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE ON UPDATE CASCADE