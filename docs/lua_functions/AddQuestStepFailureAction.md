### Function: AddQuestStepFailureAction(Quest, Step, FunctionName)

**Description:** Associates a Lua function (by name) to be called if the specified quest step fails. This allows custom script handling when a step’s failure condition is met (e.g., timer runs out).

**Parameters:**
`Quest`: Quest – The quest object.
`Step`: Int32 – The step number to attach the failure action to.
`FunctionName`: String – The name of a Lua function to call on failure.

**Returns:** None.

**Example:**

```lua
-- Example usage (if step 2 fails, call "OnStealthFail" in the quest script)
AddQuestStepFailureAction(Quest, 2, "OnStealthFail")
```