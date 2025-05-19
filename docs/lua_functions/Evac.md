Function: Evac(Player, X, Y, Z, Heading)

Description: Evacuates the Player (optional field) or their group (No fields) to a safe spot, typically the zone’s designated evacuation point (e.g., the zone entrance). This mimics the effect of an evac spell.

Parameters:

    Player: Spawn – The player (usually the caster of the evac or the one whose group to evac).  This is optional, if Player is set Evac is self only.  Otherwise Evac() is for a spell script against all spell targets.
    X: Float - Optional X Coordinate (must supply X,Y,Z,Heading).  If for group set Player to nil.
	Y: Float - Optional Y Coordinate (must supply X,Y,Z,Heading).  If for group set Player to nil.
	Z: Float - Optional Z Coordinate (must supply X,Y,Z,Heading).  If for group set Player to nil.
	Heading: Float - Optional Heading Coordinate (must supply X,Y,Z,Heading).  If for group set Player to nil.
	
Returns: None.

Example:

-- In a Spell Script such as Fighter\Crusader\Shadowknight\ShadowyElusion.lua
function cast(Caster, Target)
    Evac()
end

-- Evac just Player
Evac(Player, X, Y, Z, Heading)

-- Evac group to specific coordinates
Evac(nil, X, Y, Z, Heading)
