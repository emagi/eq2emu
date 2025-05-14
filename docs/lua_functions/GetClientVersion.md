Function: GetClientVersion(Spawn)

Description: Retrieves the game client version of the specified player (useful if the server supports multiple client versions). This can determine differences in available features or UI.

Parameters:

    Spawn: Spawn – The player whose client version to get.

Returns: Int32 – The client version number (for example, corresponding to certain expansions or patches).

Example:

-- Example usage (check if player’s client supports a feature)
if GetClientVersion(Player) < REQUIRED_CLIENT_VERSION then
    SendMessage(Player, "Please update your client for the best experience.", "yellow")
end