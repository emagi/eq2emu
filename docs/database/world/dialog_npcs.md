## Table: `dialog_npcs`

**Description:**

Defines `dialog_npcs` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, NOT NULL)
- `zone` (text, NOT NULL)
- `name` (text, NOT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `IndexZone_Name` (`zone`(100),`name`(100))