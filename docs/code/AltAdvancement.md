# File: `AltAdvancement.h`

## Classes

- `AltAdvanceData`
- `MasterAAList`
- `TreeNodeData`
- `MasterAANodeList`

## Functions

- `void AddAltAdvancement(AltAdvanceData* data);`
- `int Size();`
- `void DestroyAltAdvancements();`
- `void DisplayAA(Client* client,int8 newtemplate,int8 changemode);`
- `void AddTreeNode(TreeNodeData* data);`
- `int Size();`
- `void DestroyTreeNodes();`

## Notable Comments

- /*
- */
- // defines for AA tabs based on group # from DB
- /// <summary>Sorts the Alternate Advancements for the given client, creates and sends the OP_AdventureList packet.</summary>
- /// <param name='client'>The Client calling this function</param>
- /// <returns>EQ2Packet*</returns>
- /// <summary>Add Alternate Advancement data to the global list.</summary>
- /// <param name='data'>The Alternate Advancement data to add.</param>
- /// <summary>Get the total number of Alternate Advancements in the global list.</summary>
- /// <summary>Get the Alternate Advancement data for the given spell.</summary>
