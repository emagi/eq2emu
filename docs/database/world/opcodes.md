## Table: `opcodes`

**Description:**

Defines `opcodes` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `version_range1` (smallint(5), NOT NULL, DEFAULT 0)
- `version_range2` (smallint(5), NOT NULL, DEFAULT 0)
- `name` (varchar(255), NOT NULL, DEFAULT '')
- `opcode` (smallint(5), NOT NULL, DEFAULT 0)
- `table_data_version` (smallint(5), NOT NULL, DEFAULT 1)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `newindex` (`version_range1`,`name`,`version_range2`)