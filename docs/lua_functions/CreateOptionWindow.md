Function: CreateOptionWindow()

Description: Creates a new custom option selection window. This window can hold multiple options (buttons) that the script can present to the player, usually as part of a custom UI or dialogue.

Parameters: None (after creation, use other functions to add options).

Returns: OptionWindow – A handle or reference to the newly created option window object.

Example:

-- Example usage (creating an option window with choices)
local window = CreateOptionWindow()
AddOptionWindowOption(window, "Accept Quest", "Do you accept the quest?")
AddOptionWindowOption(window, "Decline Quest", "Maybe later.")
SendOptionWindow(window, Player)