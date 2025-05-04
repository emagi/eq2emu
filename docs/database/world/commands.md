## Table: `commands`

**Description:**

Defines `commands` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `type` (tinyint(3), DEFAULT 1)
- `command` (varchar(64), DEFAULT NULL)
- `subcommand` (varchar(64), DEFAULT NULL)
- `handler` (int(10), NOT NULL, DEFAULT 0)
- `required_status` (smallint(5), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `CommandIDX` (`command`,`subcommand`)