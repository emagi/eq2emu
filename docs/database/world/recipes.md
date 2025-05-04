## Table: `recipes`

**Description:**

Defines `recipes` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `recipe_id` (int(10), NOT NULL, DEFAULT 0)
- `tier` (tinyint(3), NOT NULL, DEFAULT 0)
- `level` (tinyint(3), NOT NULL, DEFAULT 0)
- `icon` (smallint(5), NOT NULL, DEFAULT 0)
- `skill_level` (tinyint(3), NOT NULL, DEFAULT 0)
- `technique` (int(10), NOT NULL, DEFAULT 0)
- `knowledge` (int(10), NOT NULL, DEFAULT 0)
- `name` (varchar(200), DEFAULT 'Unknown')
- `book` (varchar(200), DEFAULT 'Unknown')
- `device` (enum('Chemistry, NOT NULL)
- `product_classes` (tinyint(3), NOT NULL, DEFAULT 0)
- `unknown2` (int(10), NOT NULL, DEFAULT 0)
- `unknown3` (int(10), NOT NULL, DEFAULT 0)
- `unknown4` (int(10), NOT NULL, DEFAULT 0)
- `product_item_id` (int(10), NOT NULL)
- `product_name` (varchar(200), DEFAULT NULL)
- `product_qty` (smallint(5), NOT NULL)
- `primary_comp_title` (varchar(200), DEFAULT NULL)
- `build_comp_title` (varchar(200), DEFAULT NULL)
- `build2_comp_title` (varchar(200), DEFAULT NULL)
- `build3_comp_title` (varchar(200), DEFAULT NULL)
- `build4_comp_title` (varchar(200), DEFAULT NULL)
- `build_comp_qty` (varchar(200), DEFAULT NULL)
- `build2_comp_qty` (varchar(200), DEFAULT NULL)
- `build3_comp_qty` (varchar(200), DEFAULT NULL)
- `build4_comp_qty` (varchar(200), DEFAULT NULL)
- `fuel_comp_title` (varchar(200), DEFAULT NULL)
- `fuel_comp_qty` (varchar(200), DEFAULT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `recipe_id` (`recipe_id`)