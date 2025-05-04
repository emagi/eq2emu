## Table: `spell_data`

**Description:**

Defines `spell_data` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `spell_id` (int(10), NOT NULL, DEFAULT 0)
- `tier` (int(10), NOT NULL, DEFAULT 1)
- `index_field` (tinyint(3), NOT NULL, DEFAULT 0)
- `value_type` (enum('INT','FLOAT','BOOL','STRING'), NOT NULL, DEFAULT 'INT')
- `value` (varchar(255), NOT NULL, DEFAULT '0')
- `value2` (varchar(255), NOT NULL, DEFAULT '0')
- `dynamic_helper` (varchar(255), NOT NULL, DEFAULT '0')

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `SpellTierIDX` (`spell_id`,`tier`,`index_field`)
- CONSTRAINT `FK_spell_data` FOREIGN KEY (`spell_id`) REFERENCES `spells` (`id`) ON DELETE CASCADE ON UPDATE CASCADE