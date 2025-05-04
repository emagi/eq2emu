## Table: `groundspawns`

**Description:**

Defines `groundspawns` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `groundspawn_id` (int(10), NOT NULL, DEFAULT 0)
- `min_skill_level` (smallint(5), NOT NULL, DEFAULT 1)
- `min_adventure_level` (smallint(5), NOT NULL, DEFAULT 0)
- `bonus_table` (tinyint(1), NOT NULL, DEFAULT 0)
- `harvest1` (float, NOT NULL, DEFAULT 70)
- `harvest3` (float, NOT NULL, DEFAULT 20)
- `harvest5` (float, NOT NULL, DEFAULT 8)
- `harvest_imbue` (float, NOT NULL, DEFAULT 1)
- `harvest_rare` (float, NOT NULL, DEFAULT 0.7)
- `harvest10` (float, NOT NULL, DEFAULT 0.3)
- `harvest_coin` (int(10), NOT NULL, DEFAULT 0)
- `enabled` (tinyint(1), NOT NULL, DEFAULT 1)
- `tablename` (varchar(64), DEFAULT NULL)

**Primary Keys:**
- id