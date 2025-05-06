### Command: /bot action [id]

**Handler Macro:** COMMAND_BOT

**Handler Value:** 500

**Required Status:** 0

**Arguments:**
- `arg[0]`: `string action`
- `arg[1]`: `int id`
- Optional arguments for /bot create, `arg[1]` : `int race`, `arg[2]` : `int gender`, `arg[3]` : `int class`, `arg[4]` : `string name`.

**Notes:**
- Action can be camp, attack, spells, maintank, delete, follow, stopfollow, summon, list, customize, spawn, inv, setings, help
- /bot create [race] [gender] [class] [name], creates a new bot with the specified arguments.
- /bot camp, must have bot targetted
- /bot attack, all bots in the group will attack your target
- /bot spells, lists the targetted bots spells
- /bot maintank, sets your target as the main tank for the group of all bot participants you control.
- /bot delete `id`, deletes the bot from the database
- /bot follow `id`, tells the bot by the id to follow you
- /bot stopfollow `id`
- /bot summon group, summons all your bots to your location.

- /bot list, shows all bots you have in the database
- /bot customize, this is not supported
- /bot spawn `id`, spawns the bot with the id specified from /bot list
- /bot inv `[give/list/remove]` - manage bot equipment, for remove a slot must be provide
- /bot settings `[helm/hood/cloak/taunt] [0/1]` - Turn setting on (1) or off(0)
- /bot help