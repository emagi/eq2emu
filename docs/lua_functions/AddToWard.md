Function: AddToWard(AdditionalAmount)

Description: Used inside a Spell Script.  Increases the strength of an existing ward on the target(s) of the spell by the specified amount. If the target(s) has this ward active, this tops it up.

Parameters:

    AdditionalAmount: Int32 – How much to increase the ward’s remaining absorption by.

Returns: None.

Example:

-- Example usage (reinforce a companion’s ward during battle)
AddToWard(200)