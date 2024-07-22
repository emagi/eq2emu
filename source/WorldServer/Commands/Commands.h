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
#ifndef __EQ2_COMMANDS__
#define __EQ2_COMMANDS__
#include "../../common/DataBuffer.h"
#include "../../common/MiscFunctions.h"
#include "../../common/types.h"
#include "../../common/opcodemgr.h"
#include <vector>
#include <string>
#include <map>
#include "../../common/debug.h"
using namespace std;

class Client;
class Spawn;
class ZoneServer;
extern map<int16,OpcodeManager*>EQOpcodeManager;

#define CHANNEL_COLOR_RED				3
#define CHANNEL_COLOR_CHAT_RELATIONSHIP	4
#define CHANNEL_COLOR_YELLOW			5
#define CHANNEL_COLOR_NEW_LOOT			84
#define CHANNEL_COLOR_NEWEST_LOOT		89

#define UPDATE_COLOR_WHITE				254  // For UpdateText

#define CHANNEL_ALL_TEXT					0
#define CHANNEL_GAME_TEXT					1
#define CHANNEL_DEFAULT						2
#define CHANNEL_ERROR						3
#define CHANNEL_STATUS						4
#define CHANNEL_MOTD						5
#define CHANNEL_CHAT_TEXT					6
#define CHANNEL_NEARBY_CHAT					7
#define CHANNEL_SAY							8
#define CHANNEL_SHOUT						9
#define CHANNEL_EMOTE						10
#define CHANNEL_YELL						11
#define CHANNEL_NARRATIVE					12 //white
#define CHANNEL_NONPLAYER_SAY				13
#define CHANNEL_GROUP_CHAT					14
#define CHANNEL_GROUP_SAY					15 // Use this for group chat
#define CHANNEL_RAID_SAY					16
#define CHANNEL_GUILD_CHAT					17
#define CHANNEL_GUILD_SAY					18 // Use this for guild chat
#define CHANNEL_OFFICER_SAY					19
#define CHANNEL_GUILD_MOTD					20
#define CHANNEL_GUILD_MEMBER_ONLINE			21
#define CHANNEL_GUILD_EVENT					22
#define CHANNEL_GUILD_RECRUITING_PAGE		23
#define CHANNEL_GUILD_RECRUITING_PAGE_OTHER	24
#define CHANNEL_PRIVATE_CHAT				25
#define CHANNEL_NONPLAYER_TELL				26
#define CHANNEL_OBJECT_TEXT					27
#define CHANNEL_PRIVATE_TELL				28
#define CHANNEL_TELL_FROM_CS				29
#define CHANNEL_ARENA						30
#define CHANNEL_CHAT_CHANNEL_TEXT			31
#define CHANNEL_OUT_OF_CHARACTER			32
#define CHANNEL_AUCTION						33
#define CHANNEL_CUSTOM_CHANNEL				34 // 34 is nothing, message with 34 as type will not show on client
#define CHANNEL_CHARACTER_TEXT				35
#define CHANNEL_REWARD						36
#define CHANNEL_DEATH						37
#define CHANNEL_PET_CHAT					38
#define CHANNEL_SKILL						39
#define CHANNEL_FACTION						40
// Combat related chat channels start here
#define CHANNEL_SPELLS						41
#define CHANNEL_YOU_CAST					42
#define CHANNEL_YOU_FAIL					43
#define CHANNEL_CRITICAL_CAST				44
#define CHANNEL_FRIENDLY_CAST				45
#define CHANNEL_FRIENDLY_FAIL				46
#define CHANNEL_OTHER_CAST					47
#define CHANNEL_OTHER_FAIL					48
#define CHANNEL_HOSTILE_CAST				49
#define CHANNEL_HOSTILE_FAIL				50
#define CHANNEL_WORN_OFF					51
#define CHANNEL_SPELLS_OTHER				52
#define CHANNEL_HEAL_SPELLS					53
#define CHANNEL_HEALS						54
#define CHANNEL_FRIENDLY_HEALS				55
#define CHANNEL_OTHER_HEALS					56
#define CHANNEL_HOSTILE_HEALS				57
#define CHANNEL_CRITICAL_HEALS				58
#define CHANNEL_COMBAT						59
#define CHANNEL_GENERAL_COMBAT				60
#define CHANNEL_HEROIC_OPPORTUNITY			61
#define CHANNEL_NON_MELEE_DAMAGE			62
#define CHANNEL_DAMAGE_SHIELD				63
#define CHANNEL_WARD						64
#define CHANNEL_DAMAGE_INTERCEPT			65
#define CHANNEL_MELEE_COMBAT				66
#define CHANNEL_WARNINGS					67
#define CHANNEL_YOU_HIT						68
#define CHANNEL_YOU_MISS					69
#define CHANNEL_ATTACKER_HITS				70
#define CHANNEL_ATTACKER_MISSES				71
#define CHANNEL_YOUR_PET_HITS				72
#define CHANNEL_YOUR_PET_MISSES				73
#define CHANNEL_ATTACKER_HITS_PET			74
#define CHANNEL_ATTACKER_MISSES_PET			75
#define CHANNEL_OTHER_HIT					76
#define CHANNEL_OTHER_MISSES				77
#define CHANNEL_CRITICAL_HIT				78
#define CHANNEL_HATE_ADJUSTMENTS			79
#define CHANNEL_YOUR_HATE					80
#define CHANNEL_OTHERS_HATE					81
#define CHANNEL_DISPELS_AND_CURES			82
#define CHANNEL_DISPEL_YOU					83
#define CHANNEL_DISPEL_OTHER				84
#define CHANNEL_CURE_YOU					85
#define CHANNEL_CURE_OTHER					86
// End of combat chat channels
#define CHANNEL_OTHER						87
#define CHANNEL_MONEY_SPLIT					88
#define CHANNEL_LOOT						89
#define CHANNEL_LOOT_ROLLS					90
#define CHANNEL_COMMAND_TEXT				91
#define CHANNEL_BROADCAST					92  // Goes to all chat windows no matter what
#define CHANNEL_WHO							93
#define CHANNEL_COMMANDS					94
#define CHANNEL_MERCHANT					95
#define CHANNEL_MERCHANT_BUY_SELL			96
#define CHANNEL_CONSIDER_MESSAGE			97
#define CHANNEL_CON_MINUS_2					98
#define CHANNEL_CON_MINUS_1					99
#define CHANNEL_CON_0						100
#define CHANNEL_CON_1						101
#define CHANNEL_CON_2						102
#define CHANNEL_TRADESKILLS					103
#define CHANNEL_HARVESTING					104
#define CHANNEL_HARVESTING_WARNINGS			105
// 106 is nothing, message sent with this channel won't display in the client
#define CHANNEL_VOICE_CHAT					107
// 108+ will crash the client DO NOT USE

