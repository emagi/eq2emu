## Table: `versioned_spell_errors`

**Description:**

Defines `versioned_spell_errors` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `version` (smallint(5), NOT NULL, DEFAULT 0)
- `error_index` (smallint(5), NOT NULL, DEFAULT 0)
- `value` (smallint(5), NOT NULL, DEFAULT 0)
- `name` (char(64), NOT NULL, DEFAULT '')

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `SpellErrorIDX` (`version`,`error_index`)