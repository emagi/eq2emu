## Table: `map_data`

**Description:**

Defines `map_data` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `map_id` (int(10), NOT NULL, DEFAULT 0)
- `zone_name` (varchar(128), DEFAULT NULL)
- `highest` (float, NOT NULL, DEFAULT 0)
- `lowest` (float, NOT NULL, DEFAULT 0)
- `explored_map_name` (varchar(255), DEFAULT NULL)
- `unexplored_map_name` (varchar(255), DEFAULT NULL)
- `bounds1_x` (float, NOT NULL, DEFAULT 0)
- `bounds1_z` (float, NOT NULL, DEFAULT 0)
- `bounds2_x` (float, NOT NULL, DEFAULT 0)
- `bounds2_z` (float, NOT NULL, DEFAULT 0)
- `bounds3_x` (float, NOT NULL, DEFAULT 0)
- `bounds3_z` (float, NOT NULL, DEFAULT 0)
- `bounds4_x` (float, NOT NULL, DEFAULT 0)
- `bounds4_z` (float, NOT NULL, DEFAULT 0)
- `explored_key` (bigint(20), NOT NULL, DEFAULT 0)
- `unexplored_key` (bigint(20), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `MapIDX` (`map_id`,`zone_name`,`unexplored_map_name`,`explored_map_name`)