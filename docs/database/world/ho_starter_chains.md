## Table: `ho_starter_chains`

**Description:**

Defines `ho_starter_chains` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `starter_class` (tinyint(3), NOT NULL, DEFAULT 0)
- `first_ability` (smallint(5), NOT NULL, DEFAULT 65535)
- `second_ability` (smallint(5), NOT NULL, DEFAULT 65535)
- `third_ability` (smallint(5), NOT NULL, DEFAULT 65535)
- `fourth_ability` (smallint(5), NOT NULL, DEFAULT 65535)
- `fifth_ability` (smallint(5), NOT NULL, DEFAULT 65535)
- `sixth_ability` (smallint(5), NOT NULL, DEFAULT 65535)

**Primary Keys:**
- id