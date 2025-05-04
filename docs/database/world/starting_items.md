## Table: `starting_items`

**Description:**

Defines `starting_items` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `class_id` (tinyint(3), NOT NULL, DEFAULT 255)
- `race_id` (tinyint(3), NOT NULL, DEFAULT 255)
- `type` (enum('EQUIPPED','NOT-EQUIPPED'), NOT NULL, DEFAULT 'NOT-EQUIPPED')
- `item_id` (int(10), NOT NULL, DEFAULT 0)
- `creator` (varchar(64), DEFAULT NULL)
- `condition_` (tinyint(3), NOT NULL, DEFAULT 100)
- `attuned` (tinyint(3), NOT NULL, DEFAULT 0)
- `count` (tinyint(3), NOT NULL, DEFAULT 1)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `NewIndex` (`class_id`,`race_id`,`type`,`item_id`)
- KEY `FK_starting_items` (`item_id`)
- CONSTRAINT `FK_starting_items` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`) ON UPDATE CASCADE