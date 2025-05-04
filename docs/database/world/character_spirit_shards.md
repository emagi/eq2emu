## Table: `character_spirit_shards`

**Description:**

Defines `character_spirit_shards` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `timestamp` (timestamp, NOT NULL, DEFAULT current_timestamp())
- `name` (varchar(64), NOT NULL, DEFAULT '')
- `level` (int(10), NOT NULL, DEFAULT 0)
- `race` (tinyint(3), NOT NULL, DEFAULT 0)
- `gender` (tinyint(3), NOT NULL, DEFAULT 0)
- `adventure_class` (tinyint(3), NOT NULL, DEFAULT 0)
- `model_type` (mediumint(8), NOT NULL, DEFAULT 0)
- `soga_model_type` (mediumint(8), NOT NULL, DEFAULT 0)
- `hair_type` (mediumint(8), NOT NULL, DEFAULT 0)
- `hair_face_type` (mediumint(8), NOT NULL, DEFAULT 0)
- `wing_type` (mediumint(8), NOT NULL, DEFAULT 0)
- `chest_type` (mediumint(8), NOT NULL, DEFAULT 0)
- `legs_type` (mediumint(8), NOT NULL, DEFAULT 0)
- `soga_hair_type` (mediumint(8), NOT NULL, DEFAULT 0)
- `soga_hair_face_type` (mediumint(8), NOT NULL, DEFAULT 0)
- `hide_hood` (tinyint(3), NOT NULL, DEFAULT 0)
- `size` (mediumint(8), NOT NULL, DEFAULT 0)
- `collision_radius` (mediumint(8), NOT NULL, DEFAULT 0)
- `action_state` (mediumint(8), NOT NULL, DEFAULT 0)
- `visual_state` (mediumint(8), NOT NULL, DEFAULT 0)
- `mood_state` (mediumint(8), NOT NULL, DEFAULT 0)
- `emote_state` (mediumint(8), NOT NULL, DEFAULT 0)
- `pos_state` (mediumint(8), NOT NULL, DEFAULT 0)
- `activity_status` (mediumint(8), NOT NULL, DEFAULT 0)
- `sub_title` (varchar(255), NOT NULL, DEFAULT '')
- `prefix_title` (varchar(128), NOT NULL, DEFAULT '')
- `suffix_title` (varchar(128), NOT NULL, DEFAULT '')
- `lastname` (varchar(64), NOT NULL, DEFAULT '')
- `x` (float, NOT NULL, DEFAULT 0)
- `y` (float, NOT NULL, DEFAULT 0)
- `z` (float, NOT NULL, DEFAULT 0)
- `heading` (float, NOT NULL, DEFAULT 0)
- `gridid` (int(10), NOT NULL, DEFAULT 0)
- `zoneid` (int(10), NOT NULL, DEFAULT 0)
- `instanceid` (int(10), NOT NULL, DEFAULT 0)
- `charid` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id