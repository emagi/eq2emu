## Table: `spawn_templates`

**Description:**

Defines `spawn_templates` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `name` (varchar(64), NOT NULL, DEFAULT '')
- `spawn_location_id` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `TemplateName` (`name`)