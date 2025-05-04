## Table: `variables`

**Description:**

Defines `variables` table in the World database.

**Columns:**
- `variable_name` (varchar(64), PRIMARY KEY, NOT NULL, DEFAULT '')
- `variable_value` (varchar(512), NOT NULL, DEFAULT '')
- `comment` (varchar(255), DEFAULT NULL)

**Primary Keys:**
- variable_name