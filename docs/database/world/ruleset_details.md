## Table: `ruleset_details`

**Description:**

Defines `ruleset_details` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `ruleset_id` (int(10), NOT NULL, DEFAULT 1)
- `rule_category` (varchar(64), NOT NULL, DEFAULT '')
- `rule_type` (varchar(64), NOT NULL, DEFAULT '')
- `rule_value` (varchar(257), NOT NULL, DEFAULT '')
- `description` (varchar(256), DEFAULT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `RuleCatTypeIDX` (`ruleset_id`,`rule_category`,`rule_type`)