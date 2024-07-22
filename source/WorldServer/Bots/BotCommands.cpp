#include "../Commands/Commands.h"
#include "../WorldDatabase.h"
#include "../classes.h"
#include "../races.h"
#include "../Bots/Bot.h"
#include "../../common/Log.h"
#include "../Trade.h"
#include "../PlayerGroups.h"
#include "../World.h"
#include "../../common/GlobalHeaders.h"

extern WorldDatabase database;
extern ConfigReader configReader;
extern World world;
extern MasterSpellList master_spell_list;

void Commands::Command_Bot(Client* client, Seperator* sep) {

	if (sep && sep->IsSet(0)) {
		if (strncasecmp("camp", sep->arg[0], 4) == 0) {
			if (!client->GetPlayer()->GetTarget() || !client->GetPlayer()->GetTarget()->IsBot()) {
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You must target a bot");
				return;
			}

			Bot* bot = (Bot*)client->GetPlayer()->GetTarget();
			if (bot->GetOwner() != client->GetPlayer()) {
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You can only camp your own bots.");
				return;
			}

			bot->Camp();
			return;
		}
		else if (strncasecmp("attack", sep->arg[0], 6) == 0) {
			if (client->GetPlayer()->GetTarget() && client->GetPlayer()->GetTarget()->IsEntity() && client->GetPlayer()->GetTarget()->Alive()) {
				Entity* target = (Entity*)client->GetPlayer()->GetTarget();

				if (client->GetPlayer()->GetDistance(target) <= 50) {
					if (client->GetPlayer()->AttackAllowed(target)) {
						GroupMemberInfo* gmi = client->GetPlayer()->GetGroupMemberInfo();
						if (gmi) {
							PlayerGroup* group = world.GetGroupManager()->GetGroup(gmi->group_id);
							if (group)
							{
								group->MGroupMembers.readlock(__FUNCTION__, __LINE__);
								deque<GroupMemberInfo*>* members = group->GetMembers();
								deque<GroupMemberInfo*>::iterator itr;
								
 								for (itr = members->begin(); itr != members->end(); itr++) {
//devn00b compile says this is no good, commenting out for now.
//if(!member) 
//  continue;
									if ((*itr)->member && (*itr)->member->IsBot() && ((Bot*)(*itr)->member)->GetOwner() == client->GetPlayer()) {
                                      
										((Bot*)(*itr)->member)->SetCombatTarget(target->GetID());
									}
								}
								group->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
							}
						}
					}
					else
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You can not attack that target.");
				}
				else
					client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Target is to far away.");
			}
			else
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Not a valid target.");

			return;
		}
		else if (strncasecmp("spells", sep->arg[0], 6) == 0) {
			if (client->GetPlayer()->GetTarget() && client->GetPlayer()->GetTarget()->IsBot()) {
				Bot* bot = (Bot*)client->GetPlayer()->GetTarget();

				map<int32, int8>* spells = bot->GetBotSpells();
				map<int32, int8>::iterator itr;
				string output;
				for (itr = spells->begin(); itr != spells->end(); itr++) {
					Spell* spell = master_spell_list.GetSpell(itr->first, itr->second);
					if (spell) {
						output += spell->GetName();
						output += "\n";
					}
				}

				client->SimpleMessage(CHANNEL_COLOR_YELLOW, output.c_str());
				return;
			}
		}
		else if (strncasecmp("maintank", sep->arg[0], 8) == 0) {
			if (!client->GetPlayer()->GetTarget() || !client->GetPlayer()->GetTarget()->IsEntity()) {
				client->SimpleMessage(CHANNEL_COMMAND_TEXT, "Not a valid target.");
				return;
			}

			GroupMemberInfo* gmi = client->GetPlayer()->GetGroupMemberInfo();
			if (!gmi) {
				client->SimpleMessage(CHANNEL_COMMAND_TEXT, "You are not in a group.");
				return;
			}

			Entity* target = (Entity*)client->GetPlayer()->GetTarget();
			if (!world.GetGroupManager()->IsInGroup(gmi->group_id, target)) {
				client->SimpleMessage(CHANNEL_COMMAND_TEXT, "Target is not in your group.");
				return;
			}

			PlayerGroup* group = world.GetGroupManager()->GetGroup(gmi->group_id);
			if (group)
			{
				group->MGroupMembers.readlock(__FUNCTION__, __LINE__);
				deque<GroupMemberInfo*>* members = group->GetMembers(); 
				
				for (int8 i = 0; i < members->size(); i++) {
					GroupMemberInfo* gmi2 = members->at(i);
					if(!gmi2 || !gmi2->member)
						continue;
					if (gmi2->member->IsBot() && ((Bot*)gmi2->member)->GetOwner() == client->GetPlayer()) {
						((Bot*)gmi2->member)->SetMainTank(target);
						client->Message(CHANNEL_COMMAND_TEXT, "Setting main tank for %s to %s", gmi2->member->GetName(), target->GetName());
					}
				}
				group->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
			}
			return;
		}
		else if (strncasecmp("delete", sep->arg[0], 6) == 0) {
			if (sep->IsSet(1) && sep->IsNumber(1)) {
				int32 index = atoi(sep->arg[1]);

				// Check if bot is currently spawned and if so camp it out
				if (client->GetPlayer()->SpawnedBots.count(index) > 0) {
					Spawn* bot = client->GetCurrentZone()->GetSpawnByID(client->GetPlayer()->SpawnedBots[index]);
					if (bot && bot->IsBot())
						((Bot*)bot)->Camp();
				}

				database.DeleteBot(client->GetCharacterID(), index);
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Bot has been deleted.");
				return;
			}
			else {
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You must give the id (from /bot list) to delete a bot");
				return;
			}
		}
		else if (strncasecmp("follow", sep->arg[0], 6) == 0) {
			if (sep->IsSet(1) && sep->IsNumber(1)) {
				int32 index = atoi(sep->arg[1]);

				// Check if bot is currently spawned and if so camp it out
				if (client->GetPlayer()->SpawnedBots.count(index) > 0) {
					Spawn* bot = client->GetCurrentZone()->GetSpawnByID(client->GetPlayer()->SpawnedBots[index]);
					if (bot && bot->IsBot())
						((Bot*)bot)->SetFollowTarget(client->GetPlayer(), 5);
				}
				return;
			}
			else {
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You must give the id (from /bot list) to have a bot follow you");
				return;
			}
		}
		else if (strncasecmp("stopfollow", sep->arg[0], 10) == 0) {
			if (sep->IsSet(1) && sep->IsNumber(1)) {
				int32 index = atoi(sep->arg[1]);
 
				// Check if bot is currently spawned and if so camp it out
				if (client->GetPlayer()->SpawnedBots.count(index) > 0) {
					Spawn* bot = client->GetCurrentZone()->GetSpawnByID(client->GetPlayer()->SpawnedBots[index]);
					if (bot && bot->IsBot())
						((Bot*)bot)->SetFollowTarget(nullptr);
				}
				return;
			}
			else {
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You must give the id (from /bot list) to stop a following bot");
				return;
			}
		}
		else if (strncasecmp("summon", sep->arg[0], 6) == 0) {
			if (sep->IsSet(1) && strncasecmp("group", sep->arg[1], 5) == 0) {
				GroupMemberInfo* gmi = client->GetPlayer()->GetGroupMemberInfo();
				if (gmi) {
					Player* player = client->GetPlayer();
					PlayerGroup* group = world.GetGroupManager()->GetGroup(gmi->group_id);
					if (group)
					{
						group->MGroupMembers.readlock(__FUNCTION__, __LINE__);
						deque<GroupMemberInfo*>* members = group->GetMembers();
						for (int8 i = 0; i < members->size(); i++) {
							Entity* member = members->at(i)->member;
							
							if(!member)
								continue;
							
							if (member->IsBot() && ((Bot*)member)->GetOwner() == player) {
								if(member->GetZone() && member->GetLocation() != player->GetLocation()) {
									member->SetLocation(player->GetLocation());
								}
								member->SetX(player->GetX());
								member->SetY(player->GetY());
								member->SetZ(player->GetZ());
								client->Message(CHANNEL_COLOR_YELLOW, "Summoning %s.", member->GetName());
							}
						}
						group->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
					}
					return;
				}
				else {
					client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You are not in a group.");
					return;
				}
			}
			else {
				if (client->GetPlayer()->GetTarget() && client->GetPlayer()->GetTarget()->IsBot()) {
					Bot* bot = (Bot*)client->GetPlayer()->GetTarget();
					Player* player = client->GetPlayer();
					if (bot && bot->GetOwner() == player) {
						bot->SetLocation(player->GetLocation());
						bot->SetX(player->GetX());
						bot->SetY(player->GetY());
						bot->SetZ(player->GetZ());
						client->Message(CHANNEL_COLOR_YELLOW, "Summoning %s.", bot->GetName());
						return;
					}
					else {
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You can only summon your own bots.");
						return;
					}
				}
				else {
					client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You must target a bot.");
					return;
				}
			}
		}
		else if (strncasecmp("test", sep->arg[0], 4) == 0) {
			if (client->GetPlayer()->GetTarget() && client->GetPlayer()->GetTarget()->IsBot()) {
				((Bot*)client->GetPlayer()->GetTarget())->MessageGroup("Test message");
				return;
			}
		}
	}

	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "BotCommands:");
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/bot create [race] [gender] [class] [name]");
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/bot customize - customize the appearance of the bot");
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/bot list - list all the bots you have created with this character");
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/bot spawn [id] - spawns a bot into the world, id obtained from /bot list");
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/bot inv [give/list/remove] - manage bot equipment, for remove a slot must be provided");
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/bot settings [helm/hood/cloak/taunt] [0/1] - Turn setting on (1) or off(0)");
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/bot camp - removes the bot from your group and despawns them");
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/bot attack - commands your bots to attack your target");
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/bot spells - lists bot spells, not fully implemented yet");
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/bot maintank - sets targeted group member as the main tank for your bots");
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/bot delete [id] - deletes the bot with the given id (obtained from /bot list)");
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/bot help");
}

