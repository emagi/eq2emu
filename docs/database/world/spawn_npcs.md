## Table: `spawn_npcs`

**Description:**

Defines `spawn_npcs` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `spawn_id` (int(10), NOT NULL, DEFAULT 0)
- `min_level` (tinyint(3), NOT NULL, DEFAULT 0)
- `max_level` (tinyint(3), NOT NULL, DEFAULT 0)
- `enc_level` (tinyint(3), NOT NULL, DEFAULT 0)
- `class_` (tinyint(3), NOT NULL, DEFAULT 0)
- `gender` (tinyint(3), NOT NULL, DEFAULT 0)
- `min_group_size` (tinyint(3), NOT NULL, DEFAULT 0)
- `max_group_size` (tinyint(3), NOT NULL, DEFAULT 0)
- `hair_type_id` (smallint(5), NOT NULL, DEFAULT 0)
- `facial_hair_type_id` (smallint(5), NOT NULL, DEFAULT 0)
- `wing_type_id` (smallint(5), NOT NULL, DEFAULT 0)
- `chest_type_id` (smallint(5), NOT NULL, DEFAULT 0)
- `legs_type_id` (smallint(5), NOT NULL, DEFAULT 0)
- `soga_hair_type_id` (smallint(5), NOT NULL, DEFAULT 0)
- `soga_facial_hair_type_id` (smallint(5), NOT NULL, DEFAULT 0)
- `soga_model_type` (smallint(5), NOT NULL, DEFAULT 0)
- `hide_hood` (tinyint(1), NOT NULL, DEFAULT 0)
- `heroic_flag` (tinyint(3), NOT NULL, DEFAULT 0)
- `action_state` (smallint(5), NOT NULL, DEFAULT 0)
- `mood_state` (smallint(5), NOT NULL, DEFAULT 0)
- `emote_state` (smallint(5), NOT NULL, DEFAULT 0)
- `initial_state` (smallint(5), NOT NULL, DEFAULT 0)
- `activity_status` (smallint(5), NOT NULL, DEFAULT 0)
- `attack_type` (tinyint(3), NOT NULL, DEFAULT 0)
- `aggro_radius` (float, NOT NULL, DEFAULT 10)
- `ai_strategy` (enum('BALANCED','OFFENSIVE','DEFENSIVE'), NOT NULL, DEFAULT 'BALANCED')
- `spell_list_id` (int(10), NOT NULL, DEFAULT 0)
- `secondary_spell_list_id` (int(10), NOT NULL, DEFAULT 0)
- `skill_list_id` (int(10), NOT NULL, DEFAULT 0)
- `secondary_skill_list_id` (int(10), NOT NULL, DEFAULT 0)
- `equipment_list_id` (int(10), NOT NULL, DEFAULT 0)
- `str` (smallint(5), NOT NULL, DEFAULT 0)
- `sta` (smallint(5), NOT NULL, DEFAULT 0)
- `wis` (smallint(5), NOT NULL, DEFAULT 0)
- `intel` (smallint(5), NOT NULL, DEFAULT 0)
- `agi` (smallint(5), NOT NULL, DEFAULT 0)
- `heat` (smallint(5), NOT NULL, DEFAULT 0)
- `cold` (smallint(5), NOT NULL, DEFAULT 0)
- `magic` (smallint(5), NOT NULL, DEFAULT 0)
- `mental` (smallint(5), NOT NULL, DEFAULT 0)
- `divine` (smallint(5), NOT NULL, DEFAULT 0)
- `disease` (smallint(5), NOT NULL, DEFAULT 0)
- `poison` (smallint(5), NOT NULL, DEFAULT 0)
- `elemental` (smallint(5), NOT NULL, DEFAULT 0)
- `arcane` (smallint(5), NOT NULL, DEFAULT 0)
- `noxious` (smallint(5), NOT NULL, DEFAULT 0)
- `cast_percentage` (tinyint(3), NOT NULL, DEFAULT 10)
- `randomize` (int(10), NOT NULL, DEFAULT 0)
- `alignment` (tinyint(3), NOT NULL, DEFAULT 0)
- `water_type` (tinyint(1), NOT NULL, DEFAULT 0)
- `flying_type` (tinyint(1), NOT NULL, DEFAULT 0)
- `scared_by_strong_players` (tinyint(3), NOT NULL, DEFAULT 0)
- `action_state_str` (varchar(64), NOT NULL, DEFAULT '')

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `SpawnIDX` (`spawn_id`)
- CONSTRAINT `FK_npcs_spawn` FOREIGN KEY (`spawn_id`) REFERENCES `spawn` (`id`) ON DELETE CASCADE ON UPDATE CASCADE