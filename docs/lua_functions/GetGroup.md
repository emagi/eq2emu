Function: GetGroup(Spawn)

Description: Returns the group object that the given spawn (player or NPC) belongs to. This can be used to iterate over group members or to perform group-wide actions.

Parameters:

    Spawn: Spawn – The entity whose group is requested (typically a Player).

Returns: Group – A group object or identifier that represents the spawn’s group. If the spawn is not in a group, this may return nil or an empty group reference.

Example:

-- Example usage (send a message to all members of a player's group)
local group = GetGroup(Player)
if group ~= nil then
    for _, member in ipairs(GetGroupMembers(group)) do
        SendMessage(member, "A teammate has activated the device!")
    end
end