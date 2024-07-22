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

#include "AltAdvancement.h"
#include "../../common/ConfigReader.h"
#include "../../common/Log.h"
#include "../Spells.h"
#include "../classes.h"
#include "../Rules/Rules.h"
#include <map>
#include <assert.h>
#include <mysql.h>
#include "../../common/DatabaseNew.h"
#include "../WorldDatabase.h"
extern ConfigReader configReader;
extern MasterSpellList master_spell_list;
extern Classes classes;
extern RuleManager rule_manager;
extern MasterAANodeList master_tree_nodes;

MasterAAList::MasterAAList()
{
	MMasterAAList.SetName("MasterAAList::AAList");
}

MasterAAList::~MasterAAList()
{
	DestroyAltAdvancements();
}

void MasterAAList::AddAltAdvancement(AltAdvanceData* data) {
	MMasterAAList.writelock(__FUNCTION__, __LINE__);
	AAList.push_back(data);
	MMasterAAList.releasewritelock(__FUNCTION__, __LINE__);
}

int MasterAAList::Size() {
	return AAList.size();
}

// Jabantiz: Probably a better way to do this but can't think of it right now
AltAdvanceData* MasterAAList::GetAltAdvancement(int32 spellID) {
	vector<AltAdvanceData*>::iterator itr;
	AltAdvanceData* data = NULL;

	MMasterAAList.readlock(__FUNCTION__, __LINE__);
	for (itr = AAList.begin(); itr != AAList.end(); itr++) {
		if ((*itr)->spellID == spellID) {
			data = (*itr);
			break;
		}
	}
	MMasterAAList.releasereadlock(__FUNCTION__, __LINE__);

	return data;
}

void MasterAAList::DestroyAltAdvancements() {
	MMasterAAList.writelock(__FUNCTION__, __LINE__);
	vector<AltAdvanceData*>::iterator itr;
	for (itr = AAList.begin(); itr != AAList.end(); itr++)
		safe_delete(*itr);
	AAList.clear();
	MMasterAAList.releasewritelock(__FUNCTION__, __LINE__);
}

MasterAANodeList::MasterAANodeList() {
}

MasterAANodeList::~MasterAANodeList() {
	DestroyTreeNodes();
}

void MasterAANodeList::AddTreeNode(TreeNodeData* data) {
	TreeNodeList.push_back(data);
}

void MasterAANodeList::DestroyTreeNodes() {
	vector<TreeNodeData*>::iterator itr;
	for (itr = TreeNodeList.begin(); itr != TreeNodeList.end(); itr++)
		safe_delete(*itr);
	TreeNodeList.clear();
}

int MasterAANodeList::Size() {
	return TreeNodeList.size();
}

vector<TreeNodeData*> MasterAANodeList::GetTreeNodes() {
	return TreeNodeList;
}

EQ2Packet* MasterAAList::GetAAListPacket(Client* client)
{

	/*
	-- OP_DispatchESMsg --
	5/24/2011 20:54:15
	199.108.12.165 -> 192.168.0.197
	0000:	00 38 3B 00 00 00 FF A3 02 FF FF FF FF 00 00 00 .8;.............
	0010:	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ................
	0020:	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ................
	0030:	00 00 00 00 00 00 00 00 FF FF FF FF 00 00 00 00 ................
	0040	00                                              .
	*/

	uchar blah[] = {0xFF,0xE8,0x01,
0x00, //unknown
0x07,0x00,0x00,0x00, //unknown2
0x07,0x00,0x57,0x61,0x72,0x72,0x69,0x6F,0x72, //class_title_tab
0x0C,0x00, //unknown3
0x64,0x00,0x00,0x00, //max_class_aa
0xFD,0x74,0xB6,0x73, //class_id
0x00, //kos_req
0x00,0x00,0x00,0x00, //num_class_items

0x0B,0x00,0x00,0x00, //unknown10
0x11,0x00,0x00,0x00, //class_points_spent
0x00,0x00,0x3B,0x81,0x01,0x00, //unknown11
0x00,0x00, //unknown12
0x00,0x00, //unknown13
0x00,0x00,0x00,0x00, //unknown14
0x00,0x00, //unknown15
0x00,0x00,0x00,0x00,0x00,0x00,0x00, //unknown16
0x09,0x00,0x42,0x65,0x72,0x73,0x65,0x72,0x6B,0x65,0x72, //subclass_title_tab
0x0E,0x00, //unknown17
0x64,0x00,0x00,0x00, //max_subclass_aa
0x5F,0xD6,0xAF,0x50, //subclass_id
0x00, //eof_req
0x00,0x00,0x00,0x00, //num_subclass_items

0x0C,0x00,0x00,0x00, //unknown20
0x08,0x00,0x00,0x00, //subclass_points_spent
0x00,0x00,0x3B,0x81,0x03,0x14, //unknown21
0x00,0x00,0x00, //unknown22
0x1D,0x00,0x3A,0x63,0x65,0x31,0x38,0x36,0x34,0x63,0x37,0x66,0x35,0x33,0x66,0x65,0x62,0x37,0x62,0x5F,0x31,0x3A,0x42,0x65,0x72,0x73,0x65,0x72,0x6B,0x65,0x72, //unknown23
0x01,0x00,0x00,0x00, //unknown24
0x1D,0x00,0x3A,0x63,0x65,0x31,0x38,0x36,0x34,0x63,0x37,0x35,0x66,0x39,0x34,0x61,0x32,0x64,0x37,0x5F,0x31,0x3A,0x45,0x78,0x70,0x65,0x72,0x74,0x69,0x73,0x65, //unknown25
0x00,0x00,0x00,0x00,0x00,0x00, //unknown26
0x07,0x00,0x53,0x68,0x61,0x64,0x6F,0x77,0x73, //shadows_tab_title
0x2C,0x00, //unknown27
0x46,0x00,0x00,0x00, //max_shadows_aa
0x53,0x88,0x59,0x62, //shadows_id
0x00, //rok_req
0x00,0x00,0x00,0x00, //num_shadow_items

0x0E,0x00,0x00,0x00, //unknown30
0x00,0x00,0x00,0x00, //shadows_points_spent
0x00,0x00,0x3B,0x81,0x03,0x00, //unknown31
0x00,0x00,0x00, //unknown32
0x00,0x00, //uknown33
0x00,0x00,0x00,0x00, //unknown34
0x00,0x00, //unknown35
0x00,0x00,0x00,0x00,0x00,0x00, //unknown36
0x06,0x00,0x48,0x65,0x72,0x6F,0x69,0x63, //heroic_tab_title
0x48,0x00, //unknown37
0x32,0x00,0x00,0x00, //max_heroic_aa
0xC0,0x6B,0xFC,0x3C, //heroic_id
0x01, //heroic_dov_req
0x00,0x00,0x00,0x00, //num_heroic_items

0x10,0x00,0x00,0x00, //unknown40
0x00,0x00,0x00,0x00, //heroic_points_spent
0x00,0x00,0x3B,0x81,0x03,0x00, //unknown41
0x00,0x00,0x00, //unknown42
0x00,0x00, //unknown43
0x00,0x00,0x00,0x00, //unknown44
0x00,0x00, //unknown45
0x00,0x00,0x00,0x00,0x00,0x00, //unknown46
0x0A,0x00,0x54,0x72,0x61,0x64,0x65,0x73,0x6B,0x69,0x6C,0x6C, //tradeskill_tab_title
0x49,0x00, //unknown47
0x28,0x00,0x00,0x00, //max_tradeskill_aa
0x1E,0xDB,0x41,0x2F, //tradeskill_id
0x00, //exp_req
0x00,0x00,0x00,0x00, //num_tradeskill_items

0x00,0x00,0x00,0x00, //unknown50
0x00,0x00,0x00,0x00, //tradeskill_points_spent
0x00,0x00,0x3B,0x81,0x03,0x00, //unknown51
0x00,0x00,0x00, //unknown52
0x00,0x00, //unknown53
0x00,0x00,0x00,0x00, //unknown54
0x00,0x00, //unknown55
0x03,0x00,0x00,0x00,0x00,0x00, //unknown56
0x08,0x00,0x50,0x72,0x65,0x73,0x74,0x69,0x67,0x65, //prestige_tab_title
0x67,0x00, //unknown57
0x19,0x00,0x00,0x00, //max_prestige_aa
0xC6,0xA8,0x83,0xBD, //prestige_id
0x01, //prestige_dov_req
0x00,0x00,0x00,0x00, //num_prestige_items

0x10,0x00,0x00,0x00, //unknown60
0x00,0x00,0x00,0x00, //prestige_points_spent
0x00,0x00,0x3B,0x81,0x03,0x06, //unknown61
0x00,0x00,0x00, //unknown62
0x1D,0x00,0x3A,0x34,0x39,0x33,0x64,0x65,0x62,0x62,0x33,0x65,0x36,0x37,0x38,0x62,0x39,0x37,0x37,0x5F,0x35,0x35,0x3A,0x50,0x72,0x65,0x73,0x74,0x69,0x67,0x65, //unknown63
0x01,0x00,0x00,0x00, //unknown64
0x27,0x00,0x3A,0x34,0x39,0x33,0x64,0x65,0x62,0x62,0x33,0x65,0x36,0x61,0x38,0x62,0x62,0x37,0x39,0x5F,0x31,0x32,0x3A,0x50,0x72,0x65,0x73,0x74,0x69,0x67,0x65,0x20,0x45,0x78,0x70,0x65,0x72,0x74,0x69,0x73,0x65, //unknown65
0x02,0x00,0x00,0x00,0x00,0x00, //unknown66
0x13,0x00,0x54,0x72,0x61,0x64,0x65,0x73,0x6B,0x69,0x6C,0x6C,0x20,0x50,0x72,0x65,0x73,0x74,0x69,0x67,0x65, //tradeskill_prestige_tab_title
0x79,0x00, //unknown67
0x19,0x00,0x00,0x00, //max_tradeskill_prestige_aa
0x18,0x2C,0x0B,0x74, //tradeskill_prestige_id
0x01, //coe_req
0x00,0x00,0x00,0x00, //num_tradeskill_prestige_items

0x12,0x00,0x00,0x00, //unknown70
0x00,0x00,0x00,0x00, //tradeskill_prestige_points_spent
0x00,0x00,0x3B,0x81,0x03,0x00, //unknown71
0x00,0x00,0x00, //unknown72
0x00,0x00, //unknown73
0x00,0x00,0x00,0x00, //unknown74
0x00,0x00, //unknown75
0x04,0x00,0x00,0x00,0x00,0x00,0x40,0x00,0x00,0x00, //unknown76
0x00,0x00,0x00,0x01,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //unknown77
0x01, //num_templates
0x64, //template_unknown1
0x03,0x00,0x4E,0x65,0x77, //template_name
0x00, //template_unknown2
0x00,0x00}; //num_tabs

			return (new EQ2Packet(OP_AdventureList, blah, sizeof(blah)));
}



struct AAEntry {
	int8 template_id;
	int8 tab_id;
	int32 aa_id;
	int16	order;
	int8 treeid;
};
void MasterAAList::DisplayAA(Client* client,int8 newtemplate,int8 changemode) {
	map <int8, int32> AAtree_id;
	map <int8, vector<TreeNodeData*> >::iterator itr_tree2;
	vector<TreeNodeData*>::iterator itr_tree3;
	map <int8, vector<TreeNodeData*> > Nodes;
	vector<TreeNodeData*> TreeNodeList = master_tree_nodes.GetTreeNodes();
	if (TreeNodeList.size() == 0)
		return;
	vector<vector<vector<AAEntry> > > AAEntryList ;
	Query query, query2;
	MYSQL_ROW row;
	int32 Pid = client->GetCharacterID();

	AAEntryList.resize(8);   // max number of templates
	for (int i = 0; i < 8; i++) {
		AAEntryList[i].resize(5);  // max number of tabs

	}

	// load templates 1-3 Personal
	
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `template_id`,`tab_id`,`aa_id`,`order`,treeid FROM character_aa_defaults where class = %i order by `order`", client->GetPlayer()->GetAdventureClass());

	while (result && (row = mysql_fetch_row(result))) {
		AAEntry newentry;
		newentry.template_id = strtoul(row[0], NULL, 0);
		newentry.tab_id = strtoul(row[1], NULL, 0);
		newentry.aa_id = strtoul(row[2], NULL, 0);
		newentry.order = strtoul(row[3], NULL, 0);
		newentry.treeid = strtoul(row[4], NULL, 0);
		AAEntryList[newentry.template_id][newentry.tab_id].push_back(newentry);
	}
	LogWrite(SPELL__INFO, 0, "AA", "Loaded %u AA Tree Nodes", AAEntryList.size());
	// load tmplates 4-6 Server
	MYSQL_RES* result2 = query2.RunQuery2(Q_SELECT, "SELECT `template_id`,`tab_id`,`aa_id`,`order`,treeid FROM character_aa where char_id = %i order by `order`", client->GetCharacterID());

	while (result2 && (row = mysql_fetch_row(result2))) {
		AAEntry newentry;
		newentry.template_id = strtoul(row[0], NULL, 0);
		newentry.tab_id = strtoul(row[1], NULL, 0);
		newentry.aa_id = strtoul(row[2], NULL, 0);
		newentry.order = strtoul(row[3], NULL, 0);
		newentry.treeid = strtoul(row[4], NULL, 0);
		AAEntryList[newentry.template_id][newentry.tab_id].push_back(newentry);
	}
	LogWrite(SPELL__INFO, 0, "AA", "Loaded %u AA Tree Nodes", AAEntryList.size());




	for (int x = 0; x < TreeNodeList.size(); x++)
	{
		int8 class_id = client->GetPlayer()->GetInfoStruct()->get_class3();
		
		if (TreeNodeList[x]->classID == class_id)
		{
			itr_tree2 = Nodes.lower_bound(TreeNodeList[x]->classID);
			if (itr_tree2 != Nodes.end() && !(Nodes.key_comp()(TreeNodeList[x]->classID, itr_tree2->first)))
			{
				(itr_tree2->second).push_back(TreeNodeList[x]);
				LogWrite(SPELL__TRACE, 0, "AA", "Added AA Tree node ID: %u", TreeNodeList[x]->treeID);
			}
			else
			{
				vector<TreeNodeData*> tmpVec;
				tmpVec.push_back(TreeNodeList[x]);
				Nodes.insert(make_pair(TreeNodeList[x]->classID, tmpVec));
				LogWrite(SPELL__TRACE, 0, "AA", "Added AA Tree node ID: %u", TreeNodeList[x]->treeID);
			}
		}
	}

	map <int8, vector<AltAdvanceData*> >::iterator itr2;
	vector<AltAdvanceData*>::iterator itr3;

	map <int8, vector<AltAdvanceData*> > ClassTab;
	map <int8, vector<AltAdvanceData*> > SubclassTab;
	map <int8, vector<AltAdvanceData*> > ShadowsTab;
	map <int8, vector<AltAdvanceData*> > HeroicTab;
	map <int8, vector<AltAdvanceData*> > TradeskillTab;
	map <int8, vector<AltAdvanceData*> > PrestigeTab;
	map <int8, vector<AltAdvanceData*> > TradeskillPrestigeTab;
	map <int8, vector<AltAdvanceData*> > DragonTab;
	map <int8, vector<AltAdvanceData*> > DragonclassTab;
	map <int8, vector<AltAdvanceData*> > FarseasTab;

	MMasterAAList.readlock(__FUNCTION__, __LINE__);
	// Get Tree Node ID's
	map <int8, int8> node_id;
	map <int8, int32> classid;
	
	
	for (itr_tree2 = Nodes.begin(); itr_tree2 != Nodes.end(); itr_tree2++) {
		int8 x = 0;
		for (itr_tree3 = itr_tree2->second.begin(); itr_tree3 != itr_tree2->second.end(); itr_tree3++, x++ ) {
			node_id[x] = (*itr_tree3)->treeID;
			classid[(*itr_tree3)->treeID] = (*itr_tree3)->AAtreeID;
		}
	}
	int rrr = 0;
	for (int i =0; i < Size(); i++) {
		if (AAList[i]->group == node_id[AA_CLASS]) {
			itr2 = ClassTab.lower_bound(AAList[i]->group);
			if (itr2 != ClassTab.end() && !(ClassTab.key_comp()(AAList[i]->group, itr2->first))) {
				(itr2->second).push_back(AAList[i]);
				LogWrite(SPELL__TRACE, 0, "AA", "Added...%u ", AAList[i]->spellID);
			}
			else {
				vector<AltAdvanceData*> tmpVec;
				tmpVec.push_back(AAList[i]);
				ClassTab.insert(make_pair(AAList[i]->group, tmpVec));
				LogWrite(SPELL__TRACE, 0, "AA", "Added...%u ", AAList[i]->spellID);
			}
		}
		// Sort for Subclass Tab
		if (AAList[i]->group == node_id[AA_SUBCLASS]) {
			itr2 = SubclassTab.lower_bound(AAList[i]->group);
			if (itr2 != SubclassTab.end() && !(SubclassTab.key_comp()(AAList[i]->group, itr2->first))) {
				(itr2->second).push_back(AAList[i]);
				LogWrite(SPELL__TRACE, 0, "AA", "Added...%u ", AAList[i]->spellID);
			}
			else {
				vector<AltAdvanceData*> tmpVec;
				tmpVec.push_back(AAList[i]);
				SubclassTab.insert(make_pair(AAList[i]->group, tmpVec));
				LogWrite(SPELL__TRACE, 0, "AA", "Added...%u ", AAList[i]->spellID);
			}
		}
		// Sort for Shadows Tab
		if (AAList[i]->group == node_id[AA_SHADOW]) {
			itr2 = ShadowsTab.lower_bound(AAList[i]->group);
			if (itr2 != ShadowsTab.end() && !(ShadowsTab.key_comp()(AAList[i]->group, itr2->first))) {
				(itr2->second).push_back(AAList[i]);
				LogWrite(SPELL__TRACE, 0, "AA", "Added...%u ", AAList[i]->spellID);
			}
			else {
				vector<AltAdvanceData*> tmpVec;
				tmpVec.push_back(AAList[i]);
				ShadowsTab.insert(make_pair(AAList[i]->group, tmpVec));
				LogWrite(SPELL__TRACE, 0, "AA", "Added...%u ", AAList[i]->spellID);
			}
		}
		// Sort for Heroic Tab
		if (AAList[i]->group == node_id[AA_HEROIC]) {
			itr2 = HeroicTab.lower_bound(AAList[i]->group);
			if (itr2 != HeroicTab.end() && !(HeroicTab.key_comp()(AAList[i]->group, itr2->first))) {
				(itr2->second).push_back(AAList[i]);
				LogWrite(SPELL__TRACE, 0, "AA", "Added...%u ", AAList[i]->spellID);
			}
			else {
				vector<AltAdvanceData*> tmpVec;
				tmpVec.push_back(AAList[i]);
				HeroicTab.insert(make_pair(AAList[i]->group, tmpVec));
				LogWrite(SPELL__TRACE, 0, "AA", "Added...%u ", AAList[i]->spellID);
			}
		}
		// Sort for Tradeskill Tab
		if (AAList[i]->group == node_id[AA_TRADESKILL]) {
			itr2 = TradeskillTab.lower_bound(AAList[i]->group);
			if (itr2 != TradeskillTab.end() && !(TradeskillTab.key_comp()(AAList[i]->group, itr2->first))) {
				(itr2->second).push_back(AAList[i]);
				LogWrite(SPELL__TRACE, 0, "AA", "Added...%u ", AAList[i]->spellID);
			}
			else {
				vector<AltAdvanceData*> tmpVec;
				tmpVec.push_back(AAList[i]);
				TradeskillTab.insert(make_pair(AAList[i]->group, tmpVec));
				LogWrite(SPELL__TRACE, 0, "AA", "Added...%u ", AAList[i]->spellID);
			}
		}
		// Sort for Prestige Tab
		if (AAList[i]->group == node_id[AA_PRESTIGE]) {
			itr2 = PrestigeTab.lower_bound(AAList[i]->group);
			if (itr2 != PrestigeTab.end() && !(PrestigeTab.key_comp()(AAList[i]->group, itr2->first))) {
				(itr2->second).push_back(AAList[i]);
				LogWrite(SPELL__TRACE, 0, "AA", "Added...%u ", AAList[i]->spellID);
			}
			else {
				vector<AltAdvanceData*> tmpVec;
				tmpVec.push_back(AAList[i]);
				PrestigeTab.insert(make_pair(AAList[i]->group, tmpVec));
				LogWrite(SPELL__TRACE, 0, "AA", "Added...%u ", AAList[i]->spellID);
			}
		}
		// Sort for TradeskillPrestige Tab
		if (AAList[i]->group == node_id[AA_TRADESKILL_PRESTIGE]) {
			itr2 = TradeskillPrestigeTab.lower_bound(AAList[i]->group);
			if (itr2 != TradeskillPrestigeTab.end() && !(TradeskillPrestigeTab.key_comp()(AAList[i]->group, itr2->first))) {
				(itr2->second).push_back(AAList[i]);
				LogWrite(SPELL__TRACE, 0, "AA", "Added...%u ", AAList[i]->spellID);
			}
			else {
				vector<AltAdvanceData*> tmpVec;
				tmpVec.push_back(AAList[i]);
				TradeskillPrestigeTab.insert(make_pair(AAList[i]->group, tmpVec));
				LogWrite(SPELL__TRACE, 0, "AA", "Added...%u ", AAList[i]->spellID);
			}
		}
		// Sort for Dragon Tab
		if (AAList[i]->group == node_id[AA_DRAGON]) {
			itr2 = DragonTab.lower_bound(AAList[i]->group);
			if (itr2 != DragonTab.end() && !(DragonTab.key_comp()(AAList[i]->group, itr2->first))) {
				(itr2->second).push_back(AAList[i]);
				LogWrite(SPELL__TRACE, 0, "AA", "Added...%u ", AAList[i]->spellID);
			}
			else {
				vector<AltAdvanceData*> tmpVec;
				tmpVec.push_back(AAList[i]);
				DragonTab.insert(make_pair(AAList[i]->group, tmpVec));
				LogWrite(SPELL__TRACE, 0, "AA", "Added...%u ", AAList[i]->spellID);
			}
		}
		// Sort for Dragon Class Tab
		if (AAList[i]->group == node_id[AA_DRAGONCLASS]) {
			itr2 = DragonclassTab.lower_bound(AAList[i]->group);
			if (itr2 != DragonclassTab.end() && !(DragonclassTab.key_comp()(AAList[i]->group, itr2->first))) {
				(itr2->second).push_back(AAList[i]);
				LogWrite(SPELL__TRACE, 0, "AA", "Added...%u ", AAList[i]->spellID);
			}
			else {
				vector<AltAdvanceData*> tmpVec;
				tmpVec.push_back(AAList[i]);
				DragonclassTab.insert(make_pair(AAList[i]->group, tmpVec));
				LogWrite(SPELL__TRACE, 0, "AA", "Added...%u ", AAList[i]->spellID);
			}
		}
		// Sort for Farseas Tab
		if (AAList[i]->group == node_id[AA_FARSEAS]) {
			itr2 = FarseasTab.lower_bound(AAList[i]->group);
			if (itr2 != FarseasTab.end() && !(FarseasTab.key_comp()(AAList[i]->group, itr2->first))) {
				(itr2->second).push_back(AAList[i]);
				LogWrite(SPELL__TRACE, 0, "AA", "Added...%u ", AAList[i]->spellID);
			}
			else {
				vector<AltAdvanceData*> tmpVec;
				tmpVec.push_back(AAList[i]);
				FarseasTab.insert(make_pair(AAList[i]->group, tmpVec));
				LogWrite(SPELL__TRACE, 0, "AA", "Added...%u ", AAList[i]->spellID);
			}
		}
	}
	MMasterAAList.releasereadlock(__FUNCTION__, __LINE__);

	int16	version = 0;
	int8	class_num_items = 0;
	int8	subclass_num_items = 0;
	int8	shadows_num_items = 0;
	int8	heroic_num_items = 0;
	int8	tradeskill_num_items = 0;
	int8	prestige_num_items = 0;
	int8	tradeskillprestige_num_items = 0;
	int8	dragon_num_items = 0;
	int8	dragonclass_num_items = 0;
	int8	farseas_num_items = 0;
	int8	index = 0;
	Spell*	spell = 0;
	int8	current_rank = 0;
	int32	class_node_id = 0;

	if (client)
		version = client->GetVersion();

	
	
	PacketStruct* packet = configReader.getStruct("WS_AdventureList", version);



	if (version >= 58617) {
		packet->setDataByName("num_aa_trees", 10);// number of AA tabs
	}
	else if (version >= 1193) {
		packet->setDataByName("num_aa_trees", 7);// number of AA tabs
	}
	else if (version >= 1096) {
		packet->setDataByName("num_aa_trees", 4);// number of AA tabs
	}
	// since we do not have a proper way of supporting 3 levels of nested arrays the first array is manual here and not looped

	//__________________________________________________________START OF CLASS TREE____________________________________________________________________________________
	// Get the value for num_class_items based on size of ClassTab vector
	for (itr2 = ClassTab.begin(); itr2 != ClassTab.end(); itr2++) {
		class_num_items += (itr2->second).size();
	}
	LogWrite(SPELL__DEBUG, 0, "AA", "ClassTab Size...%i ", class_num_items);
	index = 0;
	packet->setDataByName("class_tab_title",  classes.GetClassNameCase(classes.GetSecondaryBaseClass(client->GetPlayer()->GetAdventureClass())).c_str());
	packet->setDataByName("class_tree_node_id",  node_id[AA_CLASS]);
	packet->setDataByName("class_max_aa",  rule_manager.GetGlobalRule(R_Player, MaxClassAA)->GetInt32());
	int32 class_id = TreeNodeList[node_id[AA_CLASS]]->AAtreeID;
	class_id = classid[node_id[AA_CLASS]];
	packet->setDataByName("class_id", classid[node_id[AA_CLASS]]);
	packet->setDataByName("class_kos_req", 0);
	packet->setArrayLengthByName("class_num_items", class_num_items,0);
	for (itr2 = ClassTab.begin(); itr2 != ClassTab.end(); itr2++) {
		for (itr3 = itr2->second.begin(); itr3 != itr2->second.end(); itr3++, index++) {
			spell = master_spell_list.GetSpell((*itr3)->spellID, (*itr3)->tier);
			current_rank = client->GetPlayer()->GetSpellTier((*itr3)->spellID);
			if (index == 0)
				class_node_id = (*itr3)->spellID;
			//if (spell) {
				packet->setArrayDataByName("class_parent_id", (*itr3)->rankPrereqID, index);
				packet->setArrayDataByName("class_req_tier", (*itr3)->rankPrereq, index);
				packet->setArrayDataByName("class_spell_id", (*itr3)->spellID, index);
				int myrank = (current_rank == 0 ? 0 : (current_rank >= (*itr3)->maxRank) ? 2 : 1);
				packet->setArrayDataByName("class_active", (current_rank == 0 ? 0 : (current_rank >= (*itr3)->maxRank) ? 2 : 1), index);  //1= have tier >= 1;  3 = not available for selection; 0 available for selection
				packet->setArrayDataByName("class_spell_name", (*itr3)->name.c_str(), index);
				packet->setArrayDataByName("class_spell_description", (*itr3)->description.c_str(), index);
				packet->setArrayDataByName("class_icon", (*itr3)->icon, index);
				packet->setArrayDataByName("class_icon2",(*itr3)->icon2, index);
				packet->setArrayDataByName("class_current_rank", current_rank, index); // TODO: need to get this value from the DB
				packet->setArrayDataByName("class_max_rank", (*itr3)->maxRank , index);
				packet->setArrayDataByName("class_rank_cost", (*itr3)->rankCost, index);
				packet->setArrayDataByName("class_min_lev", (*itr3)->min_level, index);
				packet->setSubArrayLengthByName("class_unknown5_numitems", 0,index, 0);
				//packet->setSubArrayDataByName("class_unknown5", 308397057, index, 0);
				//packet->setSubArrayDataByName("class_unknown5", 3215564363, index, 1);
				//packet->setSubArrayDataByName("class_unknown5", 445192837, index, 2);
				//packet->setSubArrayDataByName("class_unknown5", 3345493294, index, 3);
				//packet->setSubArrayDataByName("class_unknown5", 696953971, index, 4);
				packet->setArrayDataByName("class_unknown6", 4294967295, index);
				packet->setArrayDataByName("class_unknown7", 1, index);
				packet->setArrayDataByName("class_classification1", (*itr3)->class_name.c_str(), index);
				packet->setArrayDataByName("class_points_req", (*itr3)->req_points, index);
				packet->setArrayDataByName("class_unknown8", 0, index);
				packet->setArrayDataByName("class_classification2", (*itr3)->subclass_name.c_str(), index);
				packet->setArrayDataByName("class_col", (*itr3)->col, index);
				packet->setArrayDataByName("class_row", (*itr3)->row, index);
				packet->setArrayDataByName("class_line_title", (*itr3)->line_title.c_str(), index);
				packet->setArrayDataByName("class_unknown9", ((*itr3)->title_level > 0 ? 258 : 0 ), index);
				packet->setArrayDataByName("class_points_to_unlock", (*itr3)->req_tree_points, index);
				packet->setArrayDataByName("class_unknown9b", current_rank, index);// aom
			//}
			//else
				//LogWrite(SPELL__ERROR, 0, "AltAdvancement","Could not find Spell ID %u and Tier %i in Master Spell List", (*itr3)->spellID, (*itr3)->tier);
		}
	}
	
	packet->setDataByName("class_points_spent", 11);
	if (version >= 58617) {
		packet->setDataByName("class_unknown10", 11);

		packet->setDataByName("class_unknown11a", 0);
		packet->setDataByName("class_unknown11b", 0);
		packet->setDataByName("class_unknown11c", 1);
	}
	else if (version >= 1193) {
		packet->setDataByName("class_unknown10", 11);
		packet->setDataByName("class_unknown11", 0, 0);
		packet->setDataByName("class_unknown11", 1, 1);
		packet->setDataByName("class_unknown11", 1, 2);
	}
	else if (version >= 1096) {
		packet->setDataByName("class_unknown10", 11);
		packet->setDataByName("class_unknown11", 0, 0);
		packet->setDataByName("class_unknown11", 0, 1);
		packet->setDataByName("class_unknown11", 1, 2);
		packet->setDataByName("class_unknown11", 0, 3);
		packet->setDataByName("class_unknown11", 1, 4);
	}
	else {  // this will change if there is ever a lower client supported
		packet->setDataByName("class_unknown10", 11);
		packet->setDataByName("class_unknown11", 0, 0);
		packet->setDataByName("class_unknown11", 0, 1);
		packet->setDataByName("class_unknown11", 1, 2);
		packet->setDataByName("class_unknown11", 0, 3);
		packet->setDataByName("class_unknown11", 1, 4);
	}





	//__________________________________________________________START OF SUBCLASS TREE____________________________________________________________________________________
	// Get the value for num_class_items based on size of SubclassTab vector
	for (itr2 = SubclassTab.begin(); itr2 != SubclassTab.end(); itr2++) {
		subclass_num_items += (itr2->second).size();
	}
	LogWrite(SPELL__DEBUG, 0, "AA", "SubclassTab Size...%i ", subclass_num_items);
	index = 0;
	packet->setDataByName("subclass_tab_title", classes.GetClassNameCase(client->GetPlayer()->GetAdventureClass()).c_str());
	packet->setDataByName("subclass_tree_node_id",  node_id[AA_SUBCLASS]);
	packet->setDataByName("subclass_max_aa", rule_manager.GetGlobalRule(R_Player, MaxSubclassAA)->GetInt32());
	int32 unknown3 = TreeNodeList[node_id[AA_SUBCLASS]]->AAtreeID;
		
	packet->setDataByName("subclass_id", classid[node_id[AA_SUBCLASS]]);
	packet->setDataByName("subclass_eof_req", 0);
	packet->setArrayLengthByName("subclass_num_items", subclass_num_items, 0);
	for (itr2 = SubclassTab.begin(); itr2 != SubclassTab.end(); itr2++) {
		for (itr3 = itr2->second.begin(); itr3 != itr2->second.end(); itr3++, index++) {
			//spell = master_spell_list.GetSpell((*itr3)->spellID, (*itr3)->tier);
			//current_rank = client->GetPlayer()->GetSpellTier((*itr3)->spellID);
			if (index == 0)
				class_node_id = (*itr3)->spellID;
			//if (spell) {
			packet->setArrayDataByName("subclass_parent_id", (*itr3)->rankPrereqID, index);
			packet->setArrayDataByName("subclass_req_tier", (*itr3)->rankPrereq, index);
			packet->setArrayDataByName("subclass_spell_id", (*itr3)->spellID, index);
			packet->setArrayDataByName("subclass_active", (current_rank == 0 ? 0 : (current_rank >= (*itr3)->maxRank) ? 2 : 1), index);
			packet->setArrayDataByName("subclass_spell_name", (*itr3)->name.c_str(), index);
			packet->setArrayDataByName("subclass_spell_description", (*itr3)->description.c_str(), index);
			packet->setArrayDataByName("subclass_icon", (*itr3)->icon, index);
			packet->setArrayDataByName("subclass_icon2", (*itr3)->icon2, index);
			packet->setArrayDataByName("subclass_current_rank", current_rank, index); // TODO: need to get this value from the DB
			packet->setArrayDataByName("subclass_max_rank", (*itr3)->maxRank, index);
			packet->setArrayDataByName("subclass_rank_cost", (*itr3)->rankCost, index);
			packet->setArrayDataByName("subclass_min_lev", (*itr3)->min_level, index);
			packet->setSubArrayLengthByName("subclass_unknown5_numitems", 0,index,0);
			//packet->setSubArrayDataByName("subclass_unknown5", 308397057, index, 0);
			//packet->setSubArrayDataByName("subclass_unknown5", 3215564363, index, 1);
			//packet->setSubArrayDataByName("subclass_unknown5", 445192837, index, 2);
			//packet->setSubArrayDataByName("subclass_unknown5", 3345493294, index, 3);
			//packet->setSubArrayDataByName("subclass_unknown5", 696953971, index, 4);
			packet->setArrayDataByName("subclass_unknown6", 4294967295, index);
			packet->setArrayDataByName("subclass_unknown7", 1, index);
			packet->setArrayDataByName("subclass_classification1", (*itr3)->class_name.c_str(), index);
			packet->setArrayDataByName("subclass_points_req", (*itr3)->req_points, index);
			packet->setArrayDataByName("subclass_unknown8", 0, index);
			packet->setArrayDataByName("subclass_classification2", (*itr3)->subclass_name.c_str(), index);
			packet->setArrayDataByName("subclass_col", (*itr3)->col, index);
			packet->setArrayDataByName("subclass_row", (*itr3)->row, index);
			packet->setArrayDataByName("subclass_line_title", (*itr3)->line_title.c_str(), index);
			packet->setArrayDataByName("subclass_unknown9", ((*itr3)->title_level > 0 ? 258 : 0), index);
			packet->setArrayDataByName("subclass_points_to_unlock", (*itr3)->req_tree_points, index);
			packet->setArrayDataByName("subclass_unknown9b", 0, index); //added with 68617 AOM something to do with points
			//}
			//else
				//LogWrite(SPELL__ERROR, 0, "AltAdvancement","Could not find Spell ID %u and Tier %i in Master Spell List", (*itr3)->spellID, (*itr3)->tier);
		}
	}
	
	packet->setDataByName("subclass_points_spent", 12);  // to change the 34 to a track value
	
	
	if (version >= 58617) {
		packet->setDataByName("subclass_unknown10", 12);

		packet->setDataByName("subclass_unknown11a", 0);
		packet->setDataByName("subclass_unknown11b", 50386);
		packet->setDataByName("subclass_unknown11c", 5123);

		packet->setDataByName("subclass_unknown12", 0, 0);
		packet->setDataByName("subclass_unknown12", 0, 1);
		packet->setDataByName("subclass_unknown12", 0, 2);
		packet->setDataByName("subclass_unknown13",":493debb3e678b977_91:test_unknown13");// this is based on class
		packet->setDataByName("subclass_unknown14", 1);
		packet->setDataByName("subclass_unknown15", ":ce1864c75f94a2d7_14:Expertise");
		packet->setDataByName("subclass_unknown16", 0, 0);
		packet->setDataByName("subclass_unknown16", 0, 1);
		packet->setDataByName("subclass_unknown16", 0, 2);
		packet->setDataByName("subclass_unknown16", 0, 3);
		packet->setDataByName("subclass_unknown16", 0, 4);
		packet->setDataByName("subclass_unknown16", 0, 5);
	}
	else if (version >= 1193) {
		packet->setDataByName("subclass_unknown10", 12);
		packet->setDataByName("subclass_unknown11", 0, 0);
		packet->setDataByName("subclass_unknown11", 1, 1);
		packet->setDataByName("subclass_unknown11", 5123, 2);
		packet->setDataByName("subclass_unknown12", 0, 0);
		packet->setDataByName("subclass_unknown12", 0, 1);
		packet->setDataByName("subclass_unknown12", 0, 2);
		packet->setDataByName("subclass_unknown13", ":493debb3e678b977_91:test_unknown13");// this is based on class
		packet->setDataByName("subclass_unknown14", 1);
		packet->setDataByName("subclass_unknown15", ":ce1864c75f94a2d7_14:Expertise");
		packet->setDataByName("subclass_unknown16", 111, 0);
		packet->setDataByName("subclass_unknown16", 108, 1);
		packet->setDataByName("subclass_unknown16", 108, 2);
		packet->setDataByName("subclass_unknown16", 1101, 3);
		packet->setDataByName("subclass_unknown16", 121, 4);
		packet->setDataByName("subclass_unknown16", 129, 5);
	}
	else if (version >= 1096) {
		packet->setDataByName("subclass_unknown10", 12);
		packet->setDataByName("subclass_unknown11", 0, 0);
		packet->setDataByName("subclass_unknown11", 0, 1);
		packet->setDataByName("subclass_unknown11", 1, 2);
		packet->setDataByName("subclass_unknown11", 0, 3);
		packet->setDataByName("subclass_unknown11", 3, 4);
	}
	else {  // this will change if there is ever a lower client supported
		packet->setDataByName("subclass_unknown10", 12);
		packet->setDataByName("subclass_unknown11", 0, 0);
		packet->setDataByName("subclass_unknown11", 0, 1);
		packet->setDataByName("subclass_unknown11", 1, 2);
		packet->setDataByName("subclass_unknown11", 0, 3);
		packet->setDataByName("subclass_unknown11", 3, 4);
	}
	//__________________________________________________________START OF SHADOWS TREE____________________________________________________________________________________
	
	for (itr2 = ShadowsTab.begin(); itr2 != ShadowsTab.end(); itr2++) {
		shadows_num_items += (itr2->second).size();
	}
	LogWrite(SPELL__DEBUG, 0, "AA", "ShadowsTab Size...%i ", shadows_num_items);
	index = 0;
	packet->setDataByName("shadows_tab_title", "Shadows");
	packet->setDataByName("shadows_tree_node_id", node_id[AA_SHADOW]);
	packet->setDataByName("shadows_max_aa", rule_manager.GetGlobalRule(R_Player, MaxShadowsAA)->GetInt32());
	packet->setDataByName("shadows_id", classid[node_id[AA_SHADOW]]);
	packet->setDataByName("shadows_eof_req", 0);
	packet->setArrayLengthByName("shadows_num_items", shadows_num_items, 0);
	//packet->PrintPacket();
	for (itr2 = ShadowsTab.begin(); itr2 != ShadowsTab.end(); itr2++) {
		for (itr3 = itr2->second.begin(); itr3 != itr2->second.end(); itr3++, index++) {
			spell = master_spell_list.GetSpell((*itr3)->spellID, (*itr3)->tier);
			current_rank = client->GetPlayer()->GetSpellTier((*itr3)->spellID);
			if (index == 0)
				class_node_id = (*itr3)->spellID;
			//if (spell) {
			packet->setArrayDataByName("shadows_parent_id", (*itr3)->rankPrereqID, index);
			packet->setArrayDataByName("shadows_req_tier", (*itr3)->rankPrereq, index);
			packet->setArrayDataByName("shadows_spell_id", (*itr3)->spellID, index);
			packet->setArrayDataByName("shadows_active", (current_rank == 0 ? 0 : (current_rank >= (*itr3)->maxRank) ? 2 : 1), index);
			packet->setArrayDataByName("shadows_spell_name", (*itr3)->name.c_str(), index);
			packet->setArrayDataByName("shadows_spell_description", (*itr3)->description.c_str(), index);
			packet->setArrayDataByName("shadows_icon", (*itr3)->icon, index);
			packet->setArrayDataByName("shadows_icon2", (*itr3)->icon2, index);
			packet->setArrayDataByName("shadows_current_rank", current_rank, index); // TODO: need to get this value from the DB
			packet->setArrayDataByName("shadows_max_rank", (*itr3)->maxRank, index);
			packet->setArrayDataByName("shadows_rank_cost", (*itr3)->rankCost, index);
			packet->setArrayDataByName("shadows_min_lev", (*itr3)->min_level, index);
			packet->setSubArrayLengthByName("shadows_unknown5_num_items", 0, index, 0);
			//packet->setSubArrayDataByName("shadows_unknown5", 308397057, index, 0);
			//packet->setSubArrayDataByName("shadows_unknown5", 3215564363, index, 1);
			//packet->setSubArrayDataByName("shadows_unknown5", 445192837, index, 2);
			//packet->setSubArrayDataByName("shadows_unknown5", 3345493294, index, 3);
			//packet->setSubArrayDataByName("shadows_unknown5", 696953971, index, 4);
			packet->setArrayDataByName("shadows_unknown6", 4294967295, index);
			packet->setArrayDataByName("shadows_unknown7", 1, index);
			packet->setArrayDataByName("shadows_classification1", (*itr3)->class_name.c_str(), index);
			packet->setArrayDataByName("shadows_points_req", (*itr3)->req_points, index);
			packet->setArrayDataByName("shadows_unknown8", 0, index);
			packet->setArrayDataByName("shadows_classification2", (*itr3)->subclass_name.c_str(), index);
			packet->setArrayDataByName("shadows_col", (*itr3)->col, index);
			packet->setArrayDataByName("shadows_row", (*itr3)->row, index);
			packet->setArrayDataByName("shadows_line_title", (*itr3)->line_title.c_str(), index);
			packet->setArrayDataByName("shadows_unknown9", ((*itr3)->title_level > 0 ? 258 : 0), index);
			packet->setArrayDataByName("shadows_points_to_unlock", (*itr3)->req_tree_points, index);
			packet->setArrayDataByName("shadows_unknown9b", 0, index);
			//}
			//else
				//LogWrite(SPELL__ERROR, 0, "AltAdvancement","Could not find Spell ID %u and Tier %i in Master Spell List", (*itr3)->spellID, (*itr3)->tier);
		}
	}
	
	packet->setDataByName("shadows_points_spent", 14);


	if (version >= 58617) {
		packet->setDataByName("shadows_unknown10", 14);

		packet->setDataByName("shadows_unknown11a", 0);
		packet->setDataByName("shadows_unknown11b", 50386);
		packet->setDataByName("shadows_unknown11c", 3);

		packet->setDataByName("shadows_unknown12", 0, 0);
		packet->setDataByName("shadows_unknown12", 0, 1);
		packet->setDataByName("shadows_unknown12", 0, 2);
	}
	else if (version >= 1193) {
		packet->setDataByName("shadows_unknown10", 14);
		packet->setDataByName("shadows_unknown11", 0, 0);
		packet->setDataByName("shadows_unknown11", 1, 1);
		packet->setDataByName("shadows_unknown11", 3, 2);
		packet->setDataByName("shadows_unknown12", 0, 0);
		packet->setDataByName("shadows_unknown12", 0, 1);
		packet->setDataByName("shadows_unknown12", 0, 2);
	}
	else if (version >= 1096) {
		packet->setDataByName("shadows_unknown11", 0, 0);
		packet->setDataByName("shadows_unknown11", 0, 1);
		packet->setDataByName("shadows_unknown11", 1, 2);
		packet->setDataByName("shadows_unknown11", 0, 3);
		packet->setDataByName("shadows_unknown11", 3, 4);
	}
	else {  // this will change if there is ever a lower client supported
		packet->setDataByName("shadows_unknown11", 0, 0);
		packet->setDataByName("shadows_unknown11", 0, 1);
		packet->setDataByName("shadows_unknown11", 1, 2);
		packet->setDataByName("shadows_unknown11", 0, 3);
		packet->setDataByName("shadows_unknown11", 3, 4);
		packet->setDataByName("shadows_unknown12", 103, 0);
		packet->setDataByName("shadows_unknown12", 101, 1);
		packet->setDataByName("shadows_unknown12", 114, 2);
		packet->setDataByName("shadows_unknown14", 1835365408);
		packet->setDataByName("shadows_unknown16", 114, 0);
		packet->setDataByName("shadows_unknown16", 97, 1);
		packet->setDataByName("shadows_unknown16", 114, 2);
		packet->setDataByName("shadows_unknown16", 121, 3);
		packet->setDataByName("shadows_unknown16", 32, 4);
		packet->setDataByName("shadows_unknown16", 98, 5);
	}
	//__________________________________________________________START OF HEROIC TREE____________________________________________________________________________________
	for (itr2 = HeroicTab.begin(); itr2 != HeroicTab.end(); itr2++) {
		heroic_num_items += (itr2->second).size();
	}
	LogWrite(SPELL__DEBUG, 0, "AA", "HeroicTab Size...%i ", heroic_num_items);
	index = 0; 
	packet->setDataByName("heroic_tab_title", "Heroic");
	packet->setDataByName("heroic_tree_node_id", node_id[AA_HEROIC]);
	packet->setDataByName("heroic_max_aa", rule_manager.GetGlobalRule(R_Player, MaxHeroicAA)->GetInt32());
	packet->setDataByName("heroic_id", classid[node_id[AA_HEROIC]]);
	packet->setDataByName("heroic_eof_req", 0);
	packet->setArrayLengthByName("heroic_num_items", heroic_num_items, 0);
	for (itr2 = HeroicTab.begin(); itr2 != HeroicTab.end(); itr2++) {
		for (itr3 = itr2->second.begin(); itr3 != itr2->second.end(); itr3++, index++) {
			//spell = master_spell_list.GetSpell((*itr3)->spellID, (*itr3)->tier);
			//current_rank = client->GetPlayer()->GetSpellTier((*itr3)->spellID);
			if (index == 0)
				class_node_id = (*itr3)->spellID;
			//if (spell) {
			packet->setArrayDataByName("heroic_parent_id", (*itr3)->rankPrereqID, index);
			packet->setArrayDataByName("heroic_req_tier", (*itr3)->rankPrereq, index);
			packet->setArrayDataByName("heroic_spell_id", (*itr3)->spellID, index);
			packet->setArrayDataByName("heroic_active", (current_rank == 0 ? 0 : (current_rank >= (*itr3)->maxRank) ? 2 : 1), index);
			packet->setArrayDataByName("heroic_spell_name", (*itr3)->name.c_str(), index);
			packet->setArrayDataByName("heroic_spell_description", (*itr3)->description.c_str(), index);
			packet->setArrayDataByName("heroic_icon", (*itr3)->icon, index);
			packet->setArrayDataByName("heroic_icon2", (*itr3)->icon2, index);
			packet->setArrayDataByName("heroic_current_rank", current_rank, index); // TODO: need to get this value from the DB
			packet->setArrayDataByName("heroic_max_rank", (*itr3)->maxRank, index);
			packet->setArrayDataByName("heroic_rank_cost", (*itr3)->rankCost, index);
			packet->setArrayDataByName("heroic_min_lev", (*itr3)->min_level, index);
			packet->setSubArrayLengthByName("heroic_unknown5_num_items", 0, index, 0);
			//packet->setSubArrayDataByName("heroic_unknown5", 308397057, index, 0);
			//packet->setSubArrayDataByName("heroic_unknown5", 3215564363, index, 1);
			//packet->setSubArrayDataByName("heroic_unknown5", 445192837, index, 2);
			//packet->setSubArrayDataByName("heroic_unknown5", 3345493294, index, 3);
			//packet->setSubArrayDataByName("heroic_unknown5", 696953971, index, 4);
			packet->setArrayDataByName("heroic_unknown6", 4294967295, index);
			packet->setArrayDataByName("heroic_unknown7", 1, index);
			packet->setArrayDataByName("heroic_classification1", (*itr3)->class_name.c_str(), index);
			packet->setArrayDataByName("heroic_points_req", (*itr3)->req_points, index);
			packet->setArrayDataByName("heroic_unknown8", 0, index);
			packet->setArrayDataByName("heroic_classification2", (*itr3)->subclass_name.c_str(), index);
			packet->setArrayDataByName("heroic_col", (*itr3)->col, index);
			packet->setArrayDataByName("heroic_row", (*itr3)->row, index);
			packet->setArrayDataByName("heroic_line_title", (*itr3)->line_title.c_str(), index);
			packet->setArrayDataByName("heroic_unknown9", ((*itr3)->title_level > 0 ? 258 : 0), index);
			packet->setArrayDataByName("heroic_points_to_unlock", (*itr3)->req_tree_points, index);
			packet->setArrayDataByName("heroic_unknown9b", 0, index);
			//}
			//else
				//LogWrite(SPELL__ERROR, 0, "AltAdvancement","Could not find Spell ID %u and Tier %i in Master Spell List", (*itr3)->spellID, (*itr3)->tier);
		}
	}
	
	packet->setDataByName("heroic_points_spent", 16);

	if (version >= 58617) {
		packet->setDataByName("heroic_unknown10", 16);

		packet->setDataByName("heroic_unknown11a", 0);
		packet->setDataByName("heroic_unknown11b", 50386);
		packet->setDataByName("heroic_unknown11c", 3);

		packet->setDataByName("heroic_unknown12", 0, 0);
		packet->setDataByName("heroic_unknown12", 0, 1);
		packet->setDataByName("heroic_unknown12", 0, 2);

		packet->setDataByName("heroic_unknown14", 0);
		packet->setDataByName("heroic_unknown16", 39, 0);
		packet->setDataByName("heroic_unknown16", 115, 1);
		packet->setDataByName("heroic_unknown16", 32, 2);
		packet->setDataByName("heroic_unknown16", 115, 3);
		packet->setDataByName("heroic_unknown16", 101, 4);
		packet->setDataByName("heroic_unknown16", 108, 5);
	}
	else if (version >= 1193) {
		packet->setDataByName("heroic_unknown10", 16);
		packet->setDataByName("heroic_unknown11", 0, 0);
		packet->setDataByName("heroic_unknown11", 1, 1);
		packet->setDataByName("heroic_unknown11", 3, 2);
		packet->setDataByName("heroic_unknown12", 0, 0);
		packet->setDataByName("heroic_unknown12", 0, 1);
		packet->setDataByName("heroic_unknown12", 0, 2);
		packet->setDataByName("heroic_unknown14", 0);
		packet->setDataByName("heroic_unknown16", 0, 0);
		packet->setDataByName("heroic_unknown16", 0, 1);
		packet->setDataByName("heroic_unknown16", 0, 2);
		packet->setDataByName("heroic_unknown16", 0, 3);
		packet->setDataByName("heroic_unknown16", 0, 4);
		packet->setDataByName("heroic_unknown16", 0, 5);
	}
	else if (version >= 1096) {
		packet->setDataByName("heroic_unknown11", 0, 0);
		packet->setDataByName("heroic_unknown11", 0, 1);
		packet->setDataByName("heroic_unknown11", 1, 2);
		packet->setDataByName("heroic_unknown11", 0, 3);
		packet->setDataByName("heroic_unknown11", 3, 4);
	}
	else {  // this will change if there is ever a lower client supported
		packet->setDataByName("heroic_unknown11", 0, 0);
		packet->setDataByName("heroic_unknown11", 0, 1);
		packet->setDataByName("heroic_unknown11", 1, 2);
		packet->setDataByName("heroic_unknown11", 0, 3);
		packet->setDataByName("heroic_unknown11", 3, 4);
	}
	if (version >= 1193) {

		//__________________________________________________________START OF TRADESKILL TREE____________________________________________________________________________________
		for (itr2 = TradeskillTab.begin(); itr2 != TradeskillTab.end(); itr2++) {
			tradeskill_num_items += (itr2->second).size();
		}
		LogWrite(SPELL__DEBUG, 0, "AA", "TradeskillTab Size...%i ", tradeskill_num_items);
		index = 0;
		packet->setDataByName("tradeskill_tab_title", "Tradeskill");
		packet->setDataByName("tradeskill_tree_node_id", node_id[AA_TRADESKILL]);
		packet->setDataByName("tradeskill_max_aa", rule_manager.GetGlobalRule(R_Player, MaxTradeskillAA)->GetInt32());
		packet->setDataByName("tradeskill_id", classid[node_id[AA_TRADESKILL]]);
		packet->setDataByName("tradeskill_eof_req", 0);
		packet->setArrayLengthByName("tradeskill_num_items", tradeskill_num_items, 0);
		for (itr2 = TradeskillTab.begin(); itr2 != TradeskillTab.end(); itr2++) {
			for (itr3 = itr2->second.begin(); itr3 != itr2->second.end(); itr3++, index++) {
				//spell = master_spell_list.GetSpell((*itr3)->spellID, (*itr3)->tier);
				//current_rank = client->GetPlayer()->GetSpellTier((*itr3)->spellID);
				if (index == 0)
					class_node_id = (*itr3)->spellID;
				//if (spell) {
				packet->setArrayDataByName("tradeskill_parent_id", (*itr3)->rankPrereqID, index);
				packet->setArrayDataByName("tradeskill_req_tier", (*itr3)->rankPrereq, index);
				packet->setArrayDataByName("tradeskill_spell_id", (*itr3)->spellID, index);
				packet->setArrayDataByName("tradeskill_active", (current_rank == 0 ? 0 : (current_rank >= (*itr3)->maxRank) ? 2 : 1), index);
				packet->setArrayDataByName("tradeskill_spell_name", (*itr3)->name.c_str(), index);
				packet->setArrayDataByName("tradeskill_spell_description", (*itr3)->description.c_str(), index);
				packet->setArrayDataByName("tradeskill_icon", (*itr3)->icon, index);
				packet->setArrayDataByName("tradeskill_icon2", (*itr3)->icon2, index);
				packet->setArrayDataByName("tradeskill_current_rank", current_rank, index); // TODO: need to get this value from the DB
				packet->setArrayDataByName("tradeskill_max_rank", (*itr3)->maxRank, index);
				packet->setArrayDataByName("tradeskill_rank_cost", (*itr3)->rankCost, index);
				packet->setArrayDataByName("tradeskill_min_lev", (*itr3)->min_level, index);
				packet->setSubArrayLengthByName("tradeskill_unknown5_num_items", 0, index, 0);
				//packet->setSubArrayDataByName("tradeskill_unknown5", 308397057, index, 0);
				//packet->setSubArrayDataByName("tradeskill_unknown5", 3215564363, index, 1);
				//packet->setSubArrayDataByName("tradeskill_unknown5", 445192837, index, 2);
				//packet->setSubArrayDataByName("tradeskill_unknown5", 3345493294, index, 3);
				//packet->setSubArrayDataByName("tradeskill_unknown5", 696953971, index, 4);
				packet->setArrayDataByName("tradeskill_unknown6", 4294967295, index);
				packet->setArrayDataByName("tradeskill_unknown7", 0, index);
				packet->setArrayDataByName("tradeskill_classification1", (*itr3)->class_name.c_str(), index);
				packet->setArrayDataByName("tradeskill_points_req", (*itr3)->req_points, index);
				packet->setArrayDataByName("tradeskill_unknown8", 0, index);
				packet->setArrayDataByName("tradeskill_classification2", (*itr3)->subclass_name.c_str(), index);
				packet->setArrayDataByName("tradeskill_col", (*itr3)->col, index);
				packet->setArrayDataByName("tradeskill_row", (*itr3)->row, index);
				packet->setArrayDataByName("tradeskill_line_title", (*itr3)->line_title.c_str(), index);
				packet->setArrayDataByName("tradeskill_unknown9", ((*itr3)->title_level > 0 ? 258 : 0), index);
				packet->setArrayDataByName("tradeskill_points_to_unlock", (*itr3)->req_tree_points, index);
				packet->setArrayDataByName("tradeskill_unknown9b", 0, index);
				//}
				//else
					//LogWrite(SPELL__ERROR, 0, "AltAdvancement","Could not find Spell ID %u and Tier %i in Master Spell List", (*itr3)->spellID, (*itr3)->tier);
			}
		}
		packet->setDataByName("tradeskill_unknown10", 0);
		packet->setDataByName("tradeskill_points_spent", 22);
		if (version >= 58617) {
			packet->setDataByName("tradeskill_unknown10", 11);

			packet->setDataByName("tradeskill_unknown11a", 0);
			packet->setDataByName("tradeskill_unknown11b", 50386);
			packet->setDataByName("tradeskill_unknown11c", 3);

			packet->setDataByName("tradeskill_unknown12", 0, 0);
			packet->setDataByName("tradeskill_unknown12", 0, 1);
			packet->setDataByName("tradeskill_unknown12", 0, 2);

			packet->setDataByName("tradeskill_unknown14", 0);
			packet->setDataByName("tradeskill_unknown16", 3, 0);
			packet->setDataByName("tradeskill_unknown16", 0, 1);
			packet->setDataByName("tradeskill_unknown16", 0, 2);
			packet->setDataByName("tradeskill_unknown16", 0, 3);
			packet->setDataByName("tradeskill_unknown16", 0, 4);
			packet->setDataByName("tradeskill_unknown16", 0, 5);
		}
		else if (version >= 1193) {
			packet->setDataByName("tradeskill_unknown10", 0);
			packet->setDataByName("tradeskill_unknown11", 0, 0);
			packet->setDataByName("tradeskill_unknown11", 1, 1);
			packet->setDataByName("tradeskill_unknown11", 3, 2);
			packet->setDataByName("tradeskill_unknown12", 0, 0);
			packet->setDataByName("tradeskill_unknown12", 0, 1);
			packet->setDataByName("tradeskill_unknown12", 0, 2);
			packet->setDataByName("tradeskill_unknown14", 0);
			packet->setDataByName("tradeskill_unknown16", 3, 0);
			packet->setDataByName("tradeskill_unknown16", 0, 1);
			packet->setDataByName("tradeskill_unknown16", 0, 2);
			packet->setDataByName("tradeskill_unknown16", 0, 3);
			packet->setDataByName("tradeskill_unknown16", 0, 4);
			packet->setDataByName("tradeskill_unknown16", 0, 5);
		}
		
		//__________________________________________________________START OF PRESTIGE TREE____________________________________________________________________________________
		for (itr2 = PrestigeTab.begin(); itr2 != PrestigeTab.end(); itr2++) {
			prestige_num_items += (itr2->second).size();
		}
		LogWrite(SPELL__DEBUG, 0, "AA", "PrestigeTab Size...%i ", prestige_num_items);
		index = 0;
		packet->setDataByName("prestige_tab_title", "Prestige");
		packet->setDataByName("prestige_tree_node_id", node_id[AA_PRESTIGE]);
		packet->setDataByName("prestige_max_aa", rule_manager.GetGlobalRule(R_Player, MaxPrestigeAA)->GetInt32());
		packet->setDataByName("prestige_id", classid[node_id[AA_PRESTIGE]]);
		packet->setDataByName("prestige_eof_req", 0);
		packet->setArrayLengthByName("prestige_num_items", prestige_num_items, 0);
		for (itr2 = PrestigeTab.begin(); itr2 != PrestigeTab.end(); itr2++) {
			for (itr3 = itr2->second.begin(); itr3 != itr2->second.end(); itr3++, index++) {
				//spell = master_spell_list.GetSpell((*itr3)->spellID, (*itr3)->tier);
				//current_rank = client->GetPlayer()->GetSpellTier((*itr3)->spellID);
				if (index == 0)
					class_node_id = (*itr3)->spellID;
				//if (spell) {
				packet->setArrayDataByName("prestige_parent_id", (*itr3)->rankPrereqID, index);
				packet->setArrayDataByName("prestige_req_tier", (*itr3)->rankPrereq, index);
				packet->setArrayDataByName("prestige_spell_id", (*itr3)->spellID, index);
				packet->setArrayDataByName("prestige_active", (current_rank == 0 ? 0 : (current_rank >= (*itr3)->maxRank) ? 2 : 1), index);
				packet->setArrayDataByName("prestige_spell_name", (*itr3)->name.c_str(), index);
				packet->setArrayDataByName("prestige_spell_description", (*itr3)->description.c_str(), index);
				packet->setArrayDataByName("prestige_icon", (*itr3)->icon, index);
				packet->setArrayDataByName("prestige_icon2", (*itr3)->icon2, index);
				packet->setArrayDataByName("prestige_current_rank", current_rank, index); // TODO: need to get this value from the DB
				packet->setArrayDataByName("prestige_max_rank", (*itr3)->maxRank, index);
				packet->setArrayDataByName("prestige_rank_cost", (*itr3)->rankCost, index);
				packet->setArrayDataByName("prestige_min_lev", (*itr3)->min_level, index);
				packet->setSubArrayLengthByName("prestige_unknown5_num_items", 0, index, 0);
				//packet->setSubArrayDataByName("prestige_unknown5", 308397057, index, 0);
				//packet->setSubArrayDataByName("prestige_unknown5", 3215564363, index, 1);
				//packet->setSubArrayDataByName("prestige_unknown5", 445192837, index, 2);
				//packet->setSubArrayDataByName("prestige_unknown5", 3345493294, index, 3);
				//packet->setSubArrayDataByName("prestige_unknown5", 696953971, index, 4);
				packet->setArrayDataByName("prestige_unknown6", 4294967295, index);
				packet->setArrayDataByName("prestige_unknown7", 1, index);
				packet->setArrayDataByName("prestige_classification1", (*itr3)->class_name.c_str(), index);
				packet->setArrayDataByName("prestige_points_req", (*itr3)->req_points, index);
				packet->setArrayDataByName("prestige_unknown8", 0, index);
				packet->setArrayDataByName("prestige_classification2", (*itr3)->subclass_name.c_str(), index);
				packet->setArrayDataByName("prestige_col", (*itr3)->col, index);
				packet->setArrayDataByName("prestige_row", (*itr3)->row, index);
				packet->setArrayDataByName("prestige_line_title", (*itr3)->line_title.c_str(), index);
				packet->setArrayDataByName("prestige_unknown9", ((*itr3)->title_level > 0 ? 258 : 0), index);
				packet->setArrayDataByName("prestige_points_to_unlock", (*itr3)->req_tree_points, index);
				packet->setArrayDataByName("prestige_unknown9b", 0, index);

				//}
				//else
					//LogWrite(SPELL__ERROR, 0, "AltAdvancement","Could not find Spell ID %u and Tier %i in Master Spell List", (*itr3)->spellID, (*itr3)->tier);
			}
		}
		
		packet->setDataByName("prestige_points_spent", 34);

		if (version >= 58617) {
			packet->setDataByName("prestige_unknown10", 16);

			packet->setDataByName("prestige_unknown11a", 0);
			packet->setDataByName("prestige_unknown11b", 50386);
			packet->setDataByName("prestige_unknown11c", 1539);

			packet->setDataByName("prestige_unknown12", 0, 0);
			packet->setDataByName("prestige_unknown12", 0, 1);
			packet->setDataByName("prestige_unknown12", 0, 2);

			packet->setDataByName("prestige_unknown13", ":493debb3e678b977_91:Prestige");
			packet->setDataByName("prestige_unknown14", 1);
			packet->setDataByName("prestige_unknown15", ":493debb3e6a8bb79_20:Prestige Expertise");
			packet->setDataByName("prestige_unknown16", 2, 0);
			packet->setDataByName("prestige_unknown16", 0, 1);
			packet->setDataByName("prestige_unknown16", 0, 2);
			packet->setDataByName("prestige_unknown16", 0, 3);
			packet->setDataByName("prestige_unknown16", 0, 4);
			packet->setDataByName("prestige_unknown16", 0, 5);
		}
		else if (version >= 1193) {
			packet->setDataByName("prestige_unknown10", 16);
			packet->setDataByName("prestige_unknown11", 0, 0);
			packet->setDataByName("prestige_unknown11", 1, 1);
			packet->setDataByName("prestige_unknown11", 1539, 2);
			packet->setDataByName("prestige_unknown12", 0, 0);
			packet->setDataByName("prestige_unknown12", 0, 1);
			packet->setDataByName("prestige_unknown12", 0, 2);
			packet->setDataByName("prestige_unknown13", ":493debb3e678b977_91:Prestige");
			packet->setDataByName("prestige_unknown14", 1);
			packet->setDataByName("prestige_unknown15", ":493debb3e6a8bb79_20:Prestige Expertise");
			packet->setDataByName("prestige_unknown16", 2, 0);
			packet->setDataByName("prestige_unknown16", 0, 1);
			packet->setDataByName("prestige_unknown16", 0, 2);
			packet->setDataByName("prestige_unknown16", 0, 3);
			packet->setDataByName("prestige_unknown16", 0, 4);
			packet->setDataByName("prestige_unknown16", 0, 5);
		}
		
		//__________________________________________________________START OF TRADESKILL PRESTIGE TREE____________________________________________________________________________________
		for (itr2 = TradeskillPrestigeTab.begin(); itr2 != TradeskillPrestigeTab.end(); itr2++) {
			tradeskillprestige_num_items += (itr2->second).size();
		}
		LogWrite(SPELL__DEBUG, 0, "AA", "TradeskillPrestigeTab Size...%i ", tradeskillprestige_num_items);
		index = 0;
		if (version >= 58617) {
			packet->setDataByName("tradeskillprestige_tab_title", "General");
		}
		else {
			packet->setDataByName("tradeskillprestige_tab_title", "Tradeskill Prestige");
		}
		packet->setDataByName("tradeskillprestige_tree_node_id",  node_id[AA_TRADESKILL_PRESTIGE]);
		packet->setDataByName("tradeskillprestige_max_aa", rule_manager.GetGlobalRule(R_Player, MaxPrestigeAA)->GetInt32());
		packet->setDataByName("tradeskillprestige_id", classid[node_id[AA_TRADESKILL_PRESTIGE]]);
		packet->setDataByName("tradeskillprestige_eof_req", 0);
		packet->setArrayLengthByName("tradeskillprestige_num_items", tradeskillprestige_num_items, 0);
		for (itr2 = PrestigeTab.begin(); itr2 != PrestigeTab.end(); itr2++) {
			for (itr3 = itr2->second.begin(); itr3 != itr2->second.end(); itr3++, index++) {
				//spell = master_spell_list.GetSpell((*itr3)->spellID, (*itr3)->tier);
				//current_rank = client->GetPlayer()->GetSpellTier((*itr3)->spellID);
				if (index == 0)
					class_node_id = (*itr3)->spellID;
				//if (spell) {
				packet->setArrayDataByName("tradeskillprestige_parent_id", (*itr3)->rankPrereqID, index);
				packet->setArrayDataByName("tradeskillprestige_req_tier", (*itr3)->rankPrereq, index);
				packet->setArrayDataByName("tradeskillprestige_spell_id", (*itr3)->spellID, index);
				packet->setArrayDataByName("tradeskillprestige_active", (current_rank == 0 ? 0 : (current_rank >= (*itr3)->maxRank) ? 2 : 1), index);
				packet->setArrayDataByName("tradeskillprestige_spell_name", (*itr3)->name.c_str(), index);
				packet->setArrayDataByName("tradeskillprestige_spell_description", (*itr3)->description.c_str(), index);
				packet->setArrayDataByName("tradeskillprestige_icon", (*itr3)->icon, index);
				packet->setArrayDataByName("tradeskillprestige_icon2", (*itr3)->icon2, index);
				packet->setArrayDataByName("tradeskillprestige_current_rank", current_rank, index); // TODO: need to get this value from the DB
				packet->setArrayDataByName("tradeskillprestige_max_rank", (*itr3)->maxRank, index);
				packet->setArrayDataByName("tradeskillprestige_rank_cost", (*itr3)->rankCost, index);
				packet->setArrayDataByName("tradeskillprestige_min_lev", (*itr3)->min_level, index);
				packet->setSubArrayLengthByName("tradeskillprestige_unknown5_num_items", 0, index, 0);
				//packet->setSubArrayDataByName("tradeskillprestige_unknown5", 308397057, index, 0);
				//packet->setSubArrayDataByName("tradeskillprestige_unknown5", 3215564363, index, 1);
				//packet->setSubArrayDataByName("tradeskillprestige_unknown5", 445192837, index, 2);
				//packet->setSubArrayDataByName("tradeskillprestige_unknown5", 3345493294, index, 3);
				//packet->setSubArrayDataByName("tradeskillprestige_unknown5", 696953971, index, 4);
				packet->setArrayDataByName("tradeskillprestige_unknown6", 4294967295, index);
				packet->setArrayDataByName("tradeskillprestige_unknown7", 1, index);
				packet->setArrayDataByName("tradeskillprestige_classification1", (*itr3)->class_name.c_str(), index);
				packet->setArrayDataByName("tradeskillprestige_points_req", (*itr3)->req_points, index);
				packet->setArrayDataByName("tradeskillprestige_unknown8", 0, index);
				packet->setArrayDataByName("tradeskillprestige_classification2", (*itr3)->subclass_name.c_str(), index);
				packet->setArrayDataByName("tradeskillprestige_col", (*itr3)->col, index);
				packet->setArrayDataByName("tradeskillprestige_row", (*itr3)->row, index);
				packet->setArrayDataByName("tradeskillprestige_line_title", (*itr3)->line_title.c_str(), index);
				packet->setArrayDataByName("tradeskillprestige_unknown9", ((*itr3)->title_level > 0 ? 258 : 0), index);
				packet->setArrayDataByName("tradeskillprestige_points_to_unlock", (*itr3)->req_tree_points, index);
				packet->setArrayDataByName("tradeskillprestige_unknown9b", 0, index);
				//}
				//else
					//LogWrite(SPELL__ERROR, 0, "AltAdvancement","Could not find Spell ID %u and Tier %i in Master Spell List", (*itr3)->spellID, (*itr3)->tier);
			}
		}
		
		packet->setDataByName("tradeskillprestige_points_spent", 18);
		if (version >= 58617) {
			packet->setDataByName("tradeskillprestige_unknown10", 18);

			packet->setDataByName("tradeskillprestige_unknown11a", 0);
			packet->setDataByName("tradeskillprestige_unknown11b", 50386);
			packet->setDataByName("tradeskillprestige_unknown11c", 3);

			packet->setDataByName("tradeskillprestige_unknown12", 0, 0);
			packet->setDataByName("tradeskillprestige_unknown12", 0, 1);
			packet->setDataByName("tradeskillprestige_unknown12", 0, 2);

			packet->setDataByName("tradeskillprestige_unknown14", 0);
			packet->setDataByName("tradeskillprestige_unknown16", 4, 0);
			packet->setDataByName("tradeskillprestige_unknown16", 0, 1);
			packet->setDataByName("tradeskillprestige_unknown16", 0, 2);
			packet->setDataByName("tradeskillprestige_unknown16", 0, 3);
			packet->setDataByName("tradeskillprestige_unknown16", 0, 4);
			packet->setDataByName("tradeskillprestige_unknown16", 0, 5);
		}
		else if (version >= 1193) {
			packet->setDataByName("tradeskillprestige_unknown10", 16);
			packet->setDataByName("tradeskillprestige_unknown11", 0, 0);
			packet->setDataByName("tradeskillprestige_unknown11", 1, 1);
			packet->setDataByName("tradeskillprestige_unknown11", 3, 2);

			packet->setDataByName("tradeskillprestige_unknown12", 0, 0);
			packet->setDataByName("tradeskillprestige_unknown12", 0, 1);
			packet->setDataByName("tradeskillprestige_unknown12", 0, 2);
			
			packet->setDataByName("tradeskillprestige_unknown16", 4, 0);
			packet->setDataByName("tradeskillprestige_unknown16", 0, 1);
			packet->setDataByName("tradeskillprestige_unknown16", 0, 2);
			packet->setDataByName("tradeskillprestige_unknown16", 0, 3);
			packet->setDataByName("tradeskillprestige_unknown16", 0, 4);
			packet->setDataByName("tradeskillprestige_unknown16", 0, 5);
		}
	}
	if (version >= 58617) {
		//__________________________________________________________START OF DRAGON TREE____________________________________________________________________________________
		for (itr2 = DragonTab.begin(); itr2 != DragonTab.end(); itr2++) {
			dragon_num_items += (itr2->second).size();
		}
		LogWrite(SPELL__DEBUG, 0, "AA", "DragonTab Size...%i ", dragon_num_items);
		index = 0;
		packet->setDataByName("dragon_tab_title", "Dragon");
		packet->setDataByName("dragon_tree_node_id", node_id[AA_DRAGON]);
		packet->setDataByName("dragon_max_aa", rule_manager.GetGlobalRule(R_Player, MaxDragonAA)->GetInt32());
		packet->setDataByName("dragon_id", classid[node_id[AA_DRAGON]]);
		packet->setDataByName("dragon_eof_req", 0);
		packet->setArrayLengthByName("dragon_num_items", dragon_num_items, 0);
		for (itr2 = DragonTab.begin(); itr2 != DragonTab.end(); itr2++) {
			for (itr3 = itr2->second.begin(); itr3 != itr2->second.end(); itr3++, index++) {
				//spell = master_spell_list.GetSpell((*itr3)->spellID, (*itr3)->tier);
				//current_rank = client->GetPlayer()->GetSpellTier((*itr3)->spellID);
				if (index == 0)
					class_node_id = (*itr3)->spellID;
					
				
				//if (spell) {
				packet->setArrayDataByName("dragon_parent_id", (*itr3)->rankPrereqID, index);
				packet->setArrayDataByName("dragon_req_tier", (*itr3)->rankPrereq, index);
				packet->setArrayDataByName("dragon_spell_id", (*itr3)->spellID, index);
				packet->setArrayDataByName("dragon_active", (current_rank == 0 ? 0 : (current_rank >= (*itr3)->maxRank) ? 2 : 1), index);
				packet->setArrayDataByName("dragon_spell_name", (*itr3)->name.c_str(), index);
				packet->setArrayDataByName("dragon_spell_description", (*itr3)->description.c_str(), index);
				packet->setArrayDataByName("dragon_icon", (*itr3)->icon, index);
				packet->setArrayDataByName("dragon_icon2", (*itr3)->icon2, index);
				packet->setArrayDataByName("dragon_current_rank", current_rank, index); // TODO: need to get this value from the DB
				packet->setArrayDataByName("dragon_max_rank", (*itr3)->maxRank, index);
				packet->setArrayDataByName("dragon_rank_cost", (*itr3)->rankCost, index);
				packet->setArrayDataByName("dragon_min_lev", (*itr3)->min_level, index);
				packet->setSubArrayLengthByName("dragon_unknown5_num_items", 0, index, 0);
				//packet->setSubArrayDataByName("dragon_unknown5", 308397057, index, 0);
				//packet->setSubArrayDataByName("dragon_unknown5", 3215564363, index, 1);
				//packet->setSubArrayDataByName("dragon_unknown5", 445192837, index, 2);
				//packet->setSubArrayDataByName("dragon_unknown5", 3345493294, index, 3);
				//packet->setSubArrayDataByName("dragon_unknown5", 696953971, index, 4);
				packet->setArrayDataByName("dragon_unknown6", 4294967295, index);
				packet->setArrayDataByName("dragon_unknown7", 1, index);
				packet->setArrayDataByName("dragon_classification1", (*itr3)->class_name.c_str(), index);
				packet->setArrayDataByName("dragon_points_req", (*itr3)->req_points, index);
				packet->setArrayDataByName("dragon_unknown8", 0, index);
				packet->setArrayDataByName("dragon_classification2", (*itr3)->subclass_name.c_str(), index);
				packet->setArrayDataByName("dragon_col", (*itr3)->col, index);
				packet->setArrayDataByName("dragon_row", (*itr3)->row, index);
				packet->setArrayDataByName("dragon_line_title", (*itr3)->line_title.c_str(), index);
				packet->setArrayDataByName("dragon_unknown9", ((*itr3)->title_level > 0 ? 258 : 0), index);
				packet->setArrayDataByName("dragon_points_to_unlock", (*itr3)->req_tree_points, index);
				packet->setArrayDataByName("dragon_unknown9b", 0, index);

				//}
				//else
					//LogWrite(SPELL__ERROR, 0, "AltAdvancement","Could not find Spell ID %u and Tier %i in Master Spell List", (*itr3)->spellID, (*itr3)->tier);
			}
		}
		
		packet->setDataByName("dragon_points_spent", 19);

		if (version >= 58617) {
			packet->setDataByName("dragon_unknown10", 19);

			packet->setDataByName("dragon_unknown11a", 0);
			packet->setDataByName("dragon_unknown11b", 50386);
			packet->setDataByName("dragon_unknown11c", 1);

			packet->setDataByName("dragon_unknown12", 0, 0);
			packet->setDataByName("dragon_unknown12", 0, 1);
			packet->setDataByName("dragon_unknown12", 0, 2);

			packet->setDataByName("dragon_unknown14", 0);
			packet->setDataByName("dragon_unknown16", 0, 0);
			packet->setDataByName("dragon_unknown16", 0, 1);
			packet->setDataByName("dragon_unknown16", 0, 2);
			packet->setDataByName("dragon_unknown16", 0, 3);
			packet->setDataByName("dragon_unknown16", 0, 4);
			packet->setDataByName("dragon_unknown16", 0, 5);
		}
		//__________________________________________________________START OF DRAGON CLASS TREE____________________________________________________________________________________
		for (itr2 = DragonclassTab.begin(); itr2 != DragonclassTab.end(); itr2++) {
			dragonclass_num_items += (itr2->second).size();
		}
		LogWrite(SPELL__DEBUG, 0, "AA", "DragonclassTab Size...%i ", dragonclass_num_items);
		index = 0;
		packet->setDataByName("dragonclass_tab_title", classes.GetClassNameCase(client->GetPlayer()->GetAdventureClass()).c_str());
		packet->setDataByName("dragonclass_tree_node_id",  node_id[AA_DRAGONCLASS]);
		packet->setDataByName("dragonclass_max_aa", rule_manager.GetGlobalRule(R_Player, MaxDragonAA)->GetInt32());
		packet->setDataByName("dragonclass_id", classid[node_id[AA_DRAGONCLASS]]);
		packet->setDataByName("dragonclass_eof_req", 0);
		packet->setArrayLengthByName("dragonclass_num_items", dragonclass_num_items, 0);
		for (itr2 = DragonclassTab.begin(); itr2 != DragonclassTab.end(); itr2++) {
			for (itr3 = itr2->second.begin(); itr3 != itr2->second.end(); itr3++, index++) {
				//spell = master_spell_list.GetSpell((*itr3)->spellID, (*itr3)->tier);
				//current_rank = client->GetPlayer()->GetSpellTier((*itr3)->spellID);
				if (index == 0)
					class_node_id = (*itr3)->spellID;
				//if (spell) {
				packet->setArrayDataByName("dragonclass_parent_id", (*itr3)->rankPrereqID, index);
				packet->setArrayDataByName("dragonclass_req_tier", (*itr3)->rankPrereq, index);
				packet->setArrayDataByName("dragonclass_spell_id", (*itr3)->spellID, index);
				packet->setArrayDataByName("dragonclass_active", (current_rank == 0 ? 0 : (current_rank >= (*itr3)->maxRank) ? 2 : 1), index);
				packet->setArrayDataByName("dragonclass_spell_name", (*itr3)->name.c_str(), index);
				packet->setArrayDataByName("dragonclass_spell_description", (*itr3)->description.c_str(), index);
				packet->setArrayDataByName("dragonclass_icon", (*itr3)->icon, index);
				packet->setArrayDataByName("dragonclass_icon2", (*itr3)->icon2, index);
				packet->setArrayDataByName("dragonclass_current_rank", current_rank, index); // TODO: need to get this value from the DB
				packet->setArrayDataByName("dragonclass_max_rank", (*itr3)->maxRank, index);
				packet->setArrayDataByName("dragonclass_rank_cost", (*itr3)->rankCost, index);
				packet->setArrayDataByName("dragonclass_min_lev", (*itr3)->min_level, index);
				packet->setSubArrayLengthByName("dragonclass_unknown5_num_items", 0, index, 0);
				//packet->setSubArrayDataByName("dragonclass_unknown5", 308397057, index, 0);
				//packet->setSubArrayDataByName("dragonclass_unknown5", 3215564363, index, 1);
				//packet->setSubArrayDataByName("dragonclass_unknown5", 445192837, index, 2);
				//packet->setSubArrayDataByName("dragonclass_unknown5", 3345493294, index, 3);
				//packet->setSubArrayDataByName("dragonclass_unknown5", 696953971, index, 4);
				packet->setArrayDataByName("dragonclass_unknown6", 4294967295, index);
				packet->setArrayDataByName("dragonclass_unknown7", 1, index);
				packet->setArrayDataByName("dragonclass_classification1", (*itr3)->class_name.c_str(), index);
				packet->setArrayDataByName("dragonclass_points_req", (*itr3)->req_points, index);
				packet->setArrayDataByName("dragonclass_unknown8", 0, index);
				packet->setArrayDataByName("dragonclass_classification2", (*itr3)->subclass_name.c_str(), index);
				packet->setArrayDataByName("dragonclass_col", (*itr3)->col, index);
				packet->setArrayDataByName("dragonclass_row", (*itr3)->row, index);
				packet->setArrayDataByName("dragonclass_line_title", (*itr3)->line_title.c_str(), index);
				packet->setArrayDataByName("dragonclass_unknown9", ((*itr3)->title_level > 0 ? 258 : 0), index);
				packet->setArrayDataByName("dragonclass_points_to_unlock", (*itr3)->req_tree_points, index);
				packet->setArrayDataByName("dragonclass_unknown9b", 0, index);

				//}
				//else
					//LogWrite(SPELL__ERROR, 0, "AltAdvancement","Could not find Spell ID %u and Tier %i in Master Spell List", (*itr3)->spellID, (*itr3)->tier);
			}
		}
		
		packet->setDataByName("dragonclass_points_spent",20);

		if (version >= 58617) {
			packet->setDataByName("dragonclass_unknown10", 20);

			packet->setDataByName("dragonclass_unknown11a", 0);
			packet->setDataByName("dragonclass_unknown11b", 50386);
			packet->setDataByName("dragonclass_unknown11c", 3);

			packet->setDataByName("dragonclass_unknown12", 0, 0);
			packet->setDataByName("dragonclass_unknown12", 0, 1);
			packet->setDataByName("dragonclass_unknown12", 0, 2);

			packet->setDataByName("dragonclass_unknown14", 0);
			packet->setDataByName("dragonclass_unknown16", 2, 0);
			packet->setDataByName("dragonclass_unknown16", 0, 1);
			packet->setDataByName("dragonclass_unknown16", 0, 2);
			packet->setDataByName("dragonclass_unknown16", 0, 3);
			packet->setDataByName("dragonclass_unknown16", 0, 4);
			packet->setDataByName("dragonclass_unknown16", 0, 5);
		}

		//__________________________________________________________START OF FARSEAS TREE____________________________________________________________________________________
		for (itr2 = FarseasTab.begin(); itr2 != FarseasTab.end(); itr2++) {
			farseas_num_items += (itr2->second).size();
		}
		LogWrite(SPELL__DEBUG, 0, "AA", "FarseasTab Size...%i ", farseas_num_items);
		index = 0;
		packet->setDataByName("farseas_tab_title", "Farseas");
		packet->setDataByName("farseas_tree_node_id", node_id[AA_FARSEAS]);
		packet->setDataByName("farseas_max_aa", rule_manager.GetGlobalRule(R_Player, MaxDragonAA)->GetInt32());
		packet->setDataByName("farseas_id", classid[node_id[AA_FARSEAS]]);
		packet->setDataByName("farseas_eof_req", 0);
		packet->setArrayLengthByName("farseas_num_items", farseas_num_items, 0);
		for (itr2 = FarseasTab.begin(); itr2 != FarseasTab.end(); itr2++) {
			for (itr3 = itr2->second.begin(); itr3 != itr2->second.end(); itr3++, index++) {
				//spell = master_spell_list.GetSpell((*itr3)->spellID, (*itr3)->tier);
				//current_rank = client->GetPlayer()->GetSpellTier((*itr3)->spellID);
				if (index == 0)
					class_node_id = (*itr3)->spellID;
				//if (spell) {
				packet->setArrayDataByName("farseas_parent_id", (*itr3)->rankPrereqID, index);
				packet->setArrayDataByName("farseas_req_tier", (*itr3)->rankPrereq, index);
				packet->setArrayDataByName("farseas_spell_id", (*itr3)->spellID, index);
				packet->setArrayDataByName("farseas_active", (current_rank == 0 ? 0 : (current_rank >= (*itr3)->maxRank) ? 2 : 1), index);
				packet->setArrayDataByName("farseas_spell_name", (*itr3)->name.c_str(), index);
				packet->setArrayDataByName("farseas_spell_description", (*itr3)->description.c_str(), index);
				packet->setArrayDataByName("farseas_icon", (*itr3)->icon, index);
				packet->setArrayDataByName("farseas_icon2", (*itr3)->icon2, index);
				packet->setArrayDataByName("farseas_current_rank", current_rank, index); // TODO: need to get this value from the DB
				packet->setArrayDataByName("farseas_max_rank", (*itr3)->maxRank, index);
				packet->setArrayDataByName("farseas_rank_cost", (*itr3)->rankCost, index);
				packet->setArrayDataByName("farseas_min_lev", (*itr3)->min_level, index);
				packet->setSubArrayLengthByName("farseas_unknown5_num_items", 0, index, 0);
				//packet->setSubArrayDataByName("farseas_unknown5", 308397057, index, 0);
				//packet->setSubArrayDataByName("farseas_unknown5", 3215564363, index, 1);
				//packet->setSubArrayDataByName("farseas_unknown5", 445192837, index, 2);
				//packet->setSubArrayDataByName("farseas_unknown5", 3345493294, index, 3);
				//packet->setSubArrayDataByName("farseas_unknown5", 696953971, index, 4);
				packet->setArrayDataByName("farseas_unknown6", 4294967295, index);
				packet->setArrayDataByName("farseas_unknown7", 1, index);
				packet->setArrayDataByName("farseas_classification1", (*itr3)->class_name.c_str(), index);
				packet->setArrayDataByName("farseas_points_req", (*itr3)->req_points, index);
				packet->setArrayDataByName("farseas_unknown8", 0, index);
				packet->setArrayDataByName("farseas_classification2", (*itr3)->subclass_name.c_str(), index);
				packet->setArrayDataByName("farseas_col", (*itr3)->col, index);
				packet->setArrayDataByName("farseas_row", (*itr3)->row, index);
				packet->setArrayDataByName("farseas_line_title", (*itr3)->line_title.c_str(), index);
				packet->setArrayDataByName("farseas_unknown9", ((*itr3)->title_level > 0 ? 258 : 0), index);
				packet->setArrayDataByName("farseas_points_to_unlock", (*itr3)->req_tree_points, index);
				packet->setArrayDataByName("farseas_unknown9b", 0, index);

				//}
				//else
					//LogWrite(SPELL__ERROR, 0, "AltAdvancement","Could not find Spell ID %u and Tier %i in Master Spell List", (*itr3)->spellID, (*itr3)->tier);
			}
		}
		
		packet->setDataByName("farseas_points_spent",20);

		if (version >= 58617) {
			packet->setDataByName("farseas_unknown10", 20);

			packet->setDataByName("farseas_unknown11a", 0);
			packet->setDataByName("farseas_unknown11b", 50386);
			packet->setDataByName("farseas_unknown11c", 3);

			packet->setDataByName("farseas_unknown12", 0, 0);
			packet->setDataByName("farseas_unknown12", 0, 1);
			packet->setDataByName("farseas_unknown12", 0, 2);

			packet->setDataByName("farseas_unknown14", 0);
			packet->setDataByName("farseas_unknown16", 4, 0);
			packet->setDataByName("farseas_unknown16", 0, 1);
			packet->setDataByName("farseas_unknown16", 0, 2);
			packet->setDataByName("farseas_unknown16", 0, 3);
			packet->setDataByName("farseas_unknown16", 0, 4);
			packet->setDataByName("farseas_unknown16", 0, 5);
		}
		int8 tt = 0;
		int8 numtabs = 0;
		int xxx = 0;
		bool sendblanktabs = false;
		packet->setDataByName("template_unknown1", (changemode == 2 ? 255 : 25));
		packet->setDataByName("template_unknown2a", 0);
		packet->setDataByName("template_unknown2b", 0);
		packet->setDataByName("template_unknown2c", (changemode == 3 ? 1 : 0));
		packet->setDataByName("template_unknown2d", (changemode == 1 || changemode == 2 ? 1 :0));
		packet->setDataByName("template_unknown3", (changemode == 2 ? 0 : 4294967295)); //4294967295);
		packet->setDataByName("template_unknown4", newtemplate);// active template ID
		packet->setDataByName("template_unknown5", 0);

		packet->setDataByName("num_templates", 7);
		packet->setDataByName("slot1_template_id", 0);
		packet->setDataByName("slot1_name", "Unused Slot 1");
		packet->setDataByName("slot1_active", 0);  //0 is server type ,1 = personal type
		tt = 1; // template #
		numtabs = 0;
		if (AAEntryList[tt][0].size() > 0) { numtabs++; }
		if (AAEntryList[tt][1].size() > 0) { numtabs++; }
		if (AAEntryList[tt][2].size() > 0) { numtabs++; }
		if (AAEntryList[tt][3].size() > 0) { numtabs++; }
		if (AAEntryList[tt][4].size() > 0) { numtabs++; }
		
		if (sendblanktabs == true) {
			numtabs = 4;
		}
		packet->setArrayLengthByName("slot1_num_tabs", numtabs);
		xxx = 0;
		for (int xx = 0; xx < AAEntryList[tt].size(); xx++) {
			
			if (sendblanktabs == false && AAEntryList[tt][xx].size() < 1 ) {continue;}
				packet->setArrayDataByName("slot1_tab_typenum", xx, xxx);
				packet->setArrayDataByName("slot1_tab_unknown2", 1, xxx);
				packet->setSubArrayLengthByName("slot1_num_items", AAEntryList[tt][xx].size(), xxx);
				for (int yy = 0; yy < AAEntryList[tt][xx].size(); yy++) {
					packet->setSubArrayDataByName("slot1_item_order", AAEntryList[tt][xx][yy].order, xxx, yy);
					packet->setSubArrayDataByName("slot1_item_treeid", AAEntryList[tt][xx][yy].treeid, xxx, yy);
					packet->setSubArrayDataByName("slot1_item_id", AAEntryList[tt][xx][yy].aa_id, xxx, yy);
			}
			xxx++;
		}
		
		packet->setDataByName("slot2_template_id", 1);
		packet->setDataByName("slot2_name", "Unused Slot 2");
		packet->setDataByName("slot2_active", 0);
		tt = 2; // template #
		numtabs = 0;
		if (AAEntryList[tt][0].size() > 0) { numtabs++; }
		if (AAEntryList[tt][1].size() > 0) { numtabs++; }
		if (AAEntryList[tt][2].size() > 0) { numtabs++; }
		if (AAEntryList[tt][3].size() > 0) { numtabs++; }
		if (AAEntryList[tt][4].size() > 0) { numtabs++; }
		
		if (sendblanktabs == true) {
			numtabs = 4;
		}
		packet->setArrayLengthByName("slot2_num_tabs", numtabs);
		xxx = 0;
		for (int xx = 0; xx < AAEntryList[tt].size(); xx++) {

			if (sendblanktabs == false && AAEntryList[tt][xx].size() < 1) { continue; }
			packet->setArrayDataByName("slot2_tab_typenum", xx, xxx);
			packet->setArrayDataByName("slot2_tab_unknown2", 1, xxx);
			packet->setSubArrayLengthByName("slot2_num_items", AAEntryList[tt][xx].size(), xxx);
			for (int yy = 0; yy < AAEntryList[tt][xx].size(); yy++) {
				packet->setSubArrayDataByName("slot2_item_order", AAEntryList[tt][xx][yy].order, xxx, yy);
				packet->setSubArrayDataByName("slot2_item_treeid", AAEntryList[tt][xx][yy].treeid, xxx, yy);
				packet->setSubArrayDataByName("slot2_item_id", AAEntryList[tt][xx][yy].aa_id, xxx, yy);
			}
			xxx++;
		}

		packet->setDataByName("slot3_template_id", 2);
		packet->setDataByName("slot3_name", "Unused Slot 3");
		packet->setDataByName("slot3_active", 0);
		tt = 3; // template #
		numtabs = 0;
		if (AAEntryList[tt][0].size() > 0) { numtabs++; }
		if (AAEntryList[tt][1].size() > 0) { numtabs++; }
		if (AAEntryList[tt][2].size() > 0) { numtabs++; }
		if (AAEntryList[tt][3].size() > 0) { numtabs++; }
		if (AAEntryList[tt][4].size() > 0) { numtabs++; }
		
		if (sendblanktabs == true) {
			numtabs = 4;
		}
		packet->setArrayLengthByName("slot3_num_tabs", numtabs);
		xxx = 0;
		for (int xx = 0; xx < AAEntryList[tt].size(); xx++) {

			if (sendblanktabs == false && AAEntryList[tt][xx].size() < 1) { continue; }
			packet->setArrayDataByName("slot3_tab_typenum", xx, xxx);
			packet->setArrayDataByName("slot3_tab_unknown2", 1, xxx);
			packet->setSubArrayLengthByName("slot3_num_items", AAEntryList[tt][xx].size(), xxx);
			for (int yy = 0; yy < AAEntryList[tt][xx].size(); yy++) {
				packet->setSubArrayDataByName("slot3_item_order", AAEntryList[tt][xx][yy].order, xxx, yy);
				packet->setSubArrayDataByName("slot3_item_treeid", AAEntryList[tt][xx][yy].treeid, xxx, yy);
				packet->setSubArrayDataByName("slot3_item_id", AAEntryList[tt][xx][yy].aa_id, xxx, yy);
			}
			xxx++;
		}

		packet->setDataByName("slot4_template_id", 20);
		packet->setDataByName("slot4_name", "Basic Leveling Profile - Solo");
		packet->setDataByName("slot4_active", 1);
		tt = 4; // template #
		numtabs = 0;
		if (AAEntryList[tt][0].size() > 0) { numtabs++; }
		if (AAEntryList[tt][1].size() > 0) { numtabs++; }
		if (AAEntryList[tt][2].size() > 0) { numtabs++; }
		if (AAEntryList[tt][3].size() > 0) { numtabs++; }
		if (AAEntryList[tt][4].size() > 0) { numtabs++; }
		
		if (sendblanktabs == true) {
			numtabs = 4;
		}
		packet->setArrayLengthByName("slot4_num_tabs", numtabs);
		xxx = 0;
		for (int xx = 0; xx < AAEntryList[tt].size(); xx++) {

			if (sendblanktabs == false && AAEntryList[tt][xx].size() < 1) { continue; }
			packet->setArrayDataByName("slot4_tab_typenum", xx, xxx);
			packet->setArrayDataByName("slot4_tab_unknown2", 1, xxx);
			packet->setSubArrayLengthByName("slot4_num_items", AAEntryList[tt][xx].size(), xxx);
			for (int yy = 0; yy < AAEntryList[tt][xx].size(); yy++) {
				packet->setSubArrayDataByName("slot4_item_order", AAEntryList[tt][xx][yy].order, xxx, yy);
				packet->setSubArrayDataByName("slot4_item_treeid", AAEntryList[tt][xx][yy].treeid, xxx, yy);
				packet->setSubArrayDataByName("slot4_item_id", AAEntryList[tt][xx][yy].aa_id, xxx, yy);
			}
			xxx++;
		}


		packet->setDataByName("slot5_template_id", 21);
		packet->setDataByName("slot5_name", "Basic Leveling Profile - Group");
		packet->setDataByName("slot5_active", 1);
		tt = 5; // template #
		numtabs = 0;
		if (AAEntryList[tt][0].size() > 0) { numtabs++; }
		if (AAEntryList[tt][1].size() > 0) { numtabs++; }
		if (AAEntryList[tt][2].size() > 0) { numtabs++; }
		if (AAEntryList[tt][3].size() > 0) { numtabs++; }
		if (AAEntryList[tt][4].size() > 0) { numtabs++; }
	
		if (sendblanktabs == true) {
			numtabs = 4;
		}
		packet->setArrayLengthByName("slot5_num_tabs", numtabs);
		xxx = 0;
		for (int xx = 0; xx < AAEntryList[tt].size(); xx++) {

			if (sendblanktabs == false && AAEntryList[tt][xx].size() < 1) { continue; }
			packet->setArrayDataByName("slot5_tab_typenum", xx, xxx);
			packet->setArrayDataByName("slot5_tab_unknown2", 1, xxx);
			packet->setSubArrayLengthByName("slot5_num_items", AAEntryList[tt][xx].size(), xxx);
			for (int yy = 0; yy < AAEntryList[tt][xx].size(); yy++) {
				packet->setSubArrayDataByName("slot5_item_order", AAEntryList[tt][xx][yy].order, xxx, yy);
				packet->setSubArrayDataByName("slot5_item_treeid", AAEntryList[tt][xx][yy].treeid, xxx, yy);
				packet->setSubArrayDataByName("slot5_item_id", AAEntryList[tt][xx][yy].aa_id, xxx, yy);
			}
			xxx++;
		}

		packet->setDataByName("slot6_template_id", 22);
		packet->setDataByName("slot6_name", "Basic Leveling Profile - PVP");
		packet->setDataByName("slot6_active", 1);
		tt = 6; // template #
		numtabs = 0;
		if (AAEntryList[tt][0].size() > 0) { numtabs++; }
		if (AAEntryList[tt][1].size() > 0) { numtabs++; }
		if (AAEntryList[tt][2].size() > 0) { numtabs++; }
		if (AAEntryList[tt][3].size() > 0) { numtabs++; }
		if (AAEntryList[tt][4].size() > 0) { numtabs++; }
		
		if (sendblanktabs == true) {
			numtabs = 4;
		}
		packet->setArrayLengthByName("slot6_num_tabs", numtabs);
		xxx = 0;
		for (int xx = 0; xx < AAEntryList[tt].size(); xx++) {

			if (sendblanktabs == false && AAEntryList[tt][xx].size() < 1) { continue; }
			packet->setArrayDataByName("slot6_tab_typenum", xx, xxx);
			packet->setArrayDataByName("slot6_tab_unknown2", 1, xxx);
			packet->setSubArrayLengthByName("slot6_num_items", AAEntryList[tt][xx].size(), xxx);
			for (int yy = 0; yy < AAEntryList[tt][xx].size(); yy++) {
				packet->setSubArrayDataByName("slot6_item_order", AAEntryList[tt][xx][yy].order, xxx, yy);
				packet->setSubArrayDataByName("slot6_item_treeid", AAEntryList[tt][xx][yy].treeid, xxx, yy);
				packet->setSubArrayDataByName("slot6_item_id", AAEntryList[tt][xx][yy].aa_id, xxx, yy);
			}
			xxx++;
		}

		packet->setDataByName("slot7_template_id",100);
		packet->setDataByName("slot7_name", "New");
		packet->setDataByName("slot7_active", 0);
		tt = 7; // template #
		numtabs = 0;
		if (AAEntryList[tt][0].size() > 0) { numtabs++; }
		if (AAEntryList[tt][1].size() > 0) { numtabs++; }
		if (AAEntryList[tt][2].size() > 0) { numtabs++; }
		if (AAEntryList[tt][3].size() > 0) { numtabs++; }
		if (AAEntryList[tt][4].size() > 0) { numtabs++; }

		if (sendblanktabs == true) {
			numtabs = 4;
		}
		packet->setArrayLengthByName("slot7_num_tabs", numtabs);
		xxx = 0;
		for (int xx = 0; xx < AAEntryList[tt].size(); xx++) {

			if (sendblanktabs == false && AAEntryList[tt][xx].size() < 1) { continue; }
			packet->setArrayDataByName("slot7_tab_typenum", xx, xxx);
			packet->setArrayDataByName("slot7_tab_unknown2", 1, xxx);
			packet->setSubArrayLengthByName("slot7_num_items", AAEntryList[tt][xx].size(), xxx);
			for (int yy = 0; yy < AAEntryList[tt][xx].size(); yy++) {
				packet->setSubArrayDataByName("slot7_item_order", AAEntryList[tt][xx][yy].order, xxx, yy);
				packet->setSubArrayDataByName("slot7_item_treeid", AAEntryList[tt][xx][yy].treeid, xxx, yy);
				packet->setSubArrayDataByName("slot7_item_id", AAEntryList[tt][xx][yy].aa_id, xxx, yy);
			}
			xxx++;
		}
		//packet->PrintPacket();
	}
	//packet->PrintPacket();
	EQ2Packet* data = packet->serialize();
	EQ2Packet* app = new EQ2Packet(OP_AdventureList, data->pBuffer, data->size);
	//DumpPacket(app);
	client->QueuePacket(app);
	safe_delete(packet);
	safe_delete(data);
}