# File: `Chat.h`

## Classes

- `Chat`

## Functions

- `void AddChannel(ChatChannel *channel);`
- `bool ChannelExists(const char *channel_name);`
- `bool HasPassword(const char *channel_name);`
- `bool PasswordMatches(const char *channel_name, const char *password);`
- `bool CreateChannel(const char *channel_name);`
- `bool CreateChannel(const char *channel_name, const char *password);`
- `bool IsInChannel(Client *client, const char *channel_name);`
- `bool JoinChannel(Client *client, const char *channel_name);`
- `bool LeaveChannel(Client *client, const char *channel_name);`
- `bool LeaveAllChannels(Client *client);`
- `bool TellChannel(Client *client, const char *channel_name, const char *message, const char* name = 0);`
- `bool SendChannelUserList(Client *client, const char *channel_name);`
- `int PushDiscordMsg(const char*, const char*);`

## Notable Comments

- /*
- */
- /*
- */
- //devn00b