/* Using this in the /zone details command so that we do not have to store a whole zone in memory while changing zone attributes.  Also,
   ran into a problem when deleting a zone pointer (for zones that were not running), it would try to shut down a zone which was not
   running, causing world to crash. */
struct ZoneInfo {
	int32	id;
	int8	expansion_id;
	char	name[64];
	char	file[64];
	char	description[256];
	float	safe_x;
	float	safe_y;
	float	safe_z;
	float	underworld;
	int8	min_recommended;
	int8	max_recommended;
	char	zone_type[64];
	bool	always_loaded;
	bool	city_zone;
	sint16	min_status;
	int16	min_level;
	int16	max_level;
	int8	start_zone;
	int8	instance_type;
	int32	default_reenter_time;
	int32	default_reset_time;
	int32	default_lockout_time;
	int8	force_group_to_zone;
	char	lua_script[256];
	int32	shutdown_timer;
	char	zone_motd[256];
	float	xp_modifier;
	int16	min_version;
	bool	weather_allowed;
	int32	ruleset_id;
	char	sky_file[64];
};

class EQ2_CommandString : public DataBuffer{
public:
	EQ2_CommandString(){ handler = 0; }
	EQ2_CommandString(uchar* buffer, int32 size){
		InitializeLoadData(buffer, size);
		LoadData(handler);
		LoadDataString(command);
	}
	EQ2_16BitString command;
	int16	handler;
};
class EQ2_RemoteCommandString : public DataBuffer{
public:
	EQ2_RemoteCommandString(){ handler = 0; }
	EQ2_RemoteCommandString(char* name, int32 in_handler, sint16 in_status){ 
		command.data = string(name);
		command.size = command.data.length();
		handler = in_handler; 
		required_status = in_status;
	}
	EQ2_RemoteCommandString(uchar* buffer, int32 size){
		required_status = 0;
		InitializeLoadData(buffer, size);
		LoadData(handler);
		LoadDataString(command);
	}

	EQ2_8BitString command;
	int16	handler;
	sint16	required_status;
};
class RemoteCommands {
public:
	RemoteCommands(){ num_commands = 0; buffer.clear(); }
	int16 num_commands;
	vector<EQ2_RemoteCommandString>	commands;
	void addCommand(EQ2_RemoteCommandString add){ commands.push_back(add); num_commands++;}
	void AddSubCommand(string command, EQ2_RemoteCommandString subcommand){ 
		subcommands[command][subcommand.command.data] = subcommand; 
	}
	bool validSubCommand(string command, string subcommand){
		if(subcommands.count(command) > 0 && subcommands[command].count(subcommand) > 0)
			return true;
		return false;
	}
	void addZero(){
		num_commands++;
		EQ2_RemoteCommandString add;
		add.handler = 0;
		add.required_status = 300;
		add.command.size = 0;
		commands.push_back(add);
	}
	void CheckAddSubCommand(string command, EQ2_RemoteCommandString subcommand){
		vector<EQ2_RemoteCommandString>::iterator itr;
		for(itr = commands.begin(); itr != commands.end();itr++){
			if((*itr).command.data == command){
				AddSubCommand(command, subcommand);
				return;
			}
		}
		// TODO: cannot seem to use LogWrite in this .h file!
		printf("Unable to find parent command '%s' for subcommand: '%s'\n\tEvery subcommand must have a parent command!", command.c_str(), subcommand.command.data.c_str());
	}
	void AddDataCommand(EQ2_RemoteCommandString* command){
		buffer.append((char*)&command->command.size, sizeof(command->command.size));
		if(command->command.size>0)
			buffer.append(command->command.data);
	}
	int32 GetCommandHandler(const char* name){
		if(!name)
			return 0xFFFFFFFF;
		int8 name_size = strlen(name);
		for(int32 i = 0; i < commands.size(); i++){
			if(commands[i].command.size > 0){
				if(strncasecmp(commands[i].command.data.c_str(), name, name_size) == 0)
					return commands[i].handler;
			}
		}
		return 0xFFFFFFFF;
	}
	string			buffer;
	EQ2Packet*	serialize(int16 version = 0);
	map<string, map <string, EQ2_RemoteCommandString> > subcommands;
};
class Commands{
public:
	Commands();
	~Commands();
	bool SetSpawnCommand(Client* client, Spawn* target, int8 type, const char* value, bool send_update = true, bool temporary = false, string* temp_value = 0, int8 index = 0);
	void UpdateDatabaseAppearance(Client* client, Spawn* target, string fieldName, sint8 r, sint8 g, sint8 b);
	bool SetZoneCommand(Client* client, int32 zone_id, ZoneServer* zone, int8 type, const char* value);
	RemoteCommands* GetRemoteCommands() { return remote_commands; }
	void	Process(int32 index, EQ2_16BitString* command_parms, Client* client, Spawn* targetOverride=NULL);
	int32 GetCommandHandler(const char* name){
		return remote_commands->GetCommandHandler(name);
	}
	int32	GetSpawnSetType(string val);

