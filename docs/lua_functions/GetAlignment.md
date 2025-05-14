Function: GetAlignment(Player)

Description: Returns the alignment of the player character – typically Good, Neutral, or Evil in EQ2. Alignment often affects starting city and some quest options.

Parameters:

    Player: Spawn – The player to query.

Returns: Int32 – An alignment value (e.g., 0=Neutral, 1=Good, 2=Evil as commonly used).

Example:

-- Example usage (greet players differently by alignment)
if GetAlignment(Player) == 1 then
    Say(NPC, "Well met, friend of Qeynos.")
elseif GetAlignment(Player) == 0 then
    Say(NPC, "I smell the stench of Freeport on you.")
elseif GetAlignment(Player) == 2 then
    Say(NPC, "Neutrality you say?")
end