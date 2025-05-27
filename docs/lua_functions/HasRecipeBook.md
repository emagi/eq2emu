### Function: HasRecipeBook(player, recipe_id)

**Description:**

Return's true if the player has a recipe book that matches the recipe_id.

**Parameters:**
- `player` (Spawn) - Spawn object representing `player`.
- `recipe_id` (uint32) - Integer value `recipe_id`.

**Returns:** True if the player has a recipe book with the recipe of recipe_id included.  Otherwise false.

**Example:**

```lua
-- From SpawnScripts/Generic/GenericCraftingTrainer.lua
function HasBooks(Spawn)
	local has_books = true

	--check if the player has certain recipe books
	if not HasRecipeBook(Spawn, artisan_ess_1) and not HasItem(Spawn, artisan_ess_1, 1) then
		has_books = false
	end
```
