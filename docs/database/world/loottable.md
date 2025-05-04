## Table: `loottable`

**Description:**

Defines `loottable` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `name` (varchar(128), DEFAULT NULL)
- `mincoin` (int(10), NOT NULL, DEFAULT 0)
- `maxcoin` (int(10), NOT NULL, DEFAULT 0)
- `maxlootitems` (smallint(5), NOT NULL, DEFAULT 0)
- `lootdrop_probability` (float, NOT NULL, DEFAULT 100)
- `coin_probability` (float, NOT NULL, DEFAULT 100)

**Primary Keys:**
- id