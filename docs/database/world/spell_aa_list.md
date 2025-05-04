## Table: `spell_aa_list`

**Description:**

Defines `spell_aa_list` table in the World database.

**Columns:**
- `list_id` (int(10), PRIMARY KEY, NOT NULL)
- `name` (varchar(64), NOT NULL, DEFAULT '')
- `level` (tinyint(3), NOT NULL, DEFAULT 0)
- `maximumpoints` (tinyint(3), NOT NULL, DEFAULT 0)
- `maxpoints` (smallint(5), NOT NULL, DEFAULT 0)
- `minimumpointsrequired` (smallint(5), NOT NULL, DEFAULT 0)
- `iswardertree` (tinyint(1), NOT NULL, DEFAULT 0)
- `last_update` (float, NOT NULL, DEFAULT 0)
- `ts` (float, NOT NULL)
- `version` (tinyint(3), NOT NULL, DEFAULT 0)

**Primary Keys:**
- list_id