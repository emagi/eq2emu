## Table: `item_stat_subtypes`

**Description:**

Defines `item_stat_subtypes` table in the World database.

**Columns:**
- `stat_id` (int(12), PRIMARY KEY, NOT NULL)
- `type_id` (int(12), DEFAULT NULL)
- `subtype_id` (int(12), DEFAULT NULL)
- `name` (varchar(255), DEFAULT NULL)
- `title` (varchar(255), DEFAULT NULL)
- `label_mask` (varchar(255), DEFAULT NULL)
- `value_type` (enum('integer','percent','string','decimal'), DEFAULT NULL)
- `description` (text, DEFAULT NULL)
- `note` (text, DEFAULT NULL)

**Primary Keys:**
- stat_id

**Indexes/Notes:**
- UNIQUE KEY `stat_id_unique` (`type_id`,`subtype_id`)
- CONSTRAINT `FK_item_stat_types` FOREIGN KEY (`type_id`) REFERENCES `item_stat_types` (`id`) ON DELETE CASCADE ON UPDATE CASCADE