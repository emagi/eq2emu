## Table: `merchants`

**Description:**

Defines `merchants` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `merchant_id` (int(10), NOT NULL, DEFAULT 0)
- `inventory_id` (int(10), NOT NULL, DEFAULT 0)
- `description` (varchar(64), DEFAULT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `MerchantInventoryIDX` (`inventory_id`,`merchant_id`)