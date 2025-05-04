## Table: `spawn_npc_spells`

**Description:**

Defines `spawn_npc_spells` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `spell_list_id` (int(10), NOT NULL, DEFAULT 0)
- `spell_id` (int(10), NOT NULL, DEFAULT 0)
- `spell_tier` (tinyint(3), NOT NULL, DEFAULT 1)
- `on_spawn_cast` (tinyint(3), DEFAULT 0)
- `on_aggro_cast` (tinyint(3), DEFAULT 0)
- `required_hp_ratio` (tinyint(4), NOT NULL, DEFAULT 0)

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `ListSpellIDX` (`spell_list_id`,`spell_id`) USING BTREE
- KEY `FK_spawn_npc_spells` (`spell_id`)
- KEY `FK_npc_spells_spell_list_id` (`spell_list_id`)
- CONSTRAINT `FK_npc_spells_spell_list_id` FOREIGN KEY (`spell_list_id`) REFERENCES `spawn_npc_spell_lists` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
- CONSTRAINT `FK_spawn_npc_spells` FOREIGN KEY (`spell_id`) REFERENCES `spells` (`id`)