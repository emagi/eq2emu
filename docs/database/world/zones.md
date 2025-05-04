## Table: `zones`

**Description:**

Defines `zones` table in the World database.

**Columns:**
- `id` (int(11), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `expansion_id` (tinyint(3), NOT NULL, DEFAULT 0)
- `name` (varchar(64), NOT NULL, DEFAULT '')
- `file` (varchar(64), NOT NULL, DEFAULT '')
- `description` (varchar(255), NOT NULL, DEFAULT 'Describe me in the zones table! :)')
- `safe_x` (float, NOT NULL, DEFAULT 0)
- `safe_y` (float, NOT NULL, DEFAULT 0)
- `safe_z` (float, NOT NULL, DEFAULT 0)
- `safe_heading` (float, NOT NULL, DEFAULT 0)
- `underworld` (float, NOT NULL, DEFAULT -1000000)
- `xp_modifier` (float, NOT NULL, DEFAULT 0)
- `min_recommended` (tinyint(3), NOT NULL, DEFAULT 0)
- `max_recommended` (tinyint(3), NOT NULL, DEFAULT 0)
- `zone_type` (varchar(64), DEFAULT '')
- `always_loaded` (tinyint(3), NOT NULL, DEFAULT 0)
- `city_zone` (tinyint(3), NOT NULL, DEFAULT 0)
- `weather_allowed` (tinyint(3), NOT NULL, DEFAULT 0)
- `min_status` (int(10), NOT NULL, DEFAULT 0)
- `min_level` (int(10), NOT NULL, DEFAULT 0)
- `max_level` (int(10), NOT NULL, DEFAULT 0)
- `start_zone` (tinyint(3), NOT NULL, DEFAULT 0)
- `instance_type` (enum('NONE','GROUP_LOCKOUT_INSTANCE','GROUP_PERSIST_INSTANCE','RAID_LOCKOUT_INSTANCE','RAID_PERSIST_INSTANCE','SOLO_LOCKOUT_INSTANCE','SOLO_PERSIST_INSTANCE','TRADESKILL_INSTANCE','PUBLIC_INSTANCE','PERSONAL_HOUSE_INSTANCE','GUILD_HOUSE_INSTANCE','QUEST_INSTANCE'), NOT NULL, DEFAULT 'NONE')
- `default_reenter_time` (int(10), NOT NULL, DEFAULT 0)
- `default_reset_time` (int(10), NOT NULL, DEFAULT 0)
- `default_lockout_time` (int(10), NOT NULL, DEFAULT 0)
- `force_group_to_zone` (smallint(5), NOT NULL, DEFAULT 0)
- `lua_script` (varchar(255), DEFAULT '')
- `shutdown_timer` (int(10), NOT NULL, DEFAULT 300 COMMENT 'In seconds') → In seconds
- `zone_motd` (varchar(250), DEFAULT '')
- `ruleset_id` (int(10), NOT NULL, DEFAULT 0)
- `login_checksum` (int(10), NOT NULL, DEFAULT 0)
- `sky_file` (varchar(64), NOT NULL, DEFAULT '')
- `can_bind` (int(11), DEFAULT 0)
- `can_gate` (int(11), DEFAULT 1)
- `can_evac` (int(11), DEFAULT 1)
- `peer_priority` (smallint(5), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `ZoneNameIDX` (`name`)
- KEY `ZoneDescIDX` (`description`)
- KEY `ZoneFileIDX` (`file`)