void Commands::Command_Bot_Create(Client* client, Seperator* sep) {
	int8 race = BARBARIAN;
	int8 gender = 0;
	int8 advClass = GUARDIAN;
	string name;

	if (sep) {
		if (sep->IsSet(0) && sep->IsNumber(0))
			race = atoi(sep->arg[0]);
		else {
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "First param of \"/bot create\" needs to be a number");
			return;
		}

		if (sep->IsSet(1) && sep->IsNumber(1))
			gender = atoi(sep->arg[1]);
		else {
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Second param of \"/bot create\" needs to be a number");
			return;
		}

		if (sep->IsSet(2) && sep->IsNumber(2))
			advClass = atoi(sep->arg[2]);
		else {
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Third param of \"/bot create\" needs to be a number");
			return;
		}

		if (sep->IsSet(3)) {
			name = string(sep->arg[3]);
			transform(name.begin(), name.begin() + 1, name.begin(), ::toupper);
			transform(name.begin() + 1, name.end(), name.begin() + 1, ::tolower);
		}
		else {
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Fourth param (name) of \"/bot create\" is required");
			return;
		}

	}
	else {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Syntax: /bot create [race ID] [Gender ID] [class ID] [name]");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "All parameters are required.  /bot help race or /bot help class for ID's.");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Gender ID's: 0 = Female, 1 = Male");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Ex: /bot create 0 0 3 Botty");
		return;
	}

	int8 result = database.CheckNameFilter(name.c_str());
	if (result == BADNAMELENGTH_REPLY) {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Name length is invalid, must be greater then 3 characters and less then 16.");
		return;
	}
	else if (result == NAMEINVALID_REPLY) {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Name is invalid, can only contain letters.");
		return;
	}
	else if (result == NAMETAKEN_REPLY) {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Name is already taken, please choose another.");
		return;
	}
	else if (result == NAMEFILTER_REPLY) {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Name failed the filter check.");
		return;
	}
	else if (result == UNKNOWNERROR_REPLY) {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Unknown error while checking the name.");
		return;
	}

	string race_string;
	switch (race) {
	case BARBARIAN:
		race_string = "/barbarian/barbarian";
		break;
	case DARK_ELF:
		race_string = "/darkelf/darkelf";
		break;
	case DWARF:
		race_string = "/dwarf/dwarf";
		break;
	case ERUDITE:
		race_string = "/erudite/erudite";
		break;
	case FROGLOK:
		race_string = "/froglok/froglok";
		break;
	case GNOME:
		race_string = "/gnome/gnome";
		break;
	case HALF_ELF:
		race_string = "/halfelf/halfelf";
		break;
	case HALFLING:
		race_string = "/halfling/halfling";
		break;
	case HIGH_ELF:
		race_string = "/highelf/highelf";
		break;
	case HUMAN:
		race_string = "/human/human";
		break;
	case IKSAR:
		race_string = "/iksar/iksar";
		break;
	case KERRA:
		race_string = "/kerra/kerra";
		break;
	case OGRE:
		race_string = "/ogre/ogre";
		break;
	case RATONGA:
		race_string = "/ratonga/ratonga";
		break;
	case TROLL:
		race_string = "/troll/troll";
		break;
	case WOOD_ELF:
		race_string = "/woodelf/woodelf";
		break;
	case FAE:
		race_string = "/fae/fae_light";
		break;
	case ARASAI:
		race_string = "/fae/fae_dark";
		break;
	case SARNAK:
		gender == 1 ? race_string = "01/sarnak_male/sarnak" : race_string = "01/sarnak_female/sarnak";
		break;
	case VAMPIRE:
		race_string = "/vampire/vampire";
		break;
	case AERAKYN:
		race_string = "/aerakyn/aerakyn";
		break;
	}

	if (race_string.length() > 0) {
		string gender_string;
		Bot* bot = 0;

		gender == 1 ? gender_string = "male" : gender_string = "female";

		vector<int16>* id_list = database.GetAppearanceIDsLikeName("ec/pc" + race_string + "_" + gender_string);

		if (id_list) {
			bot = new Bot();
			memset(&bot->appearance, 0, sizeof(bot->appearance));
			bot->appearance.pos.collision_radius = 32;
			bot->secondary_command_list_id = 0;
			bot->primary_command_list_id = 0;
			bot->appearance.display_name = 1;
			bot->appearance.show_level = 1;
			bot->appearance.attackable = 1;
			bot->appearance.show_command_icon = 1;
			bot->appearance.targetable = 1;
			bot->appearance.race = race;
			bot->appearance.gender = gender;
			bot->SetID(Spawn::NextID());
			bot->SetX(client->GetPlayer()->GetX());
			bot->SetY(client->GetPlayer()->GetY());
			bot->SetZ(client->GetPlayer()->GetZ());
			bot->SetHeading(client->GetPlayer()->GetHeading());
			bot->SetSpawnOrigX(bot->GetX());
			bot->SetSpawnOrigY(bot->GetY());
			bot->SetSpawnOrigZ(bot->GetZ());
			bot->SetSpawnOrigHeading(bot->GetHeading());
			bot->SetLocation(client->GetPlayer()->GetLocation());
			bot->SetInitialState(16512);
			bot->SetModelType(id_list->at(0));
			bot->SetAdventureClass(advClass);
			bot->SetLevel(client->GetPlayer()->GetLevel());
			bot->SetName(name.c_str());
			bot->SetDifficulty(6);
			bot->size = 32;
			if (bot->GetTotalHP() == 0) {
				bot->SetTotalHP(25 * bot->GetLevel() + 1);
				bot->SetTotalHPBaseInstance(bot->GetTotalHP());
				bot->SetHP(25 * bot->GetLevel() + 1);
			}
			if (bot->GetTotalPower() == 0) {
				bot->SetTotalPower(25 * bot->GetLevel() + 1);
				bot->SetTotalPowerBaseInstance(bot->GetTotalPower());
				bot->SetPower(25 * bot->GetLevel() + 1);
			}
			bot->SetOwner(client->GetPlayer());
			bot->GetNewSpells();
			client->GetCurrentZone()->AddSpawn(bot);

			int32 index;
			int32 bot_id = database.CreateNewBot(client->GetCharacterID(), name, race, advClass, gender, id_list->at(0), index);
			if (bot_id == 0) {
				LogWrite(PLAYER__ERROR, 0, "Player", "Error saving bot to DB.  Bot was not saved!");
				client->SimpleMessage(CHANNEL_ERROR, "Error saving bot to DB.  Bot was not saved!");
			}
			else {
				bot->BotID = bot_id;
				bot->BotIndex = index;
				client->GetPlayer()->SpawnedBots[bot->BotIndex] = bot->GetID();

				// Add Items
				database.SetBotStartingItems(bot, advClass, race);
			}
		}
		else {
			client->SimpleMessage(CHANNEL_COLOR_RED, "Error finding the id list for your race, please verify the race id.");
		}

		safe_delete(id_list);
	}
	else
		client->SimpleMessage(CHANNEL_COLOR_RED, "Error finding the race string, please verify the race id.");
}

