# File: `BotCommands.cpp`

## Classes

_None detected_

## Functions

- `void Commands::Command_Bot(Client* client, Seperator* sep) {`
- `else if (strncasecmp("attack", sep->arg[0], 6) == 0) {`
- `else if (strncasecmp("spells", sep->arg[0], 6) == 0) {`
- `else if (strncasecmp("maintank", sep->arg[0], 8) == 0) {`
- `else if (strncasecmp("delete", sep->arg[0], 6) == 0) {`
- `else if (strncasecmp("follow", sep->arg[0], 6) == 0) {`
- `else if (strncasecmp("stopfollow", sep->arg[0], 10) == 0) {`
- `else if (strncasecmp("summon", sep->arg[0], 6) == 0) {`
- `else if (strncasecmp("test", sep->arg[0], 4) == 0) {`
- `void Commands::Command_Bot_Create(Client* client, Seperator* sep) {`
- `else if (result == NAMEINVALID_REPLY) {`
- `else if (result == NAMETAKEN_REPLY) {`
- `else if (result == NAMEFILTER_REPLY) {`
- `else if (result == UNKNOWNERROR_REPLY) {`
- `void Commands::Command_Bot_Customize(Client* client, Seperator* sep) {`
- `void Commands::Command_Bot_Spawn(Client* client, Seperator* sep) {`
- `else if (result == 1)`
- `else if (result == 2)`
- `else if (result == 3)`
- `else if (result == 4)`
- `else if (result == 5)`
- `else if (result == 6)`
- `void Commands::Command_Bot_List(Client* client, Seperator* sep) {`
- `void Commands::Command_Bot_Inv(Client* client, Seperator* sep) {`
- `else if (strncasecmp("list", sep->arg[0], 4) == 0) {`
- `else if (strncasecmp("remove", sep->arg[0], 6) == 0) {`
- `void Commands::Command_Bot_Settings(Client* client, Seperator* sep) {`
- `else if (strncasecmp("cloak", sep->arg[0], 5) == 0) {`
- `else if (strncasecmp("taunt", sep->arg[0], 5) == 0) {`
- `else if (strncasecmp("hood", sep->arg[0], 4) == 0) {`
- `void Commands::Command_Bot_Help(Client* client, Seperator* sep) {`
- `else if (strncasecmp("class", sep->arg[0], 5) == 0) {`

## Notable Comments

- //devn00b compile says this is no good, commenting out for now.
- //if(!member)
- //  continue;
- // Check if bot is currently spawned and if so camp it out
- // Check if bot is currently spawned and if so camp it out
- // Check if bot is currently spawned and if so camp it out
- // Add Items
- /*if (bot && bot->GetOwner() == client->GetPlayer()) {
- //\\aITEM %u %u:%s\\/a
