## Table: `location_details`

**Description:**

Defines `location_details` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `location_id` (int(10), NOT NULL, DEFAULT 0)
- `x` (float, NOT NULL, DEFAULT 0)
- `y` (float, NOT NULL, DEFAULT 0)
- `z` (float, NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `LocationIDX` (`location_id`)
- CONSTRAINT `FK_location_details` FOREIGN KEY (`location_id`) REFERENCES `locations` (`id`) ON DELETE CASCADE ON UPDATE CASCADE