void Commands::Command_Bot_Customize(Client* client, Seperator* sep) {
	Bot* bot = 0;
	if (client->GetPlayer()->GetTarget() && client->GetPlayer()->GetTarget()->IsBot())
		bot = (Bot*)client->GetPlayer()->GetTarget();
	
	
	client->Message(CHANNEL_COLOR_RED, "This command is disabled and requires new implementation.");

	/*if (bot && bot->GetOwner() == client->GetPlayer()) {
		PacketStruct* packet = configReader.getStruct("WS_OpenCharCust", client->GetVersion());
		if (packet) {

			AppearanceData* botApp = &bot->appearance;
			CharFeatures* botFeatures = &bot->features;

			AppearanceData* playerApp = &client->GetPlayer()->appearance;
			CharFeatures* playerFeatures = &client->GetPlayer()->features;

			memcpy(&client->GetPlayer()->SavedApp, playerApp, sizeof(AppearanceData));
			memcpy(&client->GetPlayer()->SavedFeatures, playerFeatures, sizeof(CharFeatures));

			client->GetPlayer()->custNPC = true;
			client->GetPlayer()->custNPCTarget = bot;
			memcpy(playerApp, botApp, sizeof(AppearanceData));
			memcpy(playerFeatures, botFeatures, sizeof(CharFeatures));
			client->GetPlayer()->changed = true;
			client->GetPlayer()->info_changed = true;
			client->GetCurrentZone()->SendSpawnChanges(client->GetPlayer(), client);
			packet->setDataByName("race_id", 255);
			client->QueuePacket(packet->serialize());
		}
	}*/
}

