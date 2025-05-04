## Table: `dialog_voiceovers`

**Description:**

Defines `dialog_voiceovers` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, NOT NULL)
- `file` (text, NOT NULL)
- `key1` (int(10), NOT NULL, DEFAULT 0)
- `key2` (int(10), NOT NULL, DEFAULT 0)
- `bChecked` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `IndexFile` (`file`(100))