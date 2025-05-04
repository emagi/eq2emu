## Table: `dialog_flavors`

**Description:**

Defines `dialog_flavors` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, NOT NULL)
- `voiceover_id` (int(10), DEFAULT NULL)
- `text_id` (int(10), NOT NULL)
- `language` (tinyint(3), NOT NULL, DEFAULT 0)
- `understood` (tinyint(3), NOT NULL, DEFAULT 0)
- `emote` (text, NOT NULL, DEFAULT '')
- `emote_text_id` (int(10), DEFAULT NULL)
- `unknown4` (tinyint(3), NOT NULL, DEFAULT 0)
- `log_id` (int(10), NOT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `IndexEmote` (`emote`(100))
- KEY `FK__flavor_dialog_text` (`text_id`)
- KEY `FK_dialog_flavors_dialog_voiceovers` (`voiceover_id`)
- KEY `FK_dialog_flavors_dialog_text` (`emote_text_id`)
- CONSTRAINT `FK__flavor_dialog_text` FOREIGN KEY (`text_id`) REFERENCES `dialog_text` (`id`) ON UPDATE CASCADE
- CONSTRAINT `FK_dialog_flavors_dialog_text` FOREIGN KEY (`emote_text_id`) REFERENCES `dialog_text` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
- CONSTRAINT `FK_dialog_flavors_dialog_voiceovers` FOREIGN KEY (`voiceover_id`) REFERENCES `dialog_voiceovers` (`id`) ON UPDATE CASCADE