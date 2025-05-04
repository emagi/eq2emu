## Table: `bots`

**Description:**

Defines `bots` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `char_id` (int(10), NOT NULL)
- `bot_id` (int(10), NOT NULL)
- `name` (varchar(64), NOT NULL)
- `race` (tinyint(3), NOT NULL, DEFAULT 0)
- `class` (tinyint(3), NOT NULL, DEFAULT 0)
- `gender` (tinyint(3), NOT NULL, DEFAULT 0)
- `model_type` (smallint(5), NOT NULL, DEFAULT 0)
- `hair_type` (smallint(5), NOT NULL, DEFAULT 0)
- `face_type` (smallint(5), NOT NULL, DEFAULT 0)
- `wing_type` (smallint(5), NOT NULL, DEFAULT 0)
- `chest_type` (smallint(5), NOT NULL, DEFAULT 0)
- `legs_type` (smallint(5), NOT NULL, DEFAULT 0)
- `soga_model_type` (smallint(5), NOT NULL, DEFAULT 0)
- `soga_hair_type` (smallint(5), NOT NULL, DEFAULT 0)
- `soga_face_type` (smallint(5), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `char_id_bot_id` (`char_id`,`bot_id`)
- CONSTRAINT `FK_char_id` FOREIGN KEY (`char_id`) REFERENCES `characters` (`id`) ON DELETE CASCADE ON UPDATE CASCADE