void Commands::Command_Bot_Spawn(Client* client, Seperator* sep) {

	if (sep && sep->IsSet(0) && sep->IsNumber(0)) {
		int32 bot_id = atoi(sep->arg[0]);
		if (client->GetPlayer()->SpawnedBots.count(bot_id) > 0) {
			client->Message(CHANNEL_COLOR_YELLOW, "The bot with id %u is already spawned.", bot_id);
			return;
		}

		Bot* bot = new Bot();
		memset(&bot->appearance, 0, sizeof(bot->appearance));

		if (database.LoadBot(client->GetCharacterID(), bot_id, bot)) {
			bot->SetFollowTarget(client->GetPlayer(), 5);
			bot->appearance.pos.collision_radius = 32;
			bot->secondary_command_list_id = 0;
			bot->primary_command_list_id = 0;
			bot->appearance.display_name = 1;
			bot->appearance.show_level = 1;
			bot->appearance.attackable = 1;
			bot->appearance.show_command_icon = 1;
			bot->appearance.targetable = 1;
			bot->SetID(Spawn::NextID());
			bot->SetX(client->GetPlayer()->GetX());
			bot->SetY(client->GetPlayer()->GetY());
			bot->SetZ(client->GetPlayer()->GetZ());
			bot->SetHeading(client->GetPlayer()->GetHeading());
			bot->SetSpawnOrigX(bot->GetX());
			bot->SetSpawnOrigY(bot->GetY());
			bot->SetSpawnOrigZ(bot->GetZ());
			bot->SetSpawnOrigHeading(bot->GetHeading());
			bot->SetLocation(client->GetPlayer()->GetLocation());
			bot->SetInitialState(16512);
			bot->SetLevel(client->GetPlayer()->GetLevel());
			bot->SetDifficulty(6);
			bot->size = 32;
			if (bot->GetTotalHP() == 0) {
				bot->SetTotalHP(25 * bot->GetLevel() + 1);
				bot->SetHP(25 * bot->GetLevel() + 1);
			}
			if (bot->GetTotalPower() == 0) {
				bot->SetTotalPower(25 * bot->GetLevel() + 1);
				bot->SetPower(25 * bot->GetLevel() + 1);
			}
			bot->SetOwner(client->GetPlayer());
			bot->UpdateWeapons();
			bot->CalculateBonuses();
			bot->GetNewSpells();
			client->GetCurrentZone()->AddSpawn(bot);
			
			if (sep->IsSet(1) && sep->IsNumber(1) && atoi(sep->arg[1]) == 1) {
				client->GetCurrentZone()->SendSpawn(bot, client);
				
				int8 result = world.GetGroupManager()->Invite(client->GetPlayer(), bot);

				if (result == 0)
					client->Message(CHANNEL_COMMANDS, "You invite %s to group with you.", bot->GetName());
				else if (result == 1)
					client->SimpleMessage(CHANNEL_COMMANDS, "That player is already in a group.");
				else if (result == 2)
					client->SimpleMessage(CHANNEL_COMMANDS, "That player has been invited to another group.");
				else if (result == 3)
					client->SimpleMessage(CHANNEL_COMMANDS, "Your group is already full.");
				else if (result == 4)
					client->SimpleMessage(CHANNEL_COMMANDS, "You have a pending invitation, cancel it first.");
				else if (result == 5)
					client->SimpleMessage(CHANNEL_COMMANDS, "You cannot invite yourself!");
				else if (result == 6)
					client->SimpleMessage(CHANNEL_COMMANDS, "Could not locate the player.");
				else
					client->SimpleMessage(CHANNEL_COMMANDS, "Group invite failed, unknown error!");
			}

			client->GetPlayer()->SpawnedBots[bot_id] = bot->GetID();
		}
		else {
			client->Message(CHANNEL_ERROR, "Error spawning bot (%u)", bot_id);
		}
	}
	else {
		Command_Bot(client, sep);
	}
}

