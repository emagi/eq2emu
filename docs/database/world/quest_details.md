## Table: `quest_details`

**Description:**

Defines `quest_details` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `quest_id` (int(10), NOT NULL, DEFAULT 0)
- `type` (enum('None','Prereq','Reward'), NOT NULL, DEFAULT 'None')
- `subtype` (enum('None','Experience','Faction','Item','Quest','Race','AdvLevel','TSLevel','Coin','Selectable','MaxCoin','MaxAdvLevel','MaxTSLevel','TSExperience','Class'), NOT NULL, DEFAULT 'None')
- `value` (int(10), NOT NULL, DEFAULT 0)
- `faction_id` (smallint(5), NOT NULL, DEFAULT 0)
- `quantity` (tinyint(3), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `QuestIDX` (`quest_id`,`type`,`subtype`,`value`,`faction_id`,`quantity`)
- CONSTRAINT `FK_quest_details` FOREIGN KEY (`quest_id`) REFERENCES `quests` (`quest_id`) ON DELETE CASCADE ON UPDATE CASCADE