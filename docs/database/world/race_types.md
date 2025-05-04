## Table: `race_types`

**Description:**

Defines `race_types` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `category` (varchar(64), DEFAULT NULL)
- `subcategory` (varchar(64), DEFAULT NULL)
- `model_type` (int(10), NOT NULL)
- `model_name` (varchar(250), DEFAULT NULL)
- `race_id` (smallint(5), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `NewIndex1` (`model_type`)