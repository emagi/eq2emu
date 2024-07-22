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

#include <assert.h>
#include "../../common/Log.h"
#include "../WorldDatabase.h"

extern RuleManager rule_manager;

void WorldDatabase::LoadGlobalRuleSet() {
	Query query;
	MYSQL_ROW row;
	MYSQL_RES *res;
	int32 rule_set_id = 0;

	res = query.RunQuery2(Q_SELECT,	"SELECT `variable_value`\n"
									"FROM `variables`\n"
									"WHERE `variable_name`='default_ruleset_id'");
	if (res && (row = mysql_fetch_row(res)))
	{
		rule_set_id = atoul(row[0]);
		LogWrite(RULESYS__DEBUG, 5, "Rules", "\t\tLoading Global Ruleset id %i", rule_set_id);
	}

	if (rule_set_id > 0 && !rule_manager.SetGlobalRuleSet(rule_set_id))
		LogWrite(RULESYS__ERROR, 0, "Rules", "Error loading global rule set. A rule set with ID %u does not exist.", rule_set_id);
	else if(rule_set_id == 0)
		LogWrite(RULESYS__ERROR, 0, "Rules", "Variables table is missing default_ruleset_id variable name, this means the global rules will be code-default, database entries not used.  Use query such as \"insert into variables set variable_name='default_ruleset_id',variable_value='1',comment='Default ruleset';\" to resolve.");

}

void WorldDatabase::LoadRuleSets(bool reload) {
	RuleSet *rule_set;
	Query query;
	MYSQL_ROW row;
	MYSQL_RES *res;

	if (reload)
		rule_manager.Flush(true);

	/* first load the coded defaults in */
	rule_manager.LoadCodedDefaultsIntoRuleSet(rule_manager.GetGlobalRuleSet());

	res = query.RunQuery2(Q_SELECT,	"SELECT `ruleset_id`,`ruleset_name`\n"
									"FROM `rulesets`\n"
									"WHERE `ruleset_active`>0");
	if (res) {
		while ((row = mysql_fetch_row(res))) {
			rule_set = new RuleSet();
			rule_set->SetID(atoul(row[0]));
			rule_set->SetName(row[1]);
			if (rule_manager.AddRuleSet(rule_set))
			{
				LogWrite(RULESYS__DEBUG, 5, "Rules", "\t\tLoading rule set '%s' (%u)", rule_set->GetName(), rule_set->GetID());
				LoadRuleSetDetails(rule_set);
			}
			else {
				LogWrite(RULESYS__ERROR, 0, "Rules", "Unable to add rule set '%s'. A ruleset with ID %u already exists.", rule_set->GetName(), rule_set->GetID());
				safe_delete(rule_set);
			}
		}
	}

	LogWrite(RULESYS__DEBUG, 3, "Rules", "--Loaded %u Rule Sets", rule_manager.GetNumRuleSets());
	
	LoadGlobalRuleSet();
}

void WorldDatabase::LoadRuleSetDetails(RuleSet *rule_set) {
	Rule *rule;
	Query query;
	MYSQL_ROW row;
	MYSQL_RES *res;

	assert(rule_set);

	rule_set->CopyRulesInto(rule_manager.GetGlobalRuleSet());
	res = query.RunQuery2(Q_SELECT,	"SELECT `rule_category`,`rule_type`,`rule_value`\n"
									"FROM `ruleset_details`\n"
									"WHERE `ruleset_id`=%u",
									rule_set->GetID());
	if (res) {
		while ((row = mysql_fetch_row(res))) {
			if (!(rule = rule_set->GetRule(row[0], row[1]))) {
				LogWrite(RULESYS__WARNING, 0, "Rules", "Unknown rule with category '%s' and type '%s'", row[0], row[1]);
				continue;
			}
			LogWrite(RULESYS__DEBUG, 5, "Rules", "---Setting rule category '%s', type '%s' to value: %s", row[0], row[1], row[2]);
			rule->SetValue(row[2]);
		}
	}
}
