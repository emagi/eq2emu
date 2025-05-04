## Table: `ho_wheel`

**Description:**

Defines `ho_wheel` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `starter_id` (int(10), NOT NULL, DEFAULT 0)
- `order` (tinyint(3), NOT NULL, DEFAULT 0)
- `spell_id` (int(10), NOT NULL, DEFAULT 0)
- `first_ability` (smallint(5), NOT NULL, DEFAULT 65535)
- `second_ability` (smallint(5), NOT NULL, DEFAULT 65535)
- `third_ability` (smallint(5), NOT NULL, DEFAULT 65535)
- `fourth_ability` (smallint(5), NOT NULL, DEFAULT 65535)
- `fifth_ability` (smallint(5), NOT NULL, DEFAULT 65535)
- `sixth_ability` (smallint(5), NOT NULL, DEFAULT 65535)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `FK_Starter` (`starter_id`)
- KEY `FK_Spell` (`spell_id`)
- CONSTRAINT `FK_Spell` FOREIGN KEY (`spell_id`) REFERENCES `spells` (`id`)
- CONSTRAINT `FK_Starter` FOREIGN KEY (`starter_id`) REFERENCES `ho_starter_chains` (`id`) ON DELETE CASCADE ON UPDATE CASCADE