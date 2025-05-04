## Table: `character_lua_history`

**Description:**

Defines `character_lua_history` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `char_id` (int(10), NOT NULL, DEFAULT 0)
- `event_id` (int(10), NOT NULL, DEFAULT 0)
- `value` (int(10), NOT NULL, DEFAULT 0)
- `value2` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `Characte Event` (`char_id`,`event_id`)