	// JA: New Command handlers
	void Command_AcceptAdvancement(Client* client, Seperator* sep);
	void Command_AFK(Client* client, Seperator* sep);
	void Command_Appearance(Client* client, Seperator* sep, int handler);
	void Command_CancelMaintained(Client* client, Seperator* sep);
	void Command_Claim(Client* client, Seperator* sep);
	void Command_ClearAllQueued(Client* client);
	void Command_Create(Client* client, Seperator* sep);
	void Command_CreateFromRecipe(Client* client, Seperator* sep);
	void Command_Distance(Client* client);
	void Command_Duel(Client* client, Seperator* sep);
	void Command_DuelBet(Client* client, Seperator* sep);
	void Command_DuelAccept(Client* client, Seperator* sep);
	void Command_DuelDecline(Client* client, Seperator* sep);
	void Command_DuelSurrender(Client* client, Seperator* sep);
	void Command_DuelToggle(Client* client, Seperator* sep);
	void Command_EntityCommand(Client* client, Seperator* sep, int handler);
	void Command_Follow(Client* client, Seperator* sep);
	void Command_StopFollow(Client* client, Seperator* sep);
	void Command_Grid(Client* client, Seperator* sep);
	void Command_Guild(Client* client, Seperator* sep);
	void Command_CreateGuild(Client* client);
	void Command_SetGuildOfficerNote(Client* client, Seperator* sep);
	void Command_SetGuildMemberNote(Client* client, Seperator* sep);
	void Command_OfficerSay(Client* client, Seperator* sep);
	void Command_GuildSay(Client* client, Seperator* sep);
	void Command_Guilds(Client* client);
	void Command_GuildsAdd(Client* client, Seperator* sep);
	void Command_GuildsCreate(Client* client, Seperator* sep);
	void Command_GuildsDelete(Client* client, Seperator* sep);
	void Command_GuildsList(Client* client);
	void Command_GuildsRemove(Client* client, Seperator* sep);
	void Command_InspectPlayer(Client* client, Seperator* sep);
	void Command_Inventory(Client* client, Seperator* sep, EQ2_RemoteCommandString* command);
	void Command_Languages(Client* client, Seperator* sep);
	void Command_SetLanguage(Client* client, Seperator* sep);
	void Command_LastName(Client* client, Seperator* sep);
	void Command_ConfirmLastName(Client* client, Seperator* sep);
	void Command_Location(Client* client);
	void Command_LocationAdd(Client* client, Seperator* sep);
	void Command_LocationCreate(Client* client, Seperator* sep);
	void Command_LocationDelete(Client* client, Seperator* sep);
	void Command_LocationList(Client* client, Seperator* sep);
	void Command_LocationRemove(Client* client, Seperator* sep);
	void Command_Merchant(Client* client, Seperator* sep, int handler);
	//devn00b
	void Command_Mood(Client* client, Seperator* sep);

	void Command_Modify(Client* client); // usage function
	void Command_ModifyCharacter(Client* client, Seperator* sep);
	void Command_ModifyFaction(Client* client, Seperator* sep);
	void Command_ModifyGuild(Client* client, Seperator* sep);
	void Command_ModifyItem(Client* client, Seperator* sep);
	void Command_ModifyQuest(Client* client, Seperator* sep);
	void Command_ModifySkill(Client* client, Seperator* sep);
	void Command_ModifySpawn(Client* client, Seperator* sep);
	void Command_ModifySpell(Client* client, Seperator* sep);
	void Command_ModifyZone(Client* client, Seperator* sep);

	void Command_MOTD(Client* client);
	void Command_Pet(Client* client, Seperator* sep);
	void Command_PetName(Client* client, Seperator* sep);
	void Command_NamePet(Client* client, Seperator* sep);
	void Command_Rename(Client* client, Seperator* sep);
	void Command_ConfirmRename(Client* client, Seperator* sep);
	void Command_PetOptions(Client* client, Seperator* sep);
	void Command_Random(Client* client, Seperator* sep);
	void Command_Randomize(Client* client, Seperator* sep);
	void Command_ReportBug(Client* client, Seperator* sep);
	void Command_ShowCloak(Client* client, Seperator* sep);
	void Command_ShowHelm(Client* client, Seperator* sep);
	void Command_ShowHood(Client* client, Seperator* sep);
	void Command_ShowHoodHelm(Client* client, Seperator* sep);
	void Command_ShowRanged(Client* client, Seperator* sep);
	void Command_Skills(Client* client, Seperator* sep, int handler);
	void Command_SpawnTemplate(Client* client, Seperator* sep);
	void Command_Speed(Client* client, Seperator* sep);
	void Command_StationMarketPlace(Client* client, Seperator* sep);
	void Command_StopEating(Client* client);
	void Command_StopDrinking(Client* client);
	void Command_Test(Client* client, EQ2_16BitString* command_parms);
	void Command_Title(Client* client);
	void Command_TitleList(Client* client);
	void Command_TitleSetPrefix(Client* client, Seperator* sep);
	void Command_TitleSetSuffix(Client* client, Seperator* sep);
	void Command_TitleFix(Client* client, Seperator* sep);
	void Command_Toggle_Anonymous(Client* client);
	void Command_Toggle_AutoConsume(Client* client, Seperator* sep);
	void Command_Toggle_BonusXP(Client* client);
	void Command_Toggle_CombatXP(Client* client);
	void Command_Toggle_GMHide(Client* client);
	void Command_Toggle_GMVanish(Client* client);
	void Command_Toggle_Illusions(Client* client, Seperator* sep);
	void Command_Toggle_LFG(Client* client);
	void Command_Toggle_LFW(Client* client);
	void Command_Toggle_QuestXP(Client* client);
	void Command_Toggle_Roleplaying(Client* client);
	void Command_Toggle_Duels(Client* client);
	void Command_Toggle_Trades(Client* client);
	void Command_Toggle_Guilds(Client* client);
	void Command_Toggle_Groups(Client* client);
	void Command_Toggle_Raids(Client* client);
	void Command_Toggle_LON(Client* client);
	void Command_Toggle_VoiceChat(Client* client);
	void Command_Track(Client* client);
	void Command_TradeStart(Client* client, Seperator* sep);
	void Command_TradeAccept(Client* client, Seperator* sep);
	void Command_TradeReject(Client* client, Seperator* sep);
	void Command_TradeCancel(Client* client, Seperator* sep);
	void Command_TradeSetCoin(Client* client, Seperator* sep);
	void Command_TradeAddCoin(Client* client, Seperator* sep, int handler);
	void Command_TradeRemoveCoin(Client* client, Seperator* sep, int handler);
	void Command_TradeAddItem(Client* client, Seperator* sep);
	void Command_TradeRemoveItem(Client* client, Seperator* sep);
	void Command_TryOn(Client* client, Seperator* sep);
	void Command_JoinChannel(Client *client, Seperator *sep);
	void Command_JoinChannelFromLoad(Client *client, Seperator *sep);
	void Command_TellChannel(Client *client, Seperator *sep);
	void Command_LeaveChannel(Client *client, Seperator *sep);
	void Command_WeaponStats(Client *client);
	void Command_WhoChannel(Client *client, Seperator *sep);
	void Command_ZoneSafeCoords(Client *client, Seperator *sep);
	void Command_ZoneDetails(Client *client, Seperator *sep);
	void Command_ZoneSet(Client *client, Seperator *sep);
	void Command_Rain(Client* client, Seperator* sep);
	void Command_Wind(Client* client, Seperator* sep);
	void Command_SendMerchantWindow(Client* client, Seperator* sep, bool sell = false);
	void Command_Weather(Client* client, Seperator* sep);
	void Command_Select(Client* client, Seperator* sep);
	void Command_ConsumeFood(Client* client, Seperator* sep);
	void Command_Aquaman(Client* client, Seperator* sep);
	void Command_Attune_Inv(Client* client, Seperator* sep);
	void Command_Player(Client* client, Seperator* sep);
	void Command_Player_Coins(Client* client, Seperator* sep);
	void Command_Reset_Zone_Timer(Client* client, Seperator* sep);
	void Command_AchievementAdd(Client* client, Seperator* sep);
	void Command_Editor(Client* client, Seperator* sep);
	void Command_AcceptResurrection(Client* client, Seperator* sep);
	void Command_DeclineResurrection(Client* client, Seperator* set);
	void Command_TargetItem(Client* client, Seperator* set);

