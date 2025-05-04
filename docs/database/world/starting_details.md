## Table: `starting_details`

**Description:**

Defines `starting_details` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `race_id` (tinyint(3), NOT NULL, DEFAULT 255)
- `class_id` (tinyint(3), NOT NULL, DEFAULT 255)
- `max_hp` (int(10), NOT NULL, DEFAULT 0)
- `max_power` (int(10), NOT NULL, DEFAULT 0)
- `max_savagery` (int(10), NOT NULL, DEFAULT 0)
- `max_dissonance` (int(10), NOT NULL, DEFAULT 0)
- `max_concentration` (tinyint(3), NOT NULL, DEFAULT 0)
- `str` (smallint(5), NOT NULL, DEFAULT 0)
- `agi` (smallint(5), NOT NULL, DEFAULT 0)
- `sta` (smallint(5), NOT NULL, DEFAULT 0)
- `intel` (smallint(5), NOT NULL, DEFAULT 0)
- `wis` (smallint(5), NOT NULL, DEFAULT 0)
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
- `coin_copper` (int(10), NOT NULL, DEFAULT 0)
- `coin_silver` (int(10), NOT NULL, DEFAULT 0)
- `coin_gold` (int(10), NOT NULL, DEFAULT 0)
- `coin_plat` (int(10), NOT NULL, DEFAULT 0)
- `status_points` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `RaceClassIDX` (`race_id`,`class_id`)