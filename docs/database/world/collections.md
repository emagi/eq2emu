## Table: `collections`

**Description:**

Defines `collections` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `collection_name` (varchar(512), NOT NULL, DEFAULT 'Unknown')
- `collection_category` (varchar(512), NOT NULL, DEFAULT 'Unknown')
- `level` (tinyint(3), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id