## Table: `transmuting`

**Description:**

Defines `transmuting` table in the World database.

**Columns:**
- `tier_id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `min_level` (int(10), NOT NULL)
- `max_level` (int(10), NOT NULL)
- `fragment` (int(10), NOT NULL)
- `powder` (int(10), NOT NULL)
- `infusion` (int(10), NOT NULL)
- `mana` (int(10), NOT NULL)

**Primary Keys:**
- tier_id

**Indexes/Notes:**
- KEY `FK_transmuting_items` (`fragment`)
- KEY `FK_transmuting_items_2` (`powder`)
- KEY `FK_transmuting_items_3` (`infusion`)
- KEY `FK_transmuting_items_4` (`mana`)
- CONSTRAINT `FK_transmuting_items` FOREIGN KEY (`fragment`) REFERENCES `items` (`id`) ON UPDATE CASCADE
- CONSTRAINT `FK_transmuting_items_2` FOREIGN KEY (`powder`) REFERENCES `items` (`id`) ON UPDATE CASCADE
- CONSTRAINT `FK_transmuting_items_3` FOREIGN KEY (`infusion`) REFERENCES `items` (`id`) ON UPDATE CASCADE
- CONSTRAINT `FK_transmuting_items_4` FOREIGN KEY (`mana`) REFERENCES `items` (`id`) ON UPDATE CASCADE