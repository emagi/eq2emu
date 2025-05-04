## Table: `starting_languages`

**Description:**

Defines `starting_languages` table in the World database.

**Columns:**
- `id` (int(11), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `race` (int(11), NOT NULL, DEFAULT 0)
- `starting_city` (int(11), NOT NULL, DEFAULT 0)
- `language_id` (int(11), DEFAULT NULL)
- `notes` (text, DEFAULT NULL)

**Primary Keys:**
- id