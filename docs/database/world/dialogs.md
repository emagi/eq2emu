## Table: `dialogs`

**Description:**

Defines `dialogs` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, NOT NULL)
- `npc_id` (int(10), NOT NULL)
- `voiceover_id` (int(10), DEFAULT NULL)
- `title_text_id` (int(10), DEFAULT NULL)
- `msg_text_id` (int(10), DEFAULT NULL)
- `closeable` (tinyint(1), NOT NULL, DEFAULT 1)
- `signature` (tinyint(1), NOT NULL)
- `language` (tinyint(3), NOT NULL)
- `log_id` (int(10), NOT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `FK_dialogs_dialog_npcs` (`voiceover_id`)
- KEY `FK_dialogs_dialog_npcs1` (`npc_id`)
- KEY `FK_dialogs_dialog_text` (`title_text_id`)
- KEY `FK_dialogs_dialog_text_2` (`msg_text_id`)
- CONSTRAINT `FK_dialogs_dialog_npcs1` FOREIGN KEY (`npc_id`) REFERENCES `dialog_npcs` (`id`) ON UPDATE CASCADE
- CONSTRAINT `FK_dialogs_dialog_text` FOREIGN KEY (`title_text_id`) REFERENCES `dialog_text` (`id`) ON UPDATE CASCADE
- CONSTRAINT `FK_dialogs_dialog_text_2` FOREIGN KEY (`msg_text_id`) REFERENCES `dialog_text` (`id`) ON UPDATE CASCADE
- CONSTRAINT `FK_dialogs_dialog_voiceovers` FOREIGN KEY (`voiceover_id`) REFERENCES `dialog_voiceovers` (`id`) ON UPDATE CASCADE