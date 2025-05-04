## Table: `heroic_ops`

**Description:**

Defines `heroic_ops` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `ho_type` (enum('Starter','Wheel'), NOT NULL, DEFAULT 'Starter')
- `name` (varchar(64), DEFAULT NULL)
- `starter_link_id` (int(10), NOT NULL, DEFAULT 0)
- `starter_class` (tinyint(3), NOT NULL, DEFAULT 0)
- `enhancer_class` (bigint(30), NOT NULL, DEFAULT 0)
- `starter_icon` (smallint(5), NOT NULL, DEFAULT 65535)
- `chain_order` (tinyint(3), NOT NULL, DEFAULT 0)
- `shift_icon` (smallint(5), NOT NULL, DEFAULT 41)
- `spell_id` (int(10), NOT NULL, DEFAULT 0)
- `chance` (float, NOT NULL, DEFAULT 0)
- `ability1` (smallint(5), NOT NULL, DEFAULT 65535)
- `ability2` (smallint(5), NOT NULL, DEFAULT 65535)
- `ability3` (smallint(5), NOT NULL, DEFAULT 65535)
- `ability4` (smallint(5), NOT NULL, DEFAULT 65535)
- `ability5` (smallint(5), NOT NULL, DEFAULT 65535)
- `ability6` (smallint(5), NOT NULL, DEFAULT 65535)

**Primary Keys:**
- id