	void Command_FindSpawn(Client* client, Seperator* set);

	void Command_MoveCharacter(Client* client, Seperator* set);

	// Bot Commands
	void Command_Bot(Client* client, Seperator* sep);
	void Command_Bot_Create(Client* client, Seperator* sep);
	void Command_Bot_Customize(Client* client, Seperator* sep);
	void Command_Bot_Spawn(Client* client, Seperator* sep);
	void Command_Bot_List(Client* client, Seperator* sep);
	void Command_Bot_Inv(Client* client, Seperator* sep);
	void Command_Bot_Settings(Client* client, Seperator* sep);
	void Command_Bot_Help(Client* client, Seperator* sep);
	
	void Command_CancelEffect(Client* client, Seperator* sep);
	void Command_CurePlayer(Client* client, Seperator* sep);
	void Command_ShareQuest(Client* client, Seperator* sep);
	void Command_Yell(Client* client, Seperator* sep);
	void Command_SetAutoLootMode(Client* client, Seperator* sep);
	void Command_AutoAttack(Client* client, Seperator* sep);
	void Command_Assist(Client* client, Seperator* sep);
	void Command_Target(Client* client, Seperator* sep);
	void Command_Target_Pet(Client* client, Seperator* sep);

	// AA Commands
	void Get_AA_Xml(Client* client, Seperator* sep);
	void Add_AA(Client* client, Seperator* sep);
	void Commit_AA_Profile(Client* client, Seperator* sep);
	void Begin_AA_Profile(Client* client, Seperator* sep);
	void Back_AA(Client* client, Seperator* sep);
	void Remove_AA(Client* client, Seperator* sep);
	void Switch_AA_Profile(Client* client, Seperator* sep);
	void Cancel_AA_Profile(Client* client, Seperator* sep);
	void Save_AA_Profile(Client* client, Seperator* sep);
private:
	RemoteCommands* remote_commands;
	map<string, int8> spawn_set_values;
	map<string, int8> zone_set_values;
};
#define SPAWN_SET_VALUE_LIST				0
#define SPAWN_SET_VALUE_NAME				1
#define SPAWN_SET_VALUE_LEVEL				2
#define SPAWN_SET_VALUE_DIFFICULTY			3
#define SPAWN_SET_VALUE_MODEL_TYPE			4
#define SPAWN_SET_VALUE_CLASS				5
#define SPAWN_SET_VALUE_GENDER				6
#define SPAWN_SET_VALUE_SHOW_NAME			7
#define SPAWN_SET_VALUE_ATTACKABLE			8
#define SPAWN_SET_VALUE_SHOW_LEVEL			9
#define SPAWN_SET_VALUE_TARGETABLE			10
#define SPAWN_SET_VALUE_SHOW_COMMAND_ICON	11
#define SPAWN_SET_VALUE_HAND_ICON			12
#define SPAWN_SET_VALUE_HAIR_TYPE			13
#define SPAWN_SET_VALUE_FACIAL_HAIR_TYPE	14
#define SPAWN_SET_VALUE_WING_TYPE			15
#define SPAWN_SET_VALUE_CHEST_TYPE			16
#define SPAWN_SET_VALUE_LEGS_TYPE			17
#define SPAWN_SET_VALUE_SOGA_HAIR_TYPE		18
#define SPAWN_SET_VALUE_SOGA_FACIAL_HAIR_TYPE	19
#define SPAWN_SET_VALUE_SOGA_MODEL_TYPE		20
#define SPAWN_SET_VALUE_SIZE				21
#define SPAWN_SET_VALUE_HP					22
#define SPAWN_SET_VALUE_POWER				23
#define SPAWN_SET_VALUE_HEROIC				24
#define SPAWN_SET_VALUE_RESPAWN				25
#define SPAWN_SET_VALUE_X					26
#define SPAWN_SET_VALUE_Y					27
#define SPAWN_SET_VALUE_Z					28
#define SPAWN_SET_VALUE_HEADING				29
#define SPAWN_SET_VALUE_LOCATION			30
#define SPAWN_SET_VALUE_COMMAND_PRIMARY		31
#define SPAWN_SET_VALUE_COMMAND_SECONDARY	32
#define SPAWN_SET_VALUE_VISUAL_STATE		33
#define SPAWN_SET_VALUE_ACTION_STATE		34
#define SPAWN_SET_VALUE_MOOD_STATE			35
#define SPAWN_SET_VALUE_INITIAL_STATE		36
#define SPAWN_SET_VALUE_ACTIVITY_STATE		37
#define SPAWN_SET_VALUE_COLLISION_RADIUS	38
#define SPAWN_SET_VALUE_FACTION				39
#define SPAWN_SET_VALUE_SPAWN_SCRIPT		40
#define SPAWN_SET_VALUE_SPAWNENTRY_SCRIPT	41
#define SPAWN_SET_VALUE_SPAWNLOCATION_SCRIPT 42
#define SPAWN_SET_VALUE_SUB_TITLE			43
#define SPAWN_SET_VALUE_EXPIRE				45
#define SPAWN_SET_VALUE_EXPIRE_OFFSET		46
#define SPAWN_SET_VALUE_X_OFFSET			47
#define SPAWN_SET_VALUE_Y_OFFSET			48
#define SPAWN_SET_VALUE_Z_OFFSET			49
#define SPAWN_SET_VALUE_DEVICE_ID			50
#define SPAWN_SET_VALUE_PITCH               51
#define SPAWN_SET_VALUE_ROLL                52
#define SPAWN_SET_VALUE_HIDE_HOOD           53
#define SPAWN_SET_VALUE_EMOTE_STATE         54
#define SPAWN_SET_VALUE_ICON                55
#define SPAWN_SET_VALUE_PREFIX              56
#define SPAWN_SET_VALUE_SUFFIX              57
#define SPAWN_SET_VALUE_LASTNAME            58
#define SPAWN_SET_VALUE_EXPANSION_FLAG      59
#define SPAWN_SET_VALUE_MERCHANT_MIN_LEVEL  60
#define SPAWN_SET_VALUE_MERCHANT_MAX_LEVEL  61
#define SPAWN_SET_VALUE_HOLIDAY_FLAG		62
#define SPAWN_SET_SKIN_COLOR				63
#define SPAWN_SET_AAXP_REWARDS				64

