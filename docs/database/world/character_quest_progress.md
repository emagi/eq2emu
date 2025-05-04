## Table: `character_quest_progress`

**Description:**

Defines `character_quest_progress` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `char_id` (int(10), NOT NULL, DEFAULT 0)
- `quest_id` (int(10), NOT NULL, DEFAULT 0)
- `step_id` (int(10), NOT NULL, DEFAULT 0)
- `progress` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `CharQuestStepIDX` (`char_id`,`quest_id`,`step_id`)
- KEY `FK_quest_id` (`quest_id`)
- CONSTRAINT `FK_character_quest_progress` FOREIGN KEY (`char_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
- CONSTRAINT `FK_quest_id` FOREIGN KEY (`quest_id`) REFERENCES `quests` (`quest_id`) ON DELETE CASCADE ON UPDATE CASCADE