Function: Harvest(Player, GroundSpawn)

Description: Forces a harvest action on the specified harvestable object or resource node. When called on a harvestable spawn (like a resource node), it attempts to collect from it as if a player harvested it.

Parameters:

    Player: Spawn – The Player to harvest the node.
	GroundSpawn: Spawn - The Spawn that represents the GroundSpawn.

Returns: None (the harvesting results — items or updates — are handled by the system).

Example:

-- Example usage (not commonly used in scripts; simulated harvest of a node)
Harvest(Player, Node)