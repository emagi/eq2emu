### Command: /gm (see arguments)

**Handler Macro:** COMMAND_GM

**Handler Value:** 513

**Required Status:** 200

**Arguments:**
- /gm vision [on|off] - enables access to select spawns and objects typically restricted
- /gm regiondebug [on|off] - enables region debugging for logging in game as you traverse regions
- /gm sight [on|off] - enables ghost point of view sight into your selected target
- /gm controleffects - lists the control effects on your target
- /gm luadebug [on|off] - turns on lua debug logging to client in game when interacting with spawns/quests.
- /gm tag [faction|spawngroup|race|groundspawn] [value] [icon] - only supported in AoM and "newer" clients when available.  Will tag all npc's meeting the type and value criteria with a tag icon.
- /gm tag clear


Visual Type Options:
type: faction, value: faction id of spawn(s)
type: spawngroup, value: 1 to show grouped, 0 to show not grouped
type: race, value: race id (either against base race or race id in spawn details)
type: groundspawn, value: (not used)

Icons:
1 = skull, 2 = shield half dark blue / half light blue, 3 = purple? star, 4 = yellow sword, 5 = red X
6 = green flame, 7 = Number "1", 8 = Number "2", 9 = Number "3", 10 = Number "4", 11 = Number "5", 12 = Number "6"
25 = shield, 26 = green plus, 27 = crossed swords, 28 = bow with arrow in it, 29 = light blue lightning bolt
30 = bard instrument (hard to see), 31 = writ with shield, 32 = writ with green +, 33 = writ with crossed sword
34 = writ with bow, 35 = writ with light blue lightning bolt, 36 = same as 30, 37 = party with crossed sword, shield and lightning bolt
38 = shaking hands green background, 39 = shaking hands dark green background, unlocked keylock
40 = red aura icon with black shadow of person and big red aura, 41 = green aura icon with black shadow of person big green aura

**Notes:**
- This is a nested command and not easily explained beyond listing the commands above.