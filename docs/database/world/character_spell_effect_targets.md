## Table: `character_spell_effect_targets`

**Description:**

Defines `character_spell_effect_targets` table in the World database.

**Columns:**
- `caster_char_id` (int(10), NOT NULL, DEFAULT 0)
- `target_char_id` (int(10), NOT NULL, DEFAULT 0)
- `target_type` (tinyint(3), NOT NULL, DEFAULT 0)
- `db_effect_type` (tinyint(3), NOT NULL, DEFAULT 0)
- `spell_id` (int(10), NOT NULL, DEFAULT 0)
- `effect_slot` (int(10), NOT NULL, DEFAULT 0)
- `slot_pos` (int(10), NOT NULL, DEFAULT 0)