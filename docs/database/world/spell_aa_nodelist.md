## Table: `spell_aa_nodelist`

**Description:**

Defines `spell_aa_nodelist` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `aa_list_fk` (int(10), NOT NULL, DEFAULT 0)
- `displayedclassification` (varchar(64), NOT NULL, DEFAULT '')
- `requiredclassification` (varchar(64), NOT NULL, DEFAULT '')
- `classificationpointsrequired` (tinyint(3), NOT NULL, DEFAULT 0)
- `description` (text, DEFAULT NULL)
- `firstparentid` (int(10), NOT NULL, DEFAULT 0)
- `firstparentrequiredtier` (tinyint(3), NOT NULL, DEFAULT 0)
- `maxtier` (tinyint(3), NOT NULL, DEFAULT 0)
- `minlevel` (tinyint(3), NOT NULL, DEFAULT 0)
- `name` (varchar(64), NOT NULL, DEFAULT '')
- `nodeid` (int(10), NOT NULL, DEFAULT 0)
- `pointspertier` (smallint(5), NOT NULL, DEFAULT 0)
- `pointsspentgloballytounlock` (smallint(5), NOT NULL, DEFAULT 0)
- `pointsspentintreetounlock` (smallint(5), NOT NULL, DEFAULT 0)
- `spellcrc` (int(10), NOT NULL, DEFAULT 0)
- `title` (varchar(64), NOT NULL, DEFAULT '')
- `titlelevel` (tinyint(3), NOT NULL, DEFAULT 0)
- `xcoord` (tinyint(3), NOT NULL, DEFAULT 0)
- `ycoord` (tinyint(3), NOT NULL, DEFAULT 0)
- `icon_backdrop` (int(10), DEFAULT 0)
- `icon_id` (int(10), DEFAULT 0)

**Primary Keys:**
- id