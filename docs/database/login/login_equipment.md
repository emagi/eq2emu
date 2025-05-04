## Table: `login_equipment`

**Description:**

Defines `login_equipment` table in the Login database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `login_characters_id` (int(10), NOT NULL, DEFAULT 0)
- `equip_type` (smallint(5), NOT NULL, DEFAULT 0)
- `red` (tinyint(3), NOT NULL, DEFAULT 255)
- `green` (tinyint(3), NOT NULL, DEFAULT 255)
- `blue` (tinyint(3), NOT NULL, DEFAULT 255)
- `highlight_red` (tinyint(3), NOT NULL, DEFAULT 255)
- `highlight_green` (tinyint(3), NOT NULL, DEFAULT 255)
- `highlight_blue` (tinyint(3), NOT NULL, DEFAULT 255)
- `slot` (int(11), NOT NULL, DEFAULT 0)
- `last_updated` (timestamp, NOT NULL, DEFAULT current_timestamp())

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `CharSlotIDX` (`login_characters_id`,`slot`)