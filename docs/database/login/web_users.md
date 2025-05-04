## Table: `web_users`

**Description:**

Defines `web_users` table in the Login database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `username` (varchar(50), NOT NULL, DEFAULT '')
- `passwd` (varchar(512), NOT NULL, DEFAULT '')
- `status` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id