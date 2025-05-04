## Table: `spawn`

**Description:**

Defines `spawn` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `name` (varchar(64), DEFAULT NULL)
- `sub_title` (varchar(250), DEFAULT NULL)
- `prefix` (varchar(128), NOT NULL, DEFAULT '')
- `suffix` (varchar(128), NOT NULL, DEFAULT '')
- `last_name` (varchar(64), NOT NULL, DEFAULT '')
- `race` (tinyint(3), NOT NULL, DEFAULT 0)
- `model_type` (mediumint(8), NOT NULL, DEFAULT 0)
- `size` (smallint(5), NOT NULL, DEFAULT 32)
- `size_offset` (tinyint(3), NOT NULL, DEFAULT 0)
- `targetable` (tinyint(3), NOT NULL, DEFAULT 0)
- `show_name` (tinyint(3), NOT NULL, DEFAULT 0)
- `command_primary` (int(10), NOT NULL, DEFAULT 0)
- `command_secondary` (int(10), NOT NULL, DEFAULT 0)
- `visual_state` (int(10), NOT NULL, DEFAULT 0)
- `attackable` (tinyint(3), NOT NULL, DEFAULT 0)
- `show_level` (tinyint(3), NOT NULL, DEFAULT 0)
- `show_command_icon` (tinyint(3), NOT NULL, DEFAULT 0)
- `display_hand_icon` (tinyint(3), NOT NULL, DEFAULT 0)
- `faction_id` (int(10), NOT NULL, DEFAULT 0)
- `collision_radius` (smallint(5), NOT NULL, DEFAULT 0)
- `hp` (int(10), NOT NULL, DEFAULT 0)
- `power` (int(10), NOT NULL, DEFAULT 0)
- `savagery` (int(10), NOT NULL, DEFAULT 0)
- `dissonance` (int(10), NOT NULL, DEFAULT 0)
- `merchant_id` (int(10), NOT NULL, DEFAULT 0)
- `transport_id` (int(10), NOT NULL, DEFAULT 0)
- `merchant_type` (tinyint(3), NOT NULL, DEFAULT 0)
- `processed` (tinyint(1), NOT NULL, DEFAULT 0)
- `expansion_flag` (int(10), NOT NULL, DEFAULT 0)
- `is_instanced_spawn` (tinyint(3), NOT NULL, DEFAULT 0)
- `disable_sounds` (tinyint(1), NOT NULL, DEFAULT 0)
- `merchant_min_level` (int(10), NOT NULL, DEFAULT 0)
- `merchant_max_level` (int(10), NOT NULL, DEFAULT 0)
- `holiday_flag` (int(10), NOT NULL, DEFAULT 0)
- `aaxp_rewards` (int(10), NOT NULL, DEFAULT 0)
- `loot_tier` (int(10), NOT NULL, DEFAULT 0)
- `loot_drop_type` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `SpawnIDX` (`name`)
- KEY `SpawnCmdIDX` (`command_primary`,`command_secondary`)
- KEY `SpawnMiscIDX` (`faction_id`,`merchant_id`,`transport_id`)