## Table: `character_instances`

**Description:**

Defines `character_instances` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `char_id` (int(10), NOT NULL, DEFAULT 0)
- `instance_id` (int(10), NOT NULL, DEFAULT 0)
- `instance_zone_name` (varchar(64), NOT NULL)
- `instance_type` (tinyint(3), NOT NULL, DEFAULT 0)
- `last_success_timestamp` (int(10), NOT NULL, DEFAULT 0)
- `last_failure_timestamp` (int(10), NOT NULL, DEFAULT 0)
- `success_lockout_time` (int(10), NOT NULL, DEFAULT 0)
- `failure_lockout_time` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `CharacterIDX` (`char_id`)
- KEY `InstanceIDX` (`instance_zone_name`)