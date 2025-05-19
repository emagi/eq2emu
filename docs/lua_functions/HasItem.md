Function: HasItem(Player, ItemID, IncludeBank)

Description: Determines whether a given player possesses an item with the specified Item ID.

Parameters:

    Player: Spawn – The player or NPC to check for the item. (Typically a Player.)

    ItemID: Int32 – The unique ID of the item to check for.

    IncudeBank: Boolean – If we should check the bank also

Returns: Boolean – true (1) if the spawn’s inventory contains at least one of the specified item; false (0) if the item is not present.

Example:

-- Example usage from: ItemScripts/GeldranisVial.lua
if HasItem(Player, FilledVial) == false then
    SummonItem(Player, FilledVial, 1)
end