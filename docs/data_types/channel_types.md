# EverQuest II / EQ2Emu Chat Channel IDs

| ID | Constant | Purpose / Notes |
|----|------------------------------|----------------------------------------------|
| 0  | `CHANNEL_ALL_TEXT` | All text (master channel that shows everything). |
| 1  | `CHANNEL_GAME_TEXT` | System & game-generated text. |
| 2  | `CHANNEL_DEFAULT` | Default output when no channel is specified. |
| 3  | `CHANNEL_ERROR` | Error messages. |
| 4  | `CHANNEL_STATUS` | Server-status and progress messages. |
| 5  | `CHANNEL_MOTD` | Message of the day. |
| 6  | `CHANNEL_CHAT_TEXT` | Generic chat text (catch-all). |
| 7  | `CHANNEL_NEARBY_CHAT` | Local / proximity chat (area say). |
| 8  | `CHANNEL_SAY` | Player **/say**. |
| 9  | `CHANNEL_SHOUT` | Player **/shout** (zone-wide). |
| 10 | `CHANNEL_EMOTE` | **/emote** text. |
| 11 | `CHANNEL_YELL` | **/yell** for help / break encounter. |
| 12 | `CHANNEL_NARRATIVE` | Narrative text (white system narration). |
| 13 | `CHANNEL_NONPLAYER_SAY` | NPC speech rendered as **/say**. |
| 14 | `CHANNEL_GROUP_CHAT` | General group-chat umbrella. |
| 15 | `CHANNEL_GROUP_SAY` | Group **/gsay** (use in groups). |
| 16 | `CHANNEL_RAID_SAY` | Raid **/rsay**. |
| 17 | `CHANNEL_GUILD_CHAT` | Guild chat umbrella. |
| 18 | `CHANNEL_GUILD_SAY` | Guild **/gsay** (use in guilds). |
| 19 | `CHANNEL_OFFICER_SAY` | Officer-only guild chat. |
| 20 | `CHANNEL_GUILD_MOTD` | Guild MOTD. |
| 21 | `CHANNEL_GUILD_MEMBER_ONLINE` | Guild-member login/logout notices. |
| 22 | `CHANNEL_GUILD_EVENT` | Guild-event announcements. |
| 23 | `CHANNEL_GUILD_RECRUITING_PAGE` | Your guild’s Looking-For-Members page. |
| 24 | `CHANNEL_GUILD_RECRUITING_PAGE_OTHER` | Other guilds’ recruiting pages. |
| 25 | `CHANNEL_PRIVATE_CHAT` | Generic private-chat umbrella. |
| 26 | `CHANNEL_NONPLAYER_TELL` | NPC tells to the player. |
| 27 | `CHANNEL_OBJECT_TEXT` | Interactive-object messages. |
| 28 | `CHANNEL_PRIVATE_TELL` | Player-to-player **/tell**. |
| 29 | `CHANNEL_TELL_FROM_CS` | Game-master / CS tells. |
| 30 | `CHANNEL_ARENA` | Arena-match announcements. |
| 31 | `CHANNEL_CHAT_CHANNEL_TEXT` | Custom chat-channel text. |
| 32 | `CHANNEL_OUT_OF_CHARACTER` | OOC chat. |
| 33 | `CHANNEL_AUCTION` | **/auction** channel. |
| 34 | `CHANNEL_CUSTOM_CHANNEL` | Reserved (messages here do **not** display). |
| 35 | `CHANNEL_CHARACTER_TEXT` | Character progression & trait messages. |
| 36 | `CHANNEL_REWARD` | Quest or achievement rewards. |
| 37 | `CHANNEL_DEATH` | Death notifications. |
| 38 | `CHANNEL_PET_CHAT` | Pet commands & replies. |
| 39 | `CHANNEL_SKILL` | Skill-up notices. |
| 40 | `CHANNEL_FACTION` | Faction standing changes. |

### Combat-Related Channels

