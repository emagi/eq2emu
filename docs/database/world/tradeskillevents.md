## Table: `tradeskillevents`

**Description:**

Defines `tradeskillevents` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `name` (varchar(250), DEFAULT NULL)
- `icon` (mediumint(8), NOT NULL, DEFAULT 0)
- `technique` (int(10), NOT NULL, DEFAULT 0)
- `success_progress` (mediumint(8), NOT NULL, DEFAULT 0)
- `success_durability` (mediumint(8), NOT NULL, DEFAULT 0)
- `success_hp` (mediumint(8), NOT NULL, DEFAULT 0)
- `success_power` (mediumint(8), NOT NULL, DEFAULT 0)
- `success_spell_id` (int(10), NOT NULL, DEFAULT 0)
- `success_item_id` (int(10), NOT NULL, DEFAULT 0)
- `fail_progress` (mediumint(8), NOT NULL, DEFAULT 0)
- `fail_durability` (mediumint(8), NOT NULL, DEFAULT 0)
- `fail_hp` (mediumint(8), NOT NULL, DEFAULT 0)
- `fail_power` (mediumint(8), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id