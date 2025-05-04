## Table: `character_spell_effects`

**Description:**

Defines `character_spell_effects` table in the World database.

**Columns:**
- `name` (varchar(64), NOT NULL, DEFAULT '')
- `caster_char_id` (int(10), NOT NULL, DEFAULT 0)
- `target_char_id` (int(10), NOT NULL, DEFAULT 0)
- `target_type` (tinyint(3), NOT NULL, DEFAULT 0)
- `db_effect_type` (tinyint(3), NOT NULL, DEFAULT 0)
- `spell_id` (int(10), NOT NULL, DEFAULT 0)
- `effect_slot` (int(10), NOT NULL, DEFAULT 0)
- `slot_pos` (int(10), NOT NULL, DEFAULT 0)
- `icon` (smallint(5), NOT NULL, DEFAULT 0)
- `icon_backdrop` (smallint(5), NOT NULL, DEFAULT 0)
- `conc_used` (tinyint(3), NOT NULL, DEFAULT 0)
- `tier` (tinyint(3), NOT NULL, DEFAULT 0)
- `total_time` (float, NOT NULL, DEFAULT 0)
- `expire_timestamp` (int(10), NOT NULL, DEFAULT 0)
- `lua_file` (text, NOT NULL, DEFAULT '')
- `custom_spell` (tinyint(3), NOT NULL, DEFAULT 0)
- `charid` (int(10), NOT NULL, DEFAULT 0)
- `damage_remaining` (int(10), NOT NULL, DEFAULT 0)
- `effect_bitmask` (int(10), NOT NULL, DEFAULT 0)
- `num_triggers` (smallint(5), NOT NULL, DEFAULT 0)
- `had_triggers` (tinyint(3), NOT NULL, DEFAULT 0)
- `cancel_after_triggers` (tinyint(3), NOT NULL, DEFAULT 0)
- `crit` (tinyint(3), NOT NULL, DEFAULT 0)
- `last_spellattack_hit` (tinyint(3), NOT NULL, DEFAULT 0)
- `interrupted` (tinyint(3), NOT NULL, DEFAULT 0)
- `resisted` (tinyint(3), NOT NULL, DEFAULT 0)
- `has_damaged` (tinyint(3), NOT NULL, DEFAULT 0)
- `custom_function` (text, NOT NULL)
- `caster_level` (smallint(5), NOT NULL, DEFAULT 0)