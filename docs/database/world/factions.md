## Table: `factions`

**Description:**

Defines `factions` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `name` (varchar(64), NOT NULL, DEFAULT '')
- `type` (varchar(32), NOT NULL, DEFAULT '')
- `description` (text, NOT NULL)
- `default_level` (mediumint(9), NOT NULL, DEFAULT 0)
- `negative_change` (smallint(5), NOT NULL, DEFAULT 100)
- `positive_change` (smallint(5), NOT NULL, DEFAULT 75)

**Primary Keys:**
- id