## Table: `characters`

**Description:**

Defines `characters` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `account_id` (int(10), NOT NULL, DEFAULT 0)
- `server_id` (int(10), NOT NULL, DEFAULT 0)
- `name` (varchar(64), NOT NULL, DEFAULT '')
- `race` (tinyint(3), NOT NULL, DEFAULT 0)
- `class` (tinyint(3), NOT NULL, DEFAULT 0)
- `gender` (tinyint(3), NOT NULL, DEFAULT 0)
- `deity` (tinyint(3), NOT NULL, DEFAULT 0)
- `body_size` (float, NOT NULL, DEFAULT 0)
- `body_age` (float, NOT NULL, DEFAULT 0)
- `current_zone_id` (int(11), NOT NULL, DEFAULT 253)
- `level` (int(10), NOT NULL, DEFAULT 1)
- `tradeskill_class` (tinyint(3), NOT NULL, DEFAULT 0)
- `tradeskill_level` (int(10), NOT NULL, DEFAULT 1)
- `soga_wing_type` (mediumint(8), NOT NULL)
- `soga_chest_type` (mediumint(8), NOT NULL)
- `soga_legs_type` (mediumint(8), NOT NULL)
- `soga_hair_type` (mediumint(8), NOT NULL)
- `soga_facial_hair_type` (mediumint(8), NOT NULL, DEFAULT 0)
- `soga_model_type` (mediumint(8), NOT NULL)
- `legs_type` (mediumint(8), NOT NULL)
- `chest_type` (mediumint(8), NOT NULL)
- `wing_type` (mediumint(8), NOT NULL)
- `hair_type` (mediumint(8), NOT NULL)
- `facial_hair_type` (mediumint(8), NOT NULL, DEFAULT 0)
- `model_type` (mediumint(8), NOT NULL)
- `x` (float, NOT NULL, DEFAULT 0)
- `y` (float, NOT NULL, DEFAULT 0)
- `z` (float, NOT NULL, DEFAULT 0)
- `heading` (float, NOT NULL, DEFAULT 0)
- `instance_id` (int(10), NOT NULL, DEFAULT 0)
- `starting_city` (tinyint(3), NOT NULL, DEFAULT 1)
- `deleted` (tinyint(3), NOT NULL, DEFAULT 0)
- `unix_timestamp` (int(10), NOT NULL, DEFAULT 0)
- `created_date` (timestamp, NOT NULL, DEFAULT '0000-00-00 00:00:00')
- `last_played` (timestamp, NOT NULL, DEFAULT current_timestamp() ON UPDATE current_timestamp())
- `last_saved` (int(10), NOT NULL, DEFAULT 0)
- `admin_status` (int(10), NOT NULL, DEFAULT 0)
- `is_online` (tinyint(1), NOT NULL, DEFAULT 0)
- `group_id` (int(10), NOT NULL, DEFAULT 0)
- `alignment` (tinyint(3), NOT NULL, DEFAULT 0)
- `first_world_login` (tinyint(1), NOT NULL, DEFAULT 0)
- `zone_duplicating_id` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `CharIDX` (`name`)
- KEY `CharZoneIDX` (`current_zone_id`)
- KEY `CharAcctIDX` (`account_id`)