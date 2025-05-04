## Table: `visual_states`

**Description:**

Defines `visual_states` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `visual_state_id` (int(10), NOT NULL, DEFAULT 0)
- `name` (varchar(250), NOT NULL, DEFAULT 'None')
- `min_client_version` (smallint(5), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `VisIDX` (`visual_state_id`)