void Commands::Command_Bot_List(Client* client, Seperator* sep) {
	string bot_list;
	bot_list = database.GetBotList(client->GetCharacterID());
	if (!bot_list.empty())
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, bot_list.c_str());
}

void Commands::Command_Bot_Inv(Client* client, Seperator* sep) {
	if (sep && sep->IsSet(0)) {
		if (strncasecmp("give", sep->arg[0], 4) == 0) {
			if (client->GetPlayer()->trade) {
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You are already trading.");
				return;
			}

			if (!client->GetPlayer()->GetTarget() || !client->GetPlayer()->GetTarget()->IsBot()) {
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You must target a bot");
				return;
			}

			Bot* bot = (Bot*)client->GetPlayer()->GetTarget();
			if (bot->trade) {
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Bot is already in a trade...");
				return;
			}

			if (bot->GetOwner() != client->GetPlayer()) {
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You can only trade with your own bot.");
				return;
			}

			Trade* trade = new Trade(client->GetPlayer(), bot);
			client->GetPlayer()->trade = trade;
			bot->trade = trade;
		}
		else if (strncasecmp("list", sep->arg[0], 4) == 0) {
			if (!client->GetPlayer()->GetTarget() || !client->GetPlayer()->GetTarget()->IsBot()) {
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You must target a bot");
				return;
			}

			Bot* bot = (Bot*)client->GetPlayer()->GetTarget();
			if (bot->GetOwner() != client->GetPlayer()) {
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You can only see the inventory of your own bot.");
				return;
			}

			string item_list = "Bot Items:\nSlot\tName\n";
			for (int8 i = 0; i < NUM_SLOTS; i++) {
				Item* item = bot->GetEquipmentList()->GetItem(i);
				if (item) {
					//\\aITEM %u %u:%s\\/a
					item_list += to_string(i) + ":\t" + item->CreateItemLink(client->GetVersion(), true) + "\n";
				}
			}

			client->SimpleMessage(CHANNEL_COLOR_YELLOW, item_list.c_str());
		}
		else if (strncasecmp("remove", sep->arg[0], 6) == 0) {
			if (sep->IsSet(1) && sep->IsNumber(1)) {
				int8 slot = atoi(sep->arg[1]);
				if (slot >= NUM_SLOTS) {
					client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Invalid slot");
					return;
				}

				if (!client->GetPlayer()->GetTarget() || !client->GetPlayer()->GetTarget()->IsBot()) {
					client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You must target a bot.");
					return;
				}

				Bot* bot = (Bot*)client->GetPlayer()->GetTarget();
				if (bot->GetOwner() != client->GetPlayer()) {
					client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You can only remove items from your own bot.");
					return;
				}


				if (client->GetPlayer()->trade) {
					Trade* trade = client->GetPlayer()->trade;
					if (trade->GetTradee(client->GetPlayer()) != bot) {
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You are already in a trade.");
						return;
					}
					
					bot->AddItemToTrade(slot);
				}
				else {
					if (bot->trade) {
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Your bot is already trading...");
						return;
					}

					Trade* trade = new Trade(client->GetPlayer(), bot);
					client->GetPlayer()->trade = trade;
					bot->trade = trade;

					bot->AddItemToTrade(slot);
				}
			}
		}
		else
			Command_Bot(client, sep);
	}
	else
		Command_Bot(client, sep);
}