#define SPAWN_SET_HAIR_COLOR1							65
#define SPAWN_SET_HAIR_COLOR2							66
#define SPAWN_SET_HAIR_TYPE_COLOR						67
#define SPAWN_SET_HAIR_FACE_COLOR						68
#define SPAWN_SET_HAIR_TYPE_HIGHLIGHT_COLOR				69
#define SPAWN_SET_HAIR_FACE_HIGHLIGHT_COLOR				70
#define SPAWN_SET_HAIR_HIGHLIGHT						71
#define SPAWN_SET_MODEL_COLOR							72
#define SPAWN_SET_EYE_COLOR								73
#define SPAWN_SET_SOGA_SKIN_COLOR						74
#define SPAWN_SET_SOGA_HAIR_COLOR1						75
#define SPAWN_SET_SOGA_HAIR_COLOR2						76
#define SPAWN_SET_SOGA_HAIR_TYPE_COLOR					77
#define SPAWN_SET_SOGA_HAIR_FACE_COLOR					78
#define SPAWN_SET_SOGA_HAIR_TYPE_HIGHLIGHT_COLOR		79
#define SPAWN_SET_SOGA_HAIR_FACE_HIGHLIGHT_COLOR		80
#define SPAWN_SET_SOGA_HAIR_HIGHLIGHT					81
#define SPAWN_SET_SOGA_MODEL_COLOR						82
#define SPAWN_SET_SOGA_EYE_COLOR						83

#define SPAWN_SET_CHEEK_TYPE							84
#define SPAWN_SET_CHIN_TYPE								85
#define SPAWN_SET_EAR_TYPE								86
#define SPAWN_SET_EYE_BROW_TYPE							87
#define SPAWN_SET_EYE_TYPE								88
#define SPAWN_SET_LIP_TYPE								89
#define SPAWN_SET_NOSE_TYPE								90
#define SPAWN_SET_BODY_SIZE								91
#define SPAWN_SET_BODY_AGE								92
#define SPAWN_SET_SOGA_CHEEK_TYPE						93
#define SPAWN_SET_SOGA_CHIN_TYPE						94
#define SPAWN_SET_SOGA_EAR_TYPE							95
#define SPAWN_SET_SOGA_EYE_BROW_TYPE					96
#define SPAWN_SET_SOGA_EYE_TYPE							97
#define SPAWN_SET_SOGA_LIP_TYPE							98
#define SPAWN_SET_SOGA_NOSE_TYPE						99
#define SPAWN_SET_SOGA_BODY_SIZE						100
#define SPAWN_SET_SOGA_BODY_AGE							101

#define SPAWN_SET_ATTACK_TYPE							102
#define SPAWN_SET_RACE_TYPE								103
#define SPAWN_SET_LOOT_TIER								104
#define SPAWN_SET_LOOT_DROP_TYPE						105
#define SPAWN_SET_SCARED_STRONG_PLAYERS					106

#define ZONE_SET_VALUE_EXPANSION_ID			0
#define ZONE_SET_VALUE_NAME					1
#define ZONE_SET_VALUE_FILE					2
#define ZONE_SET_VALUE_DESCRIPTION			3
#define ZONE_SET_VALUE_SAFE_X				4
#define ZONE_SET_VALUE_SAFE_Y				5
#define ZONE_SET_VALUE_SAFE_Z				6
#define ZONE_SET_VALUE_UNDERWORLD			7
#define ZONE_SET_VALUE_MIN_RECOMMENDED		8
#define ZONE_SET_VALUE_MAX_RECOMMENDED		9
#define ZONE_SET_VALUE_ZONE_TYPE			10
#define ZONE_SET_VALUE_ALWAYS_LOADED		11
#define ZONE_SET_VALUE_CITY_ZONE			12
#define ZONE_SET_VALUE_MIN_STATUS			13
#define ZONE_SET_VALUE_MIN_LEVEL			14
#define ZONE_SET_VALUE_START_ZONE			15
#define ZONE_SET_VALUE_INSTANCE_TYPE		16
#define ZONE_SET_VALUE_DEFAULT_REENTER_TIME	17
#define ZONE_SET_VALUE_DEFAULT_RESET_TIME	18
#define ZONE_SET_VALUE_DEFAULT_LOCKOUT_TIME	19
#define ZONE_SET_VALUE_FORCE_GROUP_TO_ZONE	20
#define ZONE_SET_VALUE_LUA_SCRIPT			21
#define ZONE_SET_VALUE_SHUTDOWN_TIMER		22
#define ZONE_SET_VALUE_ZONE_MOTD			23
#define ZONE_SET_VALUE_MAX_LEVEL			24
#define ZONE_SET_VALUE_WEATHER_ALLOWED		25

