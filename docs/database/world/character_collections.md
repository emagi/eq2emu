## Table: `character_collections`

**Description:**

Defines `character_collections` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `char_id` (int(10), NOT NULL)
- `collection_id` (int(10), NOT NULL)
- `completed` (tinyint(1), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `uk_charactercollections` (`char_id`,`collection_id`)
- KEY `fk_charactercollections_collectionid` (`collection_id`)
- CONSTRAINT `FK_character_collections` FOREIGN KEY (`char_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
- CONSTRAINT `fk_charactercollections_charid` FOREIGN KEY (`char_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
- CONSTRAINT `fk_charactercollections_collectionid` FOREIGN KEY (`collection_id`) REFERENCES `collections` (`id`) ON DELETE CASCADE ON UPDATE CASCADE