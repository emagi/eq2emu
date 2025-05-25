# InfoStruct Field Reference (EverQuest II Emulator)

> **Key to data types**
> `uint8 / uint16 / uint32` - Unsigned integers (8, 16, 32 bits)
> `int8 / int16 / int32` - Signed integers (8, 16, 32 bits)
> `float` - Single-precision floating-point value
> `string` - Plain UTF-8 text

---

## Identity and Classification

| Field     | Type   | Description                                   |
| --------- | ------ | --------------------------------------------- |
| name      | string | Character name shown to players.              |
| class1    | uint8  | Archetype ID (Fighter = 1, Priest = 11).      |
| class2    | uint8  | Class ID at level 10 (Warrior = 2).           |
| class3    | uint8  | Final subclass ID (Guardian = 3).             |
| race      | uint8  | Race ID (Human, Kerra, Ogre, etc.).           |
| gender    | uint8  | 1 = male, 0 = female.                         |
| alignment | int8   | 0 = evil, 1 = good, 2 = neutral.              |
| deity     | string | Chosen deity or "None".                       |
| tag1      | uint8  | Generic bit field used for GM or event flags. |

---

## Levels and Experience

| Field                                               | Type   | Description                                       |
| --------------------------------------------------- | ------ | ------------------------------------------------- |
| level / max\_level                                  | uint16 | Current and maximum adventure level.              |
| effective\_level                                    | uint16 | Level shown to others while mentored or scaled.   |
| tradeskill\_level / tradeskill\_max\_level          | uint16 | Crafting level and cap.                           |
| xp / xp\_needed                                     | uint32 | Adventure experience and amount needed to level.  |
| xp\_debt                                            | float  | Adventure debt percentage (0 to 50).              |
| xp\_yellow / xp\_blue                               | uint16 | Width of yellow or blue XP bar.                   |
| xp\_yellow\_vitality\_bar / xp\_blue\_vitality\_bar | uint16 | Vitality overlay on XP bar.                       |
| xp\_vitality                                        | float  | Rested vitality percent (0 to 100).               |
| ts\_xp / ts\_xp\_needed                             | uint32 | Tradeskill experience and amount needed to level. |
| tradeskill\_exp\_yellow / tradeskill\_exp\_blue     | uint16 | Tradeskill XP bar values.                         |
| tradeskill\_xp\_vitality                            | float  | Rested vitality for crafting.                     |
| account\_age\_base                                  | uint32 | Account age in seconds for veteran rewards.       |

---

## Core Attributes

| Field               | Type  | Description                       |
| ------------------- | ----- | --------------------------------- |
| str / str\_base     | float | Strength, current and unmodified. |
| sta / sta\_base     | float | Stamina.                          |
| agi / agi\_base     | float | Agility.                          |
| wis / wis\_base     | float | Wisdom.                           |
| intel / intel\_base | float | Intelligence.                     |

---

## Concentration and Resources

| Field                                         | Type  | Description                                   |
| --------------------------------------------- | ----- | --------------------------------------------- |
| cur\_concentration                            | uint8 | Slots used by maintained spells.              |
| max\_concentration / max\_concentration\_base | uint8 | Maximum concentration slots.                  |
| power\_regen / hp\_regen                      | int16 | In-combat power and hp regeneration per tick. |
| power\_regen\_override / hp\_regen\_override  | uint8 | Overrides displayed regen if non-zero.        |

---

## Combat Offense and Defense

| Field                                                                               | Type           | Description                                              |
| ----------------------------------------------------------------------------------- | -------------- | -------------------------------------------------------- |
| cur\_attack / attack\_base                                                          | uint16         | Attack rating.                                           |
| cur\_mitigation / mitigation\_base / max\_mitigation                                | uint16         | Armor mitigation values.                                 |
| mitigation\_modifier                                                                | int16          | Bonus or penalty to mitigation.                          |
| avoidance\_display                                                                  | uint16         | Avoidance percent \* 10 shown in UI.                     |
| cur\_avoidance                                                                      | float          | Raw avoidance percent.                                   |
| base\_avoidance\_pct / avoidance\_base / max\_avoidance                             | uint16         | Internal avoidance values.                               |
| parry / parry\_base                                                                 | float          | Parry chance, current and base.                          |
| deflection / deflection\_base                                                       | float / uint16 | Brawler deflection values.                               |
| block / block\_base                                                                 | float / uint16 | Shield block chance and skill.                           |
| ability\_modifier                                                                   | float          | Flat amount added to spell or combat art damage or heal. |
| critical\_mitigation                                                                | float          | Percent reduction to incoming critical damage.           |
| block\_chance                                                                       | float          | Total block chance.                                      |
| uncontested\_block / uncontested\_parry / uncontested\_dodge / uncontested\_riposte | float          | Avoidance that cannot be contested.                      |
| crit\_chance                                                                        | float          | Chance for any attack to crit.                           |
| crit\_bonus                                                                         | float          | Extra damage on a critical hit.                          |
| potency                                                                             | float          | Multiplier to base ability values.                       |
| hate\_mod                                                                           | float          | Threat generation modifier.                              |
| reuse\_speed / casting\_speed / recovery\_speed / spell\_reuse\_speed               | float          | Percent reductions to cast, reuse, and recovery timers.  |
| dps / dps\_multiplier                                                               | float          | Legacy DPS stats for auto-attack.                        |
| attackspeed / haste                                                                 | float          | Auto-attack haste percent.                               |
| multi\_attack / flurry                                                              | float          | Chances to add extra swings.                             |
| melee\_ae                                                                           | float          | Chance to hit additional targets with melee auto-attack. |
| strikethrough                                                                       | float          | Chance to ignore target avoidance.                       |
| accuracy                                                                            | float          | Hit rate increase.                                       |
| offensivespeed                                                                      | float          | Movement speed while in combat.                          |

