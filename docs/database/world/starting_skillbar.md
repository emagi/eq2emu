## Table: `starting_skillbar`

**Description:**

Defines `starting_skillbar` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `race_id` (tinyint(3), NOT NULL, DEFAULT 255)
- `class_id` (tinyint(3), NOT NULL, DEFAULT 255)
- `type` (tinyint(3), NOT NULL, DEFAULT 1)
- `hotbar` (int(10), NOT NULL, DEFAULT 0)
- `spell_id` (int(10), NOT NULL, DEFAULT 0)
- `slot` (int(10), NOT NULL, DEFAULT 0)
- `text_val` (varchar(255), NOT NULL, DEFAULT 'Unused')

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `NewIndex` (`hotbar`,`class_id`,`slot`,`race_id`)