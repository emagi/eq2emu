## Table: `rulesets`

**Description:**

Defines `rulesets` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `ruleset_id` (int(10), NOT NULL)
- `ruleset_name` (varchar(64), NOT NULL, DEFAULT 'default_ruleset')
- `ruleset_active` (tinyint(1), NOT NULL, DEFAULT 1)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `RuleNameIDX` (`ruleset_name`)
- UNIQUE KEY `ruleset_id` (`ruleset_id`)
- KEY `RulesIDX` (`ruleset_id`)