# File: `ChatChannel.h`

## Classes

- `ChatChannel`

## Functions

- `void SetName(const char *name) {strncpy(this->name, name, CHAT_CHANNEL_MAX_NAME);}`
- `void SetPassword(const char *password) {strncpy(this->password, password, CHAT_CHANNEL_MAX_PASSWORD);}`
- `void SetType(ChatChannelType type) {this->type = type;}`
- `void SetLevelRestriction(int16 level_restriction) {this->level_restriction = level_restriction;}`
- `void SetRacesAllowed(int64 races) {this->races = races;}`
- `void SetClassesAllowed(int64 classes) {this->classes = classes;}`
- `ChatChannelType GetType() {return type;}`
- `bool HasPassword() {return password[0] != '\0';}`
- `bool PasswordMatches(const char *password) {return strncmp(this->password, password, CHAT_CHANNEL_MAX_PASSWORD) == 0;}`
- `bool CanJoinChannelByLevel(int16 level) {return level >= level_restriction;}`
- `bool CanJoinChannelByRace(int8 race_id) {return races == 0 || (1 << race_id) & races;}`
- `bool CanJoinChannelByClass(int8 class_id) {return classes == 0 || (1 << class_id) & classes;}`
- `bool IsInChannel(int32 character_id);`
- `bool JoinChannel(Client *client);`
- `bool LeaveChannel(Client *client);`
- `bool TellChannel(Client *client, const char *message, const char* name2 = 0);`
- `bool TellChannelClient(Client* to_client, const char* message, const char* name2 = 0);`
- `bool SendChannelUserList(Client *client);`

## Notable Comments

- /*
- */
