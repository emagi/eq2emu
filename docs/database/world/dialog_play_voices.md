## Table: `dialog_play_voices`

**Description:**

Defines `dialog_play_voices` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, NOT NULL)
- `npc_id` (int(10), NOT NULL)
- `voiceover_id` (int(10), NOT NULL)
- `language` (tinyint(3), NOT NULL)
- `garbled_text_id` (int(10), DEFAULT NULL)
- `log_id` (int(10), NOT NULL)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `Unique_NpcVoGarb` (`npc_id`,`voiceover_id`,`garbled_text_id`) USING BTREE
- KEY `FK__dialog_voiceovers` (`voiceover_id`)
- KEY `FK_dialog_play_voices_dialog_text` (`garbled_text_id`)
- CONSTRAINT `FK__dialog_npcs` FOREIGN KEY (`npc_id`) REFERENCES `dialog_npcs` (`id`) ON UPDATE CASCADE
- CONSTRAINT `FK__dialog_voiceovers` FOREIGN KEY (`voiceover_id`) REFERENCES `dialog_voiceovers` (`id`) ON UPDATE CASCADE
- CONSTRAINT `FK_dialog_play_voices_dialog_text` FOREIGN KEY (`garbled_text_id`) REFERENCES `dialog_text` (`id`) ON UPDATE CASCADE