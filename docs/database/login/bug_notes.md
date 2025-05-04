## Table: `bug_notes`

**Description:**

Defines `bug_notes` table in the Login database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `bug_id` (int(10), NOT NULL, DEFAULT 0)
- `note` (text, DEFAULT NULL)
- `author` (varchar(64), NOT NULL, DEFAULT '')
- `note_date` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `BugIDX` (`bug_id`)