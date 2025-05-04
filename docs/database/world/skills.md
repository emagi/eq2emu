## Table: `skills`

**Description:**

Defines `skills` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `short_name` (varchar(32), NOT NULL, DEFAULT 'unknown')
- `name` (varchar(255), NOT NULL, DEFAULT 'unknown')
- `description` (varchar(255), NOT NULL, DEFAULT 'unknown')
- `skill_type` (int(10), NOT NULL, DEFAULT 0)
- `display` (tinyint(3), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id