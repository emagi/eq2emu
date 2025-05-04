## Table: `spawn_npc_spell_lists`

**Description:**

Defines `spawn_npc_spell_lists` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `category` (tinytext, DEFAULT NULL)
- `description` (tinytext, NOT NULL, DEFAULT '')

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `DescIndex` (`description`(100))
- KEY `IDXCategory` (`category`(100))