#define COMMAND_SPAWN				1
#define COMMAND_RACE				2
#define COMMAND_LEVEL				3
#define COMMAND_CLASS				4
#define COMMAND_NAME				6
#define	COMMAND_SAY					7
#define	COMMAND_TELL				8
#define COMMAND_YELL				9
#define COMMAND_SHOUT				10
#define COMMAND_AUCTION				11
#define COMMAND_OOC					12
#define COMMAND_EMOTE				13
#define COMMAND_GROUPSAY			14
#define COMMAND_GROUPINVITE			15
#define COMMAND_GROUPDISBAND		16
#define COMMAND_GROUP				17
#define COMMAND_CLAIM				18
#define COMMAND_HAIL				19
#define COMMAND_ZONE				20
#define COMMAND_ADMINFLAG			21
#define COMMAND_KICK				22
#define COMMAND_BAN					23
#define COMMAND_INVENTORY			24
#define COMMAND_SUMMONITEM			25
#define COMMAND_FOLLOW				26
#define COMMAND_STOP_FOLLOW			27
#define COMMAND_LASTNAME			28
#define COMMAND_CONFIRMLASTNAME		29
#define COMMAND_COLLECTION_ADDITEM	30
#define COMMAND_COLLECTION_FILTER_MATCHITEM	31
#define COMMAND_MOVE				32
#define COMMAND_INFO				33
#define COMMAND_USEABILITY			34
#define COMMAND_ENABLE_ABILITY_QUE	35
#define COMMAND_RELOAD_ITEMS		36
#define COMMAND_AUTO_ATTACK			37
#define COMMAND_WEATHER				38
#define COMMAND_SPEED				39
#define COMMAND_SPAWN_MOVE			40
#define COMMAND_WHO					41
#define COMMAND_VERSION				42
#define COMMAND_SPAWN_ADD			43
#define COMMAND_SPAWN_CREATE		44
#define COMMAND_SPAWN_SET			45
#define COMMAND_SPAWN_REMOVE		46
#define COMMAND_SPAWN_LIST			47
#define COMMAND_SIT					48
#define COMMAND_STAND				49
#define COMMAND_SPAWN_TARGET		50
#define COMMAND_SPAWN_EQUIPMENT		51
#define COMMAND_SPAWN_DETAILS		52
#define COMMAND_SELECT_JUNCTION		53
#define COMMAND_KILL				54
#define COMMAND_SUMMON				55
#define COMMAND_GOTO				56
#define COMMAND_FLYMODE				57
#define COMMAND_SETTIME				58
#define COMMAND_RELOAD_SPELLS		59
#define COMMAND_LOOT				60
#define COMMAND_USE					61
#define COMMAND_RELOADSPAWNSCRIPTS	62
#define COMMAND_RELOADLUASYSTEM		63
#define COMMAND_RELOADSTRUCTS		64
#define COMMAND_RELOAD				65
#define COMMAND_LOOT_LIST			66
#define COMMAND_LOOT_SETCOIN		67
#define COMMAND_LOOT_ADDITEM		68
#define COMMAND_LOOT_REMOVEITEM		69
#define COMMAND_BANK				70
#define COMMAND_BANK_DEPOSIT		71
#define COMMAND_BANK_WITHDRAWAL		72
#define COMMAND_BANK_CANCEL			73
#define COMMAND_ATTACK				74
#define COMMAND_REPORT_BUG			75
#define COMMAND_ACCEPT_QUEST		76
#define COMMAND_DECLINE_QUEST		77
#define COMMAND_DELETE_QUEST		78
#define COMMAND_RELOAD_QUESTS		79
#define COMMAND_SPAWN_COMBINE		80
#define COMMAND_DEPOP				81
#define COMMAND_REPOP				82
#define COMMAND_LUADEBUG			83
#define COMMAND_TEST				84
#define COMMAND_ACCEPT_REWARD		85
#define COMMAND_FROM_MERCHANT		86
#define COMMAND_MERCHANT_BUY		87
#define COMMAND_MERCHANT_SELL		88
#define COMMAND_CANCEL_MERCHANT		89
#define COMMAND_START_MERCHANT		90
#define COMMAND_BUYBACK				91
#define COMMAND_SEARCH_STORES		92
#define COMMAND_INVULNERABLE		93
#define COMMAND_SEARCH_STORES_PAGE	94
#define COMMAND_BUY_FROM_BROKER		95
#define COMMAND_GROUP_ACCEPT_INVITE	96
#define COMMAND_GROUP_DECLINE_INVITE 97
#define COMMAND_RELOAD_GROUNDSPAWNS	98
#define COMMAND_RELOAD_SPAWNS		99
#define COMMAND_LOCK				100
#define COMMAND_GIVEITEM			101
#define COMMAND_SET_COMBAT_VOICE	102
#define COMMAND_SET_EMOTE_VOICE		103
#define COMMAND_RELOAD_ZONESCRIPTS	104
#define COMMAND_GROUP_LEAVE			105
#define COMMAND_GROUP_MAKE_LEADER	106
#define COMMAND_GROUP_KICK			107
#define COMMAND_FRIEND_ADD			108
#define COMMAND_FRIEND_REMOVE		109
#define COMMAND_FRIENDS				110
#define COMMAND_IGNORE_ADD			111
#define COMMAND_IGNORE_REMOVE		112
#define COMMAND_IGNORES				113
#define COMMAND_MENDER_REPAIR		114
#define COMMAND_MENDER_REPAIR_ALL	115
#define COMMAND_REPAIR				116
#define COMMAND_USE_ITEM			117
#define COMMAND_WEAPONSTATS			118
#define COMMAND_START_MAIL			119
#define COMMAND_GET_MAIL_MESSAGE	120
#define COMMAND_TAKE_MAIL_ATTACHMENTS	121
#define COMMAND_REPORT_SPAM			122
#define COMMAND_CANCEL_MAIL			123
#define COMMAND_ADD_MAIL_PLAT		124
#define COMMAND_ADD_MAIL_GOLD		125
#define COMMAND_ADD_MAIL_SILVER		126
#define COMMAND_ADD_MAIL_COPPER		127
#define	COMMAND_SET_MAIL_ITEM		128
#define	COMMAND_CANCEL_SEND_MAIL	129
#define	COMMAND_REMOVE_MAIL_PLAT	130
#define	COMMAND_REMOVE_MAIL_GOLD	131
#define	COMMAND_REMOVE_MAIL_SILVER	132
#define	COMMAND_REMOVE_MAIL_COPPER	133
#define	COMMAND_DELETE_MAIL_MESSAGE	134
#define COMMAND_TRACK				135
#define COMMAND_INSPECT_PLAYER		136
#define COMMAND_PET					137
#define COMMAND_PETNAME				138
#define COMMAND_NAME_PET			139
#define COMMAND_RENAME				140
#define COMMAND_CONFIRMRENAME		141
#define COMMAND_PETOPTIONS			142
#define COMMAND_SPAWN_TEMPLATE		143 // JA: new /spawn template command
#define COMMAND_CANNEDEMOTE			144
#define COMMAND_BROADCAST			145
#define COMMAND_ANNOUNCE			146
#define COMMAND_AFK					147
#define COMMAND_TOGGLE_ANONYMOUS	148
#define COMMAND_TOGGLE_LFW			149
#define COMMAND_TOGGLE_LFG			150
#define COMMAND_SHOW_RANGED			151
#define COMMAND_TOGGLE_AUTOCONSUME	152
#define COMMAND_SHOW_HELM			153
#define COMMAND_SHOW_HOOD_OR_HELM	154
#define COMMAND_SHOW_CLOAK			155
#define COMMAND_STOP_EATING			156
#define COMMAND_STOP_DRINKING		157
#define COMMAND_TOGGLE_ILLUSIONS	158
#define COMMAND_SHOW_HOOD			159
#define COMMAND_TOGGLE_DUELS		160
#define COMMAND_TOGGLE_TRADES		161
#define COMMAND_TOGGLE_GUILDS		162
#define COMMAND_TOGGLE_GROUPS		163
#define COMMAND_TOGGLE_RAIDS		164
#define COMMAND_TOGGLE_LON			165

