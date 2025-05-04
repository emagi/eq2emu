## Table: `item_details_achievement_profile`

**Description:**

Defines `item_details_achievement_profile` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `item_id` (int(10), NOT NULL)
- `status_reduction` (int(10), NOT NULL, DEFAULT 0)
- `coin_reduction` (float, NOT NULL, DEFAULT 0)
- `house_type` (tinyint(3), NOT NULL, DEFAULT 0)
- `unk_string` (text, DEFAULT NULL)
- `unk1` (tinyint(3), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `FK_item_details_achievement_profile_items` (`item_id`)
- CONSTRAINT `FK_item_details_achievement_profile_items` FOREIGN KEY (`item_id`) REFERENCES `items` (`id`) ON DELETE CASCADE ON UPDATE CASCADE