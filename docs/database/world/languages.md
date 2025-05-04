## Table: `languages`

**Description:**

Defines `languages` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `language` (varchar(25), DEFAULT 'Unknown')

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `language` (`language`)