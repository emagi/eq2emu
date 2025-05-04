## Table: `recipe_products`

**Description:**

Defines `recipe_products` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `recipe_id` (int(10), NOT NULL)
- `stage` (tinyint(3), NOT NULL)
- `product_id` (int(10), NOT NULL)
- `byproduct_id` (int(10), NOT NULL)
- `product_qty` (smallint(5), NOT NULL)
- `byproduct_qty` (tinyint(3), NOT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `FK_RECIPE_ID` (`recipe_id`)
- CONSTRAINT `FK_RECIPE_ID` FOREIGN KEY (`recipe_id`) REFERENCES `recipes` (`recipe_id`) ON DELETE CASCADE ON UPDATE CASCADE