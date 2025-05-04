## Table: `eq2models`

**Description:**

Defines `eq2models` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `category` (varchar(64), DEFAULT NULL)
- `subcategory` (varchar(64), DEFAULT NULL)
- `model_type` (int(10), NOT NULL)
- `model_name` (varchar(250), DEFAULT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `NewIndex1` (`model_type`)