#define COMMAND_TOGGLE_GM_HIDE		167
#define COMMAND_TOGGLE_GM_VANISH	168
#define COMMAND_SPAWN_GROUP			169
#define COMMAND_TOGGLE_ROLEPLAYING	170
#define COMMAND_TOGGLE_VCINVITE		171
#define COMMAND_START_TRADE			172
#define COMMAND_ACCEPT_TRADE		173
#define COMMAND_REJECT_TRADE		174
#define COMMAND_CANCEL_TRADE		175
#define COMMAND_SET_TRADE_COIN		176
#define COMMAND_ADD_TRADE_COPPER	177
#define COMMAND_ADD_TRADE_SILVER	178
#define COMMAND_ADD_TRADE_GOLD		179
#define COMMAND_ADD_TRADE_PLAT		180
#define COMMAND_REMOVE_TRADE_COPPER	181
#define COMMAND_REMOVE_TRADE_SILVER	182
#define COMMAND_REMOVE_TRADE_GOLD	183
#define COMMAND_REMOVE_TRADE_PLAT	184
#define COMMAND_ADD_TRADE_ITEM		185
#define COMMAND_REMOVE_TRADE_ITEM	186
#define COMMAND_TOGGLE_COMBAT_EXP	187
#define COMMAND_TOGGLE_QUEST_EXP	188
#define COMMAND_TOGGLE_BONUS_EXP	189
#define COMMAND_ZONE_SHUTDOWN		190
#define COMMAND_ZONE_SAFE			191
#define COMMAND_ZONE_REVIVE			192
#define COMMAND_RELOAD_ZONES		193

#define COMMAND_DUEL				200
#define COMMAND_DUELBET				201
#define COMMAND_DUEL_ACCEPT			202
#define COMMAND_DUEL_DECLINE		203
#define COMMAND_DUEL_SURRENDER		204
#define COMMAND_DUEL_TOGGLE			205

#define COMMAND_ANIMTEST			211
#define COMMAND_ITEMSEARCH			212