void Commands::Command_Bot_Settings(Client* client, Seperator* sep) {
	if (sep && sep->IsSet(0) && sep->IsSet(1) && sep->IsNumber(1)) {
		if (client->GetPlayer()->GetTarget() && client->GetPlayer()->GetTarget()->IsBot()) {
			Bot* bot = (Bot*)client->GetPlayer()->GetTarget();
			if (bot->GetOwner() == client->GetPlayer()) {
				if (strncasecmp("helm", sep->arg[0], 4) == 0) {
					bot->ShowHelm = (atoi(sep->arg[1]) == 1) ? true : false;
					bot->info_changed = true;
					bot->changed = true;
					bot->GetZone()->SendSpawnChanges(bot);
				}
				else if (strncasecmp("cloak", sep->arg[0], 5) == 0) {
					bot->ShowCloak = (atoi(sep->arg[1]) == 1) ? true : false;
					bot->info_changed = true;
					bot->changed = true;
					bot->GetZone()->SendSpawnChanges(bot);
				}
				else if (strncasecmp("taunt", sep->arg[0], 5) == 0) {
					bot->CanTaunt = (atoi(sep->arg[1]) == 1) ? true : false;
				}
				else if (strncasecmp("hood", sep->arg[0], 4) == 0) {
					bot->SetHideHood((atoi(sep->arg[0]) == 1) ? 0 : 1);
				}
				else
					Command_Bot(client, sep);
			}
			else
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You can only change settings on your own bot.");
		}
		else
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You must target a bot.");
	}
	else
		Command_Bot(client, sep);
}