| ID | Constant | Purpose / Notes |
|----|------------------------------|----------------------------------------------|
| 41 | `CHANNEL_SPELLS` | General spell messages. |
| 42 | `CHANNEL_YOU_CAST` | You successfully cast. |
| 43 | `CHANNEL_YOU_FAIL` | Your spell fizzles / is resisted. |
| 44 | `CHANNEL_CRITICAL_CAST` | Your spell crits. |
| 45 | `CHANNEL_FRIENDLY_CAST` | Ally casts beneficial spell. |
| 46 | `CHANNEL_FRIENDLY_FAIL` | Ally spell fails. |
| 47 | `CHANNEL_OTHER_CAST` | Neutral/unknown entity casts. |
| 48 | `CHANNEL_OTHER_FAIL` | Neutral entity spell fails. |
| 49 | `CHANNEL_HOSTILE_CAST` | Enemy spell cast. |
| 50 | `CHANNEL_HOSTILE_FAIL` | Enemy spell fails. |
| 51 | `CHANNEL_WORN_OFF` | Buffs/debuffs fade. |
| 52 | `CHANNEL_SPELLS_OTHER` | Miscellaneous spell events. |
| 53 | `CHANNEL_HEAL_SPELLS` | Generic healing messages. |
| 54 | `CHANNEL_HEALS` | Your heals. |
| 55 | `CHANNEL_FRIENDLY_HEALS` | Ally heals. |
| 56 | `CHANNEL_OTHER_HEALS` | Neutral heals. |
| 57 | `CHANNEL_HOSTILE_HEALS` | Enemy heals. |
| 58 | `CHANNEL_CRITICAL_HEALS` | Critical heals (any source). |
| 59 | `CHANNEL_COMBAT` | Generic combat umbrella. |
| 60 | `CHANNEL_GENERAL_COMBAT` | General combat flow. |
| 61 | `CHANNEL_HEROIC_OPPORTUNITY` | Heroic Opportunity prompts. |
| 62 | `CHANNEL_NON_MELEE_DAMAGE` | Spell/ability damage. |
| 63 | `CHANNEL_DAMAGE_SHIELD` | Damage-shield procs. |
| 64 | `CHANNEL_WARD` | Wards absorbing damage. |
| 65 | `CHANNEL_DAMAGE_INTERCEPT` | Damage interception / stoneskins. |
| 66 | `CHANNEL_MELEE_COMBAT` | Melee combat umbrella. |
| 67 | `CHANNEL_WARNINGS` | Critical warnings (low health etc.). |
| 68 | `CHANNEL_YOU_HIT` | Your melee hit lands. |
| 69 | `CHANNEL_YOU_MISS` | Your melee swing misses. |
| 70 | `CHANNEL_ATTACKER_HITS` | Enemy hits you. |
| 71 | `CHANNEL_ATTACKER_MISSES` | Enemy misses you. |
| 72 | `CHANNEL_YOUR_PET_HITS` | Pet lands a hit. |
| 73 | `CHANNEL_YOUR_PET_MISSES` | Pet misses. |
| 74 | `CHANNEL_ATTACKER_HITS_PET` | Enemy hits your pet. |
| 75 | `CHANNEL_ATTACKER_MISSES_PET` | Enemy misses your pet. |
| 76 | `CHANNEL_OTHER_HIT` | Allied/neutral entity hits. |
| 77 | `CHANNEL_OTHER_MISSES` | Allied/neutral entity misses. |
| 78 | `CHANNEL_CRITICAL_HIT` | Any critical melee hit. |
| 79 | `CHANNEL_HATE_ADJUSTMENTS` | Threat (hate) modifications. |
| 80 | `CHANNEL_YOUR_HATE` | Your threat changes. |
| 81 | `CHANNEL_OTHERS_HATE` | Others’ threat changes. |
| 82 | `CHANNEL_DISPELS_AND_CURES` | Dispels & cures umbrella. |
| 83 | `CHANNEL_DISPEL_YOU` | You dispel. |
| 84 | `CHANNEL_DISPEL_OTHER` | Ally/other dispels. |
| 85 | `CHANNEL_CURE_YOU` | You cure. |
| 86 | `CHANNEL_CURE_OTHER` | Ally/other cures. |

### Miscellaneous / Utility Channels

| ID | Constant | Purpose / Notes |
|----|------------------------------|----------------------------------------------|
| 87 | `CHANNEL_OTHER` | Unclassified system text. |
| 88 | `CHANNEL_MONEY_SPLIT` | Coin split messages. |
| 89 | `CHANNEL_LOOT` | Loot acquisition. |
| 90 | `CHANNEL_LOOT_ROLLS` | Need/greed rolls. |
| 91 | `CHANNEL_COMMAND_TEXT` | Command feedback. |
| 92 | `CHANNEL_BROADCAST` | **Global broadcast** (always shows). |
| 93 | `CHANNEL_WHO` | **/who** command output. |
| 94 | `CHANNEL_COMMANDS` | Command usage hints. |
| 95 | `CHANNEL_MERCHANT` | Merchant dialogue. |
| 96 | `CHANNEL_MERCHANT_BUY_SELL` | Buy/sell confirmations. |
| 97 | `CHANNEL_CONSIDER_MESSAGE` | **/con** (target difficulty). |
| 98 | `CHANNEL_CON_MINUS_2` | Target is -2 levels (green). |
| 99 | `CHANNEL_CON_MINUS_1` | Target is -1 level (light blue). |
| 100 | `CHANNEL_CON_0` | Target is even-con (white). |
| 101 | `CHANNEL_CON_1` | Target is +1 level (yellow). |
| 102 | `CHANNEL_CON_2` | Target is +2 levels (orange/red). |
| 103 | `CHANNEL_TRADESKILLS` | Tradeskill system messages. |
| 104 | `CHANNEL_HARVESTING` | Harvesting successes. |
| 105 | `CHANNEL_HARVESTING_WARNINGS` | Harvesting failures & warnings. |
| 107 | `CHANNEL_VOICE_CHAT` | Voice-chat events. |

---

### Gaps & Reserved Values

* **34** – `CHANNEL_CUSTOM_CHANNEL` exists but is **suppressed** (will not display on the client).  
* **106** – intentionally unused (anything sent on 106 will not display).