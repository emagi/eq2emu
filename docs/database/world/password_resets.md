## Table: `password_resets`

**Description:**

Defines `password_resets` table in the World database.

**Columns:**
- `email` (varchar(255), NOT NULL)
- `token` (varchar(255), NOT NULL)
- `created_at` (timestamp, DEFAULT NULL)

**Indexes/Notes:**
- KEY `password_resets_email_index` (`email`)