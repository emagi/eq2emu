## Table: `entity_commands`

**Description:**

Defines `entity_commands` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `command_list_id` (int(10), NOT NULL, DEFAULT 1)
- `command_text` (varchar(64), NOT NULL, DEFAULT 'Hail')
- `distance` (float, NOT NULL, DEFAULT 0)
- `command` (varchar(64), NOT NULL, DEFAULT 'hail')
- `error_text` (varchar(64), NOT NULL)
- `cast_time` (smallint(5), NOT NULL, DEFAULT 0)
- `spell_visual` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `EntityCmdIDX` (`command_list_id`,`command_text`)