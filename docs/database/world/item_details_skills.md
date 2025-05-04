## Table: `item_details_skills`

**Description:**

Defines `item_details_skills` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `unknown12` (int(10), NOT NULL, DEFAULT 0)
- `unknown13` (smallint(5), NOT NULL, DEFAULT 0)
- `unknown14` (smallint(5), NOT NULL, DEFAULT 0)
- `unknown15` (smallint(5), NOT NULL, DEFAULT 0)
- `unknown16` (smallint(5), NOT NULL, DEFAULT 0)
- `unknown17` (smallint(5), NOT NULL, DEFAULT 0)
- `duration` (smallint(5), NOT NULL, DEFAULT 0)
- `unknown18` (int(10), NOT NULL, DEFAULT 0)
- `unknown19` (int(10), NOT NULL, DEFAULT 0)
- `unknown20` (int(10), NOT NULL, DEFAULT 0)
- `unknown21` (int(10), NOT NULL, DEFAULT 0)
- `unknown22` (tinyint(3), NOT NULL, DEFAULT 0)
- `unknown23` (smallint(5), NOT NULL, DEFAULT 0)
- `unknown24` (smallint(5), NOT NULL, DEFAULT 0)
- `power_req` (smallint(5), NOT NULL, DEFAULT 0)
- `power_upkeep_req` (smallint(5), NOT NULL, DEFAULT 0)
- `hp_req` (smallint(5), NOT NULL, DEFAULT 0)
- `hp_upkeep_req` (smallint(5), NOT NULL, DEFAULT 0)
- `cast` (smallint(5), NOT NULL, DEFAULT 0)
- `recovery` (smallint(5), NOT NULL, DEFAULT 0)
- `recast` (float, NOT NULL, DEFAULT 0)
- `unknown25` (float, NOT NULL, DEFAULT 0)
- `unknown26` (int(10), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `ItemSkillsIDX` (`item_id`)
- CONSTRAINT `FK_item_details_skills` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE