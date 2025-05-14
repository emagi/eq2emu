Function: FlashWindow(Player, WindowName, FlashSeconds)

Description: Causes a UI window to flash (usually its icon or border) on the client to draw attention. For example, flashing the quest journal icon when a new quest is added.

Parameters:

    Player: Spawn – The player whose UI to affect.
    WindowName: String – The window or UI element name to flash.
    FlashSeconds: Float - Flash time of the window in seconds.
	
Returns: None.

Example:

-- Example usage (flash the quest journal when a quest is updated for 2 seconds).
FlashWindow(Player, "MainHUD.StartMenu.quest_journal", 2.0)