#define COMMAND_ACTION				232 // JA: What is this? Exists nowhere else...
#define COMMAND_SKILL_ADD			233
#define COMMAND_SKILL_REMOVE		234
#define COMMAND_SKILL_LIST			235
#define COMMAND_SKILL				236
#define COMMAND_ZONE_SET			237
#define COMMAND_ZONE_DETAILS		238
#define COMMAND_RANDOMIZE			239
#define COMMAND_RELOAD_ENTITYCOMMANDS	240
#define COMMAND_ENTITYCOMMAND		241
#define COMMAND_ENTITYCOMMAND_LIST	242
#define COMMAND_RELOAD_FACTIONS		243
#define COMMAND_MERCHANT			244
#define COMMAND_MERCHANT_LIST		245
#define COMMAND_APPEARANCE			246
#define COMMAND_APPEARANCE_LIST		247
#define COMMAND_RELOAD_MAIL			248
#define COMMAND_DISTANCE			249
#define COMMAND_GUILDSAY			250
#define COMMAND_OFFICERSAY			251
#define COMMAND_GUILD				252
#define COMMAND_SET_GUILD_MEMBER_NOTE	253
#define COMMAND_SET_GUILD_OFFICER_NOTE	254
#define COMMAND_RELOAD_GUILDS		255
#define COMMAND_CREATE				256
#define COMMAND_CREATE_GUILD		257
#define COMMAND_GUILDS				258
#define COMMAND_GUILDS_CREATE		259
#define COMMAND_GUILDS_DELETE		260
#define COMMAND_GUILDS_ADD			261
#define COMMAND_GUILDS_REMOVE		262
#define COMMAND_GUILDS_LIST			263
#define COMMAND_LOTTO				264
#define COMMAND_CLEAR_ALL_QUEUED	265
#define COMMAND_SCRIBE_SCROLL_ITEM	266
#define COMMAND_RELOAD_LOCATIONS	267
#define COMMAND_LOCATION			268
#define COMMAND_LOCATION_CREATE		269
#define COMMAND_LOCATION_ADD		270
#define COMMAND_GRID				271
#define COMMAND_LOCATION_REMOVE		272
#define COMMAND_LOCATION_DELETE		273
#define COMMAND_LOCATION_LIST		274
#define COMMAND_USE_EQUIPPED_ITEM	275
#define COMMAND_CANCEL_MAINTAINED	276
#define COMMAND_LOOT_CORPSE			277
#define COMMAND_MOTD				278
#define COMMAND_RANDOM				279
#define COMMAND_TRY_ON				280
#define COMMAND_TITLE				281
#define COMMAND_GUILD_BANK				282
#define COMMAND_GUILD_BANK_DEPOSIT		283
#define COMMAND_GUILD_BANK_WITHDRAWAL	284
#define COMMAND_GUILD_BANK_CANCEL		285
#define COMMAND_TITLE_LIST			286
#define COMMAND_TITLE_SETPREFIX		287
#define COMMAND_TITLE_SETSUFFIX		288
#define COMMAND_TITLE_FIX			289
#define COMMAND_LANGUAGES			290
#define COMMAND_SET_LANGUAGE		291
#define COMMAND_ACCEPT_ADVANCEMENT	293

#define COMMAND_JOIN_CHANNEL			294
#define COMMAND_JOIN_CHANNEL_FROM_LOAD	295
#define COMMAND_TELL_CHANNEL			296
#define COMMAND_LEAVE_CHANNEL			297
#define COMMAND_WHO_CHANNEL				298

#define COMMAND_CREATEFROMRECIPE		299
#define COMMAND_RAIN					300
#define COMMAND_TO_MERCHANT				301
#define COMMAND_SELECT					302
#define COMMAND_SMP						303
#define COMMAND_CONSUME_FOOD			304
#define COMMAND_AQUAMAN					305
#define COMMAND_ATTUNE_INV				306
#define COMMAND_PLAYER					307
#define COMMAND_PLAYER_COINS			308
#define COMMAND_RESET_ZONE_TIMER		309
#define COMMAND_ACHIEVEMENT_ADD			310
#define COMMAND_EDITOR					311
#define COMMAND_ACCEPT_RESURRECTION     312
#define COMMAND_DECLINE_RESURRECTION    313
#define COMMAND_WIND					314
#define COMMAND_TARGETITEM              315
#define COMMAND_READ					463

#define COMMAND_BOT						500
#define COMMAND_BOT_CREATE				501
#define COMMAND_BOT_CUSTOMIZE			502
#define COMMAND_BOT_SPAWN				503
#define COMMAND_BOT_LIST				504
#define COMMAND_BOT_INV					505
#define COMMAND_BOT_SETTINGS			506
#define COMMAND_BOT_HELP				507

#define COMMAND_OPEN					508
#define COMMAND_CASTSPELL				509
#define COMMAND_DISARM					510
#define COMMAND_KNOWLEDGEWINDOWSORT		511
#define COMMAND_PLACE_HOUSE_ITEM		512
#define COMMAND_GM						513
#define COMMAND_HOUSE_UI				514
#define COMMAND_HOUSE					515
#define COMMAND_MOVE_ITEM				516
#define COMMAND_PICKUP					517
#define COMMAND_HOUSE_DEPOSIT			518

#define COMMAND_RELOAD_RULES			519
#define COMMAND_RELOAD_TRANSPORTERS		520
#define COMMAND_FINDSPAWN				521
#define COMMAND_RELOAD_STARTABILITIES	522

#define COMMAND_WAYPOINT				523

#define COMMAND_RELOADREGIONSCRIPTS		524

#define COMMAND_MOVECHARACTER			525

#define COMMAND_CRAFTITEM                526

#define COMMAND_FROMBROKER           	527

#define COMMAND_MENTOR		           	528
#define COMMAND_UNMENTOR		        529

#define COMMAND_CANCEL_EFFECT			530
#define COMMAND_CUREPLAYER			531

#define COMMAND_RELOAD_VOICEOVERS		532
#define COMMAND_SHARE_QUEST				533

#define COMMAND_SETAUTOLOOTMODE			534
#define COMMAND_ASSIST					535
#define COMMAND_TARGET					536
#define COMMAND_TARGET_PET				537

#define COMMAND_SET_CONSUME_FOOD			538

#define GET_AA_XML						750
#define ADD_AA							751
#define COMMIT_AA_PROFILE				752				
#define BEGIN_AA_PROFILE				753
#define BACK_AA							754
#define REMOVE_AA						755
#define SWITCH_AA_PROFILE				756
#define CANCEL_AA_PROFILE				757
#define SAVE_AA_PROFILE					758

#define COMMAND_MOOD				800


#define COMMAND_MODIFY				1000 // INSERT INTO `commands`(`id`,`type`,`command`,`subcommand`,`handler`,`required_status`) VALUES ( NULL,'1','modify','','1000','200'); 
#define COMMAND_MODIFY_CHARACTER	1001
#define COMMAND_MODIFY_FACTION		1002
#define COMMAND_MODIFY_GUILD		1003
#define COMMAND_MODIFY_ITEM			1004
#define COMMAND_MODIFY_QUEST		1005
#define COMMAND_MODIFY_SKILL		1006
#define COMMAND_MODIFY_SPAWN		1007
#define COMMAND_MODIFY_SPELL		1008
#define COMMAND_MODIFY_ZONE			1009

#endif
