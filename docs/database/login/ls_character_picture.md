## Table: `ls_character_picture`

**Description:**

Defines `ls_character_picture` table in the Login database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `server_id` (int(10), NOT NULL)
- `account_id` (int(10), NOT NULL)
- `character_id` (int(10), NOT NULL)
- `picture` (text, NOT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `character_id` (`character_id`,`server_id`,`account_id`)