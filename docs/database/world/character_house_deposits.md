## Table: `character_house_deposits`

**Description:**

Defines `character_house_deposits` table in the World database.

**Columns:**
- `timestamp` (int(10), NOT NULL, DEFAULT 0)
- `house_id` (int(10), NOT NULL, DEFAULT 0)
- `instance_id` (int(10), NOT NULL, DEFAULT 0)
- `name` (varchar(64), NOT NULL, DEFAULT '')
- `amount` (bigint(20), NOT NULL, DEFAULT 0)
- `last_amount` (bigint(20), NOT NULL, DEFAULT 0)
- `status` (int(10), NOT NULL, DEFAULT 0)
- `last_status` (int(10), NOT NULL, DEFAULT 0)