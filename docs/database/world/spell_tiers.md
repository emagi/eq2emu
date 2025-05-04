## Table: `spell_tiers`

**Description:**

Defines `spell_tiers` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `spell_id` (int(10), NOT NULL, DEFAULT 0)
- `tier` (tinyint(3), NOT NULL, DEFAULT 0)
- `hp_req` (mediumint(8), NOT NULL, DEFAULT 0)
- `hp_req_percent` (tinyint(3), NOT NULL, DEFAULT 0)
- `hp_upkeep` (mediumint(8), NOT NULL, DEFAULT 0)
- `power_req` (float, NOT NULL, DEFAULT 0)
- `power_req_percent` (tinyint(3), NOT NULL, DEFAULT 0)
- `power_upkeep` (mediumint(8), NOT NULL, DEFAULT 0)
- `power_by_level` (tinyint(1), NOT NULL, DEFAULT 0)
- `savagery_req` (mediumint(8), NOT NULL, DEFAULT 0)
- `savagery_req_percent` (tinyint(3), NOT NULL, DEFAULT 0)
- `savagery_upkeep` (mediumint(8), NOT NULL, DEFAULT 0)
- `dissonance_req` (mediumint(8), NOT NULL, DEFAULT 0)
- `dissonance_req_percent` (tinyint(3), NOT NULL, DEFAULT 0)
- `dissonance_upkeep` (mediumint(8), NOT NULL, DEFAULT 0)
- `req_concentration` (mediumint(8), NOT NULL, DEFAULT 0)
- `cast_time` (mediumint(8), NOT NULL, DEFAULT 100)
- `recovery` (float, NOT NULL, DEFAULT 0)
- `recast` (float, NOT NULL, DEFAULT 1)
- `radius` (float, NOT NULL, DEFAULT 0)
- `max_aoe_targets` (mediumint(8), NOT NULL, DEFAULT 0)
- `min_range` (float, NOT NULL, DEFAULT 0)
- `range` (float, NOT NULL, DEFAULT 0)
- `duration1` (int(10), NOT NULL, DEFAULT 0)
- `duration2` (int(10), NOT NULL, DEFAULT 0)
- `resistibility` (float, NOT NULL, DEFAULT 0)
- `hit_bonus` (float, NOT NULL, DEFAULT 0)
- `call_frequency` (int(10), NOT NULL, DEFAULT 0)
- `unknown9` (tinyint(3), NOT NULL, DEFAULT 0)
- `given_by` (varchar(64), NOT NULL, DEFAULT '')

**Primary Keys:**
- id

**Indexes/Notes:**
- UNIQUE KEY `SpellTierIDX` (`spell_id`,`tier`)
- CONSTRAINT `FK_spell_tiers` FOREIGN KEY (`spell_id`) REFERENCES `spells` (`id`) ON DELETE CASCADE ON UPDATE CASCADE