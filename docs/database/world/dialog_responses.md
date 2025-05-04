## Table: `dialog_responses`

**Description:**

Defines `dialog_responses` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, NOT NULL)
- `parent_dialog_id` (int(10), NOT NULL)
- `index` (int(10), NOT NULL)
- `text_id` (int(10), NOT NULL)
- `next_dialog_id` (int(10), DEFAULT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `FK__dialogs` (`parent_dialog_id`)
- KEY `FK_dialog_responses_dialog_text` (`text_id`)
- KEY `FK__dialogs_2` (`next_dialog_id`) USING BTREE
- CONSTRAINT `FK__dialogs` FOREIGN KEY (`parent_dialog_id`) REFERENCES `dialogs` (`id`) ON UPDATE CASCADE
- CONSTRAINT `FK__dialogs_2` FOREIGN KEY (`next_dialog_id`) REFERENCES `dialogs` (`id`) ON UPDATE CASCADE
- CONSTRAINT `FK_dialog_responses_dialog_text` FOREIGN KEY (`text_id`) REFERENCES `dialog_text` (`id`) ON UPDATE CASCADE