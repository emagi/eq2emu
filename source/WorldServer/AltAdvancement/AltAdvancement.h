/*  
EQ2Emulator:  Everquest II Server Emulator
Copyright (C) 2007  EQ2EMulator Development Team (http://www.eq2emulator.net)

This file is part of EQ2Emulator.

EQ2Emulator is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

EQ2Emulator is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with EQ2Emulator.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __AltAdvancement__
#define __AltAdvancement__

#include <vector>
#include "../../common/types.h"
#include "../../common/EQPacket.h"
#include "../client.h"

// defines for AA tabs based on group # from DB
#define AA_CLASS				0
#define AA_SUBCLASS				1
#define AA_SHADOW				2
#define AA_HEROIC				3
#define AA_TRADESKILL			4
#define AA_PRESTIGE				5
#define AA_TRADESKILL_PRESTIGE	6
#define AA_DRAGON				7
#define AA_DRAGONCLASS			8
#define AA_FARSEAS				9
struct AltAdvanceData
{
	int32	spellID;
	int8	min_level;
	int32	spell_crc;
	string	name;
	string	description;
	int8	group;
	int16	icon;
	int16	icon2;
	int8	col;
	int8	row;
	int8	rankCost;
	int8	maxRank;
	int32	rankPrereqID;
	int8	rankPrereq;
	int8	class_req;
	int8	tier;
	int8	req_points;
	int16	req_tree_points;
	string	class_name;
	string	subclass_name;
	string	line_title;
	int8	title_level;
	int32	node_id;
};


class MasterAAList
{
public:
	MasterAAList();
	~MasterAAList();
	/// <summary>Sorts the Alternate Advancements for the given client, creates and sends the OP_AdventureList packet.</summary>
	/// <param name='client'>The Client calling this function</param>
	/// <returns>EQ2Packet*</returns>
	EQ2Packet* GetAAListPacket(Client* client);

	/// <summary>Add Alternate Advancement data to the global list.</summary>
	/// <param name='data'>The Alternate Advancement data to add.</param>
	void AddAltAdvancement(AltAdvanceData* data);

	/// <summary>Get the total number of Alternate Advancements in the global list.</summary>
	int Size();

	/// <summary>Get the Alternate Advancement data for the given spell.</summary>
	/// <param name='spellID'>Spell ID to get Alternate Advancement data for.</param>
	AltAdvanceData* GetAltAdvancement(int32 spellID);

	/// <summary>empties the master Alternate Advancement list</summary>
	void DestroyAltAdvancements();
	void DisplayAA(Client* client,int8 newtemplate,int8 changemode);
private:
	vector <AltAdvanceData*> AAList;
	Mutex MMasterAAList;
};

struct TreeNodeData
{
	int32	classID;
	int32	treeID;
	int32	AAtreeID;
};

class MasterAANodeList
{
public:
	MasterAANodeList();
	~MasterAANodeList();
	void AddTreeNode(TreeNodeData* data);
	int Size();
	void DestroyTreeNodes();
	vector<TreeNodeData*> GetTreeNodes();

private:
	vector<TreeNodeData*> TreeNodeList;
};

#endif