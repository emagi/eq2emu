## Table: `character_collection_items`

**Description:**

Defines `character_collection_items` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `char_id` (int(10), NOT NULL)
- `collection_id` (int(10), NOT NULL)
- `collection_item_id` (int(10), NOT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `uk_charactercollectionitems` (`char_id`,`collection_id`,`collection_item_id`)
- KEY `fk_charactercollectionitems_collectionid` (`collection_id`)
- KEY `fk_charactercollectionitems_collectionitemid` (`collection_item_id`)
- CONSTRAINT `FK_char_col_items` FOREIGN KEY (`char_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
- CONSTRAINT `fk_charactercollectionitems_charid` FOREIGN KEY (`char_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
- CONSTRAINT `fk_charactercollectionitems_collectionid` FOREIGN KEY (`collection_id`) REFERENCES `collections` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
- CONSTRAINT `fk_charactercollectionitems_collectionitemid` FOREIGN KEY (`collection_item_id`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE