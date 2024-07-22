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

#ifdef WIN32
	#include <WinSock2.h>
	#include <windows.h>
#endif
#include <mysql.h>
#include <assert.h>
#include "../../common/Log.h"
#include "../../common/DatabaseNew.h"
#include "../WorldDatabase.h"
#include "AltAdvancement.h"

extern MasterAAList master_aa_list;
extern MasterAANodeList master_tree_nodes;

void WorldDatabase::LoadAltAdvancements() 
{
	Query query;
	MYSQL_ROW row;
	AltAdvanceData* data;

	//MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `spell_id`, `group`, `icon`, `icon2`, `col`, `row`, `rank_cost`, `max_cost`, `rank_prereq_id`, `rank_prereq`, `class_req`, `tier`, `class_name`, `subclass_name`, `line_title` FROM spell_aa");
	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `nodeid`,`minlevel`, `spellcrc`, `name`, `description`, `aa_list_fk`, `icon_id`, `icon_backdrop`, `xcoord`, `ycoord`, `pointspertier`, `maxtier`, `firstparentid`, `firstparentrequiredtier`, `displayedclassification`,`requiredclassification`, `classificationpointsrequired`, `pointsspentintreetounlock`, `title`,`titlelevel` FROM spell_aa_nodelist");
	while (result && (row = mysql_fetch_row(result))) {
		data = new AltAdvanceData;
		int8 i = 0;
		data->spellID = strtoul(row[0], NULL, 0);
		data->min_level = atoi(row[++i]);
		data->spell_crc = strtoul(row[++i], NULL, 0);
		data->name = string(row[++i]);
		data->description = string(row[++i]);
		data->group = atoi(row[++i]);
		data->icon = atoi(row[++i]);
		data->icon2 = atoi(row[++i]);
		data->col = atoi(row[++i]);
		data->row = atoi(row[++i]);
		data->rankCost = atoi(row[++i]);
		data->maxRank = atoi(row[++i]);
		data->rankPrereqID = strtoul(row[++i], NULL, 0);
		data->rankPrereq = atoi(row[++i]);
		data->tier = 1;
		data->class_name = string(row[++i]);
		data->subclass_name = string(row[i]);
		data->req_points = atoi(row[++i]);
		data->req_tree_points = atoi(row[++i]);
		data->line_title = string(row[++i]);
		data->title_level = atoi(row[++i]);

		master_aa_list.AddAltAdvancement(data);
	}

	LogWrite(SPELL__INFO, 0, "AA", "Loaded %u Alternate Advancement(s)", master_aa_list.Size());

}

void WorldDatabase::LoadTreeNodes()
{
	Query query;
	MYSQL_ROW row;
	TreeNodeData* data;

	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT class_id, tree_node, aa_tree_id FROM spell_aa_class_list");
	while (result && (row = mysql_fetch_row(result))) {
		data = new TreeNodeData;
		data->classID = strtoul(row[0], NULL, 0);
		data->treeID = strtoul(row[1], NULL, 0);
		data->AAtreeID = strtoul(row[2], NULL, 0);
		master_tree_nodes.AddTreeNode(data);
	}
	LogWrite(SPELL__INFO, 0, "AA", "Loaded %u AA Tree Nodes", master_tree_nodes.Size());
}
void WorldDatabase::LoadPlayerAA(Player *player)
{
	Query query;
	MYSQL_ROW row;
	

	MYSQL_RES* result = query.RunQuery2(Q_SELECT, "SELECT `template_id`,`tab_id`,`aa_id`,`order`,treeid FROM character_aa where char_id = %i order by `order`",player->id);
	while (result && (row = mysql_fetch_row(result))) {
		
	}
	LogWrite(SPELL__INFO, 0, "AA", "Loaded %u AA Tree Nodes", master_tree_nodes.Size());
}