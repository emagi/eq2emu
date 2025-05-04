## Table: `dbeditor_log`

**Description:**

Defines `dbeditor_log` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `char_id` (int(11), NOT NULL)
- `char_name` (varchar(64), DEFAULT NULL)
- `admin_status` (smallint(5), NOT NULL)
- `item_name` (varchar(250), DEFAULT NULL)
- `table_name` (varchar(32), DEFAULT NULL)
- `update_query` (text, DEFAULT NULL)
- `update_date` (int(11), NOT NULL)
- `archived` (tinyint(1), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id