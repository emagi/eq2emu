### Function: AddConversationOption(ConversationOption, Message, FunctionName)

**Description:**
Builds a conversation option into an existing ConversationOption from CreateConversation.

**Parameters:**
- `ConversationOption`: ConversationOption - Conversation Option object to apply the new text and function call.
- `Message`: string - String value option to display the player when StartConversation is called.
- `FunctionName`: string - Function name that will be called if Player selects the option.

**Returns:** None.

**Notes:**
- Must call CreateConversation() to instantiate the ConversationOption object before AddConversationOption can be used.

**Example:**

```lua
-- Example usage

function hailed(NPC, Spawn)
	FaceTarget(NPC, Spawn)
	conversation = CreateConversation()
	AddConversationOption(conversation, "Yes!", "yes_cookie")
	AddConversationOption(conversation, "No!", "no_cookie")
	StartConversation(conversation, NPC, Spawn, "Greetings traveler, would you like a cookie today?")
end

function yes_cookie(NPC, Spawn)
	-- do whatever you like to give the player a cookie!
end

function no_cookie(NPC, Spawn)
	-- do whatever you like to scorn them for rejecting your cookie!
end
```
