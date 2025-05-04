## Table: `titles`

**Description:**

Defines `titles` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `title` (varchar(64), DEFAULT '')
- `prefix` (tinyint(1), NOT NULL, DEFAULT 0)
- `description` (text, DEFAULT NULL)
- `price` (varchar(64), DEFAULT '')

**Primary Keys:**
- id