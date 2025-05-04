## Table: `spells`

**Description:**

Defines `spells` table in the World database.

**Columns:**
- `id` (int(10), PRIMARY KEY, AUTO_INCREMENT, NOT NULL)
- `soe_spell_crc` (int(10), NOT NULL, DEFAULT 0)
- `type` (smallint(5), NOT NULL, DEFAULT 0)
- `cast_type` (tinyint(3), NOT NULL, DEFAULT 0)
- `name` (varchar(250), NOT NULL, DEFAULT 'NoName')
- `description` (text, NOT NULL)
- `icon` (smallint(5), NOT NULL, DEFAULT 0)
- `icon_heroic_op` (smallint(5), NOT NULL, DEFAULT 0)
- `icon_backdrop` (smallint(5), NOT NULL, DEFAULT 0)
- `class_skill` (bigint(20), NOT NULL, DEFAULT 0)
- `mastery_skill` (bigint(20), NOT NULL, DEFAULT 0)
- `min_class_skill_req` (smallint(5), NOT NULL, DEFAULT 0)
- `duration_until_cancel` (tinyint(1), NOT NULL, DEFAULT 0)
- `target_type` (tinyint(3), NOT NULL, DEFAULT 0)
- `success_message` (varchar(255), NOT NULL, DEFAULT '')
- `fade_message` (varchar(255), NOT NULL, DEFAULT '')
- `fade_message_others` (varchar(255), NOT NULL, DEFAULT '')
- `interruptable` (tinyint(1), NOT NULL, DEFAULT 1)
- `cast_while_moving` (tinyint(1), NOT NULL, DEFAULT 0)
- `lua_script` (varchar(255), NOT NULL, DEFAULT '')
- `spell_visual` (int(10), NOT NULL, DEFAULT 0)
- `effect_message` (varchar(255), NOT NULL, DEFAULT '')
- `spell_book_type` (tinyint(3), NOT NULL, DEFAULT 0)
- `can_effect_raid` (tinyint(1), NOT NULL, DEFAULT 0)
- `affect_only_group_members` (tinyint(1), NOT NULL, DEFAULT 0)
- `display_spell_tier` (tinyint(1), NOT NULL, DEFAULT 0)
- `friendly_spell` (tinyint(1), NOT NULL, DEFAULT 0)
- `group_spell` (tinyint(1), NOT NULL, DEFAULT 0)
- `det_type` (tinyint(3), NOT NULL, DEFAULT 0)
- `control_effect_type` (tinyint(3), NOT NULL, DEFAULT 0)
- `incurable` (tinyint(1), NOT NULL, DEFAULT 0)
- `linked_timer_id` (int(10), NOT NULL, DEFAULT 0)
- `not_maintained` (tinyint(1), NOT NULL, DEFAULT 0)
- `casting_flags` (int(10), NOT NULL, DEFAULT 0)
- `persist_through_death` (tinyint(1), NOT NULL, DEFAULT 0)
- `savage_bar` (tinyint(3), NOT NULL, DEFAULT 0)
- `savage_bar_slot` (tinyint(3), NOT NULL, DEFAULT 0)
- `is_active` (tinyint(3), NOT NULL, DEFAULT 0)
- `is_aa` (tinyint(1), NOT NULL, DEFAULT 0)
- `is_deity` (tinyint(1), NOT NULL, DEFAULT 0)
- `deity` (tinyint(3), NOT NULL, DEFAULT 0)
- `spell_type` (enum('Unset','DD','DoT','Heal','HoT-Ward','Debuff','Buff','CombatBuff','Taunt','Detaunt','Rez','Cure','Food','Drink','Root','Snare','GroupTarget'), NOT NULL, DEFAULT 'Unset')
- `last_auto_update` (int(10), NOT NULL, DEFAULT 0)
- `soe_last_update` (int(10), NOT NULL, DEFAULT 0)
- `type_group_spell_id` (int(10), NOT NULL, DEFAULT 0)
- `can_fizzle` (tinyint(1), NOT NULL, DEFAULT 1)

**Primary Keys:**
- id

**Indexes/Notes:**
- KEY `SpellIDX` (`name`)
- KEY `SpellLUAIDX` (`lua_script`)
- KEY `SpellSkillsIDX` (`class_skill`,`mastery_skill`)