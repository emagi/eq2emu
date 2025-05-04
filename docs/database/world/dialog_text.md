## Table: `dialog_text`

**Description:**

Defines `dialog_text` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, NOT NULL)
- `text` (text, NOT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `IndexText` (`text`(100))