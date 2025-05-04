## Table: `eq2classes`

**Description:**

Defines `eq2classes` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `class_id` (int(10), NOT NULL, DEFAULT 0)
- `class_name` (varchar(64), DEFAULT NULL)
- `class_alignment` (enum('Good','Evil'), DEFAULT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `ClassIDX` (`class_id`)