void Commands::Command_Bot_Help(Client* client, Seperator* sep) {
	if (sep && sep->IsSet(0)) {
		if (strncasecmp("race", sep->arg[0], 4) == 0) {
			string title = "Race ID's";
			string details;
			details += "0\tBarbarian\n";
			details += "1\tDark Elf\n";
			details += "2\tDwarf\n";
			details += "3\tErudite\n";
			details += "4\tFroglok\n";
			details += "5\tGnome\n";
			details += "6\tHalf Elf\n";
			details += "7\tHalfling\n";
			details += "8\tHigh Elf\n";
			details += "9\tHuman\n";
			details += "10\tIksar\n";
			details += "11\tKerra\n";
			details += "12\tOgre\n";
			details += "13\tRatonga\n";
			details += "14\tTroll\n";
			details += "15\tWood Elf\n";
			details += "16\tFae\n";
			details += "17\tArasai\n";
			details += "18\tSarnak\n";
			details += "19\tVampire\n";
			details += "20\tAerakyn\n";
			client->SendShowBook(client->GetPlayer(), title, 0, 1, details);
			return;
		}
		else if (strncasecmp("class", sep->arg[0], 5) == 0) {
			string title = "Class ID's";
			string details;
			details += "0\tCOMMONER\n";
			details += "1\tFIGHTER\n";
			details += "2\tWARRIOR\n";
			details += "3\tGUARDIAN\n";
			details += "4\tBERSERKER\n";
			details += "5\tBRAWLER\n";
			details += "6\tMONK\n";
			details += "7\tBRUISER\n";
			details += "8\tCRUSADER\n";
			details += "9\tSHADOWKNIGHT\n";
			details += "10\tPALADIN\n";
			details += "11\tPRIEST\n";
			details += "12\tCLERIC\n";
			details += "13\tTEMPLAR\n";
			details += "14\tINQUISITOR\n";
			details += "15\tDRUID\n";
			details += "16\tWARDEN\n";
			details += "17\tFURY\n";
			details += "18\tSHAMAN\n";
			details += "19\tMYSTIC\n";
			details += "20\tDEFILER\n";

			string details2 = "21\tMAGE\n";
			details2 += "22\tSORCERER\n";
			details2 += "23\tWIZARD\n";
			details2 += "24\tWARLOCK\n";
			details2 += "25\tENCHANTER\n";
			details2 += "26\tILLUSIONIST\n";
			details2 += "27\tCOERCER\n";
			details2 += "28\tSUMMONER\n";
			details2 += "29\tCONJUROR\n";
			details2 += "30\tNECROMANCER\n";
			details2 += "31\tSCOUT\n";
			details2 += "32\tROGUE\n";
			details2 += "33\tSWASHBUCKLER\n";
			details2 += "34\tBRIGAND\n";
			details2 += "35\tBARD\n";
			details2 += "36\tTROUBADOR\n";
			details2 += "37\tDIRGE\n";
			details2 += "38\tPREDATOR\n";
			details2 += "39\tRANGER\n";
			details2 += "40\tASSASSIN\n";

			string details3 = "\\#FF0000Following aren't implemented yet.\\#000000\n";
			details3 += "41\tANIMALIST\n";
			details3 += "42\tBEASTLORD\n";
			details3 += "43\tSHAPER\n";
			details3 += "44\tCHANNELER\n";

			client->SendShowBook(client->GetPlayer(), title, 0, 3, details, details2, details3);
			return;
		}
	}
	else {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Bot help is WIP.");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/bot help race - race id list");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/bot help class - class id list");
	}
}