---

## Resistances

| Field                                              | Type   | Description                                     |
| -------------------------------------------------- | ------ | ----------------------------------------------- |
| heat, cold, magic, mental, divine, disease, poison | uint16 | Current elemental, arcane, and noxious resists. |
| heat\_base ... poison\_base                        | uint16 | Base resist values from equipment only.         |
| elemental\_base / noxious\_base / arcane\_base     | uint16 | Aggregated base resists.                        |

---

## Currency and Encumbrance

| Field                                                 | Type   | Description                          |
| ----------------------------------------------------- | ------ | ------------------------------------ |
| coin\_copper / coin\_silver / coin\_gold / coin\_plat | uint32 | Coin carried by the character.       |
| bank\_coin\_copper ... bank\_coin\_plat               | uint32 | Coin stored in the bank.             |
| status\_points                                        | uint32 | Personal status currency.            |
| weight / max\_weight                                  | uint32 | Current and maximum carrying weight. |

---

## Vital Flags and States

| Field             | Type   | Description                                           |
| ----------------- | ------ | ----------------------------------------------------- |
| flags / flags2    | uint32 | Bit fields for various account and character options. |
| water\_type       | uint8  | 0 land, 1 swimming, 2 fully underwater.               |
| flying\_type      | uint8  | 0 ground, 1 leaper, 2 glider, 3 flyer.                |
| no\_interrupt     | uint8  | 1 prevents spell interruption.                        |
| interaction\_flag | uint8  | Set when interacting with an object or NPC.           |
| mood              | uint16 | Current /mood emote.                                  |
| biography         | string | Player-authored biography text.                       |
| drunk             | float  | Drunk level (0 to 100).                               |

---

## Pet Information

| Field                              | Type   | Description                          |
| ---------------------------------- | ------ | ------------------------------------ |
| pet\_id                            | uint32 | Unique ID of the controlled pet.     |
| pet\_name                          | string | Current pet name.                    |
| pet\_health\_pct / pet\_power\_pct | float  | Pet health and power percent.        |
| pet\_movement                      | uint8  | 0 follow, 1 stay, 2 guard, etc.      |
| pet\_behavior                      | uint8  | Aggressive, defensive, passive, etc. |

---

## Weather and Environment

| Field               | Type   | Description                                          |
| ------------------- | ------ | ---------------------------------------------------- |
| rain                | float  | Rain intensity in the current zone.                  |
| wind                | float  | Wind intensity in the current zone.                  |
| vision              | uint32 | Vision mode bit mask (see invis, ultravision, etc.). |
| breathe\_underwater | uint8  | Remaining breath timer, 0 if unable to breathe.      |

---

## Mitigation Skills

| Field                                                        | Type   | Description                                      |
| ------------------------------------------------------------ | ------ | ------------------------------------------------ |
| mitigation\_skill1 / mitigation\_skill2 / mitigation\_skill3 | uint16 | Armor type skills for cloth, leather, and chain. |
| mitigation\_pve / mitigation\_pvp                            | uint16 | Damage reduction scores for PvE and PvP.         |

---

## Timers and Cool-downs

| Field                                                                                   | Type   | Description                                    |
| --------------------------------------------------------------------------------------- | ------ | ---------------------------------------------- |
| range\_last\_attack\_time / primary\_last\_attack\_time / secondary\_last\_attack\_time | uint32 | Unix time of last auto-attack swing.           |
| primary\_attack\_delay / secondary\_attack\_delay / ranged\_attack\_delay               | uint16 | Weapon delay shown to client, in milliseconds. |
| primary\_weapon\_delay / secondary\_weapon\_delay / ranged\_weapon\_delay               | uint16 | Actual delay after haste and other modifiers.  |

---

## Weapon Stats and Overrides

| Field                                                                                                                                                                                      | Type   | Description                           |
| ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | ------ | ------------------------------------- |
| primary\_weapon\_type / secondary\_weapon\_type / ranged\_weapon\_type                                                                                                                     | uint8  | 0 Slash, 1 Pierce, 2 Crush, etc.      |
| primary\_weapon\_damage\_low / primary\_weapon\_damage\_high, secondary\_weapon\_damage\_low / secondary\_weapon\_damage\_high, ranged\_weapon\_damage\_low / ranged\_weapon\_damage\_high | uint32 | Auto-attack damage ranges.            |
| wield\_type                                                                                                                                                                                | uint8  | 1 dual hand, 2 single hand, 4 two hand. |
| attack\_type                                                                                                                                                                               | uint8  | 0 melee, 1 ranged, 2 spell.           |
