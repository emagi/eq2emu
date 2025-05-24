### Function: GetClass(spawn)

**Description:**
Gets the adventure class of the Spawn.

**Parameters:**
- `spawn` (Spawn) - Spawn object representing `spawn`.

**Returns:** UInt32 adventure class id for the Spawn.  See https://github.com/emagi/eq2emu/blob/main/docs/data_types/classes.md for the ID numbers.

**Example:**

```lua
-- From ItemScripts/aQeynosianCommemorativeBundle.lua
function Weapon(Item,Player)
if GetClass(Player)==FIGHTER or GetClass(Player)==WARRIOR or GetClass(Player)==GUARDIAN or GetClass(Player)==BERSERKER then
    SummonItem(Player, 85495,1 )
elseif GetClass(Player)==BRAWLER or GetClass(Player)==MONK  or GetClass(Player)==BRUISER or GetClass(Player)==ANIMALIST or GetClass(Player)==BEASTLORD then
    SummonItem(Player,85483,1)
elseif GetClass(Player)==CRUSADER or GetClass(Player)==SHADOWKNIGHT  or GetClass(Player)==PALADIN then
    SummonItem(Player,85485,1)
    
elseif GetClass(Player)==PRIEST or GetClass(Player)==CLERIC  or GetClass(Player)==TEMPLAR or GetClass(Player)==INQUISITOR or GetClass(Player)==SHAPER or GetClass(Player)==CHANNELER then
    GSummonItem(Player,85484,1)    
elseif GetClass(Player)==DRUID or GetClass(Player)==WARDEN  or GetClass(Player)==FURY then
    SummonItem(Player,85486,1)
elseif GetClass(Player)==SHAMAN or GetClass(Player)==MYSTIC  or GetClass(Player)==DEFILER then
    SummonItem(Player,85492,1)
    
 elseif GetClass(Player)==MAGE or GetClass(Player)==SORCERER  or GetClass(Player)==WIZARD or GetClass(Player)==WARLOCK then
    SummonItem(Player,85493,1)    
elseif GetClass(Player)==ENCHANTER or GetClass(Player)==ILLUSIONIST  or GetClass(Player)==COERCER then
    SummonItem(Player,85487,1)
elseif GetClass(Player)==SUMMONER or GetClass(Player)==CONJUROR  or GetClass(Player)==NECROMANCER then
    SummonItem(Player,85494,1)   
    
 elseif GetClass(Player)==SCOUT or GetClass(Player)==ROGUE  or GetClass(Player)==SWASHBUCKLER or GetClass(Player)==BRIGAND  then
    SummonItem(Player,85491,1)    
elseif GetClass(Player)==BARD or GetClass(Player)==TROUBADOR  or GetClass(Player)==DIRGE then
    SummonItem(Player,85482,1)
elseif GetClass(Player)==RANGER or GetClass(Player)==ASSASSIN  or GetClass(Player)==PREDATOR then 
    SummonItem(Player,85489,1)
end
```
