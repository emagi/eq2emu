## Table: `transport_maps`

**Description:**

Defines `transport_maps` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `transport_id` (int(10), NOT NULL, DEFAULT 0)
- `map_name` (varchar(64), NOT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `transport_id` (`transport_id`)