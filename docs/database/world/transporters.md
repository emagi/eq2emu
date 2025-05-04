## Table: `transporters`

**Description:**

Defines `transporters` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `transport_id` (int(10), NOT NULL, DEFAULT 0)
- `transport_type` (enum('Zone','Location','Generic, DEFAULT 'Zone')
- `display_name` (varchar(64), DEFAULT NULL)
- `destination_zone_id` (int(10), NOT NULL, DEFAULT 0)
- `destination_x` (float, NOT NULL, DEFAULT 0)
- `destination_y` (float, NOT NULL, DEFAULT 0)
- `destination_z` (float, NOT NULL, DEFAULT 0)
- `destination_heading` (float, NOT NULL, DEFAULT 0)
- `trigger_location_zone_id` (int(10), NOT NULL, DEFAULT 0)
- `trigger_location_x` (float, NOT NULL, DEFAULT -1)
- `trigger_location_y` (float, NOT NULL, DEFAULT -1)
- `trigger_location_z` (float, NOT NULL, DEFAULT -1)
- `trigger_radius` (float, NOT NULL, DEFAULT -1)
- `cost` (int(10), NOT NULL, DEFAULT 0)
- `message` (varchar(255), DEFAULT NULL)
- `min_level` (tinyint(3), NOT NULL, DEFAULT 0)
- `max_level` (tinyint(3), NOT NULL, DEFAULT 0)
- `quest_req` (int(10), NOT NULL, DEFAULT 0)
- `quest_step_req` (smallint(5), NOT NULL, DEFAULT 0)
- `quest_completed` (int(10), NOT NULL, DEFAULT 0)
- `map_x` (int(10), NOT NULL, DEFAULT 0)
- `map_y` (int(10), NOT NULL, DEFAULT 0)
- `expansion_flag` (int(10), NOT NULL, DEFAULT 0)
- `min_client_version` (int(10), NOT NULL, DEFAULT 0)
- `max_client_version` (int(10), NOT NULL, DEFAULT 0)
- `flight_path_id` (int(10), NOT NULL, DEFAULT 0)
- `mount_id` (int(10), NOT NULL, DEFAULT 0)
- `mount_red_color` (smallint(5), NOT NULL, DEFAULT 255)
- `mount_green_color` (smallint(5), NOT NULL, DEFAULT 255)
- `mount_blue_color` (smallint(5), NOT NULL, DEFAULT 255)
- `holiday_flag` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `FK_transporters` (`destination_zone_id`)
- KEY `FK_transporters2` (`trigger_location_zone_id`)
- CONSTRAINT `FK_transporters` FOREIGN KEY (`destination_zone_id`) REFERENCES `zones` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
- CONSTRAINT `CONSTRAINT_1` CHECK (`transport_id` between 0 and 2000000)