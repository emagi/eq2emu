## Table: `character_quests`

**Description:**

Defines `character_quests` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `char_id` (int(10), NOT NULL, DEFAULT 0)
- `quest_id` (int(10), NOT NULL, DEFAULT 0)
- `quest_giver` (int(10), NOT NULL, DEFAULT 0)
- `given_date` (datetime, NOT NULL)
- `completed_date` (datetime, DEFAULT NULL)
- `current_quest` (tinyint(3), NOT NULL, DEFAULT 0)
- `tracked` (tinyint(3), NOT NULL, DEFAULT 0)
- `quest_flags` (int(10), NOT NULL, DEFAULT 0)
- `hidden` (tinyint(3), NOT NULL, DEFAULT 0)
- `complete_count` (smallint(5), NOT NULL, DEFAULT 0)
- `status_to_earn` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `CharQuestIDX` (`char_id`,`quest_id`)
- KEY `FK_quest_quests` (`quest_id`)
- CONSTRAINT `FK_character_quests` FOREIGN KEY (`char_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
- CONSTRAINT `FK_quest_quests` FOREIGN KEY (`quest_id`) REFERENCES `quests` (`quest_id`) ON DELETE CASCADE ON UPDATE CASCADE