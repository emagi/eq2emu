## Table: `groundspawn_items`

**Description:**

Defines `groundspawn_items` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `groundspawn_id` (int(10), NOT NULL, DEFAULT 1)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `plus_rare_id` (int(10), NOT NULL, DEFAULT 0)
- `is_rare` (tinyint(3), NOT NULL, DEFAULT 0)
- `grid_id` (int(10), NOT NULL, DEFAULT 0)
- `percent` (float, NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `FK_item_groundspawns` (`item_id`)