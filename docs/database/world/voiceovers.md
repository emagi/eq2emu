## Table: `voiceovers`

**Description:**

Defines `voiceovers` table in the World database.

**Columns:**
- `type_id` (tinyint(3), NOT NULL, DEFAULT 0)
- `id` (int(10), NOT NULL, DEFAULT 0)
- `indexed` (smallint(5), NOT NULL, DEFAULT 0)
- `mp3_string` (text, NOT NULL, DEFAULT '')
- `text_string` (text, NOT NULL, DEFAULT '')
- `emote_string` (text, NOT NULL, DEFAULT '')
- `key1` (int(10), NOT NULL, DEFAULT 0)
- `key2` (int(10), NOT NULL, DEFAULT 0)
- `garbled` (tinyint(3), NOT NULL, DEFAULT 0)
- `garble_link_id` (tinyint(3), NOT NULL, DEFAULT 0)