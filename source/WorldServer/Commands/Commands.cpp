/*  
    EQ2Emulator:  Everquest II Server Emulator
    Copyright (C) 2005 - 2026  EQ2EMulator Development Team (http://www.eq2emu.com formerly http://www.eq2emulator.net)

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

#include <sys/types.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string.hpp>

#include "Commands.h"
#include "../ClientPacketFunctions.h"
#include "../../common/version.h"
#include "../../common/seperator.h"
#include "../../common/servertalk.h"
#include "../WorldDatabase.h"
#include "../World.h"
#include "../../common/ConfigReader.h"
#include "../VisualStates.h"
#include "../../common/debug.h"
#include "../LuaInterface.h"
#include "../Quests.h"
#include "../client.h"
#include "../NPC.h"
#include "../Guilds/Guild.h"
#include "../SpellProcess.h"
#include "../Tradeskills/Tradeskills.h"
#include "../../common/Log.h"
#include "../../common/MiscFunctions.h"
#include "../Languages.h"
#include "../Traits/Traits.h"
#include "../Chat/Chat.h"
#include "../Rules/Rules.h"
#include "../AltAdvancement/AltAdvancement.h"
#include "../RaceTypes/RaceTypes.h"
#include "../classes.h"
#include "../Transmute.h"
#include "../Bots/Bot.h"
#include "../Web/PeerManager.h"
#include "../../common/GlobalHeaders.h"
#include "../Broker/BrokerManager.h"

extern WorldDatabase database;
extern MasterSpellList master_spell_list;
extern MasterTraitList master_trait_list;
extern MasterRecipeList master_recipe_list;
extern MasterRecipeBookList master_recipebook_list;
extern World world;
extern ClientList client_list;
extern ConfigReader configReader;
extern VisualStates visual_states;
extern ZoneList	zone_list;
extern LuaInterface* lua_interface;
extern MasterQuestList master_quest_list;
extern MasterCollectionList master_collection_list;
extern MasterSkillList master_skill_list;
extern MasterFactionList master_faction_list;
extern GuildList guild_list;
extern MasterLanguagesList master_languages_list;
extern Chat chat;
extern RuleManager rule_manager;
extern MasterAAList master_aa_list;
extern MasterRaceTypeList race_types_list;
extern Classes classes;
extern PeerManager peer_manager;
extern BrokerManager broker;

//devn00b: Fix for linux builds since we dont use stricmp we use strcasecmp
#if defined(__GNUC__)
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif


EQ2Packet* RemoteCommands::serialize(int16 version){
	buffer.clear();
	vector<EQ2_RemoteCommandString>::iterator command_list;
	buffer.append((char*)&num_commands, sizeof(int16));
	for( command_list = commands.begin(); command_list != commands.end(); command_list++ ) {
		AddDataCommand(&(*command_list));
	}
	EQ2Packet* app = new EQ2Packet(OP_SetRemoteCmdsMsg, (uchar*)buffer.c_str(), buffer.length() + 1);
	return app;
}

Commands::Commands(){ 
	remote_commands = new RemoteCommands(); 
	spawn_set_values["list"] = SPAWN_SET_VALUE_LIST;
	spawn_set_values["name"] = SPAWN_SET_VALUE_NAME;
	spawn_set_values["level"] = SPAWN_SET_VALUE_LEVEL;	// TODO: Fix this, min_level, max_level
	spawn_set_values["difficulty"] = SPAWN_SET_VALUE_DIFFICULTY;
	spawn_set_values["model_type"] = SPAWN_SET_VALUE_MODEL_TYPE;
	spawn_set_values["class"] = SPAWN_SET_VALUE_CLASS;
	spawn_set_values["gender"] = SPAWN_SET_VALUE_GENDER;
	spawn_set_values["show_name"] = SPAWN_SET_VALUE_SHOW_NAME;
	spawn_set_values["attackable"] = SPAWN_SET_VALUE_ATTACKABLE;
	spawn_set_values["show_level"] = SPAWN_SET_VALUE_SHOW_LEVEL;
	spawn_set_values["targetable"] = SPAWN_SET_VALUE_TARGETABLE;
	spawn_set_values["show_command_icon"] = SPAWN_SET_VALUE_SHOW_COMMAND_ICON;
	spawn_set_values["display_hand_icon"] = SPAWN_SET_VALUE_HAND_ICON;
	spawn_set_values["hair_type"] = SPAWN_SET_VALUE_HAIR_TYPE;
	spawn_set_values["facial_hair_type"] = SPAWN_SET_VALUE_FACIAL_HAIR_TYPE;
	spawn_set_values["wing_type"] = SPAWN_SET_VALUE_WING_TYPE;
	spawn_set_values["chest_type"] = SPAWN_SET_VALUE_CHEST_TYPE;
	spawn_set_values["legs_type"] = SPAWN_SET_VALUE_LEGS_TYPE;
	spawn_set_values["soga_hair_type"] = SPAWN_SET_VALUE_SOGA_HAIR_TYPE;
	spawn_set_values["soga_facial_hair_type"] = SPAWN_SET_VALUE_SOGA_FACIAL_HAIR_TYPE;
	spawn_set_values["soga_model_type"] = SPAWN_SET_VALUE_SOGA_MODEL_TYPE;
	spawn_set_values["size"] = SPAWN_SET_VALUE_SIZE;
	spawn_set_values["hp"] = SPAWN_SET_VALUE_HP;
	spawn_set_values["power"] = SPAWN_SET_VALUE_POWER;
	spawn_set_values["heroic"] = SPAWN_SET_VALUE_HEROIC;
	spawn_set_values["respawn"] = SPAWN_SET_VALUE_RESPAWN;
	spawn_set_values["x"] = SPAWN_SET_VALUE_X;
	spawn_set_values["y"] = SPAWN_SET_VALUE_Y;
	spawn_set_values["z"] = SPAWN_SET_VALUE_Z;
	spawn_set_values["heading"] = SPAWN_SET_VALUE_HEADING;
	spawn_set_values["location"] = SPAWN_SET_VALUE_LOCATION;
	spawn_set_values["command_primary"] = SPAWN_SET_VALUE_COMMAND_PRIMARY;
	spawn_set_values["command_secondary"] = SPAWN_SET_VALUE_COMMAND_SECONDARY;
	spawn_set_values["visual_state"] = SPAWN_SET_VALUE_VISUAL_STATE;
	spawn_set_values["action_state"] = SPAWN_SET_VALUE_ACTION_STATE;
	spawn_set_values["mood_state"] = SPAWN_SET_VALUE_MOOD_STATE;
	spawn_set_values["initial_state"] = SPAWN_SET_VALUE_INITIAL_STATE;
	spawn_set_values["activity_state"] = SPAWN_SET_VALUE_ACTIVITY_STATE;
	spawn_set_values["collision_radius"] = SPAWN_SET_VALUE_COLLISION_RADIUS;
	spawn_set_values["faction"] = SPAWN_SET_VALUE_FACTION;
	spawn_set_values["spawn_script"] = SPAWN_SET_VALUE_SPAWN_SCRIPT;
	spawn_set_values["spawnentry_script"] = SPAWN_SET_VALUE_SPAWNENTRY_SCRIPT;
	spawn_set_values["spawnlocation_script"] = SPAWN_SET_VALUE_SPAWNLOCATION_SCRIPT;
	spawn_set_values["sub_title"] = SPAWN_SET_VALUE_SUB_TITLE;
	spawn_set_values["expire"] = SPAWN_SET_VALUE_EXPIRE;
	spawn_set_values["expire_offset"] = SPAWN_SET_VALUE_EXPIRE_OFFSET;
	spawn_set_values["x_offset"] = SPAWN_SET_VALUE_X_OFFSET;
	spawn_set_values["y_offset"] = SPAWN_SET_VALUE_Y_OFFSET;
	spawn_set_values["z_offset"] = SPAWN_SET_VALUE_Z_OFFSET;
	spawn_set_values["device_id"] = SPAWN_SET_VALUE_DEVICE_ID;
	spawn_set_values["pitch"] = SPAWN_SET_VALUE_PITCH;
	spawn_set_values["roll"] = SPAWN_SET_VALUE_ROLL;
	spawn_set_values["hide_hood"] = SPAWN_SET_VALUE_HIDE_HOOD;
	spawn_set_values["emote_state"] = SPAWN_SET_VALUE_EMOTE_STATE;
	spawn_set_values["icon"] = SPAWN_SET_VALUE_ICON;
	spawn_set_values["prefix"] = SPAWN_SET_VALUE_PREFIX;
	spawn_set_values["suffix"] = SPAWN_SET_VALUE_SUFFIX;
	spawn_set_values["lastname"] = SPAWN_SET_VALUE_LASTNAME;
	spawn_set_values["expansion_flag"] = SPAWN_SET_VALUE_EXPANSION_FLAG;
	spawn_set_values["holiday_flag"] = SPAWN_SET_VALUE_HOLIDAY_FLAG;
	spawn_set_values["merchant_min_level"] = SPAWN_SET_VALUE_MERCHANT_MIN_LEVEL;
	spawn_set_values["merchant_max_level"] = SPAWN_SET_VALUE_MERCHANT_MAX_LEVEL;
	spawn_set_values["skin_color"] = SPAWN_SET_SKIN_COLOR;
	spawn_set_values["aaxp_rewards"] = SPAWN_SET_AAXP_REWARDS;
	
	spawn_set_values["hair_color1"] = SPAWN_SET_HAIR_COLOR1;
	spawn_set_values["hair_color2"] = SPAWN_SET_HAIR_COLOR2;
	spawn_set_values["hair_type_color"] = SPAWN_SET_HAIR_TYPE_COLOR;
	spawn_set_values["hair_face_color"] = SPAWN_SET_HAIR_FACE_COLOR;
	spawn_set_values["hair_type_highlight_color"] = SPAWN_SET_HAIR_TYPE_HIGHLIGHT_COLOR;
	spawn_set_values["face_hairlight_color"] = SPAWN_SET_HAIR_FACE_HIGHLIGHT_COLOR;
	spawn_set_values["hair_highlight"] = SPAWN_SET_HAIR_HIGHLIGHT;
	spawn_set_values["model_color"] = SPAWN_SET_MODEL_COLOR;
	spawn_set_values["eye_color"] = SPAWN_SET_EYE_COLOR;
	
	spawn_set_values["soga_skin_color"] = SPAWN_SET_SOGA_SKIN_COLOR;
	spawn_set_values["soga_hair_color1"] = SPAWN_SET_SOGA_HAIR_COLOR1;
	spawn_set_values["soga_hair_color2"] = SPAWN_SET_SOGA_HAIR_COLOR2;
	spawn_set_values["soga_hair_type_color"] = SPAWN_SET_SOGA_HAIR_TYPE_COLOR;
	spawn_set_values["soga_hair_face_color"] = SPAWN_SET_SOGA_HAIR_FACE_COLOR;
	spawn_set_values["soga_hair_type_highlight_color"] = SPAWN_SET_SOGA_HAIR_TYPE_HIGHLIGHT_COLOR;
	spawn_set_values["soga_face_hairlight_color"] = SPAWN_SET_SOGA_HAIR_FACE_HIGHLIGHT_COLOR;
	spawn_set_values["soga_hair_highlight"] = SPAWN_SET_SOGA_HAIR_HIGHLIGHT;
	spawn_set_values["soga_model_color"] = SPAWN_SET_SOGA_MODEL_COLOR;
	spawn_set_values["soga_eye_color"] = SPAWN_SET_SOGA_EYE_COLOR;
	
	spawn_set_values["cheek_type"] = SPAWN_SET_CHEEK_TYPE;
	spawn_set_values["chin_type"] = SPAWN_SET_CHIN_TYPE;
	spawn_set_values["ear_type"] = SPAWN_SET_EAR_TYPE;
	spawn_set_values["eye_brow_type"] = SPAWN_SET_EYE_BROW_TYPE;
	spawn_set_values["eye_type"] = SPAWN_SET_EYE_TYPE;
	spawn_set_values["lip_type"] = SPAWN_SET_LIP_TYPE;
	spawn_set_values["nose_type"] = SPAWN_SET_NOSE_TYPE;
	spawn_set_values["body_size"] = SPAWN_SET_BODY_SIZE;
	spawn_set_values["body_age"] = SPAWN_SET_BODY_AGE;
	spawn_set_values["soga_cheek_type"] = SPAWN_SET_SOGA_CHEEK_TYPE;
	spawn_set_values["soga_chin_type"] = SPAWN_SET_SOGA_CHIN_TYPE;
	spawn_set_values["soga_ear_type"] = SPAWN_SET_SOGA_EAR_TYPE;
	spawn_set_values["soga_eye_brow_type"] = SPAWN_SET_SOGA_EYE_BROW_TYPE;
	spawn_set_values["soga_eye_type"] = SPAWN_SET_SOGA_EYE_TYPE;
	spawn_set_values["soga_lip_type"] = SPAWN_SET_SOGA_LIP_TYPE;
	spawn_set_values["soga_nose_type"] = SPAWN_SET_SOGA_NOSE_TYPE;
	spawn_set_values["soga_body_size"] = SPAWN_SET_SOGA_BODY_SIZE;
	spawn_set_values["soga_body_age"] = SPAWN_SET_SOGA_BODY_AGE;
	spawn_set_values["attack_type"] = SPAWN_SET_ATTACK_TYPE;
	spawn_set_values["race_type"] = SPAWN_SET_RACE_TYPE;
	spawn_set_values["loot_tier"] = SPAWN_SET_LOOT_TIER;
	spawn_set_values["loot_drop_type"] = SPAWN_SET_LOOT_DROP_TYPE;
	spawn_set_values["scared_strong_players"] = SPAWN_SET_SCARED_STRONG_PLAYERS;

	zone_set_values["expansion_id"] = ZONE_SET_VALUE_EXPANSION_ID;
	zone_set_values["name"] = ZONE_SET_VALUE_NAME;
	zone_set_values["file"] = ZONE_SET_VALUE_FILE	;
	zone_set_values["description"] = ZONE_SET_VALUE_DESCRIPTION;
	zone_set_values["safe_x"] = ZONE_SET_VALUE_SAFE_X;
	zone_set_values["safe_y"] = ZONE_SET_VALUE_SAFE_Y;
	zone_set_values["safe_z"] = ZONE_SET_VALUE_SAFE_Z;
	zone_set_values["underworld"] = ZONE_SET_VALUE_UNDERWORLD;
	zone_set_values["min_recommended"] = ZONE_SET_VALUE_MIN_RECOMMENDED;
	zone_set_values["max_recommended"] = ZONE_SET_VALUE_MAX_RECOMMENDED;
	zone_set_values["zone_type"] = ZONE_SET_VALUE_ZONE_TYPE;
	zone_set_values["always_loaded"] = ZONE_SET_VALUE_ALWAYS_LOADED;
	zone_set_values["city_zone"] = ZONE_SET_VALUE_CITY_ZONE;
	zone_set_values["weather_allowed"] = ZONE_SET_VALUE_WEATHER_ALLOWED;
	zone_set_values["min_status"] = ZONE_SET_VALUE_MIN_STATUS;
	zone_set_values["min_level"] = ZONE_SET_VALUE_MIN_LEVEL;
	zone_set_values["start_zone"] = ZONE_SET_VALUE_START_ZONE;
	zone_set_values["instance_type"] = ZONE_SET_VALUE_INSTANCE_TYPE;
	zone_set_values["default_reenter_time"] = ZONE_SET_VALUE_DEFAULT_REENTER_TIME;
	zone_set_values["default_reset_time"] = ZONE_SET_VALUE_DEFAULT_RESET_TIME;
	zone_set_values["default_lockout_time"] = ZONE_SET_VALUE_DEFAULT_LOCKOUT_TIME;
	zone_set_values["force_group_to_zone"] = ZONE_SET_VALUE_FORCE_GROUP_TO_ZONE;
	zone_set_values["lua_script"] = ZONE_SET_VALUE_LUA_SCRIPT;
	zone_set_values["shutdown_timer"] = ZONE_SET_VALUE_SHUTDOWN_TIMER;
	zone_set_values["zone_motd"] = ZONE_SET_VALUE_ZONE_MOTD;
}

Commands::~Commands() { 
	safe_delete(remote_commands); 
}

int32 Commands::GetSpawnSetType(string val){
	if(spawn_set_values.count(val) > 0)
		return spawn_set_values[val];
	return 0xFFFFFFFF;
}

bool Commands::SetSpawnCommand(Client* client, Spawn* target, int8 type, const char* value, bool send_update, bool temporary, string* temp_value, int8 index){
	if(!target)
		return false;
	int32 val = 0;
	try{
		if(type != SPAWN_SET_VALUE_NAME && 
			!(type >= SPAWN_SET_VALUE_SPAWN_SCRIPT && type <= SPAWN_SET_VALUE_SUB_TITLE) && !(type >= SPAWN_SET_VALUE_PREFIX && type <= SPAWN_SET_VALUE_EXPANSION_FLAG || type == SPAWN_SET_VALUE_HOLIDAY_FLAG))
			{
				switch(type)
				{
					case SPAWN_SET_SKIN_COLOR:
					case SPAWN_SET_HAIR_COLOR1:
					case SPAWN_SET_HAIR_COLOR2:
					case SPAWN_SET_HAIR_TYPE_COLOR:
					case SPAWN_SET_HAIR_FACE_COLOR:
					case SPAWN_SET_HAIR_TYPE_HIGHLIGHT_COLOR:
					case SPAWN_SET_HAIR_FACE_HIGHLIGHT_COLOR:
					case SPAWN_SET_HAIR_HIGHLIGHT:
					case SPAWN_SET_EYE_COLOR:
					case SPAWN_SET_SOGA_SKIN_COLOR:
					case SPAWN_SET_SOGA_HAIR_COLOR1:
					case SPAWN_SET_SOGA_HAIR_COLOR2:
					case SPAWN_SET_SOGA_HAIR_TYPE_COLOR:
					case SPAWN_SET_SOGA_HAIR_FACE_COLOR:
					case SPAWN_SET_SOGA_HAIR_TYPE_HIGHLIGHT_COLOR:
					case SPAWN_SET_SOGA_HAIR_FACE_HIGHLIGHT_COLOR:
					case SPAWN_SET_SOGA_HAIR_HIGHLIGHT:
					case SPAWN_SET_SOGA_EYE_COLOR:
					case SPAWN_SET_RACE_TYPE:
					// ignore these are colors can't pass as a integer value
						break;
					default:
						val = atoul(value);
				}
			}
	}
	catch(...){
		if(client)
			client->Message(CHANNEL_COLOR_RED, "Invalid numeric spawn value: %s", value);
		return false;
	}
	if(temporary){
		char tmp[128] = {0};
		switch(type){
			case SPAWN_SET_VALUE_NAME:{
				sprintf(tmp, "%s", target->GetName());
				target->SetName(value);
				if(target->GetZone())
					target->GetZone()->SendUpdateTitles(target);
				break;
									  }
			case SPAWN_SET_VALUE_X_OFFSET: 
				{
				sprintf(tmp, "%f", target->GetXOffset());
				target->SetXOffset(float(val));
				break;
				}
			case SPAWN_SET_VALUE_Y_OFFSET: 
				{
				sprintf(tmp, "%f", target->GetYOffset());
				target->SetYOffset(float(val));
				break;
				}
			case SPAWN_SET_VALUE_Z_OFFSET: 
				{
				sprintf(tmp, "%f", target->GetZOffset());
				target->SetZOffset(float(val));
				break;
				}
			case SPAWN_SET_VALUE_EXPIRE: {
				sprintf(tmp, "%u", target->GetExpireTime());
				target->SetExpireTime(val);
				break;
											}
			case SPAWN_SET_VALUE_EXPIRE_OFFSET: {
				sprintf(tmp, "%u", target->GetExpireOffsetTime());
				target->SetExpireOffsetTime(val);
				break;
											}
			case SPAWN_SET_VALUE_SUB_TITLE: {
				sprintf(tmp, "%s", target->GetSubTitle());
				target->SetSubTitle(value);
				if(target->GetZone())
					target->GetZone()->SendUpdateTitles(target);
				break;
											}
			case SPAWN_SET_VALUE_LEVEL:{
				sprintf(tmp, "%i", target->GetLevel());
				target->SetLevel(val, send_update);
				break;
									   }
			case SPAWN_SET_VALUE_DIFFICULTY:{
				sprintf(tmp, "%i", target->GetDifficulty());
				target->SetDifficulty(val, send_update);
				break;
											}
			case SPAWN_SET_VALUE_MODEL_TYPE:{
				sprintf(tmp, "%i", target->GetModelType());
				target->SetModelType(val, send_update);
				break;
											}
			case SPAWN_SET_VALUE_CLASS:{
				sprintf(tmp, "%i", target->GetAdventureClass());
				target->SetAdventureClass(val, send_update);
				break;
									   }
			case SPAWN_SET_VALUE_GENDER:{
				sprintf(tmp, "%i", target->GetGender());
				target->SetGender(val, send_update);
				break;
										}
			case SPAWN_SET_VALUE_SHOW_NAME:{
				sprintf(tmp, "%i", target->GetShowName());
				target->SetShowName(val, send_update);
				break;
										   }
			case SPAWN_SET_VALUE_ATTACKABLE:{
				sprintf(tmp, "%i", target->GetAttackable());
				target->SetAttackable(val, send_update);
				break;
											}
			case SPAWN_SET_VALUE_SHOW_LEVEL:{
				sprintf(tmp, "%i", target->GetShowLevel());
				target->SetShowLevel(val, send_update);
				break;
											}
			case SPAWN_SET_VALUE_TARGETABLE:{
				sprintf(tmp, "%i", target->GetTargetable());
				target->SetTargetable(val, send_update);
				break;
											}
			case SPAWN_SET_VALUE_SHOW_COMMAND_ICON:{
				sprintf(tmp, "%i", target->GetShowCommandIcon());
				target->SetShowCommandIcon(val, send_update);
				break;
												   }
			case SPAWN_SET_VALUE_HAND_ICON:{
				sprintf(tmp, "%i", target->GetShowHandIcon());
				target->SetShowHandIcon(val, send_update);
				break;
										   }
			case SPAWN_SET_VALUE_HAIR_TYPE:{
				if(target->IsEntity()){
					sprintf(tmp, "%i", ((Entity*)target)->GetHairType());
					((Entity*)target)->SetHairType(val, send_update);
				}
				break;
										   }
			case SPAWN_SET_VALUE_FACIAL_HAIR_TYPE:{
				if(target->IsEntity()){
					sprintf(tmp, "%i", ((Entity*)target)->GetFacialHairType());
					((Entity*)target)->SetFacialHairType(val, send_update);
				}
				break;
												  }
			case SPAWN_SET_VALUE_WING_TYPE:{
				if(target->IsEntity()){
					sprintf(tmp, "%i", ((Entity*)target)->GetWingType());
					((Entity*)target)->SetWingType(val, send_update);
				}
				break;
										   }
			case SPAWN_SET_VALUE_CHEST_TYPE:{
				if(target->IsEntity()){
					sprintf(tmp, "%i", ((Entity*)target)->GetChestType());
					((Entity*)target)->SetChestType(val, send_update);
				}
				break;
											}
			case SPAWN_SET_VALUE_LEGS_TYPE:{
				if(target->IsEntity()){
					sprintf(tmp, "%i", ((Entity*)target)->GetLegsType());
					((Entity*)target)->SetLegsType(val, send_update);
				}
				break;
										   }
			case SPAWN_SET_VALUE_SOGA_HAIR_TYPE:{
				if(target->IsEntity()){
					sprintf(tmp, "%i", ((Entity*)target)->GetSogaHairType());
					((Entity*)target)->SetSogaHairType(val, send_update);
				}
				break;
												}
			case SPAWN_SET_VALUE_SOGA_FACIAL_HAIR_TYPE:{
				if(target->IsEntity()){
					sprintf(tmp, "%i", ((Entity*)target)->GetSogaFacialHairType());
					((Entity*)target)->SetSogaFacialHairType(val, send_update);
				}
				break;
													   }
			case SPAWN_SET_VALUE_SOGA_MODEL_TYPE:{
				sprintf(tmp, "%i", target->GetSogaModelType());
				target->SetSogaModelType(val, send_update);
				break;
												 }
			case SPAWN_SET_VALUE_SIZE:{
				sprintf(tmp, "%i", target->GetSize());
				target->SetSize(val, send_update);
				break;
									  }
			case SPAWN_SET_VALUE_HP:{
				sprintf(tmp, "%i", target->GetHP());
				target->SetTotalHPBase(val);
				target->SetHP(val, send_update);
				break;
									}
			case SPAWN_SET_VALUE_POWER:{
				sprintf(tmp, "%i", target->GetPower());
				target->SetTotalPowerBase(val);
				target->SetPower(val, send_update);
				break;
									   }
			case SPAWN_SET_VALUE_HEROIC:{
				sprintf(tmp, "%i", target->GetHeroic());
				target->SetHeroic(val, send_update);
				break;
										}
			case SPAWN_SET_VALUE_RESPAWN:{
				sprintf(tmp, "%u", target->GetRespawnTime());
				target->SetRespawnTime(val);
				break;
										 }
			case SPAWN_SET_VALUE_X:{
				sprintf(tmp, "%f", target->GetX());
				target->SetX(atof(value), send_update);
				break;
								   }
			case SPAWN_SET_VALUE_Y:{
				sprintf(tmp, "%f", target->GetY());
				target->SetY(atof(value), send_update);
				break;
								   }
			case SPAWN_SET_VALUE_Z:{
				sprintf(tmp, "%f", target->GetZ());
				target->SetZ(atof(value), send_update);
				break;
								   }
			case SPAWN_SET_VALUE_HEADING:{
				sprintf(tmp, "%f", target->GetHeading());
				target->SetHeading(atof(value) + 360, send_update);
				break;
										 }
			case SPAWN_SET_VALUE_VISUAL_STATE:{
				sprintf(tmp, "%i", target->GetVisualState());
				target->SetVisualState(val, send_update);
				break;
											  }
			case SPAWN_SET_VALUE_ACTION_STATE:{
				sprintf(tmp, "%i", target->GetActionState());
				target->SetActionState(val, send_update);
				break;
											  }
			case SPAWN_SET_VALUE_MOOD_STATE:{
				sprintf(tmp, "%i", target->GetMoodState());
				target->SetMoodState(val, send_update);
				break;
											}
			case SPAWN_SET_VALUE_INITIAL_STATE:{
				sprintf(tmp, "%i", target->GetInitialState());
				target->SetInitialState(val, send_update);
				break;
											   }
			case SPAWN_SET_VALUE_ACTIVITY_STATE:{
				sprintf(tmp, "%i", target->GetActivityStatus());
				target->SetActivityStatus(val, send_update);
				break;
												}
			case SPAWN_SET_VALUE_COLLISION_RADIUS:{
				sprintf(tmp, "%i", target->GetCollisionRadius());
				target->SetCollisionRadius(val, send_update);
				break;
												  }
			case SPAWN_SET_VALUE_DEVICE_ID: {
				if (target->IsObject()) {
					sprintf(tmp, "%i", ((Object*)target)->GetDeviceID());
					((Object*)target)->SetDeviceID(val);
				}
				break;
											}
			case SPAWN_SET_VALUE_PITCH: {
				sprintf(tmp, "%f", target->GetPitch());
				target->SetPitch(atof(value), send_update);
				break;
										}
			case SPAWN_SET_VALUE_ROLL: {
				sprintf(tmp, "%f", target->GetRoll());
				target->SetRoll(atof(value), send_update);
				break;
									   }
			case SPAWN_SET_VALUE_HIDE_HOOD: {
				sprintf(tmp, "%i", target->appearance.hide_hood);
				target->SetHideHood(val);
				break;
											}
			case SPAWN_SET_VALUE_EMOTE_STATE: {
				sprintf(tmp, "%i", target->appearance.emote_state);
				target->SetEmoteState(val);
				break;
											  }
			case SPAWN_SET_VALUE_ICON: {
			    sprintf(tmp, "%i", target->GetIconValue());
			    target->SetIcon(val);
			    break;
			}

			case SPAWN_SET_VALUE_PREFIX: {
				sprintf(tmp, "%s", target->GetPrefixTitle());
				target->SetPrefixTitle(value);
				if(target->GetZone())
					target->GetZone()->SendUpdateTitles(target);
				break;
			}

			case SPAWN_SET_VALUE_SUFFIX: {
				sprintf(tmp, "%s", target->GetSuffixTitle());
				target->SetSuffixTitle(value);
				if(target->GetZone())
					target->GetZone()->SendUpdateTitles(target);
				break;
			}

			case SPAWN_SET_VALUE_LASTNAME: {
				sprintf(tmp, "%s", target->GetLastName());
				target->SetLastName(value);
				if(target->GetZone())
					target->GetZone()->SendUpdateTitles(target);
				break;
			}
			case SPAWN_SET_VALUE_EXPANSION_FLAG:
			case SPAWN_SET_VALUE_HOLIDAY_FLAG: {
				// nothing to do must reload spawns
				break;
			}
			case SPAWN_SET_VALUE_MERCHANT_MIN_LEVEL: {
				sprintf(tmp, "%i", target->GetMerchantMinLevel());
				target->SetMerchantLevelRange(atoul(value), target->GetMerchantMaxLevel());
				break;
			}
			case SPAWN_SET_VALUE_MERCHANT_MAX_LEVEL: {
				sprintf(tmp, "%i", target->GetMerchantMaxLevel());
				target->SetMerchantLevelRange(target->GetMerchantMinLevel(), atoul(value));
				break;
			}
			case SPAWN_SET_VALUE_FACTION:{
				sprintf(tmp, "%i", target->faction_id);
				ZoneServer* zone = target->GetZone();
				if (!zone && client)
					zone = client->GetCurrentZone();

				target->faction_id = atoul(value);
				if(zone)
				{
					zone->RemoveDeadEnemyList(target);
					if(target->IsNPC())
						zone->AddEnemyList((NPC*)target);
				}
				break;
			}
			case SPAWN_SET_SKIN_COLOR:
			case SPAWN_SET_HAIR_COLOR1:
			case SPAWN_SET_HAIR_COLOR2:
			case SPAWN_SET_HAIR_TYPE_COLOR:
			case SPAWN_SET_HAIR_FACE_COLOR:
			case SPAWN_SET_HAIR_TYPE_HIGHLIGHT_COLOR:
			case SPAWN_SET_HAIR_FACE_HIGHLIGHT_COLOR:
			case SPAWN_SET_HAIR_HIGHLIGHT:
			case SPAWN_SET_MODEL_COLOR:
			case SPAWN_SET_EYE_COLOR:
			case SPAWN_SET_SOGA_SKIN_COLOR:
			case SPAWN_SET_SOGA_HAIR_COLOR1:
			case SPAWN_SET_SOGA_HAIR_COLOR2:
			case SPAWN_SET_SOGA_HAIR_TYPE_COLOR:
			case SPAWN_SET_SOGA_HAIR_FACE_COLOR:
			case SPAWN_SET_SOGA_HAIR_TYPE_HIGHLIGHT_COLOR:
			case SPAWN_SET_SOGA_HAIR_FACE_HIGHLIGHT_COLOR:
			case SPAWN_SET_SOGA_HAIR_HIGHLIGHT:
			case SPAWN_SET_SOGA_MODEL_COLOR:
			case SPAWN_SET_SOGA_EYE_COLOR:
					{
				if (target->IsEntity())
				{
					Seperator* skinsep = new Seperator(value, ' ', 3, 500, true);
					if (skinsep->IsNumber(0) && skinsep->IsNumber(1) && skinsep->IsNumber(2))
					{
						EQ2_Color clr;
						clr.red = atoul(skinsep->arg[0]);
						clr.green = atoul(skinsep->arg[1]);
						clr.blue = atoul(skinsep->arg[2]);

						switch(type)
						{
							case SPAWN_SET_SKIN_COLOR:
								((Entity*)target)->SetSkinColor(clr);
							break;
							case SPAWN_SET_HAIR_COLOR1:
								((Entity*)target)->SetHairColor1(clr);
							break;
							case SPAWN_SET_HAIR_COLOR2:
								((Entity*)target)->SetHairColor2(clr);
							break;
							case SPAWN_SET_HAIR_TYPE_COLOR:
								((Entity*)target)->SetHairColor(clr);
							break;
							case SPAWN_SET_HAIR_FACE_COLOR:
								((Entity*)target)->SetFacialHairColor(clr);
							break;
							case SPAWN_SET_HAIR_TYPE_HIGHLIGHT_COLOR:
								((Entity*)target)->SetHairTypeHighlightColor(clr);
							break;
							case SPAWN_SET_HAIR_FACE_HIGHLIGHT_COLOR:
								((Entity*)target)->SetFacialHairHighlightColor(clr);
							break;
							case SPAWN_SET_HAIR_HIGHLIGHT:
								((Entity*)target)->SetHairHighlightColor(clr);
							break;
							case SPAWN_SET_MODEL_COLOR:
								((Entity*)target)->SetModelColor(clr);
							break;
							case SPAWN_SET_EYE_COLOR:
								((Entity*)target)->SetEyeColor(clr);
							break;
							case SPAWN_SET_SOGA_SKIN_COLOR:
								((Entity*)target)->SetSogaSkinColor(clr);
							break;
							case SPAWN_SET_SOGA_HAIR_COLOR1:
								((Entity*)target)->SetSogaHairColor1(clr);
							break;
							case SPAWN_SET_SOGA_HAIR_COLOR2:
								((Entity*)target)->SetSogaHairColor2(clr);
							break;
							case SPAWN_SET_SOGA_HAIR_TYPE_COLOR:
								((Entity*)target)->SetSogaHairColor(clr);
							break;
							case SPAWN_SET_SOGA_HAIR_FACE_COLOR:
								((Entity*)target)->SetSogaFacialHairColor(clr);
							break;
							case SPAWN_SET_SOGA_HAIR_TYPE_HIGHLIGHT_COLOR:
								((Entity*)target)->SetSogaHairTypeHighlightColor(clr);
							break;
							case SPAWN_SET_SOGA_HAIR_FACE_HIGHLIGHT_COLOR:
								((Entity*)target)->SetSogaFacialHairHighlightColor(clr);
							break;
							case SPAWN_SET_SOGA_HAIR_HIGHLIGHT:
								((Entity*)target)->SetSogaHairHighlightColor(clr);
							break;
							case SPAWN_SET_SOGA_MODEL_COLOR:
								((Entity*)target)->SetSogaModelColor(clr);
							break;
							case SPAWN_SET_SOGA_EYE_COLOR:
								((Entity*)target)->SetSogaEyeColor(clr);
							break;
						}
					}
					safe_delete(skinsep);
				}
				break;
			}
			case SPAWN_SET_AAXP_REWARDS: {
				target->SetAAXPRewards(atoul(value));
				break;
			}
			case SPAWN_SET_CHEEK_TYPE:{
				if(target->IsEntity() && index < 3){
					sprintf(tmp, "%i", ((Entity*)target)->features.cheek_type[index]);
					sint8 new_value = atoi(value);
					((Entity*)target)->features.cheek_type[index] = new_value;
					((Entity*)target)->info_changed = true;
				}
				break;
			}
			case SPAWN_SET_CHIN_TYPE:{
				if(target->IsEntity() && index < 3){
					sprintf(tmp, "%i", ((Entity*)target)->features.chin_type[index]);
					sint8 new_value = atoi(value);
					((Entity*)target)->features.chin_type[index] = new_value;
					((Entity*)target)->info_changed = true;
				}
				break;
			}
			case SPAWN_SET_EAR_TYPE:{
				if(target->IsEntity() && index < 3){
					sprintf(tmp, "%i", ((Entity*)target)->features.ear_type[index]);
					sint8 new_value = atoi(value);
					((Entity*)target)->features.ear_type[index] = new_value;
					((Entity*)target)->info_changed = true;
				}
				break;
			}
			case SPAWN_SET_EYE_BROW_TYPE:{
				if(target->IsEntity() && index < 3){
					sprintf(tmp, "%i", ((Entity*)target)->features.eye_brow_type[index]);
					sint8 new_value = atoi(value);
					((Entity*)target)->features.eye_brow_type[index] = new_value;
					((Entity*)target)->info_changed = true;
				}
				break;
			}
			case SPAWN_SET_EYE_TYPE:{
				if(target->IsEntity() && index < 3){
					sprintf(tmp, "%i", ((Entity*)target)->features.eye_type[index]);
					sint8 new_value = atoi(value);
					((Entity*)target)->features.eye_type[index] = new_value;
					((Entity*)target)->info_changed = true;
				}
				break;
			}
			case SPAWN_SET_LIP_TYPE:{
				if(target->IsEntity() && index < 3){
					sprintf(tmp, "%i", ((Entity*)target)->features.lip_type[index]);
					sint8 new_value = atoi(value);
					((Entity*)target)->features.lip_type[index] = new_value;
					((Entity*)target)->info_changed = true;
				}
				break;
			}
			case SPAWN_SET_NOSE_TYPE:{
				if(target->IsEntity() && index < 3){
					sprintf(tmp, "%i", ((Entity*)target)->features.nose_type[index]);
					sint8 new_value = atoi(value);
					((Entity*)target)->features.nose_type[index] = new_value;
					((Entity*)target)->info_changed = true;
				}
				break;
			}
			case SPAWN_SET_BODY_SIZE:{
				if(target->IsEntity()){
					sprintf(tmp, "%i", ((Entity*)target)->features.body_size);
					int8 new_value = atoul(value);
					((Entity*)target)->features.body_size = new_value;
					((Entity*)target)->info_changed = true;
				}
				break;
			}
			case SPAWN_SET_BODY_AGE:{
				if(target->IsEntity()){
					sprintf(tmp, "%i", ((Entity*)target)->features.body_age);
					int8 new_value = atoul(value);
					((Entity*)target)->features.body_age = new_value;
					((Entity*)target)->info_changed = true;
				}
				break;
			}
			case SPAWN_SET_SOGA_CHEEK_TYPE:{
				if(target->IsEntity() && index < 3){
					sprintf(tmp, "%i", ((Entity*)target)->features.soga_cheek_type[index]);
					sint8 new_value = atoi(value);
					((Entity*)target)->features.soga_cheek_type[index] = new_value;
					((Entity*)target)->info_changed = true;
				}
				break;
			}
			case SPAWN_SET_SOGA_CHIN_TYPE:{
				if(target->IsEntity() && index < 3){
					sprintf(tmp, "%i", ((Entity*)target)->features.soga_chin_type[index]);
					sint8 new_value = atoi(value);
					((Entity*)target)->features.soga_chin_type[index] = new_value;
					((Entity*)target)->info_changed = true;
				}
				break;
			}
			case SPAWN_SET_SOGA_EAR_TYPE:{
				if(target->IsEntity() && index < 3){
					sprintf(tmp, "%i", ((Entity*)target)->features.soga_ear_type[index]);
					sint8 new_value = atoi(value);
					((Entity*)target)->features.soga_ear_type[index] = new_value;
					((Entity*)target)->info_changed = true;
				}
				break;
			}
			case SPAWN_SET_SOGA_EYE_BROW_TYPE:{
				if(target->IsEntity() && index < 3){
					sprintf(tmp, "%i", ((Entity*)target)->features.soga_eye_brow_type[index]);
					sint8 new_value = atoi(value);
					((Entity*)target)->features.soga_eye_brow_type[index] = new_value;
					((Entity*)target)->info_changed = true;
				}
				break;
			}
			case SPAWN_SET_SOGA_EYE_TYPE:{
				if(target->IsEntity() && index < 3){
					sprintf(tmp, "%i", ((Entity*)target)->features.soga_eye_type[index]);
					sint8 new_value = atoi(value);
					((Entity*)target)->features.soga_eye_type[index] = new_value;
					((Entity*)target)->info_changed = true;
				}
				break;
			}
			case SPAWN_SET_SOGA_LIP_TYPE:{
				if(target->IsEntity() && index < 3){
					sprintf(tmp, "%i", ((Entity*)target)->features.soga_lip_type[index]);
					sint8 new_value = atoi(value);
					((Entity*)target)->features.soga_lip_type[index] = new_value;
					((Entity*)target)->info_changed = true;
				}
				break;
			}
			case SPAWN_SET_SOGA_BODY_SIZE:{
				if(target->IsEntity()){
					sprintf(tmp, "%i", ((Entity*)target)->features.soga_body_size);
					int8 new_value = atoul(value);
					((Entity*)target)->features.soga_body_size = new_value;
					((Entity*)target)->info_changed = true;
				}
				break;
			}
			case SPAWN_SET_SOGA_BODY_AGE:{
				if(target->IsEntity()){
					sprintf(tmp, "%i", ((Entity*)target)->features.soga_body_age);
					int8 new_value = atoul(value);
					((Entity*)target)->features.soga_body_age = new_value;
					((Entity*)target)->info_changed = true;
				}
				break;
			}
			case SPAWN_SET_ATTACK_TYPE:{
				if(target->IsEntity()){
					sprintf(tmp, "%u", ((Entity*)target)->GetInfoStruct()->get_attack_type());
					int8 new_value = atoul(value);
					((Entity*)target)->GetInfoStruct()->set_attack_type(new_value);
				}
				break;
			}
			case SPAWN_SET_RACE_TYPE:{
				if(target->GetModelType() > 0){
					Seperator* tmpsep = new Seperator(value, ' ', 3, 500, true);
					if (tmpsep->IsNumber(0))
					{
						int16 race_type = atoul(value);
						const char* category = tmpsep->IsSet(1) ? tmpsep->arg[1] : "NULL";
						const char* subcategory = tmpsep->IsSet(2) ? tmpsep->arg[2] : "NULL";
						const char* model_name = tmpsep->IsSet(3) ? tmpsep->arg[3] : "NULL";
						if(race_types_list.AddRaceType(target->GetModelType(), race_type, category, subcategory, model_name)) {
							if(client) {
								client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Race type added to memory, however not saved to database.");
							}
								
						}
						else if(client) {							
								client->SimpleMessage(CHANNEL_COLOR_RED, "Model type already on the race_types_list, use spawn set database to override and update database.");
						}
					}
					safe_delete(tmpsep);
				}
				break;
			}
			case SPAWN_SET_LOOT_TIER:{
				if(target->IsEntity()){
					sprintf(tmp, "%u", target->GetLootTier());
					int32 new_value = atoul(value);
					target->SetLootTier(new_value);
				}
				break;
			}
			case SPAWN_SET_LOOT_DROP_TYPE:{
				if(target->IsEntity()){
					sprintf(tmp, "%u", target->GetLootDropType());
					int32 new_value = atoul(value);
					target->SetLootDropType(new_value);
				}
				break;
			}
			case SPAWN_SET_SCARED_STRONG_PLAYERS:{
				if(target->IsNPC()){
					sprintf(tmp, "%u", target->IsScaredByStrongPlayers());
					int32 new_value = atoul(value);
					target->SetScaredByStrongPlayers(new_value);
				}
				break;
			}

			if(temp_value)
				*temp_value = string(tmp);
		}
	}
	else{ 
		/**** NOT TEMPORARY ELSE STATEMENT ****/
		/**** MUST RE-DEFINE WHAT IS ALREADY IN THE IF TEMPORARY BLOCK HERE ****/

		switch(type){
		case SPAWN_SET_VALUE_MERCHANT_MIN_LEVEL: {
			target->SetMerchantLevelRange(atoul(value), target->GetMerchantMaxLevel());
			break;
		}
		case SPAWN_SET_VALUE_MERCHANT_MAX_LEVEL: {
			target->SetMerchantLevelRange(target->GetMerchantMinLevel(), atoul(value));
			break;
		}
		case SPAWN_SET_VALUE_EXPANSION_FLAG: {

			if (target->GetDatabaseID() > 0)
			{
				char query[256];
				snprintf(query, 256, "update spawn set expansion_flag=%u where id=%u", atoul(value), target->GetDatabaseID());
				if (database.RunQuery(query, strnlen(query, 256)))
				{
					if(client)
						client->Message(CHANNEL_COLOR_RED, "Ran query:%s", query);
				}
			}
			break;
		}
		case SPAWN_SET_VALUE_HOLIDAY_FLAG: {

			if (target->GetDatabaseID() > 0)
			{
				char query[256];
				snprintf(query, 256, "update spawn set holiday_flag=%u where id=%u", atoul(value), target->GetDatabaseID());
				if (database.RunQuery(query, strnlen(query, 256)))
				{
					if(client)
						client->Message(CHANNEL_COLOR_RED, "Ran query:%s", query);
				}
			}
			break;
		}
		case SPAWN_SET_AAXP_REWARDS: {

			if (target->GetDatabaseID() > 0)
			{
				char query[256];
				snprintf(query, 256, "update spawn set aaxp_rewards=%u where id=%u", atoul(value), target->GetDatabaseID());
				if (database.RunQuery(query, strnlen(query, 256)))
				{
					if(client)
						client->Message(CHANNEL_COLOR_RED, "Ran query:%s", query);
				}
			}
			break;
		}
			case SPAWN_SET_VALUE_NAME:{
				target->SetName(value);
				if(target->GetZone())
					target->GetZone()->SendUpdateTitles(target);
				break;
									  }
			case SPAWN_SET_VALUE_SUB_TITLE: {
				target->SetSubTitle(value);
				if(target->GetZone())
					target->GetZone()->SendUpdateTitles(target);
				break;
											}
			case SPAWN_SET_VALUE_X_OFFSET: 
				{
				target->SetXOffset(atof(value));
				break;
				}
			case SPAWN_SET_VALUE_Y_OFFSET: 
				{
				target->SetYOffset(atof(value));
				break;
				}
			case SPAWN_SET_VALUE_Z_OFFSET: 
				{
				target->SetZOffset(atof(value));
				break;
				}
			case SPAWN_SET_VALUE_EXPIRE: {
				target->SetExpireTime(val);
				break;
											}
			case SPAWN_SET_VALUE_EXPIRE_OFFSET: {
				target->SetExpireOffsetTime(val);
				break;
											}
			case SPAWN_SET_VALUE_LEVEL:{
				target->SetLevel(val, send_update);
				break;
									   }
			case SPAWN_SET_VALUE_DIFFICULTY:{
				target->SetDifficulty(val, send_update);
				break;
											}
			case SPAWN_SET_VALUE_MODEL_TYPE:{
				target->SetModelType(val, send_update);
				break;
											}
			case SPAWN_SET_VALUE_CLASS:{
				target->SetAdventureClass(val, send_update);
				break;
									   }
			case SPAWN_SET_VALUE_GENDER:{
				target->SetGender(val, send_update);
				break;
										}
			case SPAWN_SET_VALUE_SHOW_NAME:{
				target->SetShowName(val, send_update);
				break;
										   }
			case SPAWN_SET_VALUE_ATTACKABLE:{
				target->SetAttackable(val, send_update);
				break;
											}
			case SPAWN_SET_VALUE_SHOW_LEVEL:{
				target->SetShowLevel(val, send_update);
				break;
											}
			case SPAWN_SET_VALUE_TARGETABLE:{
				target->SetTargetable(val, send_update);
				break;
											}
			case SPAWN_SET_VALUE_SHOW_COMMAND_ICON:{
				target->SetShowCommandIcon(val, send_update);
				break;
												   }
			case SPAWN_SET_VALUE_HAND_ICON:{
				target->SetShowHandIcon(val, send_update);
				break;
										   }
			case SPAWN_SET_VALUE_HAIR_TYPE:{
				if(target->IsEntity())
					((Entity*)target)->SetHairType(val, send_update);
				break;
										   }
			case SPAWN_SET_VALUE_FACIAL_HAIR_TYPE:{
				if(target->IsEntity())
					((Entity*)target)->SetFacialHairType(val, send_update);
				break;
												  }
			case SPAWN_SET_VALUE_WING_TYPE:{
				if(target->IsEntity())
					((Entity*)target)->SetWingType(val, send_update);
				break;
										   }
			case SPAWN_SET_VALUE_CHEST_TYPE:{
				if(target->IsEntity())
					((Entity*)target)->SetChestType(val, send_update);
				break;
											}
			case SPAWN_SET_VALUE_LEGS_TYPE:{
				if(target->IsEntity())
					((Entity*)target)->SetLegsType(val, send_update);
				break;
										   }
			case SPAWN_SET_VALUE_SOGA_HAIR_TYPE:{
				if(target->IsEntity())
					((Entity*)target)->SetSogaHairType(val, send_update);
				break;
												}
			case SPAWN_SET_VALUE_SOGA_FACIAL_HAIR_TYPE:{
				if(target->IsEntity())
					((Entity*)target)->SetSogaFacialHairType(val, send_update);
				break;
													   }
			case SPAWN_SET_VALUE_SOGA_MODEL_TYPE:{
				target->SetSogaModelType(val, send_update);
				break;
												 }
			case SPAWN_SET_VALUE_SIZE:{
				target->SetSize(val, send_update);
				break;
									  }
			case SPAWN_SET_VALUE_HP:{
				target->SetTotalHPBase(val);
				target->SetHP(val, send_update);
				break;
									}
			case SPAWN_SET_VALUE_POWER:{
				target->SetTotalPowerBase(val);
				target->SetPower(val, send_update);
				break;
									   }
			case SPAWN_SET_VALUE_HEROIC:{
				target->SetHeroic(val, send_update);
				break;
										}
			case SPAWN_SET_VALUE_RESPAWN:{
				target->SetRespawnTime(val);
				break;
										 }
			case SPAWN_SET_VALUE_X:{
				target->SetX(atof(value), send_update);
				target->SetSpawnOrigX(target->GetX());
				break;
								   }
			case SPAWN_SET_VALUE_Y:{
				target->SetY(atof(value), send_update);
				target->SetSpawnOrigY(target->GetY());
				break;
								   }
			case SPAWN_SET_VALUE_Z:{
				target->SetZ(atof(value), send_update);
				target->SetSpawnOrigZ(target->GetZ());
				break;
								   }
			case SPAWN_SET_VALUE_HEADING:{
				target->SetHeading(atof(value), send_update);
				target->SetSpawnOrigHeading(target->GetHeading());
				break;
										 }
			case SPAWN_SET_VALUE_COMMAND_PRIMARY:{
				ZoneServer* zone = target->GetZone();
				if (!zone && client)
					zone = client->GetCurrentZone();

				if(zone && zone->GetEntityCommandList(val))
					target->SetPrimaryCommands(zone->GetEntityCommandList(val));
				target->primary_command_list_id = val;
				break;
												 }
			case SPAWN_SET_VALUE_COMMAND_SECONDARY:{
				ZoneServer* zone = target->GetZone();
				if (!zone && client)
					zone = client->GetCurrentZone();

				if (zone && zone->GetEntityCommandList(val))
					target->SetSecondaryCommands(zone->GetEntityCommandList(val));
				target->secondary_command_list_id = val;
				break;
												   }
			case SPAWN_SET_VALUE_VISUAL_STATE:{
				target->SetVisualState(val, send_update);
				break;
											  }
			case SPAWN_SET_VALUE_ACTION_STATE:{
				target->SetActionState(val, send_update);
				break;
											  }
			case SPAWN_SET_VALUE_MOOD_STATE:{
				target->SetMoodState(val, send_update);
				break;
											}
			case SPAWN_SET_VALUE_INITIAL_STATE:{
				target->SetInitialState(val, send_update);
				break;
											   }
			case SPAWN_SET_VALUE_ACTIVITY_STATE:{
				target->SetActivityStatus(val, send_update);
				break;
												}
			case SPAWN_SET_VALUE_COLLISION_RADIUS:{
				target->SetCollisionRadius(val, send_update);
				break;
												  }
			case SPAWN_SET_VALUE_FACTION:{
				ZoneServer* zone = target->GetZone();
				if (!zone && client)
					zone = client->GetCurrentZone();

				target->faction_id = val;
				if(zone)
				{
					zone->RemoveDeadEnemyList(target);
					if(target->IsNPC())
						zone->AddEnemyList((NPC*)target);
				}
				break;
										 }
			case SPAWN_SET_VALUE_DEVICE_ID:{
				if (target->IsObject())
					((Object*)target)->SetDeviceID(val);
				break;
										   }
			case SPAWN_SET_VALUE_PITCH: {
				target->SetPitch(atof(value), send_update);
				target->SetSpawnOrigPitch(atof(value));
				break;
										}
			case SPAWN_SET_VALUE_ROLL: {
				target->SetRoll(atof(value), send_update);
				target->SetSpawnOrigRoll(atof(value));
				break;
									   }
			case SPAWN_SET_VALUE_HIDE_HOOD: {
				target->SetHideHood(val);
				break;
											}
			case SPAWN_SET_VALUE_EMOTE_STATE: {
				target->SetEmoteState(val);
				break;
											  }
			case SPAWN_SET_VALUE_ICON: {
				target->SetIcon(val);
				break;
									   }
			case SPAWN_SET_VALUE_PREFIX: {
				target->SetPrefixTitle(value);
				if(target->GetZone())
					target->GetZone()->SendUpdateTitles(target);
				break;
										 }
			case SPAWN_SET_VALUE_SUFFIX: {
				target->SetSuffixTitle(value);
				if(target->GetZone())
					target->GetZone()->SendUpdateTitles(target);
				break;
			}
			case SPAWN_SET_VALUE_LASTNAME: {
				target->SetLastName(value);
				if(target->GetZone())
					target->GetZone()->SendUpdateTitles(target);
			    break;
			}
			case SPAWN_SET_VALUE_SPAWN_SCRIPT:{
				if(lua_interface && lua_interface->GetSpawnScript(value) == 0){
					if(client){
						client->Message(CHANNEL_COLOR_RED, "Invalid Spawn Script file.  Be sure you give the absolute path.");
						client->Message(CHANNEL_COLOR_RED, "Example: /spawn set spawn_script 'SpawnScripts/example.lua'");
					}
					return false;
				}
				else if(!database.UpdateSpawnScriptData(target->GetDatabaseID(), 0, 0, value))
					return false;
				else{
					if(!world.GetSpawnLocationScript(target->GetSpawnLocationID()))
						target->SetSpawnScript(value);
				}
				break;
											  }
			case SPAWN_SET_VALUE_SPAWNLOCATION_SCRIPT:{
				if(lua_interface && lua_interface->GetSpawnScript(value) == 0){
					if(client){
						client->Message(CHANNEL_COLOR_RED, "Invalid Spawn Script file.  Be sure you give the absolute path.");
						client->Message(CHANNEL_COLOR_RED, "Example: /spawn set spawnlocation_script 'SpawnScripts/example.lua'");
					}
					return false;
				}
				else if(!database.UpdateSpawnScriptData(0, target->GetSpawnLocationID(), 0, value))
					return false;
				else{
					if(!world.GetSpawnEntryScript(target->GetSpawnEntryID()))
						target->SetSpawnScript(value);
				}
				break;
												   }
			case SPAWN_SET_VALUE_SPAWNENTRY_SCRIPT:{
				if(lua_interface && lua_interface->GetSpawnScript(value) == 0){
					if(client){
						client->Message(CHANNEL_COLOR_RED, "Invalid Spawn Script file.  Be sure you give the absolute path.");
						client->Message(CHANNEL_COLOR_RED, "Example: /spawn set spawnentry_script 'SpawnScripts/example.lua'");
					}
					return false;
				}
				else if(!database.UpdateSpawnScriptData(0, 0, target->GetSpawnEntryID(), value))
					return false;
				else
					target->SetSpawnScript(value);
				break;
			}

			case SPAWN_SET_SKIN_COLOR:
			case SPAWN_SET_HAIR_COLOR1:
			case SPAWN_SET_HAIR_COLOR2:
			case SPAWN_SET_HAIR_TYPE_COLOR:
			case SPAWN_SET_HAIR_FACE_COLOR:
			case SPAWN_SET_HAIR_TYPE_HIGHLIGHT_COLOR:
			case SPAWN_SET_HAIR_FACE_HIGHLIGHT_COLOR:
			case SPAWN_SET_HAIR_HIGHLIGHT:
			case SPAWN_SET_MODEL_COLOR:
			case SPAWN_SET_EYE_COLOR:
			case SPAWN_SET_SOGA_SKIN_COLOR:
			case SPAWN_SET_SOGA_HAIR_COLOR1:
			case SPAWN_SET_SOGA_HAIR_COLOR2:
			case SPAWN_SET_SOGA_HAIR_TYPE_COLOR:
			case SPAWN_SET_SOGA_HAIR_FACE_COLOR:
			case SPAWN_SET_SOGA_HAIR_TYPE_HIGHLIGHT_COLOR:
			case SPAWN_SET_SOGA_HAIR_FACE_HIGHLIGHT_COLOR:
			case SPAWN_SET_SOGA_HAIR_HIGHLIGHT:
			case SPAWN_SET_SOGA_MODEL_COLOR:
			case SPAWN_SET_SOGA_EYE_COLOR: {
				if (target->IsNPC())
				{
					Seperator* skinsep = new Seperator(value, ' ', 3, 500, true);
					if (skinsep->IsNumber(0) && skinsep->IsNumber(1) && skinsep->IsNumber(2))
					{
						EQ2_Color clr;
						clr.red = atoul(skinsep->arg[0]);
						clr.green = atoul(skinsep->arg[1]);
						clr.blue = atoul(skinsep->arg[2]);
						Query replaceSkinQuery;

						string fieldName("");
						switch(type)
						{
							case SPAWN_SET_SKIN_COLOR:
								((Entity*)target)->SetSkinColor(clr);
								fieldName.append("skin_color");
								break;
							case SPAWN_SET_HAIR_COLOR1:
								((Entity*)target)->SetHairColor1(clr);
								fieldName.append("hair_color1");
								break;
							case SPAWN_SET_HAIR_COLOR2:
								((Entity*)target)->SetHairColor2(clr);
								fieldName.append("hair_color2");
								break;
							case SPAWN_SET_HAIR_TYPE_COLOR:
								((Entity*)target)->SetHairColor(clr);
								fieldName.append("hair_type_color");
								break;
							case SPAWN_SET_HAIR_FACE_COLOR:
								((Entity*)target)->SetFacialHairColor(clr);
								fieldName.append("hair_face_color");
								break;
							case SPAWN_SET_HAIR_TYPE_HIGHLIGHT_COLOR:
								((Entity*)target)->SetHairTypeHighlightColor(clr);
								fieldName.append("hair_type_highlight_color");
								break;
							case SPAWN_SET_HAIR_FACE_HIGHLIGHT_COLOR:
								((Entity*)target)->SetFacialHairHighlightColor(clr);
								fieldName.append("hair_face_highlight_color");
								break;
							case SPAWN_SET_HAIR_HIGHLIGHT:
								((Entity*)target)->SetHairHighlightColor(clr);
								fieldName.append("hair_highlight");
								break;
							case SPAWN_SET_MODEL_COLOR:
								((Entity*)target)->SetModelColor(clr);
								fieldName.append("model_color");
								break;
							case SPAWN_SET_EYE_COLOR:
								((Entity*)target)->SetEyeColor(clr);
								fieldName.append("eye_color");
								break;
							case SPAWN_SET_SOGA_SKIN_COLOR:
								((Entity*)target)->SetSogaSkinColor(clr);
								fieldName.append("soga_skin_color");
								break;
							case SPAWN_SET_SOGA_HAIR_COLOR1:
								((Entity*)target)->SetSogaHairColor1(clr);
								fieldName.append("soga_hair_color1");
								break;
							case SPAWN_SET_SOGA_HAIR_COLOR2:
								((Entity*)target)->SetSogaHairColor2(clr);
								fieldName.append("soga_hair_color2");
								break;
							case SPAWN_SET_SOGA_HAIR_TYPE_COLOR:
								((Entity*)target)->SetSogaHairColor(clr);
								fieldName.append("soga_hair_type_color");
								break;
							case SPAWN_SET_SOGA_HAIR_FACE_COLOR:
								((Entity*)target)->SetSogaFacialHairColor(clr);
								fieldName.append("soga_hair_face_color");
								break;
							case SPAWN_SET_SOGA_HAIR_TYPE_HIGHLIGHT_COLOR:
								((Entity*)target)->SetSogaHairTypeHighlightColor(clr);
								fieldName.append("soga_hair_type_highlight_color");
								break;
							case SPAWN_SET_SOGA_HAIR_FACE_HIGHLIGHT_COLOR:
								((Entity*)target)->SetSogaFacialHairHighlightColor(clr);
								fieldName.append("soga_hair_face_highlight_color");
								break;
							case SPAWN_SET_SOGA_HAIR_HIGHLIGHT:
								((Entity*)target)->SetSogaHairHighlightColor(clr);
								fieldName.append("soga_hair_highlight");
								break;
							case SPAWN_SET_SOGA_MODEL_COLOR:
								((Entity*)target)->SetSogaModelColor(clr);
								fieldName.append("soga_model_color");
								break;
							case SPAWN_SET_SOGA_EYE_COLOR:
								((Entity*)target)->SetSogaEyeColor(clr);
								fieldName.append("soga_eye_color");
								break;
						}
						replaceSkinQuery.AddQueryAsync(0, &database, Q_DELETE, "delete from npc_appearance where spawn_id=%u and type='%s'", target->GetDatabaseID(), fieldName.c_str());
						replaceSkinQuery.AddQueryAsync(0, &database, Q_INSERT, "insert into npc_appearance set spawn_id=%u, type='%s', red=%u, green=%u, blue=%u", target->GetDatabaseID(), fieldName.c_str(), clr.red, clr.green, clr.blue);
					}
					safe_delete(skinsep);
				}
				break;
			}
			
			case SPAWN_SET_CHEEK_TYPE:{
				if(target->IsEntity() && index < 3){
					sint8 new_value = atoi(value);
					((Entity*)target)->features.cheek_type[index] = new_value;
					UpdateDatabaseAppearance(client, target, "cheek_type", ((Entity*)target)->features.cheek_type[0], ((Entity*)target)->features.cheek_type[1], ((Entity*)target)->features.cheek_type[2]);
				}
				break;
			}
			case SPAWN_SET_CHIN_TYPE:{
				if(target->IsEntity() && index < 3){
					sint8 new_value = atoi(value);
					((Entity*)target)->features.chin_type[index] = new_value;
					UpdateDatabaseAppearance(client, target, "chin_type", ((Entity*)target)->features.chin_type[0], ((Entity*)target)->features.chin_type[1], ((Entity*)target)->features.chin_type[2]);
				}
				break;
			}
			case SPAWN_SET_EAR_TYPE:{
				if(target->IsEntity() && index < 3){
					sint8 new_value = atoi(value);
					((Entity*)target)->features.ear_type[index] = new_value;
					UpdateDatabaseAppearance(client, target, "ear_type", ((Entity*)target)->features.ear_type[0], ((Entity*)target)->features.ear_type[1], ((Entity*)target)->features.ear_type[2]);
				}
				break;
			}
			case SPAWN_SET_EYE_BROW_TYPE:{
				if(target->IsEntity() && index < 3){
					sint8 new_value = atoi(value);
					((Entity*)target)->features.eye_brow_type[index] = new_value;
					UpdateDatabaseAppearance(client, target, "eye_brow_type", ((Entity*)target)->features.eye_brow_type[0], ((Entity*)target)->features.eye_brow_type[1], ((Entity*)target)->features.eye_brow_type[2]);
				}
				break;
			}
			case SPAWN_SET_EYE_TYPE:{
				if(target->IsEntity() && index < 3){
					sint8 new_value = atoi(value);
					((Entity*)target)->features.eye_type[index] = new_value;
					UpdateDatabaseAppearance(client, target, "eye_type", ((Entity*)target)->features.eye_type[0], ((Entity*)target)->features.eye_type[1], ((Entity*)target)->features.eye_type[2]);
				}
				break;
			}
			case SPAWN_SET_LIP_TYPE:{
				if(target->IsEntity() && index < 3){
					sint8 new_value = atoi(value);
					((Entity*)target)->features.lip_type[index] = new_value;
					UpdateDatabaseAppearance(client, target, "lip_type", ((Entity*)target)->features.lip_type[0], ((Entity*)target)->features.lip_type[1], ((Entity*)target)->features.lip_type[2]);
				}
				break;
			}
			case SPAWN_SET_NOSE_TYPE:{
				if(target->IsEntity() && index < 3){
					sint8 new_value = atoi(value);
					((Entity*)target)->features.nose_type[index] = new_value;
					UpdateDatabaseAppearance(client, target, "nose_type", ((Entity*)target)->features.nose_type[0], ((Entity*)target)->features.nose_type[1], ((Entity*)target)->features.nose_type[2]);
				}
				break;
			}
			case SPAWN_SET_BODY_SIZE:{
				if(target->IsEntity()){
					int8 new_value = atoul(value);
					((Entity*)target)->features.body_size = new_value;
					UpdateDatabaseAppearance(client, target, "body_size", ((Entity*)target)->features.body_size, 0, 0);
				}
				break;
			}
			case SPAWN_SET_BODY_AGE:{
				if(target->IsEntity()){
					int8 new_value = atoul(value);
					((Entity*)target)->features.body_age = new_value;
					UpdateDatabaseAppearance(client, target, "body_age", ((Entity*)target)->features.body_age, 0, 0);
				}
				break;
			}
			case SPAWN_SET_SOGA_CHEEK_TYPE:{
				if(target->IsEntity() && index < 3){
					sint8 new_value = atoi(value);
					((Entity*)target)->features.soga_cheek_type[index] = new_value;
					UpdateDatabaseAppearance(client, target, "soga_cheek_type", ((Entity*)target)->features.soga_cheek_type[0], ((Entity*)target)->features.soga_cheek_type[1], ((Entity*)target)->features.soga_cheek_type[2]);
				}
				break;
			}
			case SPAWN_SET_SOGA_CHIN_TYPE:{
				if(target->IsEntity() && index < 3){
					sint8 new_value = atoi(value);
					((Entity*)target)->features.soga_chin_type[index] = new_value;
					UpdateDatabaseAppearance(client, target, "soga_chin_type", ((Entity*)target)->features.soga_chin_type[0], ((Entity*)target)->features.soga_chin_type[1], ((Entity*)target)->features.soga_chin_type[2]);
				}
				break;
			}
			case SPAWN_SET_SOGA_EAR_TYPE:{
				if(target->IsEntity() && index < 3){
					sint8 new_value = atoi(value);
					((Entity*)target)->features.soga_ear_type[index] = new_value;
					UpdateDatabaseAppearance(client, target, "soga_ear_type", ((Entity*)target)->features.soga_ear_type[0], ((Entity*)target)->features.soga_ear_type[1], ((Entity*)target)->features.soga_ear_type[2]);
				}
				break;
			}
			case SPAWN_SET_SOGA_EYE_BROW_TYPE:{
				if(target->IsEntity() && index < 3){
					sint8 new_value = atoi(value);
					((Entity*)target)->features.soga_eye_brow_type[index] = new_value;
					UpdateDatabaseAppearance(client, target, "soga_eye_brow_type", ((Entity*)target)->features.soga_eye_brow_type[0], ((Entity*)target)->features.soga_eye_brow_type[1], ((Entity*)target)->features.soga_eye_brow_type[2]);
				}
				break;
			}
			case SPAWN_SET_SOGA_EYE_TYPE:{
				if(target->IsEntity() && index < 3){
					sint8 new_value = atoi(value);
					((Entity*)target)->features.soga_eye_type[index] = new_value;
					UpdateDatabaseAppearance(client, target, "soga_eye_type", ((Entity*)target)->features.soga_eye_type[0], ((Entity*)target)->features.soga_eye_type[1], ((Entity*)target)->features.soga_eye_type[2]);
				}
				break;
			}
			case SPAWN_SET_SOGA_LIP_TYPE:{
				if(target->IsEntity() && index < 3){
					sint8 new_value = atoi(value);
					((Entity*)target)->features.soga_lip_type[index] = new_value;
					UpdateDatabaseAppearance(client, target, "soga_lip_type", ((Entity*)target)->features.soga_lip_type[0], ((Entity*)target)->features.soga_lip_type[1], ((Entity*)target)->features.soga_lip_type[2]);
				}
				break;
			}
			case SPAWN_SET_SOGA_NOSE_TYPE:{
				if(target->IsEntity() && index < 3){
					sint8 new_value = atoi(value);
					((Entity*)target)->features.soga_nose_type[index] = new_value;
					UpdateDatabaseAppearance(client, target, "soga_nose_type", ((Entity*)target)->features.soga_nose_type[0], ((Entity*)target)->features.soga_nose_type[1], ((Entity*)target)->features.soga_nose_type[2]);
				}
				break;
			}
			case SPAWN_SET_SOGA_BODY_SIZE:{
				if(target->IsEntity()){
					int8 new_value = atoul(value);
					((Entity*)target)->features.body_size = new_value;
					UpdateDatabaseAppearance(client, target, "body_size", ((Entity*)target)->features.body_size, 0, 0);
				}
				break;
			}
			case SPAWN_SET_SOGA_BODY_AGE:{
				if(target->IsEntity()){
					int8 new_value = atoul(value);
					((Entity*)target)->features.body_age = new_value;
					UpdateDatabaseAppearance(client, target, "body_age", ((Entity*)target)->features.body_age, 0, 0);
				}
				break;
			}
			case SPAWN_SET_ATTACK_TYPE:{
				if(target->IsEntity()){
					int8 new_value = atoul(value);
					((Entity*)target)->GetInfoStruct()->set_attack_type(new_value);
					if(target->IsNPC()) {
						if(target->GetDatabaseID()) {
							Query spawnNPCQuery;					
							spawnNPCQuery.AddQueryAsync(0, &database, Q_INSERT, "update spawn_npcs set attack_type=%u where id=%u", new_value, target->GetDatabaseID());
						} else if(client) {
							client->Message(CHANNEL_COLOR_RED, "Invalid spawn to update the database (NPC only) or no database id for the NPC present.");
						}
					}
				}
				break;
			}
			case SPAWN_SET_RACE_TYPE:{
				if(target->GetModelType() > 0){
					Seperator* tmpsep = new Seperator(value, ' ', 3, 500, true);
					if (tmpsep->IsNumber(0))
					{
						Query typequery;
						int16 race_type = atoul(value);
						const char* category = tmpsep->IsSet(1) ? tmpsep->arg[1] : "NULL";
						const char* subcategory = tmpsep->IsSet(2) ? tmpsep->arg[2] : "NULL";
						const char* model_name = tmpsep->IsSet(3) ? tmpsep->arg[3] : "NULL";
						if(race_types_list.AddRaceType(target->GetModelType(), race_type, category, subcategory, model_name)) {	
						if(client)
								client->Message(CHANNEL_COLOR_YELLOW, "Model Type %u Race type %u inserted into memory + replaced into database, Model Type: %u set to Race Type: %u, Category: %s, SubCategory: %s, Model Name: %s.", race_type, target->GetModelType(), race_type, category, subcategory, model_name);
							typequery.AddQueryAsync(0, &database, Q_INSERT, "insert into race_types (model_type, race_id, category, subcategory, model_name) values(%u, %u, '%s', '%s', '%s')", target->GetModelType(), race_type, category, subcategory, model_name);
						}
						else {
							if(client)
								client->Message(CHANNEL_COLOR_RED, "Model Type: %u Race type %u overrided in memory + replaced into database, Model Type: %u set to Race Type: %u, Category: %s, SubCategory: %s, Model Name: %s.", race_type, target->GetModelType(), race_type, category, subcategory, model_name);
							race_types_list.AddRaceType(target->GetModelType(), race_type, category, subcategory, model_name, true);
							typequery.AddQueryAsync(0, &database, Q_INSERT, "update race_types set race_id=%u, category='%s', subcategory='%s', model_name='%s' where model_type=%u", race_type, category, subcategory, model_name, target->GetModelType());
						}
					}
					safe_delete(tmpsep);
				}
				break;
			}
			case SPAWN_SET_LOOT_TIER:{
				int32 new_value = atoul(value);
				target->SetLootTier(new_value);
					
				if (target->GetDatabaseID() > 0)
				{
					char query[256];
					snprintf(query, 256, "update spawn set loot_tier=%u where id=%u", atoul(value), target->GetDatabaseID());
					if (database.RunQuery(query, strnlen(query, 256)))
					{
						if(client)
							client->Message(CHANNEL_COLOR_RED, "Ran query:%s", query);
					}
				}
				break;
			}
			case SPAWN_SET_LOOT_DROP_TYPE:{
				int32 new_value = atoul(value);
				target->SetLootDropType(new_value);
				
				if (target->GetDatabaseID() > 0)
				{
					char query[256];
					snprintf(query, 256, "update spawn set loot_drop_type=%u where id=%u", atoul(value), target->GetDatabaseID());
					if (database.RunQuery(query, strnlen(query, 256)))
					{
						if(client)
							client->Message(CHANNEL_COLOR_RED, "Ran query:%s", query);
					}
				}
				break;
			}
			case SPAWN_SET_SCARED_STRONG_PLAYERS:{
				int32 new_value = atoul(value);
				target->SetScaredByStrongPlayers(new_value);
				
				if (target->GetDatabaseID() > 0)
				{
					char query[256];
					snprintf(query, 256, "update spawn_npcs set scared_by_strong_players=%u where id=%u", atoul(value), target->GetDatabaseID());
					if (database.RunQuery(query, strnlen(query, 256)))
					{
						if(client)
							client->Message(CHANNEL_COLOR_RED, "Ran query:%s", query);
					}
				}
				break;
			}
		}
	}
	return true;
}

void Commands::UpdateDatabaseAppearance(Client* client, Spawn* target, string fieldName, sint8 r, sint8 g, sint8 b)
{
	Query replaceQuery;

	if(target->IsBot())
	{
		Bot* bot = (Bot*)target;
		if(bot->BotID)
			database.SaveBotFloats(bot->BotID, fieldName.c_str(), r, g, b);
	}
	else if(target->IsPlayer())
	{
		replaceQuery.AddQueryAsync(0, &database, Q_DELETE, "delete from char_colors where char_id=%u and type='%s'", ((Player*)target)->GetCharacterID(), fieldName.c_str());
		replaceQuery.AddQueryAsync(0, &database, Q_INSERT, "insert into char_colors set char_id=%u, type='%s', red=%i, green=%i, blue=%i", ((Player*)target)->GetCharacterID(), fieldName.c_str(), r, g, b);
	}
	else
	{
		replaceQuery.AddQueryAsync(0, &database, Q_DELETE, "delete from npc_appearance where spawn_id=%u and type='%s'", target->GetDatabaseID(), fieldName.c_str());
		replaceQuery.AddQueryAsync(0, &database, Q_INSERT, "insert into npc_appearance set spawn_id=%u, type='%s', red=%i, green=%i, blue=%i", target->GetDatabaseID(), fieldName.c_str(), r, g, b);
	}

	((Entity*)target)->changed = true;
	((Entity*)target)->info_changed = true;
	if(target->GetZone())
		target->GetZone()->AddChangedSpawn(target);
}

/* The zone object will be NULL if the zone is not currently running.  We pass both of these in so we can update 
   the database fields always and also update the zone in memory if it's running. */
bool Commands::SetZoneCommand(Client* client, int32 zone_id, ZoneServer* zone, int8 type, const char* value) {
	if (client && zone_id > 0 && type > 0 && value) {
		sint32 int_value = 0;
		float float_value = 0;
		if (type == ZONE_SET_VALUE_SAFE_X || type == ZONE_SET_VALUE_SAFE_Y || type == ZONE_SET_VALUE_SAFE_Z || type == ZONE_SET_VALUE_UNDERWORLD) {
			try {
				float_value = atof(value);
			}
			catch (...) {
				client->Message(CHANNEL_COLOR_RED, "Error converting '%s' to a float value", value);
				return false;
			}
		}
		else if (type != ZONE_SET_VALUE_NAME && type != ZONE_SET_VALUE_FILE && type != ZONE_SET_VALUE_DESCRIPTION && type != ZONE_SET_VALUE_ZONE_TYPE && type != ZONE_SET_VALUE_LUA_SCRIPT && type != ZONE_SET_VALUE_ZONE_MOTD) {
			try {
				int_value = atoi(value);
			}
			catch (...) {
				client->Message(CHANNEL_COLOR_RED, "Error converting '%s' to an integer value", value);
				return false;
			}
		}
		switch (type) {
			case ZONE_SET_VALUE_EXPANSION_ID: {
				break;
			}
			case ZONE_SET_VALUE_NAME: {
				if (zone)
					zone->SetZoneName(const_cast<char*>(value));
				database.SaveZoneInfo(zone_id, "name", value);
				break;
			}
			case ZONE_SET_VALUE_FILE: {
				if (zone)
					zone->SetZoneFile(const_cast<char*>(value));
				database.SaveZoneInfo(zone_id, "file", value);
				break;
			}
			case ZONE_SET_VALUE_DESCRIPTION: {
				if (zone)
					zone->SetZoneDescription(const_cast<char*>(value));
				database.SaveZoneInfo(zone_id, "description", value);
				break;
			}
			case ZONE_SET_VALUE_SAFE_X: {
				if (zone)
					zone->SetSafeX(float_value);
				database.SaveZoneInfo(zone_id, "safe_x", float_value);
				break;
			}
			case ZONE_SET_VALUE_SAFE_Y: {
				if (zone)
					zone->SetSafeY(float_value);
				database.SaveZoneInfo(zone_id, "safe_y", float_value);
				break;
			}
			case ZONE_SET_VALUE_SAFE_Z: {
				if (zone)
					zone->SetSafeZ(float_value);
				database.SaveZoneInfo(zone_id, "safe_z", float_value);
				break;
			}
			case ZONE_SET_VALUE_UNDERWORLD: {
				if (zone)
					zone->SetUnderWorld(float_value);
				database.SaveZoneInfo(zone_id, "underworld", float_value);
				break;
			}
			case ZONE_SET_VALUE_MIN_RECOMMENDED: {
				break;
			}
			case ZONE_SET_VALUE_MAX_RECOMMENDED: {
				break;
			}
			case ZONE_SET_VALUE_ZONE_TYPE: {
				break;
			}
			case ZONE_SET_VALUE_ALWAYS_LOADED: {
				if (zone)
					zone->SetAlwaysLoaded(int_value == 1);
				database.SaveZoneInfo(zone_id, "always_loaded", int_value);
				break;
			}
			case ZONE_SET_VALUE_CITY_ZONE: {
				if (zone)
					zone->SetCityZone(int_value == 1);
				database.SaveZoneInfo(zone_id, "city_zone", int_value);
				break;
			}
			case ZONE_SET_VALUE_WEATHER_ALLOWED: {
				if (zone)
					zone->SetWeatherAllowed(int_value == 1);
				database.SaveZoneInfo(zone_id, "weather_allowed", int_value);
				break;
			}
			case ZONE_SET_VALUE_MIN_STATUS: {
				if (zone)
					zone->SetMinimumStatus(int_value);
				database.SaveZoneInfo(zone_id, "min_status", int_value);
				break;
			}
			case ZONE_SET_VALUE_MIN_LEVEL: {
				if (zone)
					zone->SetMinimumLevel(int_value);
				database.SaveZoneInfo(zone_id, "min_level", int_value);
				break;
			}
			case ZONE_SET_VALUE_MAX_LEVEL: {
				if (zone)
					zone->SetMaximumLevel(int_value);
				database.SaveZoneInfo(zone_id, "max_level", int_value);
				break;
			}
			case ZONE_SET_VALUE_START_ZONE: {
				database.SaveZoneInfo(zone_id, "start_zone", int_value);
				break;
			}
			case ZONE_SET_VALUE_INSTANCE_TYPE: {
				if (zone)
					zone->SetInstanceType(int_value);
				database.SaveZoneInfo(zone_id, "instance_type", int_value);
				break;
			}
			case ZONE_SET_VALUE_DEFAULT_REENTER_TIME: {
				if (zone)
					zone->SetDefaultReenterTime(int_value);
				database.SaveZoneInfo(zone_id, "default_reenter_time", int_value);
				break;
			}
			case ZONE_SET_VALUE_DEFAULT_RESET_TIME: {
				if (zone)
					zone->SetDefaultResetTime(int_value);
				database.SaveZoneInfo(zone_id, "default_reset_time", int_value);
				break;
			}
			case ZONE_SET_VALUE_DEFAULT_LOCKOUT_TIME: {
				if (zone)
					zone->SetDefaultLockoutTime(int_value);
				database.SaveZoneInfo(zone_id, "default_lockout_time", int_value);
				break;
			}
			case ZONE_SET_VALUE_FORCE_GROUP_TO_ZONE: {
				if (zone)
					zone->SetForceGroupZoneOption(int_value);
				database.SaveZoneInfo(zone_id, "force_group_to_zone", int_value);
				break;
			}
			case ZONE_SET_VALUE_LUA_SCRIPT: {
				if (lua_interface && lua_interface->GetZoneScript(value) == 0) {
					client->Message(CHANNEL_COLOR_RED, "Invalid Zone Script file.  Be sure you give the absolute path.");
					client->Message(CHANNEL_COLOR_RED, "Example: /zone set lua_script 'ZoneScripts/QueensColony.lua'");
					return false;
				}
				else {
					world.AddZoneScript(zone_id, const_cast<char*>(value));
					database.SaveZoneInfo(zone_id, "lua_script", value);
				}
				break;
			}
			case ZONE_SET_VALUE_SHUTDOWN_TIMER: {
				if (zone)
					zone->SetShutdownTimer(int_value);
				database.SaveZoneInfo(zone_id, "shutdown_timer", int_value);
				break;
			}
			case ZONE_SET_VALUE_ZONE_MOTD: {
				if (zone)
					zone->SetZoneMOTD(string(value));
				database.SaveZoneInfo(zone_id, "zone_motd", value);
				break;
			}
			default: {
				client->Message(CHANNEL_COLOR_RED, "Invalid zone attribute %i", type);
				return false;
			}
		}
	}
	else {
		if (client)
			client->SimpleMessage(CHANNEL_COLOR_RED, "An error occured saving new zone data.");
		return false;
	}
	return true;
}

void Commands::Process(int32 index, EQ2_16BitString* command_parms, Client* client, Spawn* targetOverride) {

	if (index >= remote_commands->commands.size()) {
		LogWrite(COMMAND__ERROR, 0, "Command", "Error, command handler of %u was requested, but max handler is %u", index, remote_commands->commands.size());
		return;
	}

	Spawn* cmdTarget = targetOverride ? targetOverride : client->GetPlayer()->GetTarget();

	EQ2_RemoteCommandString* parent_command = 0;
	EQ2_RemoteCommandString* command = &remote_commands->commands[index];
	Seperator* sep = 0;
	if (command_parms->size > 0) {
		sep = new Seperator(command_parms->data.c_str(), ' ', 10, 500, true);
		if (sep && sep->arg[0] && remote_commands->validSubCommand(command->command.data, string(sep->arg[0]))) {
			parent_command = command;
			command = &(remote_commands->subcommands[command->command.data][string(sep->arg[0])]);
			safe_delete(sep);
			if (command_parms->data.length() > (command->command.data.length() + 1))
				sep = new Seperator(command_parms->data.c_str() + (command->command.data.length() + 1), ' ', 10, 500, true);
			LogWrite(COMMAND__DEBUG, 1, "Command", "Handler: %u, COMMAND: '%s', SUBCOMMAND: '%s'", index, parent_command->command.data.c_str(), command->command.data.c_str());
		}
		else
			LogWrite(COMMAND__DEBUG, 1, "Command", "Handler: %u, COMMAND: '%s'", index, command->command.data.c_str());
	}

	int ndx = 0;
	if (command->required_status > client->GetAdminStatus())
	{
		LogWrite(COMMAND__ERROR, 0, "Command", "Player '%s' (%u) needs status %i to use command: %s", client->GetPlayer()->GetName(), client->GetAccountID(), command->required_status, command->command.data.c_str());
		safe_delete(sep);
		client->SimpleMessage(3, "Error: Your status is insufficient for this command.");
		return;
	}

	Player* player = client->GetPlayer();
	LogWrite(COMMAND__DEBUG, 0, "Command", "Player '%s' (%u), Command: %s", player->GetName(), client->GetAccountID(), command->command.data.c_str());

	switch (command->handler) {
	case COMMAND_RELOAD: {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Reload commands:");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/reload structs");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/reload items");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/reload luasystem");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/reload spawnscripts");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/reload spells");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/reload spells npc");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/reload quests");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/reload spawns");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/reload groundspawns");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/reload zonescripts");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/reload entity_commands");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/reload factions");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/reload mail");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/reload guilds");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/reload locations");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/reload rules");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/reload transporters");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/reload startabilities");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/reload voiceovers");
		break;
	}
	case COMMAND_RELOADSTRUCTS: {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Reloading Structs...");
		world.SetReloadingSubsystem("Structs");
		configReader.ReloadStructs();
		world.RemoveReloadingSubSystem("Structs");
		peer_manager.sendPeersMessage("/reloadcommand", command->handler);
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Done!");
		break;
	}
	case COMMAND_RELOAD_QUESTS: {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Reloading Quests...");
		world.SetReloadingSubsystem("Quests");
		master_quest_list.Reload();
		client_list.ReloadQuests();
		zone_list.ReloadClientQuests();
		world.RemoveReloadingSubSystem("Quests");
		peer_manager.sendPeersMessage("/reloadcommand", command->handler);
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Done!");
		break;
	}
	case COMMAND_RELOAD_SPAWNS: {
		client->GetCurrentZone()->ReloadSpawns();
		break;
	}
	case COMMAND_RELOAD_SPELLS: {
		if (sep && sep->arg[0] && strcmp(sep->arg[0], "npc") == 0) {
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Reloading NPC Spells Lists (Note: Must Reload Spawns/Repop to reset npc spells)...");
			world.PurgeNPCSpells();
			database.LoadNPCSpells();
			peer_manager.sendPeersMessage("/reloadcommand", command->handler, 1);
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Done!");
		}
		else {
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Reloading Spells & NPC Spell Lists (Note: Must Reload Spawns/Repop to reset npc spells)...");
			world.SetReloadingSubsystem("Spells");
			zone_list.DeleteSpellProcess();
			master_spell_list.Reload();
			zone_list.LoadSpellProcess();
			world.RemoveReloadingSubSystem("Spells");
			world.PurgeNPCSpells();
			database.LoadNPCSpells();
			peer_manager.sendPeersMessage("/reloadcommand", command->handler);
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Done!");
		}		
		break;
	}
	case COMMAND_RELOAD_GROUNDSPAWNS: {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Reloading Groundspawn Entries...");
		world.SetReloadingSubsystem("GroundSpawns");
		client->GetCurrentZone()->DeleteGroundSpawnItems();
		client->GetCurrentZone()->LoadGroundSpawnEntries();
		world.RemoveReloadingSubSystem("GroundSpawns");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Done!");
		break;
	}

	case COMMAND_RELOAD_ZONESCRIPTS: {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Reloading Zone Scripts...");
		world.SetReloadingSubsystem("ZoneScripts");
		world.ResetZoneScripts();
		database.LoadZoneScriptData();
		if (lua_interface)
			lua_interface->DestroyZoneScripts();
		world.RemoveReloadingSubSystem("ZoneScripts");
		peer_manager.sendPeersMessage("/reloadcommand", command->handler);
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Done!");
		break;
	}
	case COMMAND_RELOAD_PLAYERSCRIPTS: {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Reloading Player Scripts...");
		world.SetReloadingSubsystem("PlayerScripts");
		world.ResetPlayerScripts();
		world.LoadPlayerScripts();
		if (lua_interface)
			lua_interface->DestroyPlayerScripts();
		world.RemoveReloadingSubSystem("PlayerScripts");
		peer_manager.sendPeersMessage("/reloadcommand", command->handler);
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Done!");
		break;
	}
	case COMMAND_RELOAD_ENTITYCOMMANDS: {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Reloading Entity Commands...");
		world.SetReloadingSubsystem("EntityCommands");
		client->GetCurrentZone()->ClearEntityCommands();
		database.LoadEntityCommands(client->GetCurrentZone());
		world.RemoveReloadingSubSystem("EntityCommands");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Done!");
		break;
	}
	case COMMAND_RELOAD_FACTIONS: {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Reloading Factions...");
		world.SetReloadingSubsystem("Factions");
		master_faction_list.Clear();
		database.LoadFactionList();
		world.RemoveReloadingSubSystem("Factions");
		peer_manager.sendPeersMessage("/reloadcommand", command->handler);
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Done!");
		break;
	}
	case COMMAND_RELOAD_MAIL: {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Reloading Mail...");
		zone_list.ReloadMail();
		peer_manager.sendPeersMessage("/reloadcommand", command->handler);
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Done!");
		break;
	}
	case COMMAND_RELOAD_GUILDS: {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Reloading Guilds...");
		world.ReloadGuilds();
		peer_manager.sendPeersMessage("/reloadcommand", command->handler);
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Done!");
		break;
	}
	case COMMAND_RELOAD_LOCATIONS: {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Reloading Locations...");
		client->GetPlayer()->GetZone()->RemoveLocationGrids();
		database.LoadLocationGrids(client->GetPlayer()->GetZone());
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Done!");
		break;
	}
	case COMMAND_RELOAD_RULES: {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Reloading Rules...");
		database.LoadRuleSets(true);
		peer_manager.sendPeersMessage("/reloadcommand", command->handler);
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Done!");
		break;
	}
	case COMMAND_RELOAD_TRANSPORTERS: {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Reloading Transporters in your current zone...");
		database.LoadTransporters(client->GetCurrentZone());
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Done!");
		break;
	}
	case COMMAND_RELOAD_STARTABILITIES: {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Reloading Starting Skills/Spells...");
		world.PurgeStartingLists();
		world.LoadStartingLists();
		peer_manager.sendPeersMessage("/reloadcommand", command->handler);
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Done!");
		break;
	}
	case COMMAND_RELOAD_VOICEOVERS: {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Reloading Voiceovers...");
		world.PurgeVoiceOvers();
		world.LoadVoiceOvers();
		peer_manager.sendPeersMessage("/reloadcommand", command->handler);
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Done!");
		break;
	}
	case COMMAND_READ: {
		if (sep && sep->arg[1][0] && sep->IsNumber(1)) {
			if (strcmp(sep->arg[0], "read") == 0) {
				int32 item_index = atol(sep->arg[1]);
				Item* item = client->GetPlayer()->item_list.GetItemFromIndex(item_index);
				if (item) {
					Spawn* spawn = cmdTarget;
					client->SendShowBook(client->GetPlayer(), item->name, item->book_language, item->book_pages);
					break;
				}
			}
		}
		break;
	}
		case COMMAND_USEABILITY:{
			if (sep && sep->arg[0][0] && sep->IsNumber(0)) {
				if (!client->GetPlayer()->Alive()) {
					client->SimpleMessage(CHANNEL_COLOR_RED, "You cannot do that right now.");
				}
				else {
					int32 spell_id = atoul(sep->arg[0]);
					int8 spell_tier = 0;
					if (sep->arg[1][0] && sep->IsNumber(1))
						spell_tier = atoi(sep->arg[1]);
					else
						spell_tier = client->GetPlayer()->GetSpellTier(spell_id);
					if (!spell_tier)
						spell_tier = 1;
					Spell* spell = master_spell_list.GetSpell(spell_id, spell_tier);
					if (spell) {
						if (strncmp(spell->GetName(), "Gathering", 9) == 0 || strncmp(spell->GetName(), "Mining", 6) == 0 || strncmp(spell->GetName(), "Trapping", 8) == 0 || strncmp(spell->GetName(), "Foresting", 9) == 0 || strncmp(spell->GetName(), "Fishing", 7) == 0 || strncmp(spell->GetName(), "Collecting", 10) == 0)
							client->GetCurrentZone()->ProcessSpell(spell, client->GetPlayer(), cmdTarget, true, true);
						else
						{
							if (cmdTarget)
								client->GetCurrentZone()->ProcessSpell(spell, client->GetPlayer(), cmdTarget);
							else
								client->GetCurrentZone()->ProcessSpell(spell, client->GetPlayer(), client->GetPlayer());
						}
					}
				}
			}
			else if (cmdTarget && cmdTarget->IsWidget())
			{
				Widget* widget = (Widget*)cmdTarget;
				widget->HandleUse(client, "use", WIDGET_TYPE_DOOR);
			}
			else
			{
				client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Usage:  /useability {spell_id} [spell_tier]");
			}
			break;
								}
		case COMMAND_INFO:{
			bool check_self_trade = true;
			if(sep && sep->arg[1][0] && sep->IsNumber(1)){
				if(strcmp(sep->arg[0], "inventory") == 0){
					int32 item_index = atol(sep->arg[1]);
					
					LogWrite(COMMAND__DEBUG, 5, "Command", "/info inventory item original index %u", item_index);
					if(client->GetVersion() <= 561) {
						if(item_index <= 255) {
							item_index = 255 - item_index;
						}
						else {
							if(item_index == 256) { // first "new" item to inventory is assigned index 256 by client
								item_index = client->GetPlayer()->item_list.GetFirstNewItem();
							}
							else {
								// otherwise the slot has to be mapped out depending on the amount of new items + index sent in
								item_index = client->GetPlayer()->item_list.GetNewItemByIndex((int16)item_index - 255);
							}
						}
					}
					Item* item = client->GetPlayer()->item_list.GetItemFromIndex(item_index);
					if(item){
						if (item->IsCollectable() && client->SendCollectionsForItem(item))
							break;
						
						EQ2Packet* app = 0;
						if(client->GetVersion() <= 561) {
							app = item->serialize(client->GetVersion(), true, client->GetPlayer());
						}
						else {
							app = item->serialize(client->GetVersion(), (!item->GetItemScript() || !lua_interface), client->GetPlayer());
						}
						//DumpPacket(app);
						client->QueuePacket(app);
						if(item->GetItemScript() && lua_interface)
							lua_interface->RunItemScript(item->GetItemScript(), "examined", item, client->GetPlayer());
						else if(item->generic_info.offers_quest_id > 0){ //leave the current functionality in place if it doesnt have an item script
							Quest* quest = master_quest_list.GetQuest(item->generic_info.offers_quest_id, false);
							if(quest && client->GetPlayer()->HasQuestBeenCompleted(item->generic_info.offers_quest_id) == 0 && client->GetPlayer()->HasActiveQuest(item->generic_info.offers_quest_id) == 0)
								client->AddPendingQuest(new Quest(quest)); // copy quest since we pulled the master quest to see if it existed or not
						}
					}
					else
						LogWrite(COMMAND__ERROR, 0, "Command", "/info inventory: Unknown Index: %u", item_index);
				}
				else if(strcmp(sep->arg[0], "equipment") == 0){
					int32 item_index = client->GetPlayer()->ConvertSlotFromClient(atol(sep->arg[1]), client->GetVersion());
					Item* item = client->GetPlayer()->GetEquipmentList()->GetItem(item_index);
					if(item){
						EQ2Packet* app = item->serialize(client->GetVersion(), true, client->GetPlayer());
						client->QueuePacket(app);
						if(item->GetItemScript() && lua_interface)
							lua_interface->RunItemScript(item->GetItemScript(), "examined", item, client->GetPlayer());
					}
					else
						LogWrite(COMMAND__ERROR, 0, "Command", "/info equipment: Unknown Index: %u", item_index);
				}
				else if(strcmp(sep->arg[0], "appearance") == 0){
					int32 item_index = client->GetPlayer()->ConvertSlotFromClient(atol(sep->arg[1]), client->GetVersion());
					Item* item = client->GetPlayer()->GetAppearanceEquipmentList()->GetItem(item_index);
					if(item){
						EQ2Packet* app = item->serialize(client->GetVersion(), true, client->GetPlayer());
						client->QueuePacket(app);
						if(item->GetItemScript() && lua_interface)
							lua_interface->RunItemScript(item->GetItemScript(), "examined", item, client->GetPlayer());
					}
					else
						LogWrite(COMMAND__ERROR, 0, "Command", "/info appearance: Unknown Index: %u", item_index);
				}
				else if(strcmp(sep->arg[0], "item") == 0 || strcmp(sep->arg[0], "merchant") == 0 || strcmp(sep->arg[0], "store") == 0 || strcmp(sep->arg[0], "buyback") == 0 || strcmp(sep->arg[0], "consignment") == 0){
					int64 item_id = strtoull(sep->arg[1], NULL, 0);
					Item* item = nullptr;
					
					if (strcmp(sep->arg[0], "store") == 0)
						item = client->GetPlayer()->item_list.GetVaultItemFromUniqueID(item_id, true);
					else
						item = master_item_list.GetItem(item_id);
					
					if(!item && client->GetMerchantTransactionID() && strcmp(sep->arg[0], "merchant") == 0) {
						Spawn* merchant = client->GetPlayer()->GetZone()->GetSpawnByID(client->GetMerchantTransactionID());
						if(merchant && merchant->GetHouseCharacterID() && merchant->GetPickupUniqueItemID()) {
							if(auto itemInfo = broker.GetActiveItem(merchant->GetHouseCharacterID(), item_id)) {
								item = master_item_list.GetItem(itemInfo->item_id);
								if(item) {
									EQ2Packet* app = item->serialize(client->GetVersion(), true, client->GetPlayer());
									client->QueuePacket(app);
								}
							}
						}
					}
					else if(!item && strcmp(sep->arg[0], "consignment") == 0) {
						client->SendSellerItemByItemUniqueId(item_id);
					}
					else if(item){
						EQ2Packet* app = item->serialize(client->GetVersion(), true, client->GetPlayer());
						client->QueuePacket(app);
					}
					else
						LogWrite(COMMAND__ERROR, 0, "Command", "/info item|merchant|store|buyback|consignment: Unknown Item ID: %u (full arguments %s)", item_id, sep->argplus[0]);
				}
				else if (strcmp(sep->arg[0], "spell") == 0) {
					sint32 spell_id = atol(sep->arg[1]);
					int8 tier = atoi(sep->arg[3]);
					EQ2Packet* outapp = master_spell_list.GetSpellPacket(spell_id, tier, client, true, 0x2A);
					if (outapp)
						client->QueuePacket(outapp);
					else
						LogWrite(COMMAND__ERROR, 0, "Command", "/info spell: Unknown Spell ID and/or Tier, ID: %u, Tier: %i", spell_id, tier);
				}
				else if (strcmp(sep->arg[0], "achievement") == 0) {
					sint32 spell_id = atol(sep->arg[2]);
					
					int8 group = atoi(sep->arg[1]);
					AltAdvanceData* data = 0;
					SpellBookEntry* spellentry = 0;
					int8 tier = client->GetPlayer()->GetSpellTier(spell_id);
				
					LogWrite(COMMAND__ERROR, 0, "Command", "/info achievement: AA Spell ID and/or Tier, ID: %u, Group: %i", spell_id, group);
					EQ2Packet* outapp = master_spell_list.GetAASpellPacket(spell_id, tier, client, true, 0x45);
					if (outapp)
						client->QueuePacket(outapp);
					else
						LogWrite(COMMAND__ERROR, 0, "Command", "/info achievement: Unknown Spell ID and/or Tier, ID: %u, Tier: %i", spell_id, group);
				}
				else if (strcmp(sep->arg[0], "spellbook") == 0) {
					sint32 spell_id = atol(sep->arg[1]);
					int32 tier = atoi(sep->arg[2]);
					if (tier > 255) {
						SpellBookEntry* ent = client->GetPlayer()->GetSpellBookSpell(spell_id);
						if (ent)
							tier = ent->tier;
					}
					EQ2Packet* outapp = master_spell_list.GetSpellPacket(spell_id, (int8)tier, client, true, 0x2A);
					if (outapp)
						client->QueuePacket(outapp);
					else
						LogWrite(COMMAND__ERROR, 0, "Command", "/info spellbook: Unknown Spell ID and/or Tier, ID: %u, Tier: %i", spell_id, tier);
				}
				else if (strcmp(sep->arg[0], "recipe") == 0) {
					sint32 recipe_id = atol(sep->arg[1]);
					EQ2Packet* outapp = master_recipe_list.GetRecipePacket(recipe_id, client, true, 0x2C);
					if(outapp)
						client->QueuePacket(outapp);
					else
						LogWrite(COMMAND__ERROR, 0, "Command", "/info recipe: Unknown Recipe ID: %u", recipe_id);
				}
				else if (strcmp(sep->arg[0], "recipe_product") == 0) {
					sint32 recipe_id = atol(sep->arg[1]);
									
					PlayerRecipeList* prl = client->GetPlayer()->GetRecipeList();
					Recipe* recipe = nullptr;
					if ((recipe = prl->GetRecipe(recipe_id)) && recipe->GetProductID()) {
						RecipeProducts* rp = recipe->products[1];
						if (recipe->GetProductID() > 0) {
							Item* item = master_item_list.GetItem(recipe->GetProductID());
							if(item){
								EQ2Packet* app = item->serialize(client->GetVersion(), true, client->GetPlayer());
								client->QueuePacket(app);
							}
							else
								LogWrite(COMMAND__ERROR, 0, "Command", "/info recipe_product: Unknown Item ID: %u", recipe->GetProductID());
						}
						else {
								LogWrite(COMMAND__ERROR, 0, "Command", "/info recipe_product: recipe->GetProductID() has value 0 for recipe id %u.", recipe_id);
						}
					}
					else {
							LogWrite(COMMAND__ERROR, 0, "Command", "/info recipe_product: with recipe id %u not found (recipe missing or no product in stage 1 assigned).", recipe_id);
					}
				}
				else if (strcmp(sep->arg[0], "maintained") ==0) {
					int32 slot = atol(sep->arg[1]);
					int32 spell_id = atol(sep->arg[2]);
					LogWrite(COMMAND__DEBUG, 5, "Command", "/info maintained: Spell ID - Slot: %u unknown: %u", slot, spell_id);
					//int8 tier = client->GetPlayer()->GetSpellTier(spell_id);
					MaintainedEffects* info = client->GetPlayer()->GetMaintainedSpellBySlot(slot);
					EQ2Packet* outapp = master_spell_list.GetSpellPacket(info->spell_id, info->tier, client, true, 0x00);
					if(outapp)
						client->QueuePacket(outapp);
					else
						LogWrite(COMMAND__ERROR, 0, "Command", "/info maintained: Unknown Spell ID: %u", spell_id);
				}
				else if (strcmp(sep->arg[0], "effect") == 0) {
					int32 spell_id = atol(sep->arg[1]);
					LogWrite(COMMAND__DEBUG, 5, "Command", "/info effect: Spell ID: %u", spell_id);
					int8 tier = client->GetPlayer()->GetSpellTier(spell_id);
					int8 type = 0;
					if (client->GetVersion() <= 561)
						type = 1;
					EQ2Packet* outapp = master_spell_list.GetSpecialSpellPacket(spell_id, tier, client, true, type);
					if (outapp){
						client->QueuePacket(outapp);
					}
					else
						LogWrite(COMMAND__ERROR, 0, "Command", "/info effect: Unknown Spell ID: %u", spell_id);
				}
				else if(sep->IsNumber(1) && ((strncasecmp("your_trade", sep->arg[0], 10) == 0) ||
						(strncasecmp("their_trade", sep->arg[0], 11) == 0)))
				{
					if(strncasecmp("t", sep->arg[0], 1) == 0) { // their_trade not self trade
						check_self_trade = false;
					}
					
					int8 slot_id = atoul(sep->arg[1]);
					if(client->GetPlayer()->trade) {
						Entity* traderToCheck = client->GetPlayer();
						if(!check_self_trade) {
							traderToCheck = client->GetPlayer()->trade->GetTradee(client->GetPlayer());
						}
						Item* tradeItem = client->GetPlayer()->trade->GetTraderSlot(traderToCheck, slot_id);
						if(tradeItem != nullptr) {
							EQ2Packet* app = tradeItem->serialize(client->GetVersion(), true, client->GetPlayer(), true, 0, 0, client->GetVersion() > 561 ? true : false);
							client->QueuePacket(app);
						}
					}
				}
			}
			else if (sep && strcmp(sep->arg[0], "overflow") == 0) {
				Item* item = player->item_list.GetOverflowItem();
				if(item) {
					EQ2Packet* app = item->serialize(client->GetVersion(), true, client->GetPlayer());
					client->QueuePacket(app);
				}
				else
					LogWrite(COMMAND__ERROR, 0,"Command", "/info overflow: Unable to retrieve an overflow item.");
			}
			else
				client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Usage:  /info {inventory|equipment|spell} {id}");
			break;
						  }
		case COMMAND_USE_EQUIPPED_ITEM:{
			if (sep && sep->arg[0] && sep->IsNumber(0)){
				int16 slot_id = 0;
				if(client->GetVersion() > 561) {
					slot_id = atoul(sep->arg[0]);
				}
				else if(strlen(sep->argplus[0]) > 0) { // the way that the arguments are pulled the length is truncated, it will always be 2.  So just try to convert what we did get in the string to an integer
					const char* bufPtr = sep->argplus[0];
					slot_id = client->GetPlayer()->ConvertSlotFromClient(atoul(bufPtr), client->GetVersion());
				}
				Item* item = player->GetEquipmentList()->GetItem(slot_id);
				if (item && item->generic_info.usable && item->GetItemScript()) {
					client->UseItem(item, player->GetTarget(), true, slot_id);
				}
			}
			break;
		}
	    case COMMAND_USE_ITEM: {
			if (sep && sep->arg[0] && sep->IsNumber(0)) {
				int32 item_index = atoul(sep->arg[0]);
				
			if(client->GetVersion() <= 561) {
				if(item_index <= 255) {
					item_index = 255 - item_index;
				}
				else {
					if(item_index == 256) { // first "new" item to inventory is assigned index 256 by client
						item_index = client->GetPlayer()->item_list.GetFirstNewItem();
					}
					else {
						// otherwise the slot has to be mapped out depending on the amount of new items + index sent in
						item_index = client->GetPlayer()->item_list.GetNewItemByIndex((int16)item_index - 255);
					}
				}
			}
			
				Item* item = player->item_list.GetItemFromIndex(item_index);
				client->UseItem(item, client->GetPlayer()->GetTarget() ? client->GetPlayer()->GetTarget() : client->GetPlayer());
			}
			break;
							   }
		case COMMAND_SCRIBE_SCROLL_ITEM: {
			if (sep && sep->arg[0] && sep->IsNumber(0)) {
				Item* item = player->item_list.GetItemFromUniqueID(atoul(sep->arg[0]));
				if (item) {
					LogWrite(ITEM__DEBUG, 0, "Items", "ITEM ID: %u", item->details.item_id);

					if(item->generic_info.item_type == 6) {
						Spell* spell = master_spell_list.GetSpell(item->skill_info->spell_id, item->skill_info->spell_tier);
						int8 old_slot = 0;
						if (spell) {
							if(!spell->ScribeAllowed(client->GetPlayer())) {
								client->SimpleMessage(CHANNEL_COLOR_RED, "You do not meet one of the requirements to scribe, due to class or level.");
								break;
							}
							int16 tier_up = player->GetTierUp(spell->GetSpellTier());
							if (rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Spells, RequirePreviousTierScribe)->GetInt8() && !player->HasSpell(spell->GetSpellID(), tier_up, false, true))
								client->SimpleMessage(CHANNEL_COLOR_RED, "You have not scribed the required previous version of this ability.");
							else if (!player->HasSpell(spell->GetSpellID(), spell->GetSpellTier(), true)) {
								old_slot = player->GetSpellSlot(spell->GetSpellID());
								player->RemoveSpellBookEntry(spell->GetSpellID());
								player->AddSpellBookEntry(spell->GetSpellID(), spell->GetSpellTier(), old_slot, spell->GetSpellData()->spell_book_type, spell->GetSpellData()->linked_timer, true);
								player->UnlockSpell(spell);
								client->SendSpellUpdate(spell);
								database.DeleteItem(client->GetCharacterID(), item, 0);
								player->item_list.RemoveItem(item, true);
								client->QueuePacket(player->GetSpellBookUpdatePacket(client->GetVersion()));
								client->QueuePacket(player->SendInventoryUpdate(client->GetVersion()));

								// force purge client cache and display updated spell for hover over
								EQ2Packet* app = spell->SerializeSpell(client, false, false);
								client->QueuePacket(app);
							}
						}
						else
							LogWrite(COMMAND__ERROR, 0, "Command", "Unknown spell ID: %u and tier: %u", item->skill_info->spell_id, item->skill_info->spell_tier);
					}
					else if(item->generic_info.item_type == 7){
							if(item->recipebook_info) {
							LogWrite(TRADESKILL__DEBUG, 0, "Recipe", "Scribing recipe book %s (%u) for player %s.", item->name.c_str(), item->recipebook_info->recipe_id, player->GetName());
							client->AddRecipeBookToPlayer(item->recipebook_info->recipe_id, item);
							}
							else {
									client->Message(CHANNEL_NARRATIVE, "Recipe book is unavailable! Report to admin recipe id %u could not be retrieved.", item->recipebook_info ? item->recipebook_info->recipe_id : 0);
									LogWrite(COMMAND__ERROR, 0, "Command", "Recipe Book %u could not be retrieved for item %s.", item->recipebook_info ? item->recipebook_info->recipe_id : 0, item->name.c_str());
							}
					}
					else {
						LogWrite(COMMAND__ERROR, 0, "Command", "Recipe Book Info is not set for item %s.", item->name.c_str());
					}
				}
				else
					LogWrite(COMMAND__ERROR, 0, "Command", "Unknown unique item ID: %s", sep->arg[0]);
			}
			break;
		}
		case COMMAND_SUMMONITEM: {
			if (sep && sep->IsNumber(0)) {
				int32 item_id = atol(sep->arg[0]);
				int32 quantity = 1;
				
				if (sep->arg[1] && sep->IsNumber(1))
					quantity = atoi(sep->arg[1]);
				
				if (sep->arg[2] && strncasecmp(sep->arg[2], "bank", 4) == 0) {
					client->AddItemToBank(item_id,quantity);
				}
				else {
					client->AddItem(item_id, quantity);
				}
			}
			else {
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /summonitem {item_id} [quantity] or");
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /summonitem {item_id} {quantity} [location] where location = bank");
			}
			break;
		}
		case COMMAND_COLLECTION_ADDITEM: {
			// takes 2 params: collection_id, slot
			if (sep && sep->arg[0] && sep->arg[1] && sep->IsNumber(0) && sep->IsNumber(1)) {
				Item *item = client->GetPlayer()->GetPlayerItemList()->GetItemFromIndex(atoul(sep->arg[1]));
				if (item)
					client->HandleCollectionAddItem(atoul(sep->arg[0]), item);
				else
					LogWrite(COLLECTION__ERROR, 0, "Collect", "Error in 'COMMAND_COLLECTION_ADDITEM'. Unable to get item from player '%s' at index %u", client->GetPlayer()->GetName(), atoul(sep->arg[0]));
			}
			else
				LogWrite(COLLECTION__ERROR, 0, "Collect", "Received command 'COMMAND_COLLECTION_ADDITEM' with unhandeled argument types");
			break;
		}
		case COMMAND_COLLECTION_FILTER_MATCHITEM: {
			// takes 1 param: slot
			printf("COMMAND_COLLECTION_FILTER_MATCHITEM:\n");
			int i = 0;
			while (sep->arg[i] && strlen(sep->arg[i]) > 0) {
				printf("\t%u: %s\n", i, sep->arg[i]);
				i++;
			}
			break;
		}
		case COMMAND_WAYPOINT: {
			bool success = false;
			client->ClearWaypoint();
			    float x, y, z;
			if (sep && sep->IsNumber(0) && sep->IsNumber(1) && sep->IsNumber(2)) {
				if (!client->ShowPathToTarget(atof(sep->arg[0]), atof(sep->arg[1]), atof(sep->arg[2]), 0))
					client->Message(CHANNEL_COLOR_RED, "Invalid coordinates given");
			}
			else if ( client->GetAdminStatus() > 100 && cmdTarget ) {
				if (!client->ShowPathToTarget(cmdTarget->GetX(), cmdTarget->GetY(), cmdTarget->GetZ(), 0))
					client->Message(CHANNEL_COLOR_RED, "Invalid coordinates given");
			}
			else if(sep && sep->arg[0]) {
				// Create a stringstream object
				std::stringstream ss(sep->argplus[0]);
				float x,y,z;
				// Parse the values
				char comma; // To skip the commas
				if ((ss >> x >> comma >> y >> comma >> z) && ss.eof()) {
					if (!client->ShowPathToTarget(x, y, z, 0))
						client->Message(CHANNEL_COLOR_RED, "Invalid coordinates given");
				}
				else {
					client->Message(CHANNEL_COLOR_YELLOW, "Usage: /waypoint x, y, z");
				}
			}
			else {
				client->Message(CHANNEL_COLOR_YELLOW, "Usage: /waypoint x y z");
			}
			break;
		}
		case COMMAND_WHO:{
			const char* who = 0;
			if(sep && sep->arg[0]){
				//cout << "Who query: \n";
				who = sep->argplus[0];
				//cout << who << endl;
			}
			zone_list.ProcessWhoQuery(who, client);
			break;
						 }
		case COMMAND_SELECT_JUNCTION:{
			// transporters (bells birds) use OP_SelectZoneTeleporterDestinatio / ProcessTeleportLocation(app)
			// this is only used for revive it seems
			int32 choice = 0;
			if(sep && sep->arg[0] && sep->IsNumber(0))
				choice = atoul(sep->arg[0]);
			if(!client->GetPlayer()->Alive()){ //dead and this is a revive request
				client->HandlePlayerRevive(choice);
			}
			break;
									 }
		case COMMAND_SPAWN_MOVE: {
			if (cmdTarget && cmdTarget->IsPlayer() == false) {
				PacketStruct* packet = configReader.getStruct("WS_MoveObjectMode", client->GetVersion());
				if (packet) {
					float unknown2_3 = 0;
					int8 placement_mode = 0;
					client->SetSpawnPlacementMode(Client::ServerSpawnPlacementMode::DEFAULT);
					if (sep && sep->arg[0][0]) {
						if (strcmp(sep->arg[0], "wall") == 0) {
							placement_mode = 2;
							unknown2_3 = 150;
						}
						else if (strcmp(sep->arg[0], "ceiling") == 0)
							placement_mode = 1;
						else if (strcmp(sep->arg[0], "openheading") == 0 && cmdTarget->IsWidget())
						{
							client->SimpleMessage(CHANNEL_COLOR_YELLOW, "[PlacementMode] WIDGET OPEN HEADING MODE");
							client->SetSpawnPlacementMode(Client::ServerSpawnPlacementMode::OPEN_HEADING);
						}
						else if (strcmp(sep->arg[0], "closeheading") == 0 && cmdTarget->IsWidget())
						{
							client->SimpleMessage(CHANNEL_COLOR_YELLOW, "[PlacementMode] WIDGET CLOSE HEADING MODE");
							client->SetSpawnPlacementMode(Client::ServerSpawnPlacementMode::CLOSE_HEADING);
						}
						else if (strcmp(sep->arg[0], "myloc") == 0)
						{
							if (cmdTarget->GetSpawnLocationPlacementID() < 1) {
								client->Message(CHANNEL_COLOR_YELLOW, "[PlacementMode] Spawn %s cannot be moved it is not assigned a spawn location placement id.", cmdTarget->GetName());
								safe_delete(packet);
								break;
							}

							cmdTarget->SetX(client->GetPlayer()->GetX(), true);
							cmdTarget->SetY(client->GetPlayer()->GetY(), true);
							cmdTarget->SetZ(client->GetPlayer()->GetZ(), true);
							cmdTarget->SetHeading(client->GetPlayer()->GetHeading(), true);

							if (database.UpdateSpawnLocationSpawns(cmdTarget))
								client->Message(CHANNEL_COLOR_YELLOW, "[PlacementMode] Spawn %s placed at your location.  Updated spawn_location_placement for spawn.", cmdTarget->GetName());
							safe_delete(packet);
							break;
						}
					}
					packet->setDataByName("placement_mode", placement_mode);
					packet->setDataByName("spawn_id", client->GetPlayer()->GetIDWithPlayerSpawn(cmdTarget));
					packet->setDataByName("model_type", cmdTarget->GetModelType());
					packet->setDataByName("unknown", 1); //size
					packet->setDataByName("unknown2", 1); //size 2
					packet->setDataByName("unknown2", .5, 1); //size 3
					packet->setDataByName("unknown2", 3, 2);
					packet->setDataByName("unknown2", unknown2_3, 3);
					packet->setDataByName("max_distance", 500);
					packet->setDataByName("CoEunknown", 0xFFFFFFFF);
					client->QueuePacket(packet->serialize());
					safe_delete(packet);
				}
			}
			else {
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /spawn move (wall OR ceiling)");
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Moves a spawn anywhere you like.  Optionally wall/ceiling can be provided to place wall/ceiling items.");
			}
			break;
		}
		case COMMAND_HAIL:{
			Spawn* spawn = cmdTarget;
			if(spawn && spawn->GetTargetable())
			{
				char tmp[75] = {0};

				sprintf(tmp, "Hail, %s", spawn->GetName());

				bool show_bubble = true;

				if (spawn->IsNPC())
					show_bubble = false;
				client->GetCurrentZone()->HandleChatMessage(client->GetPlayer(), 0, CHANNEL_SAY, tmp, HEAR_SPAWN_DISTANCE, 0, show_bubble, client->GetPlayer()->GetCurrentLanguage());
				if(spawn->IsPlayer() == false && spawn->Alive() && spawn->GetDistance(client->GetPlayer()) < rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Spawn, HailDistance)->GetInt32()){
					if(spawn->IsNPC() && ((NPC*)spawn)->EngagedInCombat())
						spawn->GetZone()->CallSpawnScript(spawn, SPAWN_SCRIPT_HAILED_BUSY, client->GetPlayer());
					else
					{
						bool pauseRunback = false;
						// prime runback as the heading or anything can be altered when hailing succeeds
						if(spawn->IsNPC() && (spawn->HasMovementLoop() || spawn->HasMovementLocations()))
						{
							((NPC*)spawn)->StartRunback();
							pauseRunback = true;
						}

						if(spawn->GetZone()->CallSpawnScript(spawn, SPAWN_SCRIPT_HAILED, client->GetPlayer()) && pauseRunback)
							spawn->PauseMovement(rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Spawn, HailMovementPause)->GetInt32());
						else if(spawn->IsNPC() && pauseRunback)
							((NPC*)spawn)->ClearRunback();
					}
				}
			}
			else {
				string tmp = "Hail";
				client->GetCurrentZone()->HandleChatMessage(client->GetPlayer(), 0, CHANNEL_SAY, tmp.c_str(), HEAR_SPAWN_DISTANCE, 0, true, client->GetPlayer()->GetCurrentLanguage());
			}
			break;
						  }
		case COMMAND_SAY:{
			if (sep && sep->arg[0][0]) {
				client->GetCurrentZone()->HandleChatMessage(client->GetPlayer(), 0, CHANNEL_SAY, sep->argplus[0], HEAR_SPAWN_DISTANCE, 0, true, client->GetPlayer()->GetCurrentLanguage());
				if (cmdTarget && !(cmdTarget->IsPlayer()))
					client->GetCurrentZone()->CallSpawnScript(cmdTarget, SPAWN_SCRIPT_HEAR_SAY, client->GetPlayer(), sep->argplus[0]);
			}
			else
				client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Usage:  /say {message}");
			break;
						 }
		case COMMAND_TELL:{
			if(sep && sep->arg[0] && sep->argplus[1]){
				if(!zone_list.HandleGlobalChatMessage(client, sep->arg[0], CHANNEL_PRIVATE_TELL, sep->argplus[1], 0, client->GetPlayer()->GetCurrentLanguage()))
					client->Message(CHANNEL_COLOR_RED,"Unable to find client %s",sep->arg[0]);
			}else
				client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Usage:  /tell {character_name} {message}");
			break;
						  }
		case COMMAND_SHOUT:{
			if(sep && sep->arg[0][0])
				client->GetCurrentZone()->HandleChatMessage(client->GetPlayer(), 0, CHANNEL_SHOUT, sep->argplus[0], 0, 0, true, client->GetPlayer()->GetCurrentLanguage());
			else
				client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Usage:  /shout {message}");
			break;
						   }
		case COMMAND_AUCTION:{
			if(sep && sep->arg[0][0])
				client->GetCurrentZone()->HandleChatMessage(client->GetPlayer(), 0, CHANNEL_AUCTION, sep->argplus[0], 0, 0, true, client->GetPlayer()->GetCurrentLanguage());
			else
				client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Usage:  /auction {message}");
			break;
							 }
		case COMMAND_OOC:{
			//For now ooc will be the global chat channel, eventually when we create more channels we will create a global chat channel
			if(sep && sep->arg[0][0])
				zone_list.HandleGlobalChatMessage(client, 0, CHANNEL_OUT_OF_CHARACTER, sep->argplus[0], 0, client->GetPlayer()->GetCurrentLanguage());
			else
				client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Usage:  /ooc {message}");
			break;
						 }
		case COMMAND_EMOTE:{
			if(sep && sep->arg[0][0])
				client->GetCurrentZone()->HandleChatMessage(client->GetPlayer(), 0, CHANNEL_EMOTE, sep->argplus[0]);
			else
				client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Usage:  /emote {action}");
			break;
						   }
		case COMMAND_RACE:{
			if(sep && sep->arg[0][0] && sep->IsNumber(0)){
				if(sep->arg[1][0] && sep->IsNumber(1)){
					client->GetPlayer()->GetInfoStruct()->set_race(atoi(sep->arg[1]));
					client->GetPlayer()->SetRace(atoi(sep->arg[1]));
					client->UpdateTimeStampFlag ( RACE_UPDATE_FLAG );
					client->GetPlayer()->SetCharSheetChanged(true);
				}
				client->GetPlayer()->SetModelType(atoi(sep->arg[0]));
				//EQ2Packet* outapp = client->GetPlayer()->spawn_update_packet(client->GetPlayer(), client->GetVersion(), client->GetPlayer()->GetFeatures());
				//client->QueuePacket(outapp);
			}
			else{
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /race {race type id} {race id}");
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "{race id} is optional");
			}
			break;
						  }
		case COMMAND_BANK_DEPOSIT:{
			Spawn* banker = client->GetCurrentZone()->GetSpawnByID(client->GetBanker());
			if(banker && sep && sep->arg[0]){
				int64 amount = 0;
				string deposit = string(sep->arg[0]);
				amount = atoi64(deposit.c_str());
				client->BankDeposit(amount);
			}
			break;
								  }
		case COMMAND_BANK_WITHDRAWAL:{
			Spawn* banker = client->GetCurrentZone()->GetSpawnByID(client->GetBanker());
			if(banker && sep && sep->arg[0] && sep->IsNumber(0)){
				int64 amount = 0;
				string deposit = string(sep->arg[0]);
				amount = atoi64(deposit.c_str());
				client->BankWithdrawal(amount);
			}
			break;
									 }
		case COMMAND_BANK_CANCEL:{
			Spawn* banker = cmdTarget;
			client->Bank(banker, true);
			break;
								 }
		case COMMAND_BANK:{
			LogWrite(PLAYER__DEBUG, 0, "Players", "Open Player Personal Bank...");
			Spawn* banker = cmdTarget;
			client->Bank(banker);
			break;
						  }
		case COMMAND_GUILD_BANK:{
			LogWrite(GUILD__DEBUG, 0, "Guilds", "Open Guild Bank...");
			//Spawn* banker = client->GetPlayer()->GetTarget();
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "This will eventually open the guild bank!");
			break;
						  }
		case COMMAND_START_MAIL: {
			client->SetMailTransaction(cmdTarget);
			client->SendMailList();
			break;
								}
		case COMMAND_CANCEL_MAIL: {
			client->SetMailTransaction(0);
			break;
								  }
		case COMMAND_GET_MAIL_MESSAGE: {
			if (sep && sep->arg[0] && sep->IsNumber(0))
				client->DisplayMailMessage(atoul(sep->arg[0]));
			break;
									   }
		case COMMAND_ADD_MAIL_PLAT: {
			if (sep && sep->arg[0] && sep->IsNumber(0))
				client->AddMailCoin(0, 0, 0, atoul(sep->arg[0]));
			break;
									}
		case COMMAND_ADD_MAIL_GOLD: {
			if (sep && sep->arg[0] && sep->IsNumber(0))
				client->AddMailCoin(0, 0, atoul(sep->arg[0]));
			break;
									}
		case COMMAND_ADD_MAIL_SILVER: {
			if (sep && sep->arg[0] && sep->IsNumber(0))
				client->AddMailCoin(0, atoul(sep->arg[0]));
			break;
									  }
		case COMMAND_ADD_MAIL_COPPER: {
			if (sep && sep->arg[0] && sep->IsNumber(0))
				client->AddMailCoin(atoul(sep->arg[0]));
			break;
									  }
		case COMMAND_SET_MAIL_ITEM: {
			if(sep && sep->IsNumber(0) && sep->IsNumber(1))
			{
				Item* item = client->GetPlayer()->item_list.GetItemFromIndex(atoul(sep->arg[0]));
				if(item)
				{
					int16 quantity = atoul(sep->arg[1]);
					if(item->CheckFlag(NO_TRADE) || item->CheckFlag(ATTUNED) || item->CheckFlag(ARTIFACT) || item->CheckFlag2(HEIRLOOM))
					{
						return;
					}
					if(item->IsBag())
					{
						vector<Item*>* bag_items = player->GetPlayerItemList()->GetItemsInBag(item);
						if(bag_items && bag_items->size() > 0)
						{
							client->SimpleMessage(CHANNEL_COLOR_RED,"You cannot mail a bag with items inside it.");
							safe_delete(bag_items);
							return;
						}
						safe_delete(bag_items);
					}
					Item* itemtoadd = item;
					if(quantity > 0)
					{
						if(quantity > item->details.count)
							return;

						Item* tmpItem = new Item(item);
						tmpItem->details.count = quantity;
						itemtoadd = tmpItem;
					}
						
					if(client->AddMailItem(itemtoadd))
					{
						client->RemoveItem(item, quantity, true);
					}
				}
			}
			break;
							   }
		case COMMAND_REMOVE_MAIL_PLAT: {
			if (sep && sep->arg[0] && sep->IsNumber(0))
				client->RemoveMailCoin(0, 0, 0, atoul(sep->arg[0]));
			break;
									   }
		case COMMAND_REMOVE_MAIL_GOLD: {
			if (sep && sep->arg[0] && sep->IsNumber(0))
				client->RemoveMailCoin(0, 0, atoul(sep->arg[0]));
			break;
									   }
		case COMMAND_REMOVE_MAIL_SILVER: {
			if (sep && sep->arg[0] && sep->IsNumber(0))
				client->RemoveMailCoin(0, atoul(sep->arg[0]));
			break;
										 }
		case COMMAND_REMOVE_MAIL_COPPER: {
			if (sep && sep->arg[0] && sep->IsNumber(0))
				client->RemoveMailCoin(atoul(sep->arg[0]));
			break;
										 }
		case COMMAND_TAKE_MAIL_ATTACHMENTS: {
			if (sep && sep->arg[0] && sep->IsNumber(0))
				client->TakeMailAttachments(atoul(sep->arg[0]));
			break;
											}
		case COMMAND_CANCEL_SEND_MAIL: {
			client->ResetSendMail();
			break;
									   }
		case COMMAND_DELETE_MAIL_MESSAGE: {
			if (sep && sep->arg[0] && sep->IsNumber(0))
				client->DeleteMail(atoul(sep->arg[0]), true);
			break;
										  }
		case COMMAND_REPORT_SPAM: {
			LogWrite(MISC__TODO, 1, "TODO", " received reportspam\n\t(%s, function: %s, line #: %i)", __FILE__, __FUNCTION__, __LINE__);
			if (sep && sep->arg[0]) LogWrite(COMMAND__DEBUG, 0, "Command", "%s\n", sep->argplus[0]);
			break;
								  }
		case COMMAND_KILL:{
			Spawn* dead = 0;
			if (sep && sep->arg[0] && strncasecmp(sep->arg[0],"self",4)==0){
				dead=client->GetPlayer();
				client->GetPlayer()->SetHP(0);
				client->GetPlayer()->KillSpawn(dead);
			}else{
				dead= cmdTarget;
				if(dead && dead->IsPlayer() == false){
					if(dead->IsNPC() && ((NPC*)dead)->Brain()) {
						client->GetPlayer()->CheckEncounterState((Entity*)dead);
						((NPC*)dead)->Brain()->AddToEncounter(client->GetPlayer());
						((NPC*)dead)->AddTargetToEncounter(client->GetPlayer());
						((NPC*)dead)->Brain()->AddHate(client->GetPlayer(), 1);
					}
					dead->SetHP(0);
					if(sep && sep->arg[0] && sep->IsNumber(0) && atoi(sep->arg[0]) == 1)
						client->GetCurrentZone()->RemoveSpawn(dead, true, true, true, true, true);
					else
						client->GetPlayer()->KillSpawn(dead);
				}else{
					client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Usage:  /kill (self)");
					client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Kills currently selected non-player target.");
					client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Optionally, self may be used to kill yourself");
				}
			}
			break;
						  }
		case COMMAND_LEVEL:{
			if(sep && sep->arg[ndx][0] && sep->IsNumber(0)){
				int16 new_level = atoi(sep->arg[ndx]);
				if (!client->GetPlayer()->CheckLevelStatus(new_level))
					client->SimpleMessage(CHANNEL_COLOR_RED, "You do not have the required status to level up anymore!");
				else {
					if (new_level < 1)
						new_level = 1;
					
					if (new_level > 255)
						new_level = 255;

					client->ChangeLevel(client->GetPlayer()->GetLevel(), new_level);
				}
			}else
				client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Usage: /level {new_level}");
			break;
						   }
		case COMMAND_SIT: {
			if(client->GetPlayer()->GetHP() > 0){
				client->QueuePacket(new EQ2Packet(OP_SitMsg, 0, 0));
				client->SimpleMessage(CHANNEL_COLOR_YELLOW,"You sit down.");
				client->GetPlayer()->set_character_flag(CF_IS_SITTING);
			}
			break;
						  }
		case COMMAND_STAND: {
			if(client->GetPlayer()->GetHP() > 0){
				client->QueuePacket(new EQ2Packet(OP_StandMsg, 0, 0));
				client->SimpleMessage(CHANNEL_COLOR_YELLOW,"You stand up.");
				client->GetPlayer()->reset_character_flag(CF_IS_SITTING);
			}
			break;
							}
		case COMMAND_CLASS:{
			if(sep && sep->arg[ndx][0]){
				client->GetPlayer()->SetPlayerAdventureClass(atoi(sep->arg[ndx]), true);
			}else
				client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Usage:  /class {class_id}");
			break;
						   }
		case COMMAND_FLYMODE:{
			if(sep && sep->arg[0] && sep->IsNumber(0)){
				PrintSep(sep, "COMMAND_FLYMODE");
				int8 val = atoul(sep->arg[0]);
				ClientPacketFunctions::SendFlyMode(client, val);
			}
			else{
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage ON: /flymode [1|2] 1 = fly, 2 = no clip");
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage OFF: /flymode 0");
			}
			break;
							 }
		case COMMAND_LOOT_LIST:{
			Spawn* spawn = cmdTarget;
			if(spawn && spawn->IsEntity()){
				if (sep && sep->arg[0]) {
					if (!spawn->GetDatabaseID())
					{
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "NOTE: Spawn has no database id to assign to loottables.");
					}
					
					if (!spawn->IsNPC())
					{
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "these /loot list [add/remove/clearall] sub-commands are only designed for an NPC.");
						break;
					}

					bool reloadLoot = false;
					if (!stricmp(sep->arg[0], "remove") && sep->arg[1] && sep->IsNumber(1))
					{
						int32 loottable_id = atoul(sep->arg[1]);
						if (loottable_id > 0 && database.RemoveSpawnLootTable(spawn, loottable_id))
						{
							client->Message(CHANNEL_COLOR_YELLOW, "Spawn %u loot table %u removed.", spawn->GetDatabaseID(), loottable_id);
							reloadLoot = true;
						}
						else
							client->Message(CHANNEL_COLOR_YELLOW, "/loot list remove [loottableid] - could not match any spawn_id entries against loottable_id %u.", loottable_id);
					}
					else if (!stricmp(sep->arg[0], "add") && sep->arg[1] && sep->IsNumber(1))
					{

						int32 loottable_id = atoul(sep->arg[1]);
						if (loottable_id > 0)
						{
							database.AddLootTableToSpawn(spawn, loottable_id);
							client->Message(CHANNEL_COLOR_YELLOW, "Spawn %u loot table %u added.", spawn->GetDatabaseID(), loottable_id);
							reloadLoot = true;
						}
						else
							client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/loot list add [loottableid] - cannot add a loottable id of 0.");
					}
					else if (!stricmp(sep->arg[0], "clearall"))
					{
						if (database.RemoveSpawnLootTable(spawn, 0))
						{
							client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Spawn loot tables removed.");
							reloadLoot = true;
						}
						else
							client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/loot list clearall - could not match any spawn_id entries in loottable_id.");
					}
					else if (!stricmp(sep->arg[0], "details"))
					{
						spawn->LockLoot();
						client->SimpleMessage(CHANNEL_COMMANDS, "Loot Window List:");
						if (spawn->GetLootWindowList()->size() > 0) {
							std::map<int32, bool>::iterator itr;
							for (itr = spawn->GetLootWindowList()->begin(); itr != spawn->GetLootWindowList()->end(); itr++) {
								Spawn* looter = client->GetPlayer()->GetZone()->GetSpawnByID(itr->first);
								if (looter) {
									client->Message(CHANNEL_COLOR_YELLOW, "Looter: %s IsLootWindowOpen: %s, HasCompletedLootWindow: %s.", looter->GetName(), itr->second ? "NO" : "YES", spawn->HasSpawnLootWindowCompleted(itr->first) ? "YES" : "NO");
								}
							}
						}
						spawn->UnlockLoot();
					}
					else
					{
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/loot list argument not supported.");
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/loot list add [loottableid] - add new loottable to spawn");
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/loot list remove [loottableid] - remove existing loottable from spawn");
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/loot list clearall - remove all loottables from spawn");
						break;
					}
					
					if (reloadLoot)
					{
							database.LoadSpawnLoot(spawn->GetZone(), spawn);
							if (spawn->IsNPC())
							{
								Entity* ent = (Entity*)spawn;
								ent->SetLootCoins(0);
								ent->ClearLoot();
								spawn->GetZone()->AddLoot((NPC*)spawn);
								client->Message(CHANNEL_COLOR_YELLOW, "Spawn %u active loot purged and reloaded.", spawn->GetDatabaseID());
							}
					}
					break; // nothing further this is the end of these sub commands
				}
				else
				{
					vector<int32> loot_list = client->GetCurrentZone()->GetSpawnLootList(spawn->GetDatabaseID(), spawn->GetZone()->GetZoneID(), spawn->GetLevel(), race_types_list.GetRaceType(spawn->GetModelType()), spawn);
					if (loot_list.size() > 0) {
						client->Message(CHANNEL_COLOR_YELLOW, "%s belongs to the following loot lists: ", spawn->GetName());
						vector<int32>::iterator list_itr;
						LootTable* table = 0;
						for (list_itr = loot_list.begin(); list_itr != loot_list.end(); list_itr++) {
							table = client->GetCurrentZone()->GetLootTable(*list_itr);
							if (table)
								client->Message(CHANNEL_COLOR_YELLOW, "%u - %s", *list_itr, table->name.c_str());
						}
					}
					client->Message(CHANNEL_COLOR_YELLOW, "Coins being carried: %u", spawn->GetLootCoins());
					vector<Item*>* items = spawn->GetLootItems();
					if (items) {
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Spawn is carrying the following items: ");
						vector<Item*>::iterator itr;
						Item* item = 0;
						for (itr = items->begin(); itr != items->end(); itr++) {
							item = *itr;
							client->Message(CHANNEL_COLOR_YELLOW, "%u - %s", item->details.item_id, item->name.c_str());
						}
					}
					else
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Spawn is not carrying any items.");
				}
			}
			break;
							   }
		case COMMAND_LOOT_SETCOIN:{
			Spawn* spawn = cmdTarget;
			if(spawn && spawn->IsEntity() && sep && sep->arg[0] && sep->IsNumber(0)){
				spawn->SetLootCoins(atoul(sep->arg[0]));
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Successfully set coins.");
			}
			else
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Syntax: /loot setcoin {amount}");
			break;
								  }
		case COMMAND_LOOT_ADDITEM:{
			Spawn* spawn = cmdTarget;
			if(spawn && spawn->IsEntity() && sep && sep->arg[0] && sep->IsNumber(0)){
				int16 charges = 1;
				if(sep->arg[1] && sep->IsNumber(1))
					charges = atoi(sep->arg[1]);
				spawn->AddLootItem(atoul(sep->arg[0]), charges);
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Successfully added item.");
			}
			else
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Syntax: /loot additem {item_id}");
			break;
								  }
		case COMMAND_LOOT_REMOVEITEM:{
			Spawn* spawn = cmdTarget;
			if(spawn && spawn->IsEntity() && sep && sep->arg[0] && sep->IsNumber(0)){
				Item* item = spawn->LootItem(atoul(sep->arg[0]));
				lua_interface->SetLuaUserDataStale(item);
				safe_delete(item);
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Successfully removed item.");
			}
			else
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Syntax: /loot removeitem {item_id}");
			break;
									 }
		case COMMAND_LOOT_CORPSE:
		case COMMAND_DISARM:
		case COMMAND_LOOT:{
			if (cmdTarget && ((Entity*)cmdTarget)->IsNPC() && cmdTarget->Alive())
			{
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Your target is not dead.");
				break;
			}

			if(cmdTarget && cmdTarget->IsEntity()){
				if (cmdTarget->GetDistance(client->GetPlayer()) <= rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Loot, LootRadius)->GetFloat()){
					if (!rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Loot, AutoDisarmChest)->GetBool() && command->handler == COMMAND_DISARM )
						client->OpenChest(cmdTarget, true);
					else
						client->LootSpawnRequest(cmdTarget, rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Loot, AutoDisarmChest)->GetBool());
					if (!(cmdTarget)->HasLoot()){
						if (((Entity*)cmdTarget)->IsNPC())
							client->GetCurrentZone()->RemoveDeadSpawn(cmdTarget);
					}
				}
				else
					client->Message(CHANNEL_COLOR_YELLOW, "You are too far away to interact with that");
			}
			else if(!cmdTarget || cmdTarget->GetHP() > 0)
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Invalid target.");
			break;
						  }
		case COMMAND_RELOADSPAWNSCRIPTS:{
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Reloading Spawn Scripts....");
			if (lua_interface)
				lua_interface->SetLuaSystemReloading(true);
			world.ResetSpawnScripts();
			database.LoadSpawnScriptData();
			if(lua_interface) {
				lua_interface->DestroySpawnScripts();
				lua_interface->SetLuaSystemReloading(false);
			}
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Success!");
			break;
										}
		case COMMAND_RELOADREGIONSCRIPTS:{
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Reloading Region Scripts....");
			if(lua_interface) {
				lua_interface->DestroyRegionScripts();
			}
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Success!");
			break;
										}
		case COMMAND_RELOADLUASYSTEM:{
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Attempting to reload entire LUA system....");

			world.SetReloadingSubsystem("LuaSystem");
			
			if(lua_interface) {
				lua_interface->SetLuaSystemReloading(true);
			}
			
			zone_list.DeleteSpellProcess();
			if (lua_interface)
				lua_interface->DestroySpells();
			master_spell_list.Reload();
			zone_list.LoadSpellProcess();
			if(lua_interface){
				map<Client*, int32> debug_clients = lua_interface->GetDebugClients();
				map<Client*, int32>::iterator itr;
				for(itr = debug_clients.begin(); itr != debug_clients.end(); itr++){
					if(lua_interface)
						lua_interface->UpdateDebugClients(itr->first);
				}
			}

			world.ResetSpawnScripts();
			database.LoadSpawnScriptData();

			world.ResetZoneScripts();
			database.LoadZoneScriptData();
			
			world.ResetPlayerScripts();
			world.LoadPlayerScripts();

			if(lua_interface) {
				lua_interface->DestroySpawnScripts();
				lua_interface->DestroyRegionScripts();
				lua_interface->DestroyQuests();
				lua_interface->DestroyItemScripts();
				lua_interface->DestroyZoneScripts();
				lua_interface->DestroyPlayerScripts();
			}

			int32 quest_count = database.LoadQuests();

			int32 spell_count = 0;

			if(lua_interface) {
				spell_count = database.LoadSpellScriptData();
				lua_interface->SetLuaSystemReloading(false);
			}

			world.RemoveReloadingSubSystem("LuaSystem");

			client->Message(CHANNEL_COLOR_YELLOW, "Successfully loaded %u spell(s), %u quest(s).", spell_count, quest_count);
			break;
									 }
		case COMMAND_DECLINE_QUEST:{
			int32 quest_id = 0;
			if(sep && sep->arg[0] && sep->IsNumber(0))
				quest_id = atoul(sep->arg[0]);
			if(quest_id > 0){
				client->RemovePendingQuest(quest_id);
			}
			break;
								   }
		case COMMAND_DELETE_QUEST:{
			int32 quest_id = 0;
			if(sep && sep->arg[0] && sep->IsNumber(0))
				quest_id = atoul(sep->arg[0]);
			client->DeleteQuest(quest_id);
			break;
								  }
		case COMMAND_ACCEPT_QUEST:{
			int32 quest_id = 0;
			if(sep && sep->arg[0] && sep->IsNumber(0))
				quest_id = atoul(sep->arg[0]);
			if(quest_id > 0){
				client->AcceptQuest(quest_id);
			}
			break;
								  }
		case COMMAND_NAME:{
			if(sep && sep->arg[0]){
				client->GetPlayer()->GetInfoStruct()->set_name(sep->argplus[0]);
				client->GetPlayer()->SetCharSheetChanged(true);
				client->GetPlayer()->info_changed = true;
				client->GetPlayer()->GetZone()->AddChangedSpawn(client->GetPlayer());
				client->UpdateTimeStampFlag ( NAME_UPDATE_FLAG );
			}else
				client->Message(CHANNEL_COLOR_YELLOW,"Usage: /name {new_name}");

			break;
		}
		case COMMAND_GROUP: {
			if(sep && sep->arg[0])
				printf("Group: %s\n",sep->argplus[0]);
			break;
		}
		case COMMAND_GROUPSAY:{
			GroupMemberInfo* gmi = client->GetPlayer()->GetGroupMemberInfo();
			if(sep && sep->arg[0] && gmi) {
				world.GetGroupManager()->GroupChatMessage(gmi->group_id, client->GetPlayer(), client->GetPlayer()->GetCurrentLanguage(), sep->argplus[0]);
				peer_manager.SendPeersChannelMessage(gmi->group_id, std::string(client->GetPlayer()->GetName()), std::string(sep->argplus[0]), CHANNEL_GROUP_SAY, client->GetPlayer()->GetCurrentLanguage());
			}
			break;
		}
		case COMMAND_GROUPINVITE: {
			Entity* target = 0;
			Client* target_client = 0;

			if (client->GetPlayer()->GetGroupMemberInfo() && !client->GetPlayer()->GetGroupMemberInfo()->leader) {
				client->SimpleMessage(CHANNEL_COMMANDS, "You must be the group leader to invite.");
				return;
			}

			// name provided with command so it has to be a player, npc not supported here
			if (sep && sep->arg[0] && strlen(sep->arg[0]) > 0){
				target_client = zone_list.GetClientByCharName(sep->arg[0]);
				if (target_client) {
					if (!target_client->IsConnected() || !target_client->IsReadyForSpawns())
						target = 0;
					else
						target = target_client->GetPlayer();
				}
			}

			// if name was not provided use the players target, npc or player supported here
			if (!target && client->GetPlayer()->GetTarget() && client->GetPlayer()->GetTarget()->IsEntity()) {
				target = (Entity*)client->GetPlayer()->GetTarget();
				if (target->IsPlayer()) {
					target_client = ((Player*)target)->GetClient();
				}
			}

			int8 result = world.GetGroupManager()->Invite(client->GetPlayer(), target);
			
			if (target && result == 0) {
				client->Message(CHANNEL_COMMANDS, "You invite %s to group with you.", target->GetName());
				if (target_client) {
					client->SendReceiveOffer(target_client, 1, std::string(client->GetPlayer()->GetName()), 1);
				}
			}
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

			break;
		}
		case COMMAND_GROUPDISBAND: {
			GroupMemberInfo* gmi = client->GetPlayer()->GetGroupMemberInfo();

			if (gmi && gmi->leader) { // TODO: Leader check..DONE! :X
				// world.GetGroupManager()->SimpleGroupMessage(gmi->group_id, "Your group has been disbanded.");
				world.GetGroupManager()->RemoveGroup(gmi->group_id);
				peer_manager.sendPeersDisbandGroup(gmi->group_id);
			}

			break;
		}
		case COMMAND_GROUP_LEAVE: {
			GroupMemberInfo* gmi = client->GetPlayer()->GetGroupMemberInfo();

			if (gmi) {
				int32 group_id = gmi->group_id;
				world.GetGroupManager()->RemoveGroupMember(group_id, client->GetPlayer());
				if (!world.GetGroupManager()->IsGroupIDValid(group_id)) {
					// leader->Message(CHANNEL_COLOR_GROUP, "%s has left the group.", client->GetPlayer()->GetName());
				}
				else {
					world.GetGroupManager()->GroupMessage(group_id, "%s has left the group.", client->GetPlayer()->GetName());
				}

				client->SimpleMessage(CHANNEL_GROUP_CHAT, "You have left the group");
			}

			break;
		}
		case COMMAND_FRIEND_ADD:{
			string name = "";
			if(sep && sep->arg[0] && strlen(sep->arg[0]) > 1)
				name = database.GetPlayerName(sep->arg[0]);
			else if(cmdTarget && cmdTarget->IsPlayer())
				name = string(cmdTarget->GetName());
			if(name.length() > 0){
				if(strcmp(name.c_str(), client->GetPlayer()->GetName()) != 0){
					if(client->GetPlayer()->IsIgnored(name.c_str())){
						client->GetPlayer()->RemoveIgnore(name.c_str());
						client->Message(CHANNEL_COLOR_CHAT_RELATIONSHIP, "Removed ignore: %s", name.c_str());
						client->SendChatRelationship(3, name.c_str());
					}
					client->GetPlayer()->AddFriend(name.c_str(), true);
					client->SendChatRelationship(0, name.c_str());
					client->Message(CHANNEL_COLOR_CHAT_RELATIONSHIP, "Added friend: %s", name.c_str());
					if(zone_list.GetClientByCharName(name))
						client->Message(CHANNEL_COLOR_CHAT_RELATIONSHIP, "Friend: %s has logged in.", name.c_str());
				}
				else
					client->SimpleMessage(CHANNEL_COLOR_RED, "You cannot be more friendly towards yourself!");
			}
			else
				client->SimpleMessage(CHANNEL_COLOR_CHAT_RELATIONSHIP, "That character does not exist.");
			break;
		}
		case COMMAND_FRIEND_REMOVE:{
			string name = "";
			if(sep && sep->arg[0] && strlen(sep->arg[0]) > 1)
				name = database.GetPlayerName(sep->arg[0]);
			else if(cmdTarget && cmdTarget->IsPlayer())
				name = string(cmdTarget->GetName());
			if(name.length() > 0){
				client->GetPlayer()->RemoveFriend(name.c_str());
				client->SendChatRelationship(1, name.c_str());
				client->Message(CHANNEL_COLOR_CHAT_RELATIONSHIP, "Removed friend: %s", name.c_str());
			}
			else
				client->SimpleMessage(CHANNEL_COLOR_CHAT_RELATIONSHIP, "That character does not exist.");
			break;
		}
		case COMMAND_FRIENDS:{

			break;
		}
	    case COMMAND_IGNORE_ADD:{
			string name = "";
			if(sep && sep->arg[0] && strlen(sep->arg[0]) > 1)
				name = database.GetPlayerName(sep->arg[0]);
			else if(cmdTarget && cmdTarget->IsPlayer())
				name = string(cmdTarget->GetName());
			if(name.length() > 0){
				if(strcmp(name.c_str(), client->GetPlayer()->GetName()) != 0){
					if(client->GetPlayer()->IsFriend(name.c_str())){
						client->GetPlayer()->RemoveFriend(name.c_str());
						client->Message(CHANNEL_COLOR_CHAT_RELATIONSHIP, "Removed friend: %s", name.c_str());
						client->SendChatRelationship(1, name.c_str());
					}
					client->GetPlayer()->AddIgnore(name.c_str(), true);
					client->SendChatRelationship(2, name.c_str());
					client->Message(CHANNEL_COLOR_CHAT_RELATIONSHIP, "Added ignore: %s", name.c_str());
				}
				else
					client->SimpleMessage(CHANNEL_COLOR_RED, "You cannot ignore yourself!");
			}
			else
				client->SimpleMessage(CHANNEL_COLOR_CHAT_RELATIONSHIP, "That character does not exist.");
			break;
		}
		case COMMAND_IGNORE_REMOVE:{
			string name = "";
			if(sep && sep->arg[0] && strlen(sep->arg[0]) > 1)
				name = database.GetPlayerName(sep->arg[0]);
			else if(cmdTarget && cmdTarget->IsPlayer())
				name = string(cmdTarget->GetName());
			if(name.length() > 0){
				client->GetPlayer()->RemoveIgnore(name.c_str());
				client->SendChatRelationship(3, name.c_str());
				client->Message(CHANNEL_COLOR_CHAT_RELATIONSHIP, "Removed ignore: %s", name.c_str());
			}
			break;
		}
		case COMMAND_IGNORES:{

			break;
		}
		case COMMAND_GROUP_KICK:{
			Entity* kicked = 0;
			Client* kicked_client = 0;
			int32 group_id = 0;

			if (!client->GetPlayer()->GetGroupMemberInfo() || !client->GetPlayer()->GetGroupMemberInfo()->leader)
				return;

			if (sep && sep->arg[0]) {
				kicked_client = zone_list.GetClientByCharName(string(sep->arg[0]));
				if (kicked_client)
					kicked = kicked_client->GetPlayer();
			}
			else if (cmdTarget && cmdTarget->IsEntity()) {
				kicked = (Entity*)cmdTarget;
			}

			group_id = client->GetPlayer()->GetGroupMemberInfo()->group_id;

			bool send_kicked_message = world.GetGroupManager()->GetGroupSize(group_id) > 2;
			if (kicked)
				world.GetGroupManager()->RemoveGroupMember(group_id, kicked);

			if(send_kicked_message)
				world.GetGroupManager()->GroupMessage(group_id, "%s was kicked from the group.", kicked->GetName());
			else
				client->Message(CHANNEL_GROUP_CHAT, "You kicked %s from the group", kicked->GetName());

			if (kicked_client)
				kicked_client->SimpleMessage(CHANNEL_GROUP_CHAT, "You were kicked from the group");
				
			break;
		}
	    case COMMAND_GROUP_MAKE_LEADER:{
			
			if (!client->GetPlayer()->GetGroupMemberInfo() || !client->GetPlayer()->GetGroupMemberInfo()->leader) {
				client->SimpleMessage(CHANNEL_COMMANDS, "You are not a group leader.");
				return;
			}

			if (sep && sep->arg[0] && strlen(sep->arg[0]) > 1) {
				Client* new_leader = zone_list.GetClientByCharName(sep->arg[0]);
				if (new_leader && new_leader->GetPlayer()->GetGroupMemberInfo() && new_leader->GetPlayer()->GetGroupMemberInfo()->group_id == client->GetPlayer()->GetGroupMemberInfo()->group_id) {
					if (client->GetPlayer()->IsGroupMember(new_leader->GetPlayer())) {
						if(world.GetGroupManager()->MakeLeader(client->GetPlayer()->GetGroupMemberInfo()->group_id, new_leader->GetPlayer())) {
							world.GetGroupManager()->GroupMessage(client->GetPlayer()->GetGroupMemberInfo()->group_id, "%s is now leader of the group.", new_leader->GetPlayer()->GetName());
						}
					}
				}
				else
					client->SimpleMessage(CHANNEL_COMMANDS, "Unable to find the given player.");
			}
			else if (cmdTarget && cmdTarget->IsPlayer() && ((Player*)cmdTarget)->GetGroupMemberInfo() && ((Player*)cmdTarget)->GetGroupMemberInfo()->group_id == client->GetPlayer()->GetGroupMemberInfo()->group_id) {
				if(client->GetPlayer()->IsGroupMember((Player*)cmdTarget) && world.GetGroupManager()->MakeLeader(client->GetPlayer()->GetGroupMemberInfo()->group_id, (Entity*)cmdTarget)) {
					world.GetGroupManager()->GroupMessage(client->GetPlayer()->GetGroupMemberInfo()->group_id, "%s is now leader of the group.", cmdTarget->GetName());
				}
			}
			else
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Unable to change group leader.");

			break;
		}
		case COMMAND_GROUP_ACCEPT_INVITE: {
				int8 result = 3;
				std::string leader = world.GetGroupManager()->HasPendingInvite(client->GetPlayer());
				std::string playerName(client->GetPlayer()->GetName());
				Client* leader_client = client->GetCurrentZone()->GetClientByName((char*)leader.c_str());
				bool group_existed = false;
				if(leader_client && leader_client->GetPlayer()->GetGroupMemberInfo()) {
					group_existed = true;
				}
				if(client->GetPlayer()->GetGroupMemberInfo() && client->GetPlayer()->GetGroupMemberInfo()->leader) {
					int8 raid_result = world.GetGroupManager()->AcceptRaidInvite(std::string(client->GetPlayer()->GetName()), client->GetPlayer()->GetGroupMemberInfo()->group_id);
					if(raid_result == 1) {
						GroupOptions options;
						if(world.GetGroupManager()->GetDefaultGroupOptions(client->GetPlayer()->GetGroupMemberInfo()->group_id, &options)) {
							std::vector<int32> raidGroups;
							world.GetGroupManager()->GetRaidGroups(client->GetPlayer()->GetGroupMemberInfo()->group_id, &raidGroups);
							peer_manager.sendPeersNewGroupRequest("", 0, client->GetPlayer()->GetGroupMemberInfo()->group_id, "", "", &options, "", &raidGroups, true);
						}
						world.GetGroupManager()->ClearGroupRaidLooterFlag(client->GetPlayer()->GetGroupMemberInfo()->group_id);
						world.GetGroupManager()->SendGroupUpdate(client->GetPlayer()->GetGroupMemberInfo()->group_id);
						break;
					}
				}
				if(net.is_primary) {
					int32 group_id = 0;
					result = world.GetGroupManager()->AcceptInvite(client->GetPlayer(), &group_id, false);
					client->HandleGroupAcceptResponse(result);
					if(result == 0) {
						GroupOptions options;
						if(leader_client) {
							if(!group_existed) {
								leader_client->SetGroupOptionsReference(&options);
								peer_manager.sendPeersNewGroupRequest("", 0, group_id, leader, playerName, &options);
							}
							
							world.GetGroupManager()->AddGroupMember(group_id, leader_client->GetPlayer(), true);
							world.GetGroupManager()->GroupMessage(leader_client->GetPlayer()->GetGroupMemberInfo()->group_id, "%s has joined the group.", playerName.c_str());
							world.GetGroupManager()->AddGroupMember(leader_client->GetPlayer()->GetGroupMemberInfo()->group_id, client->GetPlayer());
						}
					}
				}
				else {
					if(leader.size() < 1) {
						client->HandleGroupAcceptResponse(1);
					}
					else {
						Client* leader_client = client->GetCurrentZone()->GetClientByName((char*)leader.c_str());
						GroupOptions options;
						if(leader_client) {
							leader_client->SetGroupOptionsReference(&options);
							world.GetGroupManager()->AddInvite(leader_client->GetPlayer(), client->GetPlayer());
							peer_manager.sendPrimaryNewGroupRequest(leader, playerName, client->GetPlayer()->GetID(), &options);
							
						}
						else {
							client->HandleGroupAcceptResponse(2);
						}
					}
				}
			break;
		}
		case COMMAND_GROUP_DECLINE_INVITE: {
				world.GetGroupManager()->DeclineInvite(client->GetPlayer()); // TODO: Add message to leader
			break;
		}
		case COMMAND_SUMMON:{
			char* search_name = 0;
			if(sep && sep->arg[0][0])
				search_name = sep->arg[0];

			if(!search_name && !cmdTarget){
				client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Usage: /summon [name]");
				client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Summons a targeted spawn or a spawn by name to you.  If more than one spawn matches name, it will summon closest.");
			}
			else{ 
				if(!client->Summon(search_name))
					client->SimpleMessage(CHANNEL_COLOR_YELLOW, "No matches found.");
			}
			break;
							}
		case COMMAND_GOTO:{
			const char* search_name = 0;
			if(sep && sep->arg[0][0])
				search_name = sep->argplus[0];
			if(!search_name && !cmdTarget){
				client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Usage: /goto [name]");
				client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Moves to targeted spawn or to a spawn by name.  If more than one spawn matches name, you will move to closest.");
			}
			else{
				if(!client->GotoSpawn(search_name))
					client->SimpleMessage(CHANNEL_COLOR_YELLOW, "No matches found.");
			}
			break;
						  }
		case COMMAND_MOVE:{
			bool success = false;
			try{
				if(sep && sep->arg[2][0] && sep->IsNumber(0) && sep->IsNumber(1) && sep->IsNumber(2)){
					EQ2Packet* app = client->GetPlayer()->Move(atof(sep->arg[0]), atof(sep->arg[1]), atof(sep->arg[2]), client->GetVersion());
					if(app){
						client->QueuePacket(app);
						success = true;
					}
				}
			}
			catch(...){}
			if(!success){
				client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Usage: /move x y z");
				client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Moves the client to the given coordinates.");
			}
			break;
						  }
		case COMMAND_SETTIME:{
			if(sep && sep->arg[0]){
				
				world.MWorldTime.writelock(__FUNCTION__, __LINE__);
				sscanf (sep->arg[0], "%d:%d", &world.GetWorldTimeStruct()->hour, &world.GetWorldTimeStruct()->minute);
				if(sep->arg[1] && sep->IsNumber(1))
					world.GetWorldTimeStruct()->month = atoi(sep->arg[1]) - 1; //zero based indexes
				if(sep->arg[2] && sep->IsNumber(2))
					world.GetWorldTimeStruct()->day = atoi(sep->arg[2]) - 1; //zero based indexes
				if(sep->arg[3] && sep->IsNumber(3))
					world.GetWorldTimeStruct()->year = atoi(sep->arg[3]);
				world.MWorldTime.releasewritelock(__FUNCTION__, __LINE__);
				client->GetCurrentZone()->SendTimeUpdateToAllClients();
			}
			else{
				client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Usage: /settime [hour:minute] (month) (day) (year)");
				client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Example: /settime 8:30");
			}
			break;
							 }
		case COMMAND_VERSION:{
			client->Message(CHANNEL_COLOR_YELLOW,"%s %s", EQ2EMU_MODULE, CURRENT_VERSION);
			client->Message(CHANNEL_COLOR_YELLOW,"Last Compiled on %s %s", COMPILE_DATE, COMPILE_TIME);
			auto [days, hours, minutes, seconds] = convertTimestampDuration((getCurrentTimestamp() - world.world_uptime));
			client->Message(CHANNEL_COLOR_YELLOW,"Uptime %u day(s), %u hour(s), %u minute(s), %u second(s).", days, hours, minutes, seconds);
			break;
							 }
		case COMMAND_ZONE:{
			int32 instanceID = 0;
			string zone;
			int32 zone_id = 0;
			bool listSearch = false;
			bool isInstance = false;
			bool notZoningCommand = false;
			ZoneServer* zsZone = 0;
			ZoneChangeDetails zone_details;
			if(sep && sep->arg[0][0])
			{
				if(strncasecmp(sep->arg[0], "list", 4) == 0)
					listSearch = true;

				else if(strncasecmp(sep->arg[0], "active", 6) == 0)
				{
					zone_list.SendZoneList(client);
					notZoningCommand = true;
					break;
				}

				else if(strncasecmp(sep->arg[0], "instance", 6) == 0)
				{
					if(sep->IsNumber(1))
						instanceID = atol(sep->arg[1]);
				}

				else if(strncasecmp(sep->arg[0], "lock", 4) == 0)
				{
					PrintSep(sep, "ZONE LOCK");

					if(sep->IsNumber(1)) {
						if(sep->IsNumber(2) && zone_list.GetDuplicateZoneDetails(&zone_details, "", atoul(sep->arg[1]), atoul(sep->arg[2]))) {
							zsZone = (ZoneServer*)zone_details.zonePtr;
						}
						else if(zone_list.GetZone(&zone_details, atoul(sep->arg[1]), "", false, false, false, false)) {
							zsZone = (ZoneServer*)zone_details.zonePtr;
						}
					}
					else {
						if(sep->IsNumber(2) && zone_list.GetDuplicateZoneDetails(&zone_details, std::string(sep->arg[1]), 0, atoul(sep->arg[2]))) {
							zsZone = (ZoneServer*)zone_details.zonePtr;
						}
						else if(zone_list.GetZone(&zone_details, 0, std::string(sep->arg[1]), false, false, false, false)) {
							zsZone = (ZoneServer*)zone_details.zonePtr;
						}
					}

					if( zsZone )
					{
						zsZone->SetZoneLockState(true);
						client->Message(CHANNEL_COLOR_YELLOW, "Zone %s (%u) is now locked.", zsZone->GetZoneName(), zsZone->GetZoneID());
					}
					else
						client->Message(CHANNEL_COLOR_RED, "Zone %s is not running and cannot be locked.", sep->arg[1]);
					notZoningCommand = true;
					break;
				}
				else if(strncasecmp(sep->arg[0], "unlock", 6) == 0)
				{
					PrintSep(sep, "ZONE UNLOCK");

					if(sep->IsNumber(1)) {
						if(sep->IsNumber(2) && zone_list.GetDuplicateZoneDetails(&zone_details, "", atoul(sep->arg[1]), atoul(sep->arg[2]))) {
							zsZone = (ZoneServer*)zone_details.zonePtr;
						}
						else if(zone_list.GetZone(&zone_details, atoul(sep->arg[1]), "", false, false, false, false)) {
							zsZone = (ZoneServer*)zone_details.zonePtr;
						}
					}
					else {
						if(sep->IsNumber(2) && zone_list.GetDuplicateZoneDetails(&zone_details, std::string(sep->arg[1]), 0, atoul(sep->arg[2]))) {
							zsZone = (ZoneServer*)zone_details.zonePtr;
						}
						else if(zone_list.GetZone(&zone_details, 0, std::string(sep->arg[1]), false, false, false, false)) {
							zsZone = (ZoneServer*)zone_details.zonePtr;
						}
					}

					if( zsZone )
					{
						zsZone->SetZoneLockState(false);
						client->Message(CHANNEL_COLOR_YELLOW, "Zone %s (%u) is now unlocked.", zsZone->GetZoneName(), zsZone->GetZoneID());
					}
					else
						client->Message(CHANNEL_COLOR_RED, "Zone %s is not running and cannot be unlocked.", sep->arg[1]);
					notZoningCommand = true;
					break;
				}
				else
				{
					if(sep->IsNumber(0))
					{
						zone_id = atoul(sep->arg[0]);
						string zonename = database.GetZoneName(zone_id);

						if(zonename.length() > 0)
							zone = zonename;
					}
					else
						zone = sep->arg[0];
				}
				if(instanceID > 0)
				{
					if(zone_list.GetZoneByInstance(&zone_details, instanceID, 0, true)) {
						instanceID = zone_details.instanceId;
						zone = zone_details.zoneName;
						zone_id = zone_details.zoneId;
					}
					isInstance = true;
				}
			}

			if(!listSearch)
			{
				if(zone.length() == 0)
					client->Message(CHANNEL_COLOR_RED, "Error: Invalid Zone");
				else if(instanceID != client->GetCurrentZone()->GetInstanceID() || 
					zone_id != client->GetCurrentZone()->GetZoneID())
				{
					const char* zonestr = zone.c_str();
					if(database.VerifyZone(zonestr))
					{
						if(client->CheckZoneAccess(zonestr))
						{
							client->Message(CHANNEL_COLOR_YELLOW,"Zoning to %s...", zonestr);
							if(isInstance)
								client->Zone(&zone_details,(ZoneServer*)zone_details.zonePtr,true,false);
							else {
								if(sep->IsNumber(2) && zone_list.GetDuplicateZoneDetails(&zone_details, zone, 0, atoul(sep->arg[2]))) {
									client->Zone(&zone_details,(ZoneServer*)zone_details.zonePtr,true,false);
								}
								else 
									client->Zone(zonestr);
							}
						}
					}
					else
						client->Message(CHANNEL_COLOR_RED, "Error: Zone '%s' not found.  To get a list type /zone list", zonestr);
				}
				else
					client->Message(CHANNEL_COLOR_RED, "Error: You are already in that zone!");
			}
			else
			{
				const char* name = 0;
				if(sep && sep->arg[1][0])
				{
					map<int32, string>* zone_names;
					name = sep->argplus[1];
					sint16 status = client->GetAdminStatus();
					if( status > 0 )
					{
						client->SimpleMessage(CHANNEL_COLOR_RED, "ADMIN MODE");
						zone_names = database.GetZoneList(name, true);
					}
					else
						zone_names = database.GetZoneList(name);

					if(!zone_names)
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "No zones found.");
					else
					{
						map<int32, string>::iterator itr;
						client->SimpleMessage(CHANNEL_COLOR_YELLOW," ID   Name: ");
						for(itr = zone_names->begin(); itr != zone_names->end(); itr++)
							client->Message(CHANNEL_COLOR_YELLOW,"%03lu %s", itr->first, itr->second.c_str());
						safe_delete(zone_names);
					}
				}
				else
				{
					client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Syntax: /zone [zone name]");
					client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Syntax: /zone list [partial zone name]");
				}
			}
			break;
		}
		case COMMAND_USE: {
			Spawn* target = cmdTarget;
			if (target && target->IsWidget())
				((Widget*)target)->HandleUse(client, "use");
			break;
		}
		case COMMAND_OPEN: {
			Spawn* target = cmdTarget;
			if (target && target->IsWidget())
				((Widget*)target)->HandleUse(client, "Open", WIDGET_TYPE_DOOR);
			break;
		}
		case COMMAND_CASTSPELL: {
			if (sep && sep->arg[0] && sep->IsNumber(0))
			{
				int8 tier = 1;
				if (sep->arg[1] && sep->IsNumber(1))
					tier = atoul(sep->arg[1]);

				int8 self = 1;
				if (sep->arg[1] && sep->IsNumber(2))
				{
					self = atoul(sep->arg[2]);
					if(self != 1 && !cmdTarget->IsEntity())
					{
						client->Message(CHANNEL_COLOR_RED, "Target is not an entity required to cast non-self spell.");
						self = 1;
					}
				}

				int32 spellid = atoul(sep->arg[0]);
				Spell* spell = master_spell_list.GetSpell(spellid, tier);

				if (spell)
				{
					client->Message(CHANNEL_COLOR_RED, "Casting spell %u.", spellid);
					SpellProcess* spellProcess = 0;
					// Get the current zones spell process
					spellProcess = client->GetCurrentZone()->GetSpellProcess();

					spellProcess->CastInstant(spell, (!cmdTarget || self == 1) ? (Entity*)client->GetPlayer() : (Entity*)cmdTarget, (cmdTarget && cmdTarget->IsEntity()) ? (Entity*)cmdTarget : (Entity*)client->GetPlayer());
				}
				else
				{
					client->Message(CHANNEL_COLOR_RED, "Could not find spell %u.",spellid);
				}
			}
			else if (sep && sep->arg[0])
			{
				database.FindSpell(client, (char*)sep->argplus[0]);
			}
			else
			{
				client->Message(CHANNEL_COLOR_YELLOW, "Syntax: /castspell [spellid] (tier=1) - Cast Spell with specified spell id, tier is optional, default of 1.");
				client->Message(CHANNEL_COLOR_YELLOW, "Syntax: /castspell [spellname] - Find spells wildcard match with a partial spell name");
			}
			break;
		}
		case COMMAND_KNOWLEDGEWINDOWSORT: {

			if (sep && (client->GetVersion() <= 561 && sep->GetArgNumber() == 3) || sep->GetArgNumber() == 4)
			{
				int32 book = atoul(sep->arg[0]); // 0 - spells, 1 - combat, 2 - abilities, 3 - tradeskill
				
				// cannot sort the ability book in this client it is greyed out
				if (client->GetVersion() <= 561 && book == SPELL_BOOK_TYPE_ABILITY)
					break;
			
				int32 sort_by = atoul(sep->arg[1]); // 0 - alpha, 1 - level, 2 - category
				int32 order = atoul(sep->arg[2]); // 0 - ascending, 1 - descending
				int32 pattern = atoul(sep->arg[3]); // 0 - zigzag, 1 - down, 2 - across
				int32 maxlvlonly = 0;
				
				if(client->GetVersion() > 561 && sep->arg[4][0]) {
					maxlvlonly = atoul(sep->arg[4]); // checkbox for newer clients
				}
				client->GetPlayer()->ResortSpellBook(sort_by, order, pattern, maxlvlonly, book);
				ClientPacketFunctions::SendSkillSlotMappings(client);
			}
			else
			{
				client->Message(CHANNEL_COLOR_YELLOW, "Syntax: /knowledgewindow_sort [book] [sort_by] [order] [pattern]");
			}
			break;
		}
		case COMMAND_MOVE_ITEM:
		{
			int32 id = 0;

			if (sep && sep->IsNumber(0))
				id = atoul(sep->arg[0]);

			Spawn* spawn = 0;
			if (id == 0)
			{
				spawn = cmdTarget;
			}
			else
				spawn = client->GetCurrentZone()->GetSpawnFromUniqueItemID(id);

			if (!spawn || !client->HasOwnerOrEditAccess() || !spawn->GetPickupItemID())
				break;

			Item* item = master_item_list.GetItem(spawn->GetPickupItemID());
			
			client->SendMoveObjectMode(spawn, (item && item->houseitem_info) ? item->houseitem_info->house_location : 0);
			break;
		}
		case COMMAND_PICKUP:
		{
			int32 id = 0;
			if (sep && sep->IsNumber(0))
				id = atoul(sep->arg[0]);

			Spawn* spawn = 0;
			if (id == 0)
			{
				spawn = cmdTarget;
			}
			else
				spawn = client->GetCurrentZone()->GetSpawnFromUniqueItemID(id);

			if (!spawn || !client->HasOwnerOrEditAccess() || !spawn->GetPickupItemID())
				break;
			Item* tmpItem = client->GetPlayer()->item_list.GetVaultItemFromUniqueID(spawn->GetPickupUniqueItemID());
			if((tmpItem && tmpItem->generic_info.item_type == ITEM_TYPE_HOUSE_CONTAINER) || client->AddItem(spawn->GetPickupItemID(), 1)) {
				if ( tmpItem && tmpItem->generic_info.item_type == ITEM_TYPE_HOUSE_CONTAINER ) {
					tmpItem->TryUnlockItem(LockReason::LockReason_House);
					client->QueuePacket(client->GetPlayer()->SendInventoryUpdate(client->GetVersion()));
				}
				Query query;
				query.RunQuery2(Q_INSERT, "delete from spawn_instance_data where spawn_id = %u and spawn_location_id = %u and pickup_item_id = %u", spawn->GetDatabaseID(), spawn->GetSpawnLocationID(), spawn->GetPickupItemID());

				if (database.RemoveSpawnFromSpawnLocation(spawn)) {
					client->GetCurrentZone()->RemoveSpawn(spawn, true, true, true, true, true);
				}

				// we had a UI Window displayed, update the house items
				if ( id > 0 )
					client->GetCurrentZone()->SendHouseItems(client);
			}

			break;
		}
		case COMMAND_HOUSE_DEPOSIT:
		{
			PrintSep(sep, "COMMAND_HOUSE_DEPOSIT");
			// arg0 = spawn_id for DoF, could also be house_id for newer clients
			// arg1 = coin (in copper)
			// arg2 = status? (not implemented yet)
			PlayerHouse* ph = nullptr;
			int32 spawn_id = 0;
			if(sep && sep->arg[0]) {
				spawn_id = atoul(sep->arg[0]);
				ph = world.GetPlayerHouse(client, spawn_id, client->GetVersion() > 561 ? spawn_id : 0, nullptr);
			}
			
			if(!ph) {
				ph = world.GetPlayerHouseByInstanceID(client->GetCurrentZone()->GetInstanceID());
			}
			if (ph && sep && sep->IsNumber(1))
			{
				int64 outValCoin = strtoull(sep->arg[1], NULL, 0);
				int32 outValStatus = 0;
				
				if(sep->IsNumber(2))
				{
					outValStatus = strtoull(sep->arg[2], NULL, 0);
					if(outValStatus > client->GetPlayer()->GetInfoStruct()->get_status_points())
						outValStatus = 0; // cheat check
				}
				
				if ((!outValCoin && outValStatus) || client->GetPlayer()->RemoveCoins(outValCoin))
				{
					if(outValStatus)
						client->GetPlayer()->GetInfoStruct()->subtract_status_points(outValStatus);
					char query[256];
					map<string,Deposit>::iterator itr = ph->depositsMap.find(string(client->GetPlayer()->GetName()));
					if (itr != ph->depositsMap.end())
					{
						snprintf(query, 256, "update character_house_deposits set timestamp = %u, amount = amount + %llu, last_amount = %llu, status = status + %u, last_status = %u where house_id = %u and instance_id = %u and name='%s'", Timer::GetUnixTimeStamp(), outValCoin, outValCoin, outValStatus, outValStatus, ph->house_id, ph->instance_id, client->GetPlayer()->GetName());
					}
					else
						snprintf(query, 256, "insert into character_house_deposits set timestamp = %u, house_id = %u, instance_id = %u, name='%s', amount = %llu, status = %u", Timer::GetUnixTimeStamp(), ph->house_id, ph->instance_id, client->GetPlayer()->GetName(), outValCoin, outValStatus);

					if (database.RunQuery(query, strnlen(query, 256)))
					{
						ph->escrow_coins += outValCoin;
						ph->escrow_status += outValStatus;
						database.UpdateHouseEscrow(ph->house_id, ph->instance_id, ph->escrow_coins, ph->escrow_status);

						database.LoadDeposits(ph);
						client->PlaySound("coin_cha_ching");
						HouseZone* hz = world.GetHouseZone(ph->house_id);
						ClientPacketFunctions::SendBaseHouseWindow(client, hz, ph, client->GetVersion() <= 561 ? spawn_id : client->GetPlayer()->GetID());
					}
					else
					{
						if(outValCoin)
							client->GetPlayer()->AddCoins(outValCoin);
							
						if(outValStatus)
							client->GetPlayer()->GetInfoStruct()->add_status_points(outValStatus);
						client->SimpleMessage(CHANNEL_COLOR_RED, "Deposit failed!");
					}
				}
			}
			break;
		}
		case COMMAND_HOUSE:
		{
			int32 spawn_id = 0;
			if (sep && sep->IsNumber(0))
			{
				PlayerHouse* ph = nullptr;
				if(!ph && sep && sep->arg[0]) {
					spawn_id = atoul(sep->arg[0]);
					ph = world.GetPlayerHouse(client, spawn_id, spawn_id, nullptr);
				}
				
				HouseZone* hz = 0;
					
				if (ph)
					hz = world.GetHouseZone(ph->house_id);
				// there is a arg[1] that is true/false, but not sure what it is for investigate more later
				ClientPacketFunctions::SendBaseHouseWindow(client, hz, ph, client->GetVersion() <= 561 ? spawn_id : client->GetPlayer()->GetID());
			}
			else if (client->GetCurrentZone()->GetInstanceType() != 0)
			{
				// inside a house or something? Send the access window
				PlayerHouse* ph = world.GetPlayerHouseByInstanceID(client->GetCurrentZone()->GetInstanceID());

				HouseZone* hz = 0;
				if ( ph )
					hz = world.GetHouseZone(ph->house_id);

				ClientPacketFunctions::SendBaseHouseWindow(client, hz, ph, client->GetVersion() <= 561 ? spawn_id : client->GetPlayer()->GetID());
				client->GetCurrentZone()->SendHouseItems(client);
			}
			break;
		}
		case COMMAND_HOUSE_UI:
		{
			if (sep && sep->IsNumber(0) && client->GetCurrentZone()->GetInstanceType() == Instance_Type::NONE)
			{
				int32 unique_id = atoi(sep->arg[0]);
				PlayerHouse* ph = world.GetPlayerHouseByUniqueID(unique_id);
				HouseZone* hz = 0;
				if ( ph )
					hz = world.GetHouseZone(ph->house_id);
				// there is a arg[1] that is true/false, but not sure what it is for investigate more later
				ClientPacketFunctions::SendBaseHouseWindow(client, hz, ph, client->GetPlayer()->GetID());
			}
			break;
		}
		case COMMAND_PLACE_HOUSE_ITEM: {
			if (sep && sep->IsNumber(0))
			{
				int64 uniqueid = strtoull(sep->arg[0], NULL, 0);

				Item* item = client->GetPlayer()->item_list.GetVaultItemFromUniqueID(uniqueid);
	
				if (item && (item->IsHouseItem() || item->IsHouseContainer()))
				{
					if(item->IsHouseContainer() && item->details.inv_slot_id != InventorySlotType::HOUSE_VAULT) { // must be in base slot of vault in house for house containers
						client->SimpleMessage(CHANNEL_COLOR_RED, "Must be in vault to place this item.");
						break;
					}
					if (!client->HasOwnerOrEditAccess())
					{
						client->SimpleMessage(CHANNEL_COLOR_RED, "This is not your home!");
						break;
					}
					else if (!item->generic_info.appearance_id)
					{
						client->Message(CHANNEL_COLOR_RED, "This item has not been configured in the database, %s (%u) needs an entry where ?? has the model type id, eg. insert into item_appearances set item_id=%u,equip_type=??;", item->name.c_str(), item->details.item_id, item->details.item_id);
						break;
					}
					else if(item->CheckFlag2(HOUSE_LORE) && client->GetCurrentZone()->HouseItemSpawnExists(item->details.item_id)) {
						client->Message(CHANNEL_COLOR_RED, "Item %s is house lore and you cannot place another.", item->name.c_str());
						break;
					}

					if (client->GetTempPlacementSpawn())
					{
						Spawn* tmp = client->GetTempPlacementSpawn();
						client->GetCurrentZone()->RemoveSpawn(tmp, true, true, true, true, true);
						client->SetTempPlacementSpawn(nullptr);
					}
					

					Object* obj = new Object();
					Spawn* spawn = (Spawn*)obj;
					memset(&spawn->appearance, 0, sizeof(spawn->appearance));
					strcpy(spawn->appearance.name, item->name.c_str());
					spawn->SetX(client->GetPlayer()->GetX());
					spawn->SetY(client->GetPlayer()->GetY());
					spawn->SetZ(client->GetPlayer()->GetZ());
					spawn->appearance.pos.collision_radius = 32;
					spawn->SetHeading(client->GetPlayer()->GetHeading());
					spawn->SetSpawnOrigX(spawn->GetX());
					spawn->SetSpawnOrigY(spawn->GetY());
					spawn->SetSpawnOrigZ(spawn->GetZ());
					spawn->SetSpawnOrigHeading(spawn->GetHeading());
					spawn->appearance.targetable = 1;
					spawn->appearance.activity_status = ACTIVITY_STATUS_SOLID;
					spawn->appearance.race = item && item->generic_info.appearance_id ? item->generic_info.appearance_id : 1472;
					spawn->SetModelType(item && item->generic_info.appearance_id ? item->generic_info.appearance_id : 1472);
					spawn->SetLocation(client->GetPlayer()->GetLocation());
					spawn->SetZone(client->GetCurrentZone());
					client->SetTempPlacementSpawn(spawn);
					client->SetPlacementUniqueItemID(uniqueid);
					client->GetCurrentZone()->AddSpawn(spawn);
				}
			}
			break;
		}
		case COMMAND_GM:
		{
			if (sep && sep->arg[0] && sep->arg[1])
			{
				bool onOff = (strcmp(sep->arg[1], "on") == 0);
				if (strcmp(sep->arg[0], "vision") == 0)
				{
					client->GetPlayer()->SetGMVision(onOff);
					#if defined(__GNUC__)
						database.insertCharacterProperty(client, CHAR_PROPERTY_GMVISION, (onOff) ? (char*)"1" : (char*)"0");
					#else
						database.insertCharacterProperty(client, CHAR_PROPERTY_GMVISION, (onOff) ? "1" : "0");
					#endif
					client->GetCurrentZone()->SendAllSpawnsForVisChange(client, false);

					if (onOff)
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "GM Vision Enabled!");
					else
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "GM Vision Disabled!");
				}
				else if (strcmp(sep->arg[0], "tag") == 0)
				{
					int32 value = 0;
					int16 tag_icon = 0;
					// groundspawn has special exception -1 argument so it needs to be handled here
					if(sep->arg[2] && sep->arg[3] && ((tag_icon = atoul(sep->arg[3])) > 0) || (strncasecmp(sep->arg[1], "groundspawn", 11) == 0)) {
						value = atoul(sep->arg[2]);
						if(strncasecmp(sep->arg[1], "faction", 7) == 0){
							if(!value) {
								client->SimpleMessage(CHANNEL_COLOR_RED, "Need to supply a valid faction id.");
								break;
							}
							if(!tag_icon) {
								client->SimpleMessage(CHANNEL_COLOR_RED, "Need to supply a valid tag icon id.");
								break;
							}
							client->GetPlayer()->AddGMVisualFilter(GMTagFilterType::GMFILTERTYPE_FACTION, value, "", tag_icon);
							client->GetCurrentZone()->SendAllSpawnsForVisChange(client, false);
							client->Message(CHANNEL_COLOR_RED, "Adding faction id %u with tag icon %u.", value, tag_icon);
						}
						else if(strncasecmp(sep->arg[1], "spawngroup", 10) == 0){
							if(value>0) {
								value = 1;
							}
							if(!tag_icon) {
								client->SimpleMessage(CHANNEL_COLOR_RED, "Need to supply a valid tag icon id.");
								break;
							}
							client->GetPlayer()->AddGMVisualFilter(GMTagFilterType::GMFILTERTYPE_SPAWNGROUP, value, "", tag_icon);
							client->GetCurrentZone()->SendAllSpawnsForVisChange(client, false);
							client->Message(CHANNEL_COLOR_RED, "Adding spawn group tag \"%s\" with tag icon %u.", (value == 1) ? "on" : "off", tag_icon);
						}
						else if(strncasecmp(sep->arg[1], "race", 4 == 0)){
							if(!value) {
								client->SimpleMessage(CHANNEL_COLOR_RED, "Need to supply a valid race id.");
								break;
							}
							if(!tag_icon) {
								client->SimpleMessage(CHANNEL_COLOR_RED, "Need to supply a valid tag icon id.");
								break;
							}
							client->GetPlayer()->AddGMVisualFilter(GMTagFilterType::GMFILTERTYPE_RACE, value, "", tag_icon);
							client->GetCurrentZone()->SendAllSpawnsForVisChange(client, false);
							client->Message(CHANNEL_COLOR_RED, "Adding race id %u with tag icon %u.", value, tag_icon);
						}
						else if(strncasecmp(sep->arg[1], "groundspawn", 11) == 0){
							// one less argument value field not tag_icon for this one
							if(!value) {
								client->SimpleMessage(CHANNEL_COLOR_RED, "Need to supply a valid tag icon id.");
								break;
							}
							client->GetPlayer()->AddGMVisualFilter(GMTagFilterType::GMFILTERTYPE_GROUNDSPAWN, 1, "", value);
							client->GetCurrentZone()->SendAllSpawnsForVisChange(client, false);
							client->Message(CHANNEL_COLOR_RED, "Adding groundspawns with tag icon %u.", value);
						}
					}
					else {
						if(strncasecmp(sep->arg[1], "clear", 5) == 0){
							client->GetPlayer()->ClearGMVisualFilters();
							client->SimpleMessage(CHANNEL_COLOR_YELLOW, "GM Tags Cleared!");
							client->GetCurrentZone()->SendAllSpawnsForVisChange(client, false);
						}
						else {
						client->SimpleMessage(CHANNEL_COLOR_RED, "GM Tagging Missing Arguments:");
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/gm tag [type] [value] [visual_icon_display]");
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/gm tag clear - Clears all GM visual tags");

						client->SimpleMessage(CHANNEL_COLOR_RED, "Visual Type Options:");
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "type: faction, value: faction id of spawn(s)");
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "type: spawngroup, value: 1 to show grouped, 0 to show not grouped");
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "type: race, value: race id (either against base race or race id in spawn details)");
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "type: groundspawn, value: (not used)");

						client->SimpleMessage(CHANNEL_COLOR_RED, "Visual Icon Options:");

						client->SimpleMessage(CHANNEL_COLOR_YELLOW, 
						"1 = skull, 2 = shield half dark blue / half light blue, 3 = purple? star, 4 = yellow sword, 5 = red X, 6 = green flame");
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, 
						"7 = Number \"1\", 8 = Number \"2\", 9 = Number \"3\", 10 = Number \"4\", 11 = Number \"5\", 12 = Number \"6\"");
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, 
						"25 = shield, 26 = green plus, 27 = crossed swords, 28 = bow with arrow in it, 29 = light blue lightning bolt");
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, 
						"30 = bard instrument (hard to see), 31 = writ with shield, 32 = writ with green +, 33 = writ with crossed sword");
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, 
						"34 = writ with bow, 35 = writ with light blue lightning bolt, 36 = same as 30, 37 = party with crossed sword, shield and lightning bolt");
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, 
						"38 = shaking hands green background, 39 = shaking hands dark green background, unlocked keylock");
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, 
						"40 = red aura icon with black shadow of person and big red aura, 41 = green aura icon with black shadow of person big green aura");
						}
					}
				}
				else if (strcmp(sep->arg[0], "regiondebug") == 0)
				{
					client->SetRegionDebug(onOff);
					#if defined(__GNUC__)
						database.insertCharacterProperty(client, CHAR_PROPERTY_REGIONDEBUG, (onOff) ? (char*)"1" : (char*)"0");
					#else
						database.insertCharacterProperty(client, CHAR_PROPERTY_REGIONDEBUG, (onOff) ? "1" : "0");
					#endif

					if (onOff)
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Region Debug Enabled!");
					else
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Region Debug Disabled!");
				}
				else if (strcmp(sep->arg[0], "sight") == 0)
				{
					if(onOff && cmdTarget) {
						client->SetPlayerPOVGhost(cmdTarget);
					}
					else
						client->SetPlayerPOVGhost(nullptr);
				}
				else if (strcmp(sep->arg[0], "controleffects") == 0)
				{
					if(cmdTarget && cmdTarget->IsEntity()) {
						((Entity*)cmdTarget)->SendControlEffectDetailsToClient(client);
					}
					else {
						client->GetPlayer()->SendControlEffectDetailsToClient(client);
					}
				}
				else if (strcmp(sep->arg[0], "luadebug") == 0)
				{
					client->SetLuaDebugClient(onOff);
#if defined(__GNUC__)
					database.insertCharacterProperty(client, CHAR_PROPERTY_LUADEBUG, (onOff) ? (char*)"1" : (char*)"0");
#else
					database.insertCharacterProperty(client, CHAR_PROPERTY_LUADEBUG, (onOff) ? "1" : "0");
#endif

					if (onOff)
					{
						if (lua_interface)
							lua_interface->UpdateDebugClients(client);

						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You will now receive LUA error messages.");
					}
					else
					{
						if (lua_interface)
							lua_interface->RemoveDebugClients(client);
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You will no longer receive LUA error messages.");
					}
				}
			}
			break;
		}
		case COMMAND_ATTACK:
		case COMMAND_AUTO_ATTACK:{
			Command_AutoAttack(client, sep);
			break;
		}
		case COMMAND_DEPOP:{
			bool allow_respawns = false;
			if(sep && sep->arg[0] && sep->IsNumber(0)){
				if(atoi(sep->arg[0]) == 1)
					allow_respawns = true;
			}
			client->GetCurrentZone()->Depop(allow_respawns);
			break;
						   }
		case COMMAND_REPOP:{
			client->GetCurrentZone()->Depop(false, true);
			break;
						   }
		case COMMAND_BUYBACK:{
			if(sep && sep->arg[1][0] && sep->IsNumber(0) && sep->IsNumber(1)){
				// Item ID and Unique ID get combined in this command so we need to get the number as an int64 and break it into 2 int32's
				int64 id = atoi64(sep->arg[0]);
				// get the first int32, the item id
				//int32 item_id = (int32)id;
				// get the second int32, the unique id
				int32 unique_id = (int32)(id>>32);
				int8 quantity = atoi(sep->arg[1]);
				if(quantity == 255)
					quantity = 1;
				client->BuyBack(unique_id, quantity);
			}
			break;
							 }
		case COMMAND_MERCHANT_BUY:{
			if(sep && sep->arg[1][0] && sep->IsNumber(0) && sep->IsNumber(1)){
				int32 item_id = atoul(sep->arg[0]);
				int8 quantity = atoi(sep->arg[1]);
				client->BuyItem(item_id, quantity);
			}
			else
				Command_SendMerchantWindow(client, sep);
			break;
								  }
		case COMMAND_MERCHANT_SELL:{
			if(sep && sep->arg[1][0] && sep->IsNumber(0) && sep->IsNumber(1)){
				// Item ID and Unique ID get combined in this command so we need to get the number as an int64 and break it into 2 int32's
				int64 id = atoi64(sep->arg[0]);
				// get the first int32, the item id
				int32 item_id = (int32)id;
				// get the second int32, the unique id
				int32 unique_id = (int32)(id>>32);

				int8 quantity = atoi(sep->arg[1]);
				LogWrite(MERCHANT__DEBUG, 0, "Merchant", "Selling Item: Item Id = %ul, unique id = %ul, Quantity = %i", item_id, unique_id, quantity);
				client->SellItem(item_id, quantity, unique_id);
			}
			break;
								   }
		case COMMAND_MENDER_REPAIR: {
			if (sep && sep->arg[0] && sep->IsNumber(0))
				client->RepairItem(atoul(sep->arg[0]));
			break;
									}
		case COMMAND_MENDER_REPAIR_ALL: {
			client->RepairAllItems();
			break;
										}
		case COMMAND_CANCEL_MERCHANT:{
			client->SetMerchantTransaction(0);
			break;
									 }
		case COMMAND_START_MERCHANT:{
			break;
									}
		case COMMAND_INVULNERABLE:{
			sint8 val = -1;
			if(sep && sep->arg[0] && sep->IsNumber(0)){
				val = atoi(sep->arg[0]);
				
//devn00b: Fix for linux builds
#if defined(__GNUC__)
 database.insertCharacterProperty(client, CHAR_PROPERTY_INVUL, (val == 1) ? (char*) "1" : (char*) "0");
#else
 database.insertCharacterProperty(client, CHAR_PROPERTY_INVUL, (val == 1) ? "1" : "0");
#endif
//				database.insertCharacterProperty(client, CHAR_PROPERTY_INVUL, (val == 1) ? "1" : "0");

				client->GetPlayer()->SetInvulnerable(val==1);
				if(client->GetPlayer()->GetInvulnerable())
					client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You are now invulnerable!");
				else
					client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You are no longer invulnerable!");
			}
			if(val == -1)
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /invulnerable [0/1]");
			break;
		}
		case COMMAND_REPAIR: {
			Spawn* spawn = cmdTarget;
			if (spawn && spawn->GetMerchantType() & MERCHANT_TYPE_REPAIR) {
				client->SendHailCommand(spawn);
				client->SetMerchantTransaction(spawn);
				client->SendRepairList();
			}
			break;
							 }
		case COMMAND_LOTTO: {
			Spawn* spawn = cmdTarget;
			if (spawn && spawn->GetMerchantType() & MERCHANT_TYPE_LOTTO) {
				client->SetMerchantTransaction(spawn);
				client->ShowLottoWindow();
			}
			break;
							}
		case COMMAND_ACCEPT_REWARD:{
			int32 unknown = 0;
			int32 selectable_item_id = 0;
			//Quest *quest = 0;
			/* no idea what the first argument is for (faction maybe?)
			   if the reward has a selectable item reward, it's sent as the second argument
			   if neither of these are included in the reward, there is no sep
			   since the regular item rewards are manditary to receive, we don't have to know what they are until we accept the collection or quest
			   only 1 quest or collection reward may be displayed at a time */
			if (sep && sep->arg[0] && sep->arg[1] && sep->IsNumber(0) && sep->IsNumber(1)) {
				unknown = atoul(sep->arg[0]);
				selectable_item_id = atoul(sep->arg[1]);
			}

			/* this logic here may seem unexpected, but the quest queue response for GetPendingQuestAcceptance is only populated if it is the current reward displayed to the client based on a quest
			** Otherwise it will likely be a DoF client scenario (pending item rewards, selectable item rewards) which is specifying an item id
			** lastly it will be a collection which also supplies an item id and you can only have one pending collection turn in at a time (they queue against Client::HandInCollections
			*/
			int32 item_id = 0;
			if(sep && sep->arg[0][0] && sep->IsNumber(0))
				item_id = atoul(sep->arg[0]);

			bool collectedItems = client->GetPlayer()->AcceptQuestReward(item_id, selectable_item_id);
			
			if (collectedItems) {
				EQ2Packet* outapp = client->GetPlayer()->SendInventoryUpdate(client->GetVersion());
				if (outapp)
					client->QueuePacket(outapp);
			}
			else
				LogWrite(COMMAND__ERROR, 0, "Command", "Error in COMMAND_ACCEPT_REWARD. No pending quest or collection reward was found (unknown=%u).", unknown);
			break;
		}
		case COMMAND_BUY_FROM_BROKER:{
			if(sep && sep->arg[1][0] && sep->IsNumber(0) && sep->IsNumber(1)){
				int64 item_id = strtoull(sep->arg[0], NULL, 0);
				int16 quantity = atoul(sep->arg[1]);
				if(client->IsGMStoreSearch()) {
					Item* item = master_item_list.GetItem(item_id);
					if(item && item->generic_info.max_charges > 1)
						quantity = item->generic_info.max_charges;
					client->AddItem(item_id, quantity, AddItemType::BUY_FROM_BROKER);
				}
				else {
					client->BuySellerItemByItemUniqueId(item_id, quantity);
					LogWrite(COMMAND__ERROR, 0, "Command", "BUY_FROM_BROKER. Item ID %u, Quantity %u, full args %s.", item_id, quantity, sep->argplus[0]);
				}
			}
			break;
		}
		case COMMAND_SEARCH_STORES_PAGE:{
			LogWrite(COMMAND__ERROR, 0, "Command", "SearchStores: %s", sep && sep->arg[0] ? sep->argplus[0] : "");
			if(sep && sep->arg[0][0] && sep->IsNumber(0)){
				int32 page = atoul(sep->arg[0]);
				client->SearchStore(page);
			}
			else {
				client->SetGMStoreSearch(false);
				PacketStruct* packet = configReader.getStruct("WS_StartBroker", client->GetVersion());
				if (packet) {
					packet->setDataByName("spawn_id", client->GetPlayer()->GetIDWithPlayerSpawn(client->GetPlayer()));
					//packet->setDataByName("unknown", 1);
					packet->setDataByName("unknown2", 5, 0);
					packet->setDataByName("unknown2", 20, 1);
					packet->setDataByName("unknown2", 58, 3);
					packet->setDataByName("unknown2", 40, 4);
					client->QueuePacket(packet->serialize());
					if(client->GetVersion() > 561) {
						PacketStruct* packet2 = configReader.getStruct("WS_BrokerBags", client->GetVersion());
						if (packet2) {
							packet2->setDataByName("char_id", client->GetCharacterID());
							client->QueuePacket(packet2->serialize()); //send this for now, needed to properly clear data
							safe_delete(packet2);
						}
						safe_delete(packet);
					}
				}
			}
			break;
		}
		case COMMAND_SEARCH_STORES:{
			if(sep && sep->arg[0][0]){
				const char* values = sep->argplus[0];
				if(values){
					LogWrite(ITEM__WARNING, 0, "Item", "SearchStores: %s", values);
					map<string, string> str_values = TranslateBrokerRequest(values);
					vector<Item*>* items = master_item_list.GetItems(str_values, client);
					if(items){
						client->SetItemSearch(items, str_values);
						client->SetSearchPage(0);
						client->SearchStore(0);
					}
				}
			}
			break;
		}
		
		case COMMAND_LUADEBUG:{
			if(sep && sep->arg[0][0] && strcmp(sep->arg[0], "start") == 0){
				client->SetLuaDebugClient(true);
				if(lua_interface)
					lua_interface->UpdateDebugClients(client);
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You will now receive LUA error messages.");
			}
			else if(sep && sep->arg[0][0] && strcmp(sep->arg[0], "stop") == 0){
				client->SetLuaDebugClient(false);
				if(lua_interface)
					lua_interface->RemoveDebugClients(client);
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You will no longer receive LUA error messages.");
			}
			else{
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Syntax: /luadebug {start | stop}");
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "This will allow you to receive lua debug messages normally seen only in the console.");
			}
			break;
		}
		case COMMAND_SPAWN_GROUP:
		{
			Spawn* target = cmdTarget;

			if(target && target->IsPlayer() == false)
			{
				if(sep && sep->arg[0][0] && strcmp(sep->arg[0], "create") == 0)
				{
					if(target->GetSpawnLocationPlacementID() > 0)
					{
						if(sep->arg[1] && !sep->IsNumber(1) && strlen(sep->arg[1]) > 0)
						{
							int32 id = database.CreateSpawnGroup(target, sep->arg[1]);

							if(id > 0)
								client->Message(CHANNEL_COLOR_YELLOW, "Successfully created new spawn group with id: %u", id);
							else
								client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Error creating group, check console for details.");

							target->SetSpawnGroupID(id);
							target->AddSpawnToGroup(target);
						}
						else
							client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Syntax: /spawn group create [name]");
					}
					else
						client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Could not use spawn to create a new group.  Likely cause would be a newly spawned spawn that did not exist when /reload spawns was last used.");
				}
				else if(sep && sep->arg[0][0] && strcmp(sep->arg[0], "add") == 0)
				{
					if(sep->arg[1] && sep->IsNumber(1))
					{
						int32 group_id = atoul(sep->arg[1]);
						Spawn* leader = client->GetCurrentZone()->GetSpawnGroup(group_id);

						if(leader)
						{
							leader->AddSpawnToGroup(target);
							target->SetSpawnGroupID(group_id);

							if(database.SpawnGroupAddSpawn(target, group_id))
								client->Message(CHANNEL_COLOR_YELLOW, "Successfully added '%s' to group id: %u", target->GetName(), group_id);
							else
								client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Error adding spawn to group, check console for details.");
						}
						else
							client->SimpleMessage(CHANNEL_COLOR_YELLOW, "No spawns found in the current zone for that spawn group, be sure to use '/spawn group create' first!");
					}
					else
						client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Syntax: /spawn group add [group id]");
				}
				else if(target->GetSpawnGroupID() == 0)
				{
					client->SimpleMessage(CHANNEL_COLOR_YELLOW, "That spawn is not in a group!");
				}
				else if(sep && sep->arg[0][0] && strcmp(sep->arg[0], "remove") == 0)
				{
					int32 group_id = target->GetSpawnGroupID();
					target->RemoveSpawnFromGroup();

					if(database.SpawnGroupRemoveSpawn(target, group_id))
						client->Message(CHANNEL_COLOR_YELLOW, "Successfully removed '%s' from group id: %u", target->GetName(), group_id);
					else
						client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Error removing spawn from group, check console for details.");
				}
				else if(sep && sep->arg[0][0] && strcmp(sep->arg[0], "associate") == 0)
				{
					int32 group_id = target->GetSpawnGroupID();

					if(sep->arg[1] && sep->IsNumber(1))
					{
						if(database.SpawnGroupAddAssociation(group_id, atoul(sep->arg[1])))
							client->Message(CHANNEL_COLOR_YELLOW, "Successfully associated group id %u with group id %u", group_id, atoul(sep->arg[1]));
						else
							client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Error associating groups, check console for details.");
					}
					else
						client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Allows you to spawn only one associated group at once.  Syntax: /spawn group associate [other group id]");
				}
				else if(sep && sep->arg[0][0] && strcmp(sep->arg[0], "deassociate") == 0)
				{
					int32 group_id = target->GetSpawnGroupID();

					if(sep->arg[1] && sep->IsNumber(1))
					{
						if(database.SpawnGroupRemoveAssociation(group_id, atoul(sep->arg[1])))
							client->Message(CHANNEL_COLOR_YELLOW, "Successfully removed association between group id %u and group id %u", group_id, atoul(sep->arg[1]));
						else
							client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Error removing group associations, check console for details.");
					}
					else
						client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Removes previous group associations.  Syntax: /spawn group deassociate [other group id]");
				}
				else if(sep && sep->arg[0][0] && strcmp(sep->arg[0], "chance") == 0)
				{
					if(sep->arg[1] && sep->IsNumber(1))
					{
						if(database.SetGroupSpawnChance(target->GetSpawnGroupID(), atof(sep->arg[1])))
							client->Message(CHANNEL_COLOR_YELLOW, "Successfully set group spawn chance to %f for group id: %u", atof(sep->arg[1]), target->GetSpawnGroupID());
						else
							client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Error setting group spawn chance, check console for details.");
					}
					else
						client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Syntax: /spawn group chance [group's spawn chance percentage]");
				}
				else
				{
					client->SimpleMessage(CHANNEL_COLOR_YELLOW, "This command allows you to modify spawn groups.");
					client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Syntax: /spawn group [create/add/remove/chance/associate/deassociate]");
				}
			}
			else
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You must select a valid spawn to use this command.");
			break;
		}
		case COMMAND_SPAWN_COMBINE:{
			Spawn* spawn = cmdTarget;
			float radius = 0;
			bool failed = true;
			if(spawn && !spawn->IsPlayer() && sep && sep->arg[0] && sep->arg[0][0]){
				failed = false;
				if(spawn->GetSpawnGroupID() > 0){
					client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Can not combine this spawn as it is in a spawn group.  Remove it from the group and try again.");
				}
				else if(sep->IsNumber(0)){
					radius = atof(sep->arg[0]);
					client->CombineSpawns(radius, spawn);
				}
				else{
					if(strncasecmp(sep->arg[0], "add", 3) == 0){
						client->AddCombineSpawn(spawn);
					}
					else if(strncasecmp(sep->arg[0], "remove", 6) == 0){
						client->RemoveCombineSpawn(spawn);
					}
					else if(strncasecmp(sep->arg[0], "save", 4) == 0){
						const char* name = 0;
						if(sep->arg[1] && sep->arg[1][0])
							name = sep->argplus[1];
						client->SaveCombineSpawns(name);
					}
					else
						failed = true;
				}
			}
			if(failed){
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "This command combines several spawns into a single spawn group.");
				client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Syntax: /spawn combine [radius/add/remove/save]");
				client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Ex: /spawn combine 1, /spawn combine save (spawn group name)");
			}
			break;
								   }
		case COMMAND_SPAWN_CREATE:{
			Spawn* spawn = 0;
			if(sep && sep->arg[4][0] && strncasecmp(sep->arg[0], "object", 6) == 0 && sep->IsNumber(1) && sep->IsNumber(2) && sep->IsNumber(3)){
				spawn = new Object();
				memset(&spawn->appearance, 0, sizeof(spawn->appearance));
			}
			else if (sep && sep->arg[4][0] && strncasecmp(sep->arg[0], "groundspawn", 11) == 0 && sep->IsNumber(1) && sep->IsNumber(2) && sep->IsNumber(3)) {
				spawn = new GroundSpawn();
				memset(&spawn->appearance, 0, sizeof(spawn->appearance));
			}
			else if (sep && sep->arg[4][0] && strncasecmp(sep->arg[0], "sign", 4) == 0 && sep->IsNumber(1) && sep->IsNumber(2) && sep->IsNumber(3)) {
				spawn = new Sign();
				memset(&spawn->appearance, 0, sizeof(spawn->appearance));
			}
			else if(sep && sep->arg[4][0] && strncasecmp(sep->arg[0], "npc", 3) == 0 && sep->IsNumber(1) && sep->IsNumber(2) && sep->IsNumber(3)){
				spawn = new NPC();
				memset(&spawn->appearance, 0, sizeof(spawn->appearance));
				spawn->appearance.pos.collision_radius = 32;
				spawn->secondary_command_list_id = 0;
				spawn->primary_command_list_id = 0;
				spawn->appearance.display_name = 1;
				spawn->appearance.show_level = 1;
				spawn->appearance.attackable = 1;
			}
			else{
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Syntax: /spawn create [spawn type] [race type] [class type] [level] [name] (difficulty) (size)");
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "All parameters are required except difficulty and size.  Valid types are Object, NPC, Sign, or GroundSpawn");
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Ex: /spawn create npc 203 1 50 'Lady Vox' 1 32");
				break;
			}
			spawn->SetID(Spawn::NextID());
			spawn->SetX(client->GetPlayer()->GetX());
			spawn->SetY(client->GetPlayer()->GetY());
			spawn->SetZ(client->GetPlayer()->GetZ());
			spawn->SetHeading(client->GetPlayer()->GetHeading());
			spawn->SetSpawnOrigX(spawn->GetX());
			spawn->SetSpawnOrigY(spawn->GetY());
			spawn->SetSpawnOrigZ(spawn->GetZ());
			spawn->SetSpawnOrigHeading(spawn->GetHeading());

			
			if(spawn->IsSign())
			{
				((Sign*)spawn)->SetSignType(SIGN_TYPE_GENERIC);
				((Sign*)spawn)->SetSignDistance(20.0f);
				((Sign*)spawn)->SetIncludeLocation(1);
				((Sign*)spawn)->SetIncludeHeading(1);
				((Sign*)spawn)->SetInitialState(1);
				((Sign*)spawn)->SetSignTitle(sep->arg[4]);
				((Sign*)spawn)->SetActivityStatus(64);
				spawn->appearance.race = 0;
				spawn->SetLevel(0);
				spawn->SetHP(0);
				spawn->SetTotalHP(0);
				spawn->SetPower(0);
				spawn->SetTotalPower(0);
				spawn->SetDifficulty(0);
				spawn->SetTargetable(0);
				spawn->SetSogaModelType(0);
				spawn->SetCollisionRadius(19);
				((Sign*)spawn)->SetWidgetX(client->GetPlayer()->GetX());
				((Sign*)spawn)->SetWidgetY(client->GetPlayer()->GetY());
				((Sign*)spawn)->SetWidgetZ(client->GetPlayer()->GetZ());
				spawn->SetLocation(client->GetPlayer()->GetLocation());
				spawn->appearance.model_type = atoul(sep->arg[1]);
			}
			else
			{
				spawn->appearance.targetable = 1;
				spawn->appearance.race = 255;
				spawn->SetLocation(client->GetPlayer()->GetLocation());
				spawn->SetModelType(atoi(sep->arg[1]));
				spawn->SetAdventureClass(atoi(sep->arg[2]));
				spawn->SetLevel(atoi(sep->arg[3]));
				spawn->SetName(sep->arg[4]);
				if(sep->arg[5][0] && sep->IsNumber(5))
					spawn->SetDifficulty(atoi(sep->arg[5]));
				if(sep->arg[6][0] && sep->IsNumber(6))
					spawn->size = atoi(sep->arg[6]);
				if(spawn->GetTotalHP() == 0){
					spawn->SetTotalHP(25*spawn->GetLevel() + 1);
					spawn->SetHP(25*spawn->GetLevel() + 1);
				}
				if(spawn->GetTotalPower() == 0){
					spawn->SetTotalPower(25*spawn->GetLevel() + 1);
					spawn->SetPower(25*spawn->GetLevel() + 1);
				}
			}

			client->GetCurrentZone()->AddSpawn(spawn);
			break;
								  }
		case COMMAND_SPAWN_EQUIPMENT:{
			Spawn* spawn = cmdTarget;
			if(spawn && sep && sep->arg[1][0] && sep->IsNumber(0) && sep->IsNumber(1) && spawn->IsEntity()){
				int8 slot = atoi(sep->arg[0]);
				int16 type = atoi(sep->arg[1]);
				int8 red = 0;
				int8 green = 0;
				int8 blue = 0;
				int8 h_red = 0;
				int8 h_green = 0;
				int8 h_blue = 0;
				if(sep->arg[2])
					red = atoi(sep->arg[2]);
				if(sep->arg[3])
					green = atoi(sep->arg[3]);
				if(sep->arg[4])
					blue = atoi(sep->arg[4]);
				if(sep->arg[5])
					h_red = atoi(sep->arg[5]);
				if(sep->arg[6])
					h_green = atoi(sep->arg[6]);
				if(sep->arg[7])
					h_blue = atoi(sep->arg[7]);
				((Entity*)spawn)->SetEquipment(slot, type, red, green, blue, h_red, h_green, h_blue);
				database.SaveNPCAppearanceEquipment(spawn->GetDatabaseID() , slot, type, red, green, blue, h_red, h_green, h_blue);
			}
			else{
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Syntax: /spawn equipment [slot] [appearance id] (red) (green) (blue) (highlight red) (hgreen) (hblue)");
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "This will set the given spawn's equipment. ");
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Slot is 0-24, Appearance ID from appearances table, Colors are 0-255");
			}
			break;
									 }
		case COMMAND_SPAWN_DETAILS: {
			Spawn* spawn = cmdTarget;
			if (sep && sep->arg[0][0]) {
				if (cmdTarget)
				{
					if (ToLower(string(sep->arg[0])) == "los")
					{
						bool hasLOS = client->GetPlayer()->CheckLoS(cmdTarget);
						if (hasLOS)
							client->Message(CHANNEL_COLOR_YELLOW, "You have line of sight with %s", spawn->GetName());
						else
							client->Message(CHANNEL_COLOR_RED, "You DO NOT have line of sight with %s", spawn->GetName());

						break;
					}
					else if (ToLower(string(sep->arg[0])) == "bestz")
					{
						glm::vec3 targPos(cmdTarget->GetX(), cmdTarget->GetZ(), cmdTarget->GetY());

						float bestZ = client->GetPlayer()->FindDestGroundZ(targPos, cmdTarget->GetYOffset());
						client->Message(CHANNEL_COLOR_YELLOW, "Best Z for %s is %f", spawn->GetName(), bestZ);
						break;
					}
					else if (ToLower(string(sep->arg[0])) == "inwater")
					{
						if (cmdTarget->GetRegionMap() == nullptr)
							client->SimpleMessage(CHANNEL_COLOR_RED, "No water map for zone.");
						else
						{
							bool inWater = cmdTarget->InWater();
							client->Message(CHANNEL_COLOR_YELLOW, "%s is %s.", cmdTarget->GetName(), inWater ? "in water" : "out of water");
						}
						break;
					}
					else if (ToLower(string(sep->arg[0])) == "inlava")
					{
						if (cmdTarget->GetRegionMap() == nullptr)
							client->SimpleMessage(CHANNEL_COLOR_RED, "No region map for zone.");
						else
						{
							bool inLava = cmdTarget->InLava();
							client->Message(CHANNEL_COLOR_YELLOW, "%s is %s.", cmdTarget->GetName(), inLava ? "in lava" : "out of lava");
						}
						break;
					}
					else if (ToLower(string(sep->arg[0])) == "regions")
					{
						glm::vec3 targPos(cmdTarget->GetX(), cmdTarget->GetY(), cmdTarget->GetZ());

						if (cmdTarget->GetRegionMap() == nullptr)
							client->SimpleMessage(CHANNEL_COLOR_RED, "No region map for zone.");
						else
						{
							cmdTarget->GetRegionMap()->IdentifyRegionsInGrid(client, targPos);
						}
						break;
					}
					else if (ToLower(string(sep->arg[0])) == "behind")
					{
						bool isBehind = client->GetPlayer()->BehindTarget(cmdTarget);
						client->Message(CHANNEL_COLOR_YELLOW, "%s %s.", isBehind ? "YOU are behind" : "YOU are NOT behind", cmdTarget->GetName());
						break;
					}
					else if (ToLower(string(sep->arg[0])) == "infront")
					{
						bool isBehind = client->GetPlayer()->InFrontSpawn(cmdTarget, client->GetPlayer()->GetX(), client->GetPlayer()->GetZ());
						client->Message(CHANNEL_COLOR_YELLOW, "%s %s.", isBehind ? "YOU are infront of" : "YOU are NOT infront of", cmdTarget->GetName());
						break;
					}
					else if (ToLower(string(sep->arg[0])) == "flank")
					{
						bool isFlanking = client->GetPlayer()->FlankingTarget(cmdTarget);
						client->Message(CHANNEL_COLOR_YELLOW, "%s is %s.", isFlanking ? "YOU are flanking" : "YOU are NOT flanking", cmdTarget->GetName());
						break;
					}
					else if (ToLower(string(sep->arg[0])) == "aggro")
					{
						if(cmdTarget->IsNPC() && ((NPC*)cmdTarget)->Brain()) {
							((NPC*)cmdTarget)->Brain()->SendEncounterList(client);
							((NPC*)cmdTarget)->Brain()->SendHateList(client);
						}
						else {
							client->SimpleMessage(CHANNEL_COLOR_RED, "/spawn details aggro is for NPCs only!");
						}
						break;
					}
					else if (ToLower(string(sep->arg[0])) == "angle")
					{
						float spawnAngle = client->GetPlayer()->GetFaceTarget(cmdTarget->GetX(), cmdTarget->GetZ());
						
						client->Message(CHANNEL_COLOR_YELLOW, "Angle %f between player %s and target %s", spawnAngle, client->GetPlayer()->GetTarget() ? client->GetPlayer()->GetTarget()->GetName() : client->GetPlayer()->GetName(), client->GetPlayer()->GetName());
						break;
					}
				}
				if (sep->IsNumber(0))
				{
					float radius = atof(sep->arg[0]);
					if (!client->GetCurrentZone()->SendRadiusSpawnInfo(client, radius))
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "No results in the radius were found.");
					break;
				}
			}
			if (spawn) {
				const char* type = "NPC";
				if (spawn->IsObject())
					type = "Object";
				else if (spawn->IsSign())
					type = "Sign";
				else if (spawn->IsWidget())
					type = "Widget";
				else if (spawn->IsGroundSpawn())
					type = "GroundSpawn";
				
				int16 race_type = race_types_list.GetRaceType(spawn->GetModelType());
				int16 race_base_type = race_types_list.GetRaceBaseType(spawn->GetModelType());
				char* category = race_types_list.GetRaceTypeCategory(spawn->GetModelType());
				char* subcategory = race_types_list.GetRaceTypeSubCategory(spawn->GetModelType());
				char* modelname = race_types_list.GetRaceTypeModelName(spawn->GetModelType());
				
				client->Message(CHANNEL_COLOR_YELLOW, "Name: %s, %s ID: %u", spawn->GetName(), type, spawn->GetDatabaseID());
				client->Message(CHANNEL_COLOR_YELLOW, "Last Name: %s, Sub-Title: %s, Prefix: %s, Suffix: %s", spawn->GetLastName(), spawn->GetSubTitle(), spawn->GetPrefixTitle(), spawn->GetSuffixTitle());
				client->Message(CHANNEL_COLOR_YELLOW, "Spawn Location ID: %u, Spawn Group ID: %u", spawn->GetSpawnLocationID(), spawn->GetSpawnGroupID());
				client->Message(CHANNEL_COLOR_YELLOW, "Faction ID: %u, Merchant ID: %u, Transporter ID: %u", spawn->GetFactionID(), spawn->GetMerchantID(), spawn->GetTransporterID());
				client->Message(CHANNEL_COLOR_YELLOW, "Grid ID: %u", spawn->GetLocation());
				client->Message(CHANNEL_COLOR_YELLOW, "Race: %i, Class: %i, Gender: %i", spawn->GetRace(), spawn->GetAdventureClass(), spawn->GetGender());
				client->Message(CHANNEL_COLOR_YELLOW, "Level: %i, HP: %u / %u, Power: %u / %u", spawn->GetLevel(), spawn->GetHP(), spawn->GetTotalHP(), spawn->GetPower(), spawn->GetTotalPower());
				client->Message(CHANNEL_COLOR_YELLOW, "Respawn Time: %u (sec), X: %f, Y: %f, Z: %f Heading: %f", spawn->GetRespawnTime(), spawn->GetX(), spawn->GetY(), spawn->GetZ(), spawn->GetHeading());
				client->Message(CHANNEL_COLOR_YELLOW, "Collision Radius: %i, Size: %i, Difficulty: %i, Heroic: %i", spawn->GetCollisionRadius(), spawn->GetSize(), spawn->GetDifficulty(), spawn->GetHeroic());
				client->Message(CHANNEL_COLOR_YELLOW, "Targetable: %i, Show Name: %i, Attackable: %i, Show Level: %i", spawn->GetTargetable(), spawn->GetShowName(), spawn->GetAttackable(), spawn->GetShowLevel());
				client->Message(CHANNEL_COLOR_YELLOW, "Show Command Icon: %i, Display Hand Icon: %i", spawn->GetShowCommandIcon(), spawn->GetShowHandIcon());
				if (spawn->IsEntity()) {
					client->Message(CHANNEL_COLOR_YELLOW, "Facial Hair Type: %i, Hair Type: %i, Chest Type: %i, Legs Type: %i", ((Entity*)spawn)->GetFacialHairType(), ((Entity*)spawn)->GetHairType(), ((Entity*)spawn)->GetChestType(), ((Entity*)spawn)->GetLegsType());
					client->Message(CHANNEL_COLOR_YELLOW, "Soga Facial Hair Type: %i, Soga Hair Type: %i, Wing Type: %i", ((Entity*)spawn)->GetSogaFacialHairType(), ((Entity*)spawn)->GetSogaHairType(), ((Entity*)spawn)->GetWingType());
				}

				client->Message(CHANNEL_COLOR_YELLOW, "Model Type: %i, Soga Race Type: %i, Race Type: %u, Race Base Type: %u, Race Category: %s (subcategory: %s), Model Name: %s.", spawn->GetModelType(), spawn->GetSogaModelType(), race_type, race_base_type, category, subcategory, modelname);
				client->Message(CHANNEL_COLOR_YELLOW, "Primary Command Type: %u, Secondary Command Type: %u", spawn->GetPrimaryCommandListID(), spawn->GetSecondaryCommandListID());
				client->Message(CHANNEL_COLOR_YELLOW, "Visual State: %i, Action State: %i, Mood State: %i, Initial State: %i, Activity Status: %i", spawn->GetVisualState(), spawn->GetActionState(), spawn->GetMoodState(), spawn->GetInitialState(), spawn->GetActivityStatus());
				client->Message(CHANNEL_COLOR_YELLOW, "Emote State: %i, Pitch: %f, Roll: %f, Hide Hood: %i", spawn->GetEmoteState(), spawn->GetPitch(), spawn->GetRoll(), spawn->appearance.hide_hood);
				if (spawn->IsNPC())
					client->Message(CHANNEL_COLOR_YELLOW, "Randomize: %u", ((NPC*)spawn)->GetRandomize());
				if (spawn->IsNPC()){
				client->Message(CHANNEL_COLOR_YELLOW, "Heat Resist/Base: %u/%u",((NPC*)spawn)->GetHeatResistance(), ((NPC*)spawn)->GetHeatResistanceBase());					
				client->Message(CHANNEL_COLOR_YELLOW, "Cold Resist/Base: %u/%u",((NPC*)spawn)->GetColdResistance(), ((NPC*)spawn)->GetColdResistanceBase());
				client->Message(CHANNEL_COLOR_YELLOW, "Magic Resist/Base: %u/%u",((NPC*)spawn)->GetMagicResistance(), ((NPC*)spawn)->GetMagicResistanceBase());
				client->Message(CHANNEL_COLOR_YELLOW, "Mental Resist/Base: %u/%u",((NPC*)spawn)->GetMentalResistance(), ((NPC*)spawn)->GetMentalResistanceBase());
				client->Message(CHANNEL_COLOR_YELLOW, "Divine Resist/Base: %u/%u",((NPC*)spawn)->GetDivineResistance(), ((NPC*)spawn)->GetDivineResistanceBase());
				client->Message(CHANNEL_COLOR_YELLOW, "Disease Resist/Base: %u/%u",((NPC*)spawn)->GetDiseaseResistance(), ((NPC*)spawn)->GetDiseaseResistanceBase());
				client->Message(CHANNEL_COLOR_YELLOW, "Poison Resist/Base: %u/%u",((NPC*)spawn)->GetPoisonResistance(), ((NPC*)spawn)->GetPoisonResistanceBase());
				}
				
				string details;
				details += "\\#0000FFName:	" + string(spawn->GetName()) + "\n";
				details += "Type:	" + string(type) + "\n";
				details += "ID:	" + to_string(spawn->GetDatabaseID()) + "\n";
				details += "Last Name:	" + string(spawn->GetLastName()) + "\n";
				details += "Sub-Title:	" + string(spawn->GetSubTitle()) + "\n";
				details += "Prefix:	" + string(spawn->GetPrefixTitle()) + "\n";
				details += "Suffix:	" + string(spawn->GetSuffixTitle()) + "\n";
				details += "Race:		" + to_string(spawn->GetRace()) + "\n";
				details += "Class:		" + to_string(spawn->GetAdventureClass()) + "\n";
				details += "Gender:		" + to_string(spawn->GetGender()) + "\n";
				details += "Level:		" + to_string(spawn->GetLevel()) + "\n";
				details += "HP:		" + to_string(spawn->GetHP()) + " / " + to_string(spawn->GetTotalHP()) + "(" + to_string(spawn->GetIntHPRatio()) + "%)\n";
				details += "Power:		" + to_string(spawn->GetPower()) + + " / " + to_string(spawn->GetTotalPower()) + "\n";
				details += "Difficulty:		" + to_string(spawn->GetDifficulty()) + "\n";
				details += "Heroic:		" + to_string(spawn->GetHeroic()) + "\n";
				details += "Group ID:		" + to_string(spawn->GetSpawnGroupID()) + "\n";
				details += "Faction ID:		" + to_string(spawn->GetFactionID()) + "\n";
				details += "Merchant ID:	" + to_string(spawn->GetMerchantID()) + "\n";
				details += "Transport ID:	" + to_string(spawn->GetTransporterID()) + "\n";
				details += "Location ID:	" + to_string(spawn->GetSpawnLocationID()) + "\n";
				char x[16];
				sprintf(x, "%.2f", spawn->GetX());
				char y[16];
				sprintf(y, "%.2f", spawn->GetY());
				char z[16];
				sprintf(z, "%.2f", spawn->GetZ());
				details += "Location:	" + string(x) + ", " + string(y) + ", " + string(z) + "\n";

				string details2;
				details2 += "Heading:		" + to_string(spawn->GetHeading()) + "\n";
				details2 += "Grid ID:		" + to_string(spawn->GetLocation()) + "\n";
				details2 += "Size:		" + to_string(spawn->GetSize()) + "\n";
				details2 += "Collision Radius:	" + to_string(spawn->GetCollisionRadius()) + "\n";
				details2 += "Respawn Time:	" + to_string(spawn->GetRespawnTime()) + "\n";
				details2 += "Targetable:		" + to_string(spawn->GetTargetable()) + "\n";
				details2 += "Show Name:	" + to_string(spawn->GetShowName()) + "\n";
				details2 += "Attackable:		" + to_string(spawn->GetAttackable()) + "\n";
				details2 += "Show Level:		" + to_string(spawn->GetShowLevel()) + "\n";
				details2 += "Show Command Icon:	" + to_string(spawn->GetShowCommandIcon()) + "\n";
				details2 += "Display Hand Icon:	" + to_string(spawn->GetShowHandIcon()) + "\n";
				details2 += "Model Type:	" + to_string(spawn->GetModelType()) + "\n";
				details2 += "Soga Race Type:	" + to_string(spawn->GetSogaModelType()) + "\n";
				details2 += "Race Type:			" + to_string(race_type) + "\n";
				details2 += "Race Base Type:	" + to_string(race_base_type) + "\n";
				details2 += "Race Category (Sub Category): "	 + std::string(category) + " (" + std::string(subcategory) + ")\n";
				details2 += "Model Name:	"	  + std::string(modelname) + "\n";
				details2 += "Primary Command ID:	" + to_string(spawn->GetPrimaryCommandListID()) + "\n";
				details2 += "Secondary Cmd ID:	" + to_string(spawn->GetSecondaryCommandListID()) + "\n";
				details2 += "Visual State:		" + to_string(spawn->GetVisualState()) + "\n";
				details2 += "Action State:	" + to_string(spawn->GetActionState()) + "\n";
				details2 += "Mood State:		" + to_string(spawn->GetMoodState()) + "\n";
				details2 += "Initial State:		" + to_string(spawn->GetInitialState()) + "\n";
				details2 += "Activity Status:	" + to_string(spawn->GetActivityStatus()) + "\n";
				details2 += "Emote State:		" + to_string(spawn->GetEmoteState()) + "\n";

				string details3;
				details3 += "Pitch:	" + to_string(spawn->GetPitch()) + "\n";
				details3 += "Roll:	" + to_string(spawn->GetRoll()) + "\n";
				details3 += "Hide Hood:	" + to_string(spawn->appearance.hide_hood) + "\n";
				details3 += "Speed:	" + to_string(spawn->GetSpeed()) + "\n";
				details3 += "BaseSpeed:	" + to_string(spawn->GetBaseSpeed()) + "\n";

				if(spawn->IsSign()) {
					details3 += "Sign Language: " + to_string(((Sign*)spawn)->GetLanguage()) + "\n";
				}
				
				if(spawn->IsEntity())
				{
					Entity* ent = (Entity*)spawn;
					details3 += "STR / STRBase:	" + to_string(ent->GetInfoStruct()->get_str()) + " / " + to_string(ent->GetInfoStruct()->get_str_base()) + "\n";
					details3 += "AGI / AGIBase:	" + to_string(ent->GetInfoStruct()->get_agi()) + " / " + to_string(ent->GetInfoStruct()->get_agi_base()) + "\n";
					details3 += "STA / STABase:	" + to_string(ent->GetInfoStruct()->get_sta()) + " / " + to_string(ent->GetInfoStruct()->get_sta_base()) + "\n";
					details3 += "INT / INTBase:	" + to_string(ent->GetInfoStruct()->get_intel()) + " / " + to_string(ent->GetInfoStruct()->get_intel_base()) + "\n";
					details3 += "WIS / WISBase:	" + to_string(ent->GetInfoStruct()->get_wis()) + " / " + to_string(ent->GetInfoStruct()->get_wis_base()) + "\n";
				}

				string details4;
				if (spawn->IsEntity()) {
					details4 += "Facial Hair Type:	" + to_string(((Entity*)spawn)->GetFacialHairType()) + "\n";
					details4 += "Hair Type:		" + to_string(((Entity*)spawn)->GetHairType()) + "\n";
					details4 += "Chest Type:		" + to_string(((Entity*)spawn)->GetChestType()) + "\n";
					details4 += "Legs Type:		" + to_string(((Entity*)spawn)->GetLegsType()) + "\n";
					details4 += "Soga Facial Hair Type:	" + to_string(((Entity*)spawn)->GetSogaFacialHairType()) + "\n";
					details4 += "Soga Hair Type:	" + to_string(((Entity*)spawn)->GetSogaHairType()) + "\n";
					details4 += "Wing Type:		" + to_string(((Entity*)spawn)->GetWingType()) + "\n";
					if (spawn->IsNPC()) {
						details4 += "\nRandomize:		" + to_string(((NPC*)spawn)->GetRandomize()) + "\n";
					}
				}
				const char* spawnScriptMsg = (spawn->GetSpawnScript() && strlen(spawn->GetSpawnScript())>0) ? spawn->GetSpawnScript() : "Not Set";
				details4 += "\nSpawnScript:		" + std::string(spawnScriptMsg) + "\n";

				string title = string(spawn->GetName()) + "(" + to_string(spawn->GetDatabaseID()) + ")";
				client->SendShowBook(client->GetPlayer(), title, 0, 4, details, details2, details3, details4);
			}
			else {
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Syntax: /spawn details (radius)");
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "This will display information about the currently selected spawn, or in the case of specifying a numerical radius will show minor details about spawns in the players radius.");
			}
			break;
								   }
		case COMMAND_SPAWN_TARGET:{
			if(sep && sep->arg[0][0] && sep->IsNumber(0)){
				int32 spawn_id = atoul(sep->arg[0]);
				int16 response = client->GetCurrentZone()->SetSpawnTargetable(spawn_id);
				client->Message(CHANNEL_COLOR_YELLOW, "%i spawn(s) in the current zone were reset to targetable.", response);
			}
			else if(sep && sep->arg[0][0] && sep->arg[1][0] && sep->IsNumber(1) && ToLower(string(sep->arg[0])) == "radius"){
				float distance = atof(sep->arg[1]);
				int16 response = client->GetCurrentZone()->SetSpawnTargetable(client->GetPlayer(), distance);
				client->Message(CHANNEL_COLOR_YELLOW, "%i spawn(s) in the current zone were reset to targetable.", response);
			}
			else{
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Syntax: /spawn target [spawn id]");
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "This will set the given spawn as targetable.  Used to change a spawn if it was set to untargetable.");
			}
			break;
								  }
		case COMMAND_SPAWN_SET:
			{
			Spawn* spawn = cmdTarget;
			sint16 set_type = -1; 
			string type_str;

			if (spawn)
			{
				// check if parameters are (location or list or not player and not a 2nd param), and that there is at least 1 value
				if(sep && ((sep->arg[0][0] && ToLower(string(sep->arg[0])) == "location") || (sep->arg[0][0] && ToLower(string(sep->arg[0])) == "list") || (spawn && spawn->IsPlayer() == false && sep->arg[1][0])) && spawn_set_values.count(ToLower(string(sep->arg[0]))) == 1)
				{
					// set the type, which will be 0 if location or list or invalid
					set_type = spawn_set_values[ToLower(string(sep->arg[0]))];
				}

				if(set_type > 0)
				{
					// check if spawn set is NOT a char update, or not location, or isn't a number
					if(!(set_type >= SPAWN_SET_VALUE_PREFIX) && !(set_type <= SPAWN_SET_VALUE_MERCHANT_MAX_LEVEL) && set_type != SPAWN_SET_VALUE_NAME && ((set_type < SPAWN_SET_VALUE_SPAWN_SCRIPT) || (set_type > SPAWN_SET_VALUE_SUB_TITLE)) && set_type != SPAWN_SET_VALUE_LOCATION && sep->IsNumber(1) == false)
					{
						client->SimpleMessage(CHANNEL_COLOR_RED, "Invalid value for set command.");
					}
					else
					{
						string name = string(spawn->GetName());
						bool customSetSpawn = false;
						if(set_type >= SPAWN_SET_CHEEK_TYPE && set_type <= SPAWN_SET_SOGA_NOSE_TYPE)
						{
							int8 index = sep->IsNumber(2) ? atoul(sep->arg[2]) : 0;

							// override the standard setspawncommand, we pass arguments different!
							if(SetSpawnCommand(client, spawn, set_type, sep->arg[1], true, false, nullptr, index))
								customSetSpawn = true;
						}

						if(customSetSpawn || SetSpawnCommand(client, spawn, set_type, sep->argplus[1]))
						{
							if (set_type == SPAWN_SET_VALUE_EXPANSION_FLAG || set_type == SPAWN_SET_VALUE_HOLIDAY_FLAG)
							{
								client->SimpleMessage(CHANNEL_COLOR_YELLOW, "A /reload spawns is required to properly update the spawns with the xpack/holiday flag.");
							}
							else if (set_type == SPAWN_SET_VALUE_NAME)
							{
								client->SimpleMessage(CHANNEL_COLOR_YELLOW, "New name will not be effective until zone reload.");
							}
							else if (set_type == SPAWN_SET_SKIN_COLOR || (set_type >= SPAWN_SET_HAIR_COLOR1 && set_type <= SPAWN_SET_SOGA_EYE_COLOR))
							{
								client->Message(CHANNEL_COLOR_YELLOW, "Successfully set color field to R G B: %s.", sep->argplus[1]);
							}
							else if(set_type == SPAWN_SET_VALUE_LOCATION)
							{
								spawn->SetLocation(client->GetPlayer()->GetLocation());
								client->Message(CHANNEL_COLOR_YELLOW, "Successfully set '%s' to '%u' for spawn '%s' (DBID: %u)", sep->arg[0], client->GetPlayer()->GetLocation(), name.c_str(), spawn->GetDatabaseID());
							}
							else
							{
								client->Message(CHANNEL_COLOR_YELLOW, "Successfully set '%s' to '%s' for spawn '%s' (DBID: %u)", sep->arg[0], sep->arg[1], name.c_str(), spawn->GetDatabaseID());
							}

							switch (set_type)
							{
								case SPAWN_SET_VALUE_EXPANSION_FLAG:
								case SPAWN_SET_VALUE_HOLIDAY_FLAG:
								case SPAWN_SET_VALUE_FACTION:
								case SPAWN_SET_AAXP_REWARDS:
								case SPAWN_SET_CHEEK_TYPE:
								case SPAWN_SET_CHIN_TYPE:
								case SPAWN_SET_EAR_TYPE:
								case SPAWN_SET_EYE_BROW_TYPE:
								case SPAWN_SET_EYE_TYPE:
								case SPAWN_SET_LIP_TYPE:
								case SPAWN_SET_NOSE_TYPE:
								case SPAWN_SET_BODY_SIZE:
								case SPAWN_SET_BODY_AGE:
								case SPAWN_SET_SOGA_CHEEK_TYPE:
								case SPAWN_SET_SOGA_CHIN_TYPE:
								case SPAWN_SET_SOGA_EAR_TYPE:
								case SPAWN_SET_SOGA_EYE_BROW_TYPE:
								case SPAWN_SET_SOGA_EYE_TYPE:
								case SPAWN_SET_SOGA_LIP_TYPE:
								case SPAWN_SET_SOGA_NOSE_TYPE:
								case SPAWN_SET_SOGA_BODY_SIZE:
								case SPAWN_SET_SOGA_BODY_AGE:
								case SPAWN_SET_ATTACK_TYPE:
								case SPAWN_SET_RACE_TYPE:
								case SPAWN_SET_LOOT_TIER:
								case SPAWN_SET_LOOT_DROP_TYPE:
								case SPAWN_SET_SCARED_STRONG_PLAYERS:
								{
									// not applicable already ran db command
									break;
								}
								default:
								{
									client->GetCurrentZone()->ApplySetSpawnCommand(client, spawn, set_type, sep->argplus[1]);
									break;
								}
							}
						
							if((set_type >= SPAWN_SET_VALUE_RESPAWN && set_type <=SPAWN_SET_VALUE_LOCATION) || (set_type >= SPAWN_SET_VALUE_EXPIRE && set_type <=SPAWN_SET_VALUE_Z_OFFSET) || (set_type == SPAWN_SET_VALUE_PITCH || set_type == SPAWN_SET_VALUE_ROLL))
							{
								if(spawn->GetSpawnLocationID() > 0 && database.UpdateSpawnLocationSpawns(spawn))
								{
									client->Message(CHANNEL_COLOR_YELLOW, "Successfully saved spawn information for spawn '%s' (DBID: %u)", name.c_str(), spawn->GetDatabaseID());
								}
								else if(spawn->GetSpawnLocationID() > 0)
								{
									client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Error saving spawn information, see console window for details.");				
								}
							}
							else
							{
								if(spawn->GetDatabaseID() > 0 && database.SaveSpawnInfo(spawn))
								{
									client->Message(CHANNEL_COLOR_YELLOW, "Successfully saved spawn for spawn '%s' (DBID: %u)", name.c_str(), spawn->GetDatabaseID());
								}
								else if(spawn->GetDatabaseID() > 0)
								{
									client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Error saving spawn, see console window for details.");
								}
							}
						}
					}
				}
				else if(set_type == 0)
				{
					// /spawn set list - lists all possible attributes that can be changed with this command, 10 per line.
					map<string, int8>::iterator itr;
					int i=0;
					string list;
					for(itr = spawn_set_values.begin(); itr != spawn_set_values.end(); itr++, i++)
					{
						if(i==10)
						{
							client->SimpleMessage(CHANNEL_COLOR_YELLOW, list.c_str());
							i = 0;
						}

						if(i>0)
						{
							list.append(", ").append(itr->first);
						}
						else
						{
							list = itr->first;
						}
					}

					if(list.length() > 0)
					{
						// if 1 or more, display in client.
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, list.c_str());
					}
				}
				else
				{
					// syntax fail
					client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Syntax: /spawn set [type] [value]");
					client->SimpleMessage(CHANNEL_COLOR_YELLOW, "This command is used to change various settings for the targeted NPC or Object.");
					client->SimpleMessage(CHANNEL_COLOR_YELLOW, "For a list of changeable settings use /spawn set list");
					client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Note: /spawn set location does not require a value. The client's current location is used.");
				}
			}
			break;
			}
		case COMMAND_SPAWN_REMOVE:{
			Spawn* spawn = cmdTarget;
			if(spawn && !spawn->IsPlayer()){
				if(spawn->GetSpawnLocationID() > 0){
					string name = string(spawn->GetName());
					int32 dbid = spawn->GetDatabaseID();
					if(database.RemoveSpawnFromSpawnLocation(spawn)){
						client->GetCurrentZone()->RemoveSpawn(spawn, true, false, true, true, true);
						client->Message(CHANNEL_COLOR_YELLOW, "Successfully removed spawn from zone for spawn '%s' (DBID: %u)", name.c_str(), dbid);
					}
					else
						client->SimpleMessage(CHANNEL_COLOR_RED, "Error removing spawn, see console window for details.");
				}
				else
					client->SimpleMessage(CHANNEL_COLOR_RED, "This spawn does not have a spawn group associated with it");
			}
			else{
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Syntax: /spawn remove");
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "This command is used for removing the targeted NPC or Object from the zone.");
			}
			break;
								  }
		case COMMAND_SPAWN_LIST:{
			if(sep && sep->arg[0][0]){
				vector<string>* results = database.GetSpawnNameList(sep->argplus[0]);
				vector<string>::iterator itr;
				if(results){
					for(itr=results->begin();itr!=results->end();itr++){
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, (*itr).c_str());
					}
					safe_delete(results);
				}
				else
					client->SimpleMessage(CHANNEL_COLOR_YELLOW, "No matches found. ");
			}
			else{
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Syntax: /spawn list [name]");
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Name can be a partial match.");
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Ex: /spawn list Qeynos Guard");
			}
			break;
								}
		case COMMAND_SPAWN_ADD:{
			Spawn* spawn = cmdTarget;
			if(spawn && sep && ((sep->arg[1][0] && sep->IsNumber(0)) || (sep->arg[0][0] && strncasecmp(sep->arg[0], "new", 3) == 0))){
				if(spawn->GetSpawnLocationID() > 0){
					client->Message(CHANNEL_COLOR_RED, "This spawn already has a spawn group id of %u, use /spawn remove to reassign it", spawn->GetSpawnLocationID());
					break;
				}
				if(spawn->GetDatabaseID() == 0){
					if(database.SaveSpawnInfo(spawn)) {
						char spawn_type[32];
						memset(spawn_type, 0, sizeof(spawn_type));
						if (spawn->IsNPC())
							strncpy(spawn_type, "NPC", sizeof(spawn_type) - 1);
						else if (spawn->IsObject())
							strncpy(spawn_type, "Object", sizeof(spawn_type) - 1);
						else if (spawn->IsSign())
							strncpy(spawn_type, "Sign", sizeof(spawn_type) - 1);
						else if (spawn->IsGroundSpawn())
							strncpy(spawn_type, "GroundSpawn", sizeof(spawn_type) - 1);
						else
							strncpy(spawn_type, "Unknown", sizeof(spawn_type) - 1);
						client->Message(CHANNEL_COLOR_YELLOW, "Successfully saved spawn information with a %s id of %u", spawn_type, spawn->GetDatabaseID());
					}
					else
						client->SimpleMessage(CHANNEL_COLOR_RED, "Error saving spawn information, see console window for details.");
				}
				int32 spawn_group_id = 0;
				if(strncasecmp(sep->arg[0], "new", 3) == 0)
					spawn_group_id = database.GetNextSpawnLocation();
				else
					spawn_group_id = atol(sep->arg[0]);
				int8 percent = 100;
				if(sep->arg[2] && sep->IsNumber(2))
					percent = atoi(sep->arg[2]);
				spawn->SetSpawnLocationID(spawn_group_id);
				float x_offset = database.GetSpawnLocationPlacementOffsetX(spawn->GetSpawnLocationID());
				float y_offset = database.GetSpawnLocationPlacementOffsetY(spawn->GetSpawnLocationID());
				float z_offset = database.GetSpawnLocationPlacementOffsetZ(spawn->GetSpawnLocationID());
				if(database.SaveSpawnEntry(spawn, sep->arg[1], percent, x_offset, y_offset, z_offset))
					client->Message(CHANNEL_COLOR_YELLOW, "Successfully saved spawn location with a spawn group of %u", spawn->GetSpawnLocationID());
				else
					client->SimpleMessage(CHANNEL_COLOR_RED, "Error saving spawn location, see console window for details.");	
			}
			else{
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Syntax: /spawn add [spawn group id] [spawn group name] (percentage)");
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "This command is used for adding the targeted NPC or Object to the database.");
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You can substitute new for [spawn group id] to create a new one.");
			}
			break;
							   }
		case COMMAND_SPAWN:{
			int32 id = 0;
			Spawn* spawn = 0;
			if(sep && sep->arg[0] && sep->IsNumber(0)){
				id = atol(sep->arg[0]);
				spawn = client->GetCurrentZone()->GetSpawn(id);
			}
			if(id > 0 && spawn && spawn->appearance.name[0] == 0)
				id = 0;
			if(!id || !spawn){
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Syntax: /spawn [spawn id] (x) (y) (z) (heading) (location)");
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "All parameters are optional except the id.  The spawn id must be in the database.");
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Ex: /spawn 1 0 0 0 0");
				safe_delete(sep);
				return;
			}
			if(sep && sep->arg[1][0]){
				float x = atof(sep->arg[1]);
				spawn->appearance.pos.X = x;
			}
			else
				spawn->SetX(client->GetPlayer()->GetX(), false);
			if(sep && sep->arg[2][0]){
				float y = atof(sep->arg[2]);
				spawn->appearance.pos.Y = y;
			}
			else
				spawn->SetY(client->GetPlayer()->GetY(), false);
			if(sep && sep->arg[3][0]){
				float z = atof(sep->arg[3]);
				spawn->appearance.pos.Z = z;
			}
			else
				spawn->SetZ(client->GetPlayer()->GetZ(), false);
			if(sep && sep->arg[4][0]){
				float heading = atof(sep->arg[4]);
				spawn->SetHeading(heading);
			}
			else
				spawn->SetHeading(client->GetPlayer()->GetHeading(), false);
			spawn->SetSpawnOrigX(spawn->GetX());
			spawn->SetSpawnOrigY(spawn->GetY());
			spawn->SetSpawnOrigZ(spawn->GetZ());
			spawn->SetSpawnOrigHeading(spawn->GetHeading());
			if(sep && sep->arg[5][0])
				spawn->SetLocation(atoul(sep->arg[5]));
			else
				spawn->SetLocation(client->GetPlayer()->GetLocation());

			if(spawn->IsNPC() && spawn->GetTotalHP() == 0){
				spawn->SetTotalHP(spawn->GetLevel() * 15);
				spawn->SetHP(spawn->GetTotalHP());
			}
			if(spawn->GetTotalPower() == 0){
				spawn->SetTotalPower(spawn->GetLevel() * 15);
				spawn->SetPower(spawn->GetTotalPower());
			}
			const char* script = world.GetSpawnScript(id);
			if(script && lua_interface && lua_interface->GetSpawnScript(script) != 0)
				spawn->SetSpawnScript(string(script));

			spawn->GetZone()->CallSpawnScript(spawn, SPAWN_SCRIPT_PRESPAWN);

			client->GetCurrentZone()->AddSpawn(spawn);
			if(spawn->IsNPC())
				spawn->GetZone()->AddLoot((NPC*)spawn);
			spawn->GetZone()->CallSpawnScript(spawn, SPAWN_SCRIPT_SPAWN);
			LogWrite(COMMAND__INFO, 0, "Command", "Received spawn command - Parms: %s", command_parms->data.c_str());
			break;
						   }
		case COMMAND_ADMINFLAG:
			{
				if(sep && sep->arg[0]){
					sint16 tmp_status = database.GetCharacterAdminStatus(sep->arg[0]);
					sint16 new_status = atoi(sep->arg[1]);
					if(tmp_status == -10)
						client->SimpleMessage(CHANNEL_ERROR,"Unable to flag character.  Reason: Character does not exist.");
					else if(tmp_status >= client->GetAdminStatus())
						client->SimpleMessage(CHANNEL_ERROR,"Unable to flag character.  Reason: Character has same or higher level status.");
					else if (new_status > client->GetAdminStatus())
						client->SimpleMessage(CHANNEL_ERROR, "Unable to flag character.  Reason: New status is higher then your status.");
					else{
						Client* client2 = client->GetCurrentZone()->GetClientByName(sep->arg[0]);
						if (!client2)
							client2 = zone_list.GetClientByCharName(sep->arg[0]);
						
						if(database.UpdateAdminStatus(sep->arg[0],new_status)) {
							client->Message(CHANNEL_COLOR_YELLOW,"Character status updated to %i for %s.",new_status,sep->arg[0]);
							if (client2) {
								client2->SetAdminStatus(new_status);
								client2->Message(CHANNEL_COLOR_YELLOW, "%s has set your admin status to %i.", client->GetPlayer()->GetName(), new_status);
							}
						}
						else
							client->SimpleMessage(CHANNEL_ERROR,"Unable to flag character.  Unknown reason.");
					}
				}else{
					sint16 status = database.GetCharacterAdminStatus(client->GetPlayer()->GetName());
					if(status != client->GetAdminStatus())
					{
						client->Message(CHANNEL_COLOR_YELLOW,"Flag status was changed from %i to %i.",status,client->GetAdminStatus());
						client->SetAdminStatus(status);
					}
					else
					{
						client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Usage: /flag {name} {new_status}");
						client->SimpleMessage(CHANNEL_COLOR_YELLOW," Standard User: 0");
						client->Message(CHANNEL_COLOR_YELLOW," Admin User: %i", status);
					}
				}
				break;
			}
		case COMMAND_CANNEDEMOTE:{
				client->GetCurrentZone()->HandleEmote(client->GetPlayer(), command_parms->data);
				break;
		}
		case COMMAND_BROADCAST: {
			if (sep && sep->arg[0])
				zone_list.TransmitBroadcast(sep->argplus[0]);
			else
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /broadcast {message}");
			break;
								}
		case COMMAND_ANNOUNCE: {
			if (sep && sep->arg[0])
				zone_list.TransmitGlobalAnnouncement(sep->argplus[0]);
			else
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /announce {message}");
			break;
							   }
		case COMMAND_RELOAD_ITEMS:{
			LogWrite(COMMAND__INFO, 0, "Command", "Reloading items..");
			
			int32 item_id = (sep && sep->arg[0]) ? atoul(sep->arg[0]) : 0;
			if(item_id > 0) {
				client->Message(CHANNEL_COLOR_YELLOW, "Reloading item %u based on /reload items [item_id].", item_id);
			}
			else {
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Started Reloading items (this might take a few minutes...)");
			}
			
			database.ReloadItemList(item_id);
			
			if(!item_id) {
				database.LoadMerchantInformation(); // we skip if there is only a reload of single item not all items
			}
			
			peer_manager.sendPeersMessage("/reloadcommand", command->handler, item_id);
			
			if(item_id > 0) {
				client->Message(CHANNEL_COLOR_YELLOW, "Reloaded item %u.", item_id);
			}
			else {
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Finished Reloading items.");
			}
			break;
								  }
		case COMMAND_ENABLE_ABILITY_QUE:{
			EQ2Packet* app = client->GetPlayer()->GetSpellBookUpdatePacket(client->GetVersion());
			if(app)
				client->QueuePacket(app);
			break;
										}
		case COMMAND_ITEMSEARCH:
		case COMMAND_FROMBROKER:{
			
				if(command->handler == COMMAND_ITEMSEARCH) {
					client->SetGMStoreSearch(true);
				}
				else {
					client->SetGMStoreSearch(false);
				}
				PacketStruct* packet = configReader.getStruct("WS_StartBroker", client->GetVersion());
				if (packet) {
					packet->setDataByName("spawn_id", client->GetPlayer()->GetIDWithPlayerSpawn(client->GetPlayer()));
					//packet->setDataByName("unknown", 1);
					packet->setDataByName("unknown2", 5, 0);
					packet->setDataByName("unknown2", 20, 1);
					packet->setDataByName("unknown2", 58, 3);
					packet->setDataByName("unknown2", 40, 4);
					client->QueuePacket(packet->serialize());
					if(client->GetVersion() > 561) {
						PacketStruct* packet2 = configReader.getStruct("WS_BrokerBags", client->GetVersion());
						if (packet2) {
							packet2->setDataByName("char_id", client->GetCharacterID());
							client->QueuePacket(packet2->serialize()); //send this for now, needed to properly clear data
							safe_delete(packet2);
						}
						safe_delete(packet);
					}
				}
			break;
		}
		case COMMAND_ANIMTEST:{
			PacketStruct* command_packet = configReader.getStruct("WS_CannedEmote", client->GetVersion());
			if(command_packet){
				int32 id = client->GetPlayer()->GetIDWithPlayerSpawn(client->GetPlayer());
				if (cmdTarget)
					id = client->GetPlayer()->GetIDWithPlayerSpawn(cmdTarget);
				command_packet->setDataByName ( "spawn_id" , id);

				int animID = 1;

				if(sep && sep->arg[0] && sep->IsNumber(0))
					animID = atoi(sep->arg[0]);

				VisualState* vs = NULL;
				if(animID == 0)
				{
					vs = visual_states.FindVisualState(sep->arg[0]);
				}


				char msg[128];
				sprintf(msg,"Animation Test ID: %i",animID);
				command_packet->setMediumStringByName ( "emote_msg" , msg );

				if(vs != NULL)
					command_packet->setDataByName ( "anim_type", vs->GetID ( ) );
				else
					command_packet->setDataByName ( "anim_type", animID );

				command_packet->setDataByName ( "unknown0", 0 );
				EQ2Packet* outapp = command_packet->serialize();
				client->QueuePacket(outapp);
				safe_delete(command_packet);
			}
			break;
		}
		case COMMAND_KICK:
			{
				if( sep == 0 || sep->arg[0] == 0)
				{
					client->SimpleMessage(CHANNEL_COLOR_RED, "/kick [name]");
				}
				else
				{
					Client* kickClient = zone_list.GetClientByCharName(string(sep->arg[0]));

					if ( kickClient == client )
					{
						client->Message(CHANNEL_COLOR_RED, "You can't kick yourself!");
						break;
					}
					else if(kickClient != NULL)
					{
						sint16 maxStatus = database.GetHighestCharacterAdminStatus(kickClient->GetAccountID());

						if ( maxStatus >= client->GetAdminStatus( ) || kickClient->GetAdminStatus() >= client->GetAdminStatus() )
						{
							client->Message(CHANNEL_COLOR_RED,"Don't even think about it...");
							break;
						}

						client->Message(CHANNEL_COLOR_RED, "Kicking %s...",sep->arg[0]);

						kickClient->Disconnect();
					}
					else
					{
						client->Message(CHANNEL_COLOR_RED, "Could not find %s.",sep->arg[0]);
					}
				}

				break;
			}
		case COMMAND_LOCK:
			{
				if( sep != NULL && sep->arg[0] != NULL && sep->IsNumber(0)){
					int worldLocked = atoi(sep->arg[0]);
					net.world_locked = worldLocked;
					if ( worldLocked )
						client->Message(CHANNEL_COLOR_YELLOW,"World server has been locked.");
					else
						client->Message(CHANNEL_COLOR_YELLOW,"World server has been unlocked.");
				}
				else
					client->SimpleMessage(CHANNEL_COLOR_RED, "/lock [0/1]");

				break;
		}
		case COMMAND_BAN:{
				if( sep == 0 || sep->arg[0] == 0 || (sep->arg[1][0] != 0 && !sep->IsNumber(1) ) )
				{
					client->SimpleMessage(CHANNEL_COLOR_RED, "/ban [name] [permanent:0/1]");
				}
				else
				{
					Client* kickClient = zone_list.GetClientByCharName(sep->arg[0]);

					if ( kickClient == client )
					{
						client->Message(CHANNEL_COLOR_RED, "You can't ban yourself!");
						break;
					}
					else if(kickClient != NULL)
					{
						sint16 maxStatus = database.GetHighestCharacterAdminStatus(kickClient->GetAccountID());

						if ( maxStatus > client->GetAdminStatus( ) || 
							client->GetAdminStatus ( ) > kickClient->GetAdminStatus ( ) )
						{
							client->Message(CHANNEL_COLOR_RED,"Don't even think about it...");
							break;
						}

						client->Message(CHANNEL_COLOR_RED, "Kicking & Banning %s...",sep->arg[0]);

						int perm = atol(sep->arg[1]);
						if ( perm == 1 )
							database.UpdateAdminStatus(sep->arg[0],-2);
						else
							database.UpdateAdminStatus(sep->arg[0],-1);
						kickClient->Disconnect();
					}
					else
					{
						client->Message(CHANNEL_COLOR_RED, "Could not find %s.",sep->arg[0]);
					}
				}
				break;
		}
		case COMMAND_SET_COMBAT_VOICE:{
			int32 value = 0;
			if(sep && sep->arg[0] && sep->IsNumber(0))
				value = atoi(sep->arg[0]);
			client->GetPlayer()->SetCombatVoice(value);
			break;
		}
		case COMMAND_SET_EMOTE_VOICE:{
			int32 value = 0;
			if(sep && sep->arg[0] && sep->IsNumber(0))
				value = atoi(sep->arg[0]);
			client->GetPlayer()->SetEmoteVoice(value);
			break;
		}
		case COMMAND_GIVEITEM:{

			if(sep && sep->arg[0][0] && sep->arg[0][1] && sep->IsNumber(1)){
				Client* itemToClient = zone_list.GetClientByCharName(sep->arg[0]);

				if ( itemToClient == NULL )
					client->Message(CHANNEL_COLOR_YELLOW,"Could not find %s.",sep->arg[0]);
				else
				{
					int32 item_id = atol(sep->arg[1]);
					client->Message(CHANNEL_COLOR_YELLOW,"Gave %s item id %i.",sep->arg[0],item_id);
					itemToClient->AddItem(item_id);
				}
			}
			else
				client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Usage: /giveitem [name] [item_id]");

			break;
							  }

		case COMMAND_REPORT_BUG			: { Command_ReportBug(client, sep); break; }
		case COMMAND_INVENTORY			: { Command_Inventory(client, sep, command); break; }
		case COMMAND_WEAPONSTATS		: { Command_WeaponStats(client); break; }
		case COMMAND_SKILL				: 
		case COMMAND_SKILL_ADD			:
		case COMMAND_SKILL_REMOVE		:
		case COMMAND_SKILL_LIST			: { Command_Skills(client, sep, command->handler); break; }
		case COMMAND_ZONE_SET			: { Command_ZoneSet(client, sep); break; }
		case COMMAND_ZONE_DETAILS		: { Command_ZoneDetails(client, sep); break; }
		case COMMAND_ENTITYCOMMAND		: 
		case COMMAND_ENTITYCOMMAND_LIST	: { Command_EntityCommand(client, sep, command->handler); break; }
		case COMMAND_MERCHANT			: 
		case COMMAND_MERCHANT_LIST		: { Command_Merchant(client, sep, command->handler); break; }
		case COMMAND_APPEARANCE			: 
		case COMMAND_APPEARANCE_LIST	: { Command_Appearance(client, sep, command->handler); break; }
		case COMMAND_TRACK				: { Command_Track(client); break; }
		case COMMAND_DISTANCE			: { Command_Distance(client); break; }
		case COMMAND_INSPECT_PLAYER		: { Command_InspectPlayer(client, sep); break; }
		case COMMAND_ZONE_SAFE			: { Command_ZoneSafeCoords(client, sep); break; }
		case COMMAND_GUILDSAY			: { Command_GuildSay(client, sep); break; }
		case COMMAND_OFFICERSAY			: { Command_OfficerSay(client, sep); break; }
		case COMMAND_SET_GUILD_MEMBER_NOTE	: { Command_SetGuildMemberNote(client, sep); break; }
		case COMMAND_SET_GUILD_OFFICER_NOTE	: { Command_SetGuildOfficerNote(client, sep); break; }
		case COMMAND_GUILD				: { Command_Guild(client, sep); break; }
		case COMMAND_CREATE_GUILD		: { Command_CreateGuild(client, sep); break; }
		case COMMAND_GUILDS				: { Command_Guilds(client); break; }
		case COMMAND_GUILDS_ADD			: { Command_GuildsAdd(client, sep); break; }
		case COMMAND_GUILDS_CREATE		: { Command_GuildsCreate(client, sep); break; }
		case COMMAND_GUILDS_DELETE		: { Command_GuildsDelete(client, sep); break; }
		case COMMAND_GUILDS_LIST		: { Command_GuildsList(client); break; }
		case COMMAND_GUILDS_REMOVE		: { Command_GuildsRemove(client, sep); break; }
		case COMMAND_CLAIM				: { Command_Claim(client, sep); break; }
		case COMMAND_CLEAR_ALL_QUEUED	: { Command_ClearAllQueued(client); break; }
		case COMMAND_LOCATION			: { Command_Location(client); break; }
		case COMMAND_LOCATION_ADD		: { Command_LocationAdd(client, sep); break; }
		case COMMAND_LOCATION_CREATE	: { Command_LocationCreate(client, sep); break; }
		case COMMAND_LOCATION_DELETE	: { Command_LocationDelete(client, sep); break; }
		case COMMAND_LOCATION_LIST		: { Command_LocationList(client, sep); break; }
		case COMMAND_LOCATION_REMOVE	: { Command_LocationRemove(client, sep); break; }
		case COMMAND_GRID				: { Command_Grid(client, sep); break; }
		case COMMAND_TRY_ON				: { Command_TryOn(client, sep); break; }
		case COMMAND_RANDOMIZE			: { Command_Randomize(client, sep); break; }
		case COMMAND_AFK				: { Command_AFK(client, sep); break; }
		case COMMAND_SHOW_CLOAK			: { Command_ShowCloak(client, sep); break; }
		case COMMAND_SHOW_HELM			: { Command_ShowHelm(client, sep); break; }
		case COMMAND_SHOW_HOOD			: { Command_ShowHood(client, sep); break; }
		case COMMAND_SHOW_HOOD_OR_HELM	: { Command_ShowHoodHelm(client, sep); break; }
		case COMMAND_SHOW_RANGED		: { Command_ShowRanged(client, sep); break; }
		case COMMAND_STOP_DRINKING		: { Command_StopDrinking(client); break; }
		case COMMAND_STOP_EATING		: { Command_StopEating(client); break; }
		case COMMAND_TOGGLE_ANONYMOUS	: { Command_Toggle_Anonymous(client); break; }
		case COMMAND_TOGGLE_AUTOCONSUME	: { Command_Toggle_AutoConsume(client, sep); break; }
		case COMMAND_TOGGLE_BONUS_EXP	: { Command_Toggle_BonusXP(client); break; }
		case COMMAND_TOGGLE_COMBAT_EXP	: { Command_Toggle_CombatXP(client); break; }
		case COMMAND_TOGGLE_GM_HIDE		: { Command_Toggle_GMHide(client); break; }
		case COMMAND_TOGGLE_GM_VANISH	: { Command_Toggle_GMVanish(client); break; }
		case COMMAND_TOGGLE_ILLUSIONS	: { Command_Toggle_Illusions(client, sep); break; }
		case COMMAND_TOGGLE_LFG			: { Command_Toggle_LFG(client); break; }
		case COMMAND_TOGGLE_LFW			: { Command_Toggle_LFW(client); break; }
		case COMMAND_TOGGLE_QUEST_EXP	: { Command_Toggle_QuestXP(client); break; }
		case COMMAND_TOGGLE_ROLEPLAYING	: { Command_Toggle_Roleplaying(client); break; }
		case COMMAND_TOGGLE_DUELS		: { Command_Toggle_Duels(client); break; }
		case COMMAND_TOGGLE_TRADES		: { Command_Toggle_Trades(client); break; }
		case COMMAND_TOGGLE_GUILDS		: { Command_Toggle_Guilds(client); break; }
		case COMMAND_TOGGLE_GROUPS		: { Command_Toggle_Groups(client); break; }
		case COMMAND_TOGGLE_RAIDS		: { Command_Toggle_Raids(client); break; }
		case COMMAND_TOGGLE_LON			: { Command_Toggle_LON(client); break; }
		case COMMAND_TOGGLE_VCINVITE	: { Command_Toggle_VoiceChat(client); break; }
		case COMMAND_CANCEL_MAINTAINED	: { Command_CancelMaintained(client, sep); break; }
		case COMMAND_MOTD				: { Command_MOTD(client); break; }
		case COMMAND_RANDOM				: { Command_Random(client, sep); break; }
		case COMMAND_CREATE				: { Command_Create(client, sep); break; }
		case COMMAND_CREATEFROMRECIPE	: { Command_CreateFromRecipe(client, sep); break; }
		case COMMAND_TITLE				: { Command_Title(client); break; }
		case COMMAND_TITLE_LIST			: { Command_TitleList(client); break; }
		case COMMAND_TITLE_SETPREFIX	: { Command_TitleSetPrefix(client, sep); break; }
		case COMMAND_TITLE_SETSUFFIX	: { Command_TitleSetSuffix(client, sep); break; }
		case COMMAND_TITLE_FIX			: { Command_TitleFix(client, sep); break; }
		case COMMAND_LANGUAGES			: { Command_Languages(client, sep); break; }
		case COMMAND_SET_LANGUAGE		: { Command_SetLanguage(client, sep); break; }
		case COMMAND_FOLLOW				: { Command_Follow(client, sep); break; }
		case COMMAND_STOP_FOLLOW		: { Command_StopFollow(client, sep); break; }
		case COMMAND_LASTNAME			: { Command_LastName(client, sep); break; }
		case COMMAND_CONFIRMLASTNAME	: { Command_ConfirmLastName(client, sep); break; }
		case COMMAND_PET				: { Command_Pet(client, sep); break; }
		case COMMAND_PETNAME			: { Command_PetName(client, sep); break; }
		case COMMAND_NAME_PET			: { Command_NamePet(client, sep); break; }
		case COMMAND_RENAME				: { Command_Rename(client, sep); break; }
		case COMMAND_CONFIRMRENAME		: { Command_ConfirmRename(client, sep); break; }
		case COMMAND_PETOPTIONS			: { Command_PetOptions(client, sep); break; }
		case COMMAND_START_TRADE		: { Command_TradeStart(client, sep); break; }
		case COMMAND_ACCEPT_TRADE		: { Command_TradeAccept(client, sep); break; }
		case COMMAND_REJECT_TRADE		: { Command_TradeReject(client, sep); break; }
		case COMMAND_CANCEL_TRADE		: { Command_TradeCancel(client, sep); break; }
		case COMMAND_SET_TRADE_COIN		: { Command_TradeSetCoin(client, sep); break; }
		case COMMAND_ADD_TRADE_COPPER	: { Command_TradeAddCoin(client, sep, COMMAND_ADD_TRADE_COPPER); break; }
		case COMMAND_ADD_TRADE_SILVER	: { Command_TradeAddCoin(client, sep, COMMAND_ADD_TRADE_SILVER); break; }
		case COMMAND_ADD_TRADE_GOLD		: { Command_TradeAddCoin(client, sep, COMMAND_ADD_TRADE_GOLD); break; }
		case COMMAND_ADD_TRADE_PLAT		: { Command_TradeAddCoin(client, sep, COMMAND_ADD_TRADE_PLAT); break; }
		case COMMAND_REMOVE_TRADE_COPPER: { Command_TradeRemoveCoin(client, sep, COMMAND_REMOVE_TRADE_COPPER); break; }
		case COMMAND_REMOVE_TRADE_SILVER: { Command_TradeRemoveCoin(client, sep, COMMAND_REMOVE_TRADE_SILVER); break; }
		case COMMAND_REMOVE_TRADE_GOLD	: { Command_TradeRemoveCoin(client, sep, COMMAND_REMOVE_TRADE_GOLD); break; }
		case COMMAND_REMOVE_TRADE_PLAT	: { Command_TradeRemoveCoin(client, sep, COMMAND_REMOVE_TRADE_PLAT); break; }
		case COMMAND_ADD_TRADE_ITEM		: { Command_TradeAddItem(client, sep); break; }
		case COMMAND_REMOVE_TRADE_ITEM	: { Command_TradeRemoveItem(client, sep); break; }
		case COMMAND_ACCEPT_ADVANCEMENT	: { Command_AcceptAdvancement(client, sep); break; }
		case COMMAND_DUEL				: { Command_Duel(client, sep); break; }
		case COMMAND_DUELBET			: { Command_DuelBet(client, sep); break; }
		case COMMAND_DUEL_ACCEPT		: { Command_DuelAccept(client, sep); break; }
		case COMMAND_DUEL_DECLINE		: { Command_DuelDecline(client, sep); break; }
		case COMMAND_DUEL_SURRENDER		: { Command_DuelSurrender(client, sep); break; }
		case COMMAND_DUEL_TOGGLE		: { Command_DuelToggle(client, sep); break; }
		case COMMAND_SPAWN_TEMPLATE		: { Command_SpawnTemplate(client, sep); break; }
		//devn00b
		case COMMAND_MOOD				: { Command_Mood(client, sep); break;}

		case COMMAND_MODIFY				: { Command_Modify(client); break; }
		case COMMAND_MODIFY_CHARACTER	: { Command_ModifyCharacter(client, sep); break; }
		case COMMAND_MODIFY_QUEST		: { Command_ModifyQuest(client, sep); break; }
		case COMMAND_MODIFY_FACTION		: { Command_ModifyFaction(client, sep); break; }
		case COMMAND_MODIFY_GUILD		: { Command_ModifyGuild(client, sep); break; }
		case COMMAND_MODIFY_ITEM		: { Command_ModifyItem(client, sep); break; }
		case COMMAND_MODIFY_SKILL		: { Command_ModifySkill(client, sep); break; }
		case COMMAND_MODIFY_SPAWN		: { Command_ModifySpawn(client, sep); break; }
	    case COMMAND_MODIFY_SPELL		: { Command_ModifySpell(client, sep); break; }
		case COMMAND_MODIFY_ZONE		: { Command_ModifyZone(client, sep); break; }

		case COMMAND_JOIN_CHANNEL		: { Command_JoinChannel(client, sep); break;}
		case COMMAND_JOIN_CHANNEL_FROM_LOAD: { Command_JoinChannelFromLoad(client, sep); break;}
		case COMMAND_TELL_CHANNEL		: { Command_TellChannel(client, sep); break;}
		case COMMAND_LEAVE_CHANNEL		: { Command_LeaveChannel(client, sep); break;}
		case COMMAND_WHO_CHANNEL		: { Command_WhoChannel(client, sep); break;}
		case COMMAND_RAIN				: { Command_Rain(client, sep); break; }
		case COMMAND_WIND				: { Command_Wind(client, sep); break; }
		case COMMAND_WEATHER			: { Command_Weather(client, sep); break; }
		case COMMAND_FROM_MERCHANT		: { Command_SendMerchantWindow(client, sep); break; }
		case COMMAND_TO_MERCHANT		: { Command_SendMerchantWindow(client, sep, true); break; }
		case COMMAND_SELECT				: { Command_Select(client, sep); break; }
		case COMMAND_SMP				: { Command_StationMarketPlace(client, sep); break; }
		case COMMAND_CONSUME_FOOD		: { Command_ConsumeFood(client, sep); break; }
		case COMMAND_SET_CONSUME_FOOD	: { Command_ConsumeFood(client, sep); break; }
		case COMMAND_AQUAMAN			: { Command_Aquaman(client, sep); break; }
		case COMMAND_ATTUNE_INV			: { Command_Attune_Inv(client, sep); break; }
		case COMMAND_PLAYER				: { Command_Player(client, sep); break; }
		case COMMAND_PLAYER_COINS		: { Command_Player_Coins(client, sep); break; }
		case COMMAND_RESET_ZONE_TIMER	: { Command_Reset_Zone_Timer(client, sep); break; }
		case COMMAND_ACHIEVEMENT_ADD	: { Command_AchievementAdd(client, sep); break; }
		case COMMAND_EDITOR				: { Command_Editor(client, sep); break; }
		case COMMAND_ACCEPT_RESURRECTION: { Command_AcceptResurrection(client, sep); break; }
		case COMMAND_DECLINE_RESURRECTION:{ Command_DeclineResurrection(client, sep); break; }
		case COMMAND_TEST				: { Command_Test(client, command_parms); break; }
		case COMMAND_SPEED				: { Command_Speed(client, sep); break; }

		case COMMAND_BOT				: { Command_Bot(client, sep); break; }
		case COMMAND_BOT_CREATE			: { Command_Bot_Create(client, sep); break; }
		case COMMAND_BOT_CUSTOMIZE		: { Command_Bot_Customize(client, sep); break; }
		case COMMAND_BOT_SPAWN			: { Command_Bot_Spawn(client, sep); break; }
		case COMMAND_BOT_LIST			: { Command_Bot_List(client, sep); break; }
		case COMMAND_BOT_INV			: { Command_Bot_Inv(client, sep); break; }
		case COMMAND_BOT_SETTINGS		: { Command_Bot_Settings(client, sep); break; }
		case COMMAND_BOT_HELP			: { Command_Bot_Help(client, sep); break; }
		case GET_AA_XML					: { Get_AA_Xml(client, sep); break; }
		case ADD_AA						: { Add_AA(client, sep); break; }
		case COMMIT_AA_PROFILE			: { Commit_AA_Profile(client, sep); break; }
		case BEGIN_AA_PROFILE			: { Begin_AA_Profile(client, sep); break; }
		case BACK_AA					: { Back_AA(client, sep); break; }
		case REMOVE_AA					: { Remove_AA(client, sep); break; }
		case SWITCH_AA_PROFILE			: { Switch_AA_Profile(client, sep); break; }
		case CANCEL_AA_PROFILE			: { Cancel_AA_Profile(client, sep); break; }
		case SAVE_AA_PROFILE			: { Save_AA_Profile(client, sep); break; }
		case COMMAND_TARGETITEM			: { Command_TargetItem(client, sep); break; }
		case COMMAND_FINDSPAWN: { Command_FindSpawn(client, sep); break; }
		case COMMAND_MOVECHARACTER: { Command_MoveCharacter(client, sep); break; }
		case COMMAND_CRAFTITEM: {
					Item* item = 0;
					if (sep && sep->IsNumber(0)) {
						int32 item_id = atol(sep->arg[0]);
						int32 quantity = 1;

						if (sep->arg[1] && sep->IsNumber(1))
							quantity = atoi(sep->arg[1]);
						item = new Item(master_item_list.GetItem(item_id));
						if (!item) {
							LogWrite(TRADESKILL__ERROR, 0, "CraftItem", "Item (%u) not found.", item_id);
						}
						else {
							item->details.count = quantity;
							// use CHANNEL_COLOR_CHAT_RELATIONSHIP as that is the same value (4) as it is in a log for this message
							client->Message(CHANNEL_COLOR_CHAT_RELATIONSHIP, "You created %s.", item->CreateItemLink(client->GetVersion()).c_str());
							bool itemDeleted = false;
							client->AddItem(item, &itemDeleted);
							//Check for crafting quest updates
							int8 update_amt = 0;
							if (!itemDeleted && item->stack_count > 1)
								update_amt = 1;
							else
								update_amt = quantity;

							if(!itemDeleted)
								client->GetPlayer()->CheckQuestsCraftUpdate(item, update_amt);
						}

					}
					
					else {
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /craftitem {item_id} [quantity] ");
						}
					break;
				}
		case COMMAND_UNMENTOR:
		case COMMAND_MENTOR: {
			client->GetPlayer()->MentorTarget();
			break;
		}
		case COMMAND_CANCEL_EFFECT: { Command_CancelEffect(client, sep); break; }
		case COMMAND_CUREPLAYER: { Command_CurePlayer(client, sep); break; }
		case COMMAND_SHARE_QUEST: { Command_ShareQuest(client, sep); break; }
		case COMMAND_YELL: { Command_Yell(client, sep); break; }
		case COMMAND_SETAUTOLOOTMODE: { Command_SetAutoLootMode(client, sep); break; }
		case COMMAND_ASSIST: { Command_Assist(client, sep); break; }
		case COMMAND_TARGET: { Command_Target(client, sep); break; }
		case COMMAND_TARGET_PET: { Command_Target_Pet(client, sep); break; }
		case COMMAND_WHOGROUP: { Command_WhoGroup(client, sep); break; }
		case COMMAND_WHORAID: { Command_WhoRaid(client, sep); break; }
		case COMMAND_RAIDINVITE: { Command_RaidInvite(client, sep); break; }
		case COMMAND_RAID_LOOTER: { Command_Raid_Looter(client, sep); break; }
		case COMMAND_KICKFROMGROUP: { Command_KickFromGroup(client, sep); break; }
		case COMMAND_KICKFROMRAID: { Command_KickFromRaid(client, sep); break; }
		case COMMAND_LEAVERAID: { Command_LeaveRaid(client, sep); break; }
		case COMMAND_SPLIT: { Command_Split(client, sep); break; }
		case COMMAND_RAIDSAY: { Command_RaidSay(client, sep); break; }
		case COMMAND_RELOAD_ZONEINFO: { Command_ReloadZoneInfo(client, sep); break; }
		case COMMAND_SLE: { Command_SetLocationEntry(client, sep); break; }
		case COMMAND_STORE_LIST_ITEM: { Command_StoreListItem(client, sep); break; }
		case COMMAND_STORE_SET_PRICE: { Command_StoreSetPrice(client, sep); break; }
		case COMMAND_STORE_SET_PRICE_LOCAL: { Command_StoreSetPriceLocal(client, sep); break; }
		case COMMAND_STORE_START_SELLING: { Command_StoreStartSelling(client, sep); break; }
		case COMMAND_STORE_STOP_SELLING: { Command_StoreStopSelling(client, sep); break; }
		case COMMAND_STORE_UNLIST_ITEM: { Command_StoreUnlistItem(client, sep); break; }
		case COMMAND_CLOSE_STORE_KEEP_SELLING: { Command_CloseStoreKeepSelling(client, sep); break; }
		case COMMAND_CANCEL_STORE: { Command_CancelStore(client, sep); break; }
		default: 
		{
			LogWrite(COMMAND__WARNING, 0, "Command", "Unhandled command: %s", command->command.data.c_str());
			break;
		}

	}
	safe_delete(sep);
}


/******************** New COMMAND Handler Functions ********************/
/*
	Started breaking apart the huge switch() for commands into sepErate 
	functions so it is easier to locate the blocks of code by command
	-- JA 2012.03.03
*/

// sample function header
/* 
	Function: 
	Purpose	: 
	Params	: 
	Dev		: 
	Example	: 
*/ 
//void Commands::Command()
//{
//}


/* 
	Function: Command_AcceptAdvancement()
	Purpose	: Player accepts a new advancement option
	Params	: Spell ID
	Dev		: Jabantiz
*/ 
void Commands::Command_AcceptAdvancement(Client* client, Seperator* sep)
{
	 Player *player = client->GetPlayer();
	 if (sep && sep->IsSet(0)) {
		 int32 trait_id = atoul(sep->arg[0]);
		 TraitData* trait = nullptr;
		 if(client->GetVersion() <= 561) {
			 trait = master_trait_list.GetTraitByItemID(trait_id);
		 }
		 else {
			trait = master_trait_list.GetTrait(trait_id);
		 }

		 if(!trait) {
			LogWrite(COMMAND__ERROR, 0, "Command", "Invalid accept advancement of trait %u, no trait found.", trait_id);
			 return; // not valid lets not crash!
		 }
		 
		 if(!master_trait_list.IsPlayerAllowedTrait(client, trait)) {
			 client->SimpleMessage(CHANNEL_COLOR_RED, "Not enough trait points to accept trait.");
			 return;
		 }
		 // Check to see if this is a trait or grandmaster training (traits are always new spells, training is always upgrades)
		 if (!player->HasSpell(trait->spellID, 0, true))
		 {
			 Spell* spell = master_spell_list.GetSpell(trait->spellID, trait->tier);
			 if(spell) {
				player->AddSpellBookEntry(trait->spellID, trait->tier, player->GetFreeSpellBookSlot(spell->GetSpellData()->spell_book_type), spell->GetSpellData()->spell_book_type, spell->GetSpellData()->linked_timer, true);
			 }
			 else {
				client->Message(CHANNEL_COLOR_RED, "ERROR! Trait is not a valid spell id %u may be disabled.", trait->spellID);
			 }
		 }
		 else
		 {
			 Spell* spell = master_spell_list.GetSpell(trait->spellID, trait->tier);
			 if(spell) {
				 int8 old_slot = player->GetSpellSlot(spell->GetSpellID());
				 player->RemoveSpellBookEntry(spell->GetSpellID());
				 player->AddSpellBookEntry(spell->GetSpellID(), spell->GetSpellTier(), old_slot, spell->GetSpellData()->spell_book_type, spell->GetSpellData()->linked_timer, true);
				 player->UnlockSpell(spell);
				 client->SendSpellUpdate(spell);
			 }
			 else {
				client->Message(CHANNEL_COLOR_RED, "ERROR! Trait is not a valid spell id %u may be disabled.", trait->spellID);
			 }
		 }

		 // Spell book update
		 client->QueuePacket(player->GetSpellBookUpdatePacket(client->GetVersion()));
		 client->QueuePacket(master_trait_list.GetTraitListPacket(client));
		 
		 if(client->GetVersion() <= 561) {
			master_trait_list.ChooseNextTrait(client);
		 }
	 }
}

/* 
	Function: 
	Purpose	: 
	Params	: 
	Dev		: 
	Example	: 
*/ 
void Commands::Command_AFK(Client* client, Seperator* sep)
{
	Player* player = client->GetPlayer();

	player->toggle_character_flag(CF_AFK);
	client->Message(CHANNEL_COLOR_YELLOW,"You are %s afk.", client->GetPlayer()->get_character_flag(CF_AFK)?"now":"no longer");

	if (player->get_character_flag(CF_AFK))
	{
		if (sep && sep->argplus[0])
			player->SetAwayMessage("I am away from the keyboard, " + string(sep->argplus[0]));
		else
			player->SetAwayMessage("Sorry, I am A.F.K. (Away From Keyboard)");

		string message = string(player->GetName()) + " is going afk.";
		Spawn* target = player->GetTarget();

		if (target && target != player)
		{
			message = string(player->GetName()) + " tells " + string(target->GetName()) + " that ";
			player->GetGender() == 1 ? message += "he" : message += "she";
			message += " is going afk.";
		}

		player->GetZone()->SimpleMessage(CHANNEL_COLOR_YELLOW, message.c_str(), player, 30);
	}
	
	if (player->get_character_flag(CF_AFK))
		player->SetActivityStatus(player->GetActivityStatus() + ACTIVITY_STATUS_AFK);
	else
		player->SetActivityStatus(player->GetActivityStatus() - ACTIVITY_STATUS_AFK);
}

/* 
	Function: Command_Appearance()
	Purpose	: Handles /appearance commands
	Params	: list
	Dev		: Scatman
	Example	: /appearance list
*/ 
void Commands::Command_Appearance(Client* client, Seperator* sep, int handler)
{
	if( handler == COMMAND_APPEARANCE )
	{
		// /appearance command by itself shows help (to be extended)
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /appearance list [appearance name]");
		return;
	}
	else if( handler == COMMAND_APPEARANCE_LIST )
	{
		// /appearance list command expects "name" param
		if (sep && sep->arg[0]) 
		{
			const char* appearance_name = sep->argplus[0];
			client->Message(CHANNEL_COLOR_YELLOW, "Listing appearances like '%s':", appearance_name);
			vector<int16>* appearances = database.GetAppearanceIDsLikeName(string(appearance_name));

			if (appearances) 
			{
				vector<int16>::iterator itr;
				for (itr = appearances->begin(); itr != appearances->end(); itr++) 
				{
					int16 id = *itr;
					string name = database.GetAppearanceName(id);

					if (ToLower(name).find(ToLower(string(appearance_name))) < 0xFFFFFFFF)
						client->Message(CHANNEL_COLOR_YELLOW, "%s (%u)", name.c_str(), id);
				}
				safe_delete(appearances);
			}
		}
		else // no param
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /appearance list [appearance name]");
	}

}

/* 
	Function: Command_Claim()
	Purpose	: Summon veteran rewards
	Params	: nothing = show claim window, any number claims that item.
	Dev		: devn00b
	Example	: /claim 0 (claims the 1st item added to the list claim[0])
*/ 
void Commands::Command_Claim(Client* client, Seperator* sep)
{
	//if we were passed a claim id
	if (sep && sep->argplus[0] && sep->IsNumber(0)) 
	{
		int32 char_id = client->GetCharacterID();
		int8 my_claim_id = atoi(sep->argplus[0]);
		vector<ClaimItems> claim = database.LoadCharacterClaimItems(char_id);
		if(my_claim_id < claim.size()) {
			Item* item = master_item_list.GetItem(claim[my_claim_id].item_id);
			if(item) {
				database.ClaimItem(char_id, item->details.item_id, client);
			}
		}
		return;
	}
	else {
		//if no arg just send the window.
		client->ShowClaimWindow();
		return;
	}
	return;
}

/* 
	Function: Command_ClearAllQueued()
	Purpose	: 
	Params	: 
	Dev		: 
	Example	: 
*/ 
void Commands::Command_ClearAllQueued(Client* client)
{
	ZoneServer* zone = client->GetPlayer()->GetZone();
	if (zone && zone->GetSpellProcess())
		zone->GetSpellProcess()->RemoveSpellFromQueue(client->GetPlayer());
}

/* 
	Function: Command_CancelMaintained()
	Purpose	: Cancels maintained spells
	Params	: Maintained Spell Index
	Dev		: Zcoretri
	Example	: /cancel_maintained 1 - would cancel the spell in slot 1 of Maintained Spells list
*/ 
void Commands::Command_CancelMaintained(Client* client, Seperator* sep)
{
	if (sep && sep->arg[0] && sep->IsNumber(0)) 
	{
		int32 spell_index = atoul(sep->arg[0]);
		if(spell_index > 29)
			return;
		
		client->GetPlayer()->MMaintainedSpells.readlock(__FUNCTION__, __LINE__);
		MaintainedEffects mEffects = client->GetPlayer()->GetInfoStruct()->maintained_effects[spell_index];
		client->GetPlayer()->MMaintainedSpells.releasereadlock(__FUNCTION__, __LINE__);
		
		if (!client->GetPlayer()->GetZone()->GetSpellProcess()->DeleteCasterSpell(mEffects.spell, "canceled", false))
			client->Message(CHANNEL_COLOR_RED, "The maintained spell could not be cancelled.");
	}
}

/* 
	Function: Command_Create()
	Purpose	: Handler for starting Tradeskilling table
	Params	: 
	Dev		: Zcoretri
	Example	: 
*/ 
void Commands::Command_Create(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_CREATE");
	client->ShowRecipeBook();
}

void Commands::Command_CreateFromRecipe(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_CREATEFROMRECIPE");
	if (sep && sep->arg[0] && sep->IsNumber(0))
		ClientPacketFunctions::SendCreateFromRecipe(client, atoul(sep->arg[0]));
}

/* 
	Function: Command_Distance()
	Purpose	: Displays distance from targeted spawn
	Params	: 
	Dev		: Scatman
	Example	: /distance
*/ 
void Commands::Command_Distance(Client* client)
{
	Spawn* target = client->GetPlayer()->GetTarget();

	if (target)
		client->Message(CHANNEL_COLOR_YELLOW, "Your distance from %s is %f", target->GetName(), client->GetPlayer()->GetDistance(target));
	else
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You must have a spawn targeted to use /distance");
}

/* 
	Function: Command_Duel()
	Purpose	: Handle the /duel commands - not yet implemented
	Params	: unknown
	Dev		: 
	Example	: 
*/ 
void Commands::Command_Duel(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_DUEL");
	LogWrite(MISC__TODO, 1, "Command", "TODO-Command: Duel Command");
	client->Message(CHANNEL_COLOR_YELLOW, "You cannot duel other players (Not Implemented)");
}

/* 
	Function: Command_DuelBet()
	Purpose	: Handle the /duel commands - not yet implemented
	Params	: unknown
	Dev		: 
	Example	: 
*/ 
void Commands::Command_DuelBet(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_DUELBET");
	LogWrite(MISC__TODO, 1, "Command", "TODO-Command: Duel Bet Command");
	client->Message(CHANNEL_COLOR_YELLOW, "You cannot duel other players (Not Implemented)");
}

/* 
	Function: Command_DuelAccept()
	Purpose	: Handle the /duel commands - not yet implemented
	Params	: unknown
	Dev		: 
	Example	: 
*/ 
void Commands::Command_DuelAccept(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_DUEL_ACCEPT");
	LogWrite(MISC__TODO, 1, "Command", "TODO-Command: Accept Duel Command");
	client->Message(CHANNEL_COLOR_YELLOW, "You cannot duel other players (Not Implemented)");
}

/* 
	Function: Command_DuelDecline()
	Purpose	: Handle the /duel commands - not yet implemented
	Params	: unknown
	Dev		: 
	Example	: 
*/ 
void Commands::Command_DuelDecline(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_DUEL_DECLINE");
	LogWrite(MISC__TODO, 1, "Command", "TODO-Command: Decline Duel Request Command");
	client->Message(CHANNEL_COLOR_YELLOW, "You cannot duel other players (Not Implemented)");
}

/* 
	Function: Command_DuelSurrender()
	Purpose	: Handle the /duel commands - not yet implemented
	Params	: unknown
	Dev		: 
	Example	: 
*/ 
void Commands::Command_DuelSurrender(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_DUEL_SURRENDER");
	LogWrite(MISC__TODO, 1, "Command", "TODO-Command: Surrender Duel Command");
	client->Message(CHANNEL_COLOR_YELLOW, "You cannot duel other players (Not Implemented)");
}

/* 
	Function: Command_DuelToggle()
	Purpose	: Handle the /duel commands - not yet implemented
	Params	: unknown
	Dev		: 
	Example	: 
*/ 
void Commands::Command_DuelToggle(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_DUEL");
	LogWrite(MISC__TODO, 1, "Command", "TODO-Command: Duel Commands");
	client->Message(CHANNEL_COLOR_YELLOW, "You cannot duel other players (Not Implemented)");
}

/* 
	Function: 
	Purpose	: 
	Params	: 
	Dev		: 
	Example	: 
*/ 
void Commands::Command_EntityCommand(Client* client, Seperator* sep, int handler)
{
	if( handler == COMMAND_ENTITYCOMMAND )
	{
		// /entitycommand command by itself shows help (to be extended)
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /entity_command list [entity command name]");
		return;
	}
	else if( handler == COMMAND_ENTITYCOMMAND_LIST )
	{
		// /entitycommand list command expects "name" param
		if (sep && sep->arg[0]) 
		{
			const char* entity_command_name = sep->argplus[0];
			client->Message(CHANNEL_COLOR_YELLOW, "Listing entity commands like '%s':", entity_command_name);
			map<int32, vector<EntityCommand*>*>* entity_command_list_all = client->GetCurrentZone()->GetEntityCommandListAll();
			map<int32, vector<EntityCommand*>*>::iterator itr;

			for (itr = entity_command_list_all->begin(); itr != entity_command_list_all->end(); itr++) 
			{
				vector<EntityCommand*>* entity_command_list = itr->second;
				vector<EntityCommand*>::iterator itr2;

				for (itr2 = entity_command_list->begin(); itr2 != entity_command_list->end(); itr2++) 
				{
					EntityCommand* entity_command = *itr2;

					if (ToLower(entity_command->name).find(ToLower(string(entity_command_name))) < 0xFFFFFFFF)
						client->Message(CHANNEL_COLOR_YELLOW, "Command Text: %s, Command List ID: %u, Distance: %f\n", entity_command->name.c_str(), itr->first, entity_command->distance);
				}
			}
		}
		else // no command name
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /entity_command list [entity command name]");
	}
}


/* 
	Function: Command_Follow()
	Purpose	: Handle the /follow command - not yet implemented
	Params	: 
	Dev		: 
	Example	: 
*/ 
void Commands::Command_Follow(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_FOLLOW");
	// flag to toggle if the players target is in the players group
	bool targetInGroup = false;
	// get a pointer to the players group
	GroupMemberInfo* gmi = client->GetPlayer()->GetGroupMemberInfo();
	// If the player has a group and has a target
	if (gmi && client->GetPlayer()->GetTarget()) {
		deque<GroupMemberInfo*>::iterator itr;

		world.GetGroupManager()->GroupLock(__FUNCTION__, __LINE__);

		PlayerGroup* group = world.GetGroupManager()->GetGroup(gmi->group_id);
		if (group)
		{
			group->MGroupMembers.readlock(__FUNCTION__, __LINE__);
			deque<GroupMemberInfo*>* members = group->GetMembers();
			// Loop through the group members
			for (itr = members->begin(); itr != members->end(); itr++) {
				// If a group member matches a target
				if ((*itr)->member && (*itr)->member == client->GetPlayer()->GetTarget()) {
					// toggle the flag and break the loop
					targetInGroup = true;
					break;
				}
			}
			group->MGroupMembers.releasereadlock(__FUNCTION__, __LINE__);
		}

		world.GetGroupManager()->ReleaseGroupLock(__FUNCTION__, __LINE__);
	}
	if (targetInGroup) {
		// CHANNEL_COLOR_CHAT_RELATIONSHIP = 4, which matches the value in logs
		client->Message(CHANNEL_COLOR_CHAT_RELATIONSHIP, "You start to follow %s.", client->GetPlayer()->GetTarget()->GetName());
		client->GetPlayer()->SetFollowTarget(client->GetPlayer()->GetTarget());
		client->GetPlayer()->info_changed = true;
		client->GetPlayer()->changed = true;
	}
	else
		client->Message(CHANNEL_NARRATIVE, "You must first select a group member to follow.");
}

/* 
	Function: Command_StopFollow()
	Purpose	: Handle the /stop_follow command - not yet implemented
	Params	: 
	Dev		: 
	Example	: 
*/ 
void Commands::Command_StopFollow(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_STOP_FOLLOW");
	if (client->GetPlayer()->GetFollowTarget()) {
		// CHANNEL_COLOR_CHAT_RELATIONSHIP = 4, which matches the value in logs
		client->Message(CHANNEL_COLOR_CHAT_RELATIONSHIP, "You are no longer following %s", client->GetPlayer()->GetFollowTarget()->GetName());
		client->GetPlayer()->SetFollowTarget(0);
		client->GetPlayer()->info_changed = true;
		client->GetPlayer()->changed = true;
	}
}

/* 
	Function: Command_Grid()
	Purpose	: Show player's current Grid ID
	Params	: 
	Dev		: Scatman
	Example	: /grid
*/ 
void Commands::Command_Grid(Client* client, Seperator* sep)
{
	if (client->GetPlayer()->GetMap() != nullptr) {
		if(sep && sep->arg[0][0] && strncasecmp("spawns", sep->arg[0], 6) == 0) {
			int32 grid = client->GetPlayer()->GetLocation();
				
			if(sep->IsNumber(1))
				grid = atoul(sep->arg[1]);
			
			client->GetCurrentZone()->SendClientSpawnListInGrid(client, grid);
		}
		if(sep && sep->arg[0][0] && strncasecmp("boundary", sep->arg[0], 8) == 0) {
			int32 grid = client->GetPlayer()->GetLocation();
				
			if(sep->IsNumber(1))
				grid = atoul(sep->arg[1]);
			
			if(!client->GetPlayer()->GetMap()) {
				client->Message(CHANNEL_COLOR_RED, "No map to check grid!");
				return;
			}
			GridMapBorder* gmb = client->GetPlayer()->GetMap()->GetMapGridBorder(grid, false);
			if(gmb) {
				client->Message(CHANNEL_COLOR_YELLOW, "Grid %u border MinX %f MaxX %f, MinY %f MaxY %f, MinZ %f MaxZ %f", grid, gmb->m_MinX, gmb->m_MaxX, gmb->m_MinY, gmb->m_MaxY, gmb->m_MinZ, gmb->m_MaxZ);
			}
			else {
				client->Message(CHANNEL_COLOR_RED, "Grid %u has no grid map border", grid);
			}
		}
		else {
			client->Message(CHANNEL_COLOR_YELLOW, "Your Grid ID is %u", client->GetPlayer()->GetLocation());
			auto loc = glm::vec3(client->GetPlayer()->GetX(), client->GetPlayer()->GetZ(), client->GetPlayer()->GetY());
			uint32 GridID = 0;
			uint32 WidgetID = 0;
			float new_z = client->GetPlayer()->FindBestZ(loc, nullptr, &GridID, &WidgetID);
			float minY = client->GetPlayer()->GetMap()->GetMinY();
			float maxY = client->GetPlayer()->GetMap()->GetMaxY();
			float minZ = client->GetPlayer()->GetMap()->GetMinZ();
			float maxZ = client->GetPlayer()->GetMap()->GetMaxZ();
			int32 grid_spawn_count = client->GetPlayer()->GetZone()->GetSpawnCountInGrid(GridID);
			client->Message(CHANNEL_COLOR_YELLOW, "Grid result is %u, at EQ2 Y coordinate %f.  Spawns on grid: %u.  Min/Max Y %f/%f Z %f/%f.  Widget ID: %u", GridID, new_z, grid_spawn_count, minY, maxY, minZ, maxZ, WidgetID);
		}
	}
}

/* 
	Function: Command_Guild()
	Purpose	: Handler for all UI-related guild commands
	Dev		: Scatman
*/ 
void Commands::Command_Guild(Client* client, Seperator* sep)
{
	Guild* guild = client->GetPlayer()->GetGuild();
	if (sep && sep->GetMaxArgNum() > 0 && sep->arg[0])
	{
		const char* command = sep->arg[0];
		int32 length = strlen(command);

		LogWrite(COMMAND__DEBUG, 0, "Command", "Guild Command: %s", command);

		if (strncmp(command, "rank_name", length) == 0 && sep->arg[1] && sep->IsNumber(1) && sep->arg[2] && guild)
			guild->SetRankName(atoi(sep->arg[1]), sep->argplus[2]);
		else if (strncmp(command, "rank_permission", length) == 0 && sep->arg[1] && sep->IsNumber(1) && sep->arg[2] && sep->IsNumber(2) && sep->arg[3] && guild) {
			guild->SetPermission(atoi(sep->arg[1]), atoi(sep->arg[2]), strncmp(sep->arg[3], "true", 4) == 0 ? 1 : 0);
			peer_manager.sendPeersGuildPermission(guild->GetID(), atoul(sep->arg[1]), atoul(sep->arg[2]), strncmp(sep->arg[3], "true", 4) == 0 ? 1 : 0);
		}
		else if (strncmp(command, "filter_event", length) == 0 && sep->arg[1] && sep->IsNumber(1) && sep->arg[2] && sep->IsNumber(2) && sep->arg[3] && guild) {
			guild->SetEventFilter(atoi(sep->arg[1]), atoi(sep->arg[2]), strncmp(sep->arg[3], "true", 4) == 0 ? 1 : 0);
			peer_manager.sendPeersGuildEventFilter(guild->GetID(), atoul(sep->arg[1]), atoul(sep->arg[2]), strncmp(sep->arg[3], "true", 4) == 0 ? 1 : 0);
		}
		else if (strncmp(command, "kick", length) == 0 && sep->arg[1] && guild) {
			int32 character_id = guild->KickGuildMember(client, sep->arg[1]);
			if(character_id > 0) {
				peer_manager.sendPeersRemoveGuildMember(character_id, guild->GetID(), std::string(client->GetPlayer()->GetName()));
			}
		}
		else if (strncmp(command, "demote", length) == 0 && sep->arg[1] && guild)
			guild->DemoteGuildMember(client, sep->arg[1]);
		else if (strncmp(command, "promote", length) == 0 && sep->arg[1] && guild)
			guild->PromoteGuildMember(client, sep->arg[1]);
		else if (strncmp(command, "points", length) == 0 && guild)
		{
			if (sep->arg[1] && strncmp(sep->arg[1], "add", length) == 0)
			{
				if (sep->arg[2] && sep->IsNumber(2) && sep->arg[3])
				{
					float points = atof(sep->arg[2]);
					const char* option = sep->arg[3];
					const char* comment = sep->argplus[4];

					if (strncmp(option, "all", strlen(option)) == 0)
						guild->AddPointsToAll(client, points, comment);
					else if (strncmp(option, "online", strlen(option)) == 0)
						guild->AddPointsToAllOnline(client, points, comment);
					else if (strncmp(option, "group", strlen(option)) == 0)
						guild->AddPointsToGroup(client, points, comment);
					else if (strncmp(option, "raid", strlen(option)) == 0)
						guild->AddPointsToRaid(client, points, comment);
					else
						guild->AddPointsToGuildMember(client, points, option, comment);
				}
			}
			else if (sep->arg[1] && strncmp(sep->arg[1], "view", strlen(sep->arg[1])) == 0 && sep->arg[2])
				guild->ViewGuildMemberPoints(client, sep->arg[2]);
			else
				LogWrite(COMMAND__ERROR, 0, "Command", "Unhandled guild points command: %s", sep->argplus[0]);
		}
		else if (strncmp(command, "motd", length) == 0 && sep->arg[1] && guild)
			guild->SetMOTD(sep->argplus[1]);
		else if (strncmp(command, "recruiting", length) == 0 && guild) 
		{
			if (sep->arg[1])
			{
				const char* option = sep->arg[1];

				if (strncmp(option, "short_text", strlen(option)) == 0 && sep->arg[2])
					guild->SetRecruitingShortDesc(sep->argplus[2]);
				else if (strncmp(option, "long_text", strlen(option)) == 0 && sep->arg[2])
					guild->SetRecruitingFullDesc(sep->argplus[2]);
				else if (strncmp(option, "flag", strlen(option)) == 0 && sep->arg[2] && sep->IsNumber(2) && sep->arg[3])
					guild->SetRecruitingFlag(atoi(sep->arg[2]), strncmp(sep->arg[3], "true", 4) == 0 ? 1 : 0);
				else if (strncmp(option, "min_level", strlen(option)) == 0 && sep->arg[2] && sep->IsNumber(2))
					guild->SetRecruitingMinLevel(atoi(sep->arg[2]));
				else if (strncmp(option, "playstyle", strlen(option)) == 0 && sep->arg[2] && sep->IsNumber(2))
					guild->SetRecruitingPlayStyle(atoi(sep->arg[2]));
				else if (strncmp(option, "tag", strlen(option)) == 0 && sep->arg[2] && sep->IsNumber(2) && sep->arg[3] && sep->IsNumber(3))
					guild->SetRecruitingDescTag(atoi(sep->arg[2]), atoi(sep->arg[3]));
				else if (strncmp(command, "recruiting", strlen(option)) == 0)
					guild->ChangeMemberFlag(client, GUILD_MEMBER_FLAGS_RECRUITING_FOR_GUILD, strncmp(sep->arg[1], "true", 4) == 0 ? 1 : 0);
				else
					LogWrite(COMMAND__ERROR, 0, "Command", "Unhandled guild recruiting command: %s", sep->argplus[0]);
			}
			else
				LogWrite(COMMAND__ERROR, 0, "Command", "Unhandled guild recruiting command: %s", sep->argplus[0]);
		}
		else if (strncmp(command, "notify_online", length) == 0 && sep->arg[1] && guild)
			guild->ChangeMemberFlag(client, GUILD_MEMBER_FLAGS_NOTIFY_LOGINS, strncmp(sep->arg[1], "true", 4) == 0 ? 1 : 0);
		else if (strncmp(command, "event_privacy", length) == 0 && sep->arg[1] && guild)
			guild->ChangeMemberFlag(client, GUILD_MEMBER_FLAGS_DONT_GENERATE_EVENTS, strncmp(sep->arg[1], "true", 4) == 0 ? 1 : 0);
		else if (strncmp(command, "recruiter", length) == 0 && sep->arg[1] && sep->arg[2] && guild)
			guild->SetGuildRecruiter(client, sep->arg[1], strncmp(sep->arg[2], "true", 4) == 0 ? true : false);
		else if (strncmp(command, "recruiter_description", length) == 0 && sep->arg[1] && guild)
			guild->SetGuildRecruiterDescription(client, sep->argplus[1]);
		else if (strncmp(command, "lock_event", length) == 0 && sep->arg[1] && sep->arg[2] && guild)
			guild->LockGuildEvent(atoul(sep->arg[1]), strncmp(sep->arg[2], "true", 4) == 0 ? true : false);
		else if (strncmp(command, "delete_event", length) == 0 && sep->arg[1] && guild)
			guild->DeleteGuildEvent(atoul(sep->arg[1]));
		else if (strncmp(command, "invite", length) == 0 && guild) 
		{
			if (sep->arg[1] && strlen(sep->arg[1]) > 0)
				guild->InvitePlayer(client, sep->arg[1]);
			else 
			{
				Spawn* target = client->GetPlayer()->GetTarget();
				if (target) 
				{
					if (target->IsPlayer())
						guild->InvitePlayer(client, target->GetName());
					else
						client->Message(CHANNEL_NARRATIVE, "%s is not a player.", target->GetName());
				}
			}
		}
		else if (strncmp(command, "accept", length) == 0) 
		{
			PendingGuildInvite* pgi = client->GetPendingGuildInvite();

			if (pgi && pgi->guild && pgi->invited_by)
				pgi->guild->AddNewGuildMember(client, pgi->invited_by->GetName());
			client->SetPendingGuildInvite(0);
		}
		else if (strncmp(command, "decline", length) == 0) 
		{
			PendingGuildInvite* pgi = client->GetPendingGuildInvite();

			if (pgi && pgi->guild && pgi->invited_by && pgi->invited_by->IsPlayer())
			{
				Client* client_inviter = ((Player*)pgi->invited_by)->GetClient();

				if (client_inviter) {
					client_inviter->Message(CHANNEL_NARRATIVE, "%s has declined your invitation to join %s.", client->GetPlayer()->GetName(), pgi->guild->GetName());
				}
			}
			client->SetPendingGuildInvite(0);
		}
		else if (strncmp(command, "create", length) == 0 && sep->arg[1]) 
		{
			const char* guild_name = sep->argplus[1];
			if(!guild_name || strlen(guild_name) < 4) {
				client->SimpleMessage(CHANNEL_NARRATIVE, "Guild name is too short.");
			}
			else if (!guild_list.GetGuild(guild_name)) {
				if(net.is_primary) {
					int32 guildID = world.CreateGuild(guild_name, client, client->GetPlayer()->GetGroupMemberInfo() ? client->GetPlayer()->GetGroupMemberInfo()->group_id : 0);
					if(guildID > 0)
						peer_manager.sendPeersCreateGuild(guildID);
				}
				else {
					peer_manager.sendPrimaryCreateGuildRequest(std::string(guild_name), std::string(client->GetPlayer()->GetName()));
				}
			}
			else
				client->SimpleMessage(CHANNEL_NARRATIVE, "A guild with that name already exists.");
		}
		else if (strncmp(command, "search", length) == 0)
			client->ShowGuildSearchWindow();
		else if (strncmp(command, "recruiting_details", length) == 0 && sep->arg[1] && sep->IsNumber(1))
		{
			Guild* to_guild = guild_list.GetGuild(atoul(sep->arg[1]));

			if (to_guild)
				to_guild->SendGuildRecruitingDetails(client);
		}
		else if (strncmp(command, "recruiting_image", length) == 0 && sep->arg[1] && sep->IsNumber(1)) 
		{
			Guild* to_guild = guild_list.GetGuild(atoul(sep->arg[1]));

			if (to_guild)
				to_guild->SendGuildRecruitingImages(client);
		}
		else if (strncmp(command, "recruiter_adventure_class", length) == 0) 
		{
			if (sep->arg[1])
			{
				const char* option = sep->arg[1];

				if (strncmp(option, "toggle", strlen(option)) == 0)
					guild->ToggleGuildRecruiterAdventureClass(client);
			}
			else
				LogWrite(COMMAND__ERROR, 0, "Command", "Unhandled guild recruiter_adventure_class command: '%s'", sep->argplus[0]);
		}
		else if (strncmp(command, "display_heraldry", length) == 0)
		{
			// JA: not sure this is right...
			client->GetPlayer()->toggle_character_flag(CF_SHOW_GUILD_HERALDRY);
			client->GetPlayer()->SetCharSheetChanged(true);
		}
		else
			LogWrite(COMMAND__ERROR, 0, "Command", "Unhandled guild command: '%s'", sep->argplus[0]);
	}
}

/* 
	Function: Command_GuildCreate()
	Purpose	: Display's in-game Guild Creation window
	Dev		: Scatman
*/ 
void Commands::Command_CreateGuild(Client* client, Seperator* sep)
{
	Command_GuildsCreate(client, sep, true);
}

/* 
	Function: Command_SetGuildOfficerNote()
	Purpose	: 
	Dev		: Scatman
*/ 
void Commands::Command_SetGuildOfficerNote(Client* client, Seperator* sep)
{
	if (sep && sep->arg[0] && sep->arg[1]) 
	{
		Guild* guild = client->GetPlayer()->GetGuild();

		if (guild)
			guild->SetGuildOfficerNote(sep->arg[0], sep->argplus[1]);
	}
}

/* 
	Function: Command_SetGuildMemberNote()
	Purpose	: 
	Dev		: Scatman
*/ 
void Commands::Command_SetGuildMemberNote(Client* client, Seperator* sep)
{
	if (sep && sep->arg[0] && sep->arg[1]) 
	{
		Guild* guild = client->GetPlayer()->GetGuild();

		if (guild)
			guild->SetGuildMemberNote(sep->arg[0], sep->argplus[1]);
	}
}

/* 
	Function: Command_GuildSay()
	Purpose	: 
	Dev		: Scatman
*/ 
void Commands::Command_GuildSay(Client* client, Seperator* sep)
{
	Guild* guild = client->GetPlayer()->GetGuild();

	if (guild) 
	{
		if (sep && sep->arg[0]) {
			bool success = guild->HandleGuildSay(client, sep->argplus[0]);
			if(success)
				peer_manager.SendPeersGuildChannelMessage(guild->GetID(), std::string(client->GetPlayer()->GetName()), std::string(sep->argplus[0]), CHANNEL_GUILD_SAY, client->GetPlayer()->GetCurrentLanguage());
		}
	}
	else
		client->SimpleMessage(CHANNEL_NARRATIVE, "You are not a member of a guild");
}

/* 
	Function: Command_OfficerSay()
	Purpose	: 
	Dev		: Scatman
*/ 
void Commands::Command_OfficerSay(Client* client, Seperator* sep)
{
	Guild* guild = client->GetPlayer()->GetGuild();

	if (guild) 
	{
		if (sep && sep->arg[0]) {
			bool success = guild->HandleOfficerSay(client, sep->argplus[0]);
			if(success)
				peer_manager.SendPeersGuildChannelMessage(guild->GetID(), std::string(client->GetPlayer()->GetName()), std::string(sep->argplus[0]), CHANNEL_OFFICER_SAY, client->GetPlayer()->GetCurrentLanguage());
		}
	}
	else
		client->SimpleMessage(CHANNEL_NARRATIVE, "You are not a member of a guild");
}

/* 
	Function: Command_Guilds()
	Purpose	: Shows help for /guild command
	Params	: 
	Dev		: Scatman
	Example	: 
*/ 
void Commands::Command_Guilds(Client* client)
{
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /guilds [create|delete|add|remove|list]");
}

/* 
	Function: Command_GuildsAdd()
	Purpose	: Add's players to a guild
	Params	: guild_name|guild_id, player name
	Dev		: Scatman
	Example	: /guilds add 1 Admin  = adds player Admin to guild_id 1
*/ 
void Commands::Command_GuildsAdd(Client* client, Seperator* sep)
{
	if (sep && sep->arg[0] && strlen(sep->arg[0]) > 0) 
	{
		Guild* guild = 0;
		bool found = true;

		if (sep->IsNumber(0)) 
		{
			guild = guild_list.GetGuild(atoul(sep->arg[0]));

			if (!guild) 
			{
				client->Message(CHANNEL_COLOR_YELLOW, "Guild with ID %u does not exist.", atoul(sep->arg[0]));
				found = false;
			}
		}
		else 
		{
			guild = guild_list.GetGuild(sep->arg[0]);

			if (!guild) 
			{
				client->Message(CHANNEL_COLOR_YELLOW, "Guild '%s' does not exist.", sep->arg[0]);
				found = false;
			}
		}
		if (found) 
		{
			Client* to_client = 0;

			if (sep->arg[1] && strlen(sep->arg[1]) > 0)
				to_client = zone_list.GetClientByCharName(string(sep->arg[1]));
			else if (client->GetPlayer()->GetTarget() && client->GetPlayer()->GetTarget()->IsPlayer())
				to_client = ((Player*)client->GetPlayer()->GetTarget())->GetClient();

			if (to_client)
				guild->InvitePlayer(client, to_client->GetPlayer()->GetName());
			else
				client->Message(CHANNEL_COLOR_YELLOW, "Could not find player '%s' to invite to the guild.", sep->arg[1]);
		}
	}
	else
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /guilds add [guild name|guild id] (player name).");
}

/* 
	Function: Command_GuildsCreate()
	Purpose	: Creates a guild
	Params	: guild_name, player_name (optional)
	Dev		: Scatman
	Example	: /guilds create [guild name] (player name)
*/ 
void Commands::Command_GuildsCreate(Client* client, Seperator* sep, bool prompted_dialog)
{
	Spawn* npc = nullptr;
	if(prompted_dialog) {
		auto target_npc = client->dialog_manager.getAcceptValue("create guild");
		if(!target_npc) {
			// well this is not acceptable!! CHEATER! :D
			return;
		}
		else {
			if(!client->GetPlayer()->GetZone()){
				// player isn't in a zone? eh..
				return;
			}
			npc = client->GetPlayer()->GetZone()->GetSpawnByID(target_npc);
			if(!npc) {
				client->Message(CHANNEL_COLOR_RED, "Did not find guild registrar, please re-initiate dialog with them.");
				return;
			}
		}
	}
	if (sep && sep->arg[0]) 
	{
		const char* guild_name = sep->arg[0];
		if(prompted_dialog)
			guild_name = sep->argplus[0];
		
		int8 resp = database.CheckNameFilter(guild_name, 4, 41);
		if(!guild_name || resp == BADNAMELENGTH_REPLY) {
			client->Message(CHANNEL_COLOR_YELLOW, "Guild name is too short.");
		}
		else if(resp == NAMEINVALID_REPLY || resp == NAMEFILTER_REPLY) {
			client->Message(CHANNEL_COLOR_YELLOW, "Guild name was rejected.");
		}
		else if (!guild_list.GetGuild(guild_name)) 
		{
			bool ret = false;

			if (!prompted_dialog && sep->arg[1] && strlen(sep->arg[1]) > 0 && client->GetAdminStatus() > 0) 
			{
				Client* to_client = zone_list.GetClientByCharName(string(sep->arg[1]));

				if (to_client && !to_client->GetPlayer()->GetGuild()) 
				{
					if(net.is_primary) {
						int32 guildID = world.CreateGuild(guild_name, to_client, to_client->GetPlayer()->GetGroupMemberInfo() ? to_client->GetPlayer()->GetGroupMemberInfo()->group_id : 0);
						if(guildID > 0)
							peer_manager.sendPeersCreateGuild(guildID);
					}
					else {
						peer_manager.sendPrimaryCreateGuildRequest(std::string(guild_name), std::string(to_client->GetPlayer()->GetName()));
					}
					ret = true;
				}
				else {
					client->Message(CHANNEL_COLOR_YELLOW, "Could not find target %s or target is already in a guild.", sep->arg[1]);
				}
			}
			else if (!prompted_dialog && client->GetAdminStatus() > 0 && client->GetPlayer()->GetTarget() && client->GetPlayer()->GetTarget()->IsPlayer()) 
			{
				Client* to_client = ((Player*)client->GetPlayer()->GetTarget())->GetClient();

				if (to_client && !to_client->GetPlayer()->GetGuild()) 
				{
					if(net.is_primary) {
						int32 guildID = world.CreateGuild(guild_name, to_client, to_client->GetPlayer()->GetGroupMemberInfo() ? to_client->GetPlayer()->GetGroupMemberInfo()->group_id : 0);
						if(guildID > 0)
							peer_manager.sendPeersCreateGuild(guildID);
					}
					else {
						peer_manager.sendPrimaryCreateGuildRequest(std::string(guild_name), std::string(to_client->GetPlayer()->GetName()));
					}
					ret = true;
				}
				else {
					client->Message(CHANNEL_COLOR_YELLOW, "Could not find target %s or target is already in a guild.", (to_client != nullptr) ? to_client->GetPlayer()->GetName() : "NoTarget");
				}
			}
			else 
			{
				if(client->GetPlayer()->GetGuild()) {
					client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You are already in a guild.");
					if(client->GetAdminStatus() < 1)
						return;
				}
				else if(net.is_primary) {
					Client* leader = nullptr;
					if(prompted_dialog || client->GetAdminStatus() < 1)
						leader = client;
					int32 guildID = world.CreateGuild(guild_name, leader);
					if(guildID > 0) {
						peer_manager.sendPeersCreateGuild(guildID);
						ret = true;
					}
				}
				else {
					peer_manager.sendPrimaryCreateGuildRequest(std::string(guild_name), "", prompted_dialog, npc->GetID());
					return;
				}
			}

			if (ret) {
				client->Message(CHANNEL_COLOR_YELLOW, "Guild '%s' was successfully created.", guild_name);
				if(prompted_dialog) { // prompted dialog requires npc be defined
					client->GetCurrentZone()->CallSpawnScript(npc, SPAWN_SCRIPT_CASTED_ON, client->GetPlayer(), guild_name);
				}
			}
			else
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "There was an error creating the guild.");
		}
		else
			client->Message(CHANNEL_COLOR_YELLOW, "Guild '%s' already exists.", guild_name);
	}
	else if(!prompted_dialog)
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /guilds create [guild name] (player name).  If no player is specified, the player's target will be used.  If the player has no target, a guild with no members will be created.");
}

/* 
	Function: Command_GuildsDelete()
	Purpose	: Delete's a guild
	Params	: guild name
	Dev		: Scatman
	Example	: /guilds delete Test
*/ 
void Commands::Command_GuildsDelete(Client* client, Seperator* sep)
{
	if (sep && sep->arg[0]) 
	{
		Guild* guild = 0;
		bool found = true;

		if (sep->IsNumber(0)) 
		{
			guild = guild_list.GetGuild(atoul(sep->arg[0]));

			if (!guild) 
			{
				client->Message(CHANNEL_COLOR_YELLOW, "Guild with ID %u does not exist.", atoul(sep->arg[0]));
				found = false;
			}
		}
		else 
		{
			guild = guild_list.GetGuild(sep->arg[0]);

			if (!guild) 
			{
				client->Message(CHANNEL_COLOR_YELLOW, "Guild '%s' does not exist.", sep->arg[0]);
				found = false;
			}
		}

		if (found) 
		{
			guild->RemoveAllGuildMembers();
			database.DeleteGuild(guild);

			client->Message(CHANNEL_COLOR_YELLOW, "Guild '%s' was successfully deleted.", guild->GetName());
			guild_list.RemoveGuild(guild, true);
		}
	}
	else
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /guilds delete [guild name].");
}

/* 
	Function: Command_GuildsList()
	Purpose	: Lists current guilds on server
	Params	: 
	Dev		: Scatman
	Example	: /guilds list
*/ 
void Commands::Command_GuildsList(Client* client)
{
	MutexMap<int32, Guild*>* guilds = guild_list.GetGuilds();
	MutexMap<int32, Guild*>::iterator itr = guilds->begin();
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Guild List:");
	if (guilds->size() == 0)
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "None.");
	else {
		while (itr.Next()) {
			Guild* guild = itr.second;
			client->Message(CHANNEL_COLOR_YELLOW, "%u) %s", guild->GetID(), guild->GetName());
		}
	}
}

/* 
	Function: Command_GuildsRemove()
	Purpose	: Removes a player from a guild
	Params	: guild name|guild id, player name
	Dev		: Scatman
	Example	:  /guilds remove 1 Admin = removes Admin from guild 1
*/ 
void Commands::Command_GuildsRemove(Client* client, Seperator* sep)
{
	if (sep && sep->arg[0] && strlen(sep->arg[0]) > 0) 
	{
		Guild* guild = 0;
		bool found = true;

		if (sep->IsNumber(0)) 
		{
			guild = guild_list.GetGuild(atoul(sep->arg[0]));

			if (!guild) 
			{
				client->Message(CHANNEL_COLOR_YELLOW, "Guild with ID %u does not exist.", atoul(sep->arg[0]));
				found = false;
			}
		}
		else 
		{
			guild = guild_list.GetGuild(sep->arg[0]);

			if (!guild) 
			{
				client->Message(CHANNEL_COLOR_YELLOW, "Guild '%s' does not exist.", sep->arg[0]);
				found = false;
			}
		}

		if (found) 
		{
			Client* to_client = 0;
			char* charName = nullptr;
			if(sep->arg[1][0])
				charName = sep->arg[1];
			else if (client->GetPlayer()->GetTarget() && client->GetPlayer()->GetTarget()->IsPlayer())
				charName = ((Player*)client->GetPlayer()->GetTarget())->GetName();
			else
			{
				client->Message(CHANNEL_COLOR_YELLOW, "Missing player name or not a valid target to remove from the guild.");
				return;
			}
			int32 character_id = guild->KickGuildMember(client, charName);
			if(character_id > 0)
				peer_manager.sendPeersRemoveGuildMember(character_id, guild->GetID(), std::string(client->GetPlayer()->GetName()));
			else
				client->Message(CHANNEL_COLOR_YELLOW, "Could not find player '%s' to remove from the guild.", charName);
		}
	}
	else
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /guilds remove [guild name|guild id] (player name).");
}

/* 
	Function: Command_InspectPlayer()
	Purpose	: Handle the Inspect functions
	Params	: Client to inspect
	Dev		: Scatman
	Example	: /inspect Scatman
*/ 
void Commands::Command_InspectPlayer(Client* client, Seperator* sep)
{
	if (sep && sep->arg[0]) 
	{
		Client* inspect_client = zone_list.GetClientByCharName(string(sep->arg[0]));

		if (inspect_client)
			client->InspectPlayer(inspect_client->GetPlayer());
	}
	else 
	{
		Spawn* target = client->GetPlayer()->GetTarget();

		if (target && target->IsPlayer())
			client->InspectPlayer((Player*)target);
	}
}

/* 
	Function: Command_Inventory()
	Purpose	: Handle changes in player inventory
	Params	: <TBD>
	Dev		: All
	Example	: /inventory delete item_id
*/ 
void Commands::Command_Inventory(Client* client, Seperator* sep, EQ2_RemoteCommandString* command)
{
	PrintSep(sep, "Command_Inventory"); // temp to figure out the params

	Player* player = client->GetPlayer();

	if(sep && sep->arg[0][0])
	{
		LogWrite(COMMAND__INFO, 0, "Command", "command: %s", sep->argplus[0]);

		if(!client->GetPlayer()->Alive())
			client->SimpleMessage(CHANNEL_COLOR_RED,"You cannot do that right now.");
		else if(sep->arg[1][0] && strncasecmp("destroy", sep->arg[0], 6) == 0 && sep->IsNumber(1))
		{
			int16 index = atoi(sep->arg[1]);
			Item* item = client->GetPlayer()->item_list.GetItemFromIndex(index);

			if(item)
			{
				if(item->IsItemLocked()) {
					client->SimpleMessage(CHANNEL_COLOR_RED, "You cannot destroy the item in use.");
					return;
				}
				else if(item->IsBag() && item->details.equip_slot_id) {
					client->SimpleMessage(CHANNEL_COLOR_RED, "You cannot destroy the item, it is being used as quiver.");
					return;
				}
				else if(item->CheckFlag(NO_DESTROY)) {
					client->SimpleMessage(CHANNEL_COLOR_RED, "You can't destroy this item.");
					return;
				}
				if(client->GetPlayer()->item_list.IsItemInSlotType(item, InventorySlotType::HOUSE_VAULT) || client->GetPlayer()->item_list.IsItemInSlotType(item, InventorySlotType::BASE_INVENTORY)) {
					broker.RemoveItem(client->GetPlayer()->GetCharacterID(), item->details.unique_id, item->details.count, true);
				}
				if(item->GetItemScript() && lua_interface)
					lua_interface->RunItemScript(item->GetItemScript(), "destroyed", item, client->GetPlayer());
			
				//reobtain item make sure it wasn't removed
				item = player->item_list.GetItemFromIndex(index);
				
				int32 bag_id = 0;
				if(item){
					bag_id = item->details.inv_slot_id;
					database.DeleteItem(client->GetCharacterID(), item, 0);
				}
				client->GetPlayer()->item_list.DestroyItem(index);
				client->GetPlayer()->UpdateInventory(bag_id);
				client->GetPlayer()->CalculateApplyWeight();
				
				client->OpenShopWindow(nullptr); // update the window if it is open
			}
		}
		else if(sep->arg[4][0] && strncasecmp("move", sep->arg[0], 4) == 0 && sep->IsNumber(1) && sep->IsNumber(2) && sep->IsNumber(3) && sep->IsNumber(4))
		{
			int16 from_index = atoi(sep->arg[1]);
			sint16 to_slot = atoi(sep->arg[2]); // don't convert slot since this is inventory not equipment
			sint32 bag_id = atol(sep->arg[3]);
			int8 charges = atoi(sep->arg[4]);
			Item* item = client->GetPlayer()->item_list.GetItemFromIndex(from_index);
			int64 unique_id = 0;
			int16 count = 0;
			if(!item) {
				client->SimpleMessage(CHANNEL_COLOR_RED, "You have no item.");
				return;
			}
			unique_id = item->details.unique_id;
			count = item->details.count;
			if(to_slot == item->details.slot_id && (bag_id == item->details.inv_slot_id)) {
				return;
			}
			if(item->IsItemLocked())
			{
				client->SimpleMessage(CHANNEL_COLOR_RED, "You cannot move the item in use.");
				return;
			}
			if(bag_id == InventorySlotType::SHARED_BANK && !client->GetPlayer()->item_list.SharedBankAddAllowed(item))
			{
				client->SimpleMessage(CHANNEL_COLOR_RED, "That item (or an item inside) cannot be shared.");
				return;
			}
			sint32 old_inventory_id = 0;

			if(item)
				old_inventory_id = item->details.inv_slot_id;

			//autobank
			if (bag_id == InventorySlotType::BANK && to_slot == -1) 
			{ 
				if (player->HasFreeBankSlot())
					to_slot = player->FindFreeBankSlot();
				else 
				{
					client->SimpleMessage(CHANNEL_COLOR_RED, "You do not have any free bank slots.");
					return;
				}
			}

			//auto inventory
			if (bag_id == 0 && to_slot == -1) 
			{
				if (!player->item_list.GetFirstFreeSlot(&bag_id, &to_slot)) 
				{
					client->SimpleMessage(CHANNEL_COLOR_RED, "You do not have any free slots.");
					return;
				}
			}

			bool item_deleted = false;
			EQ2Packet* outapp = client->GetPlayer()->MoveInventoryItem(bag_id, from_index, (int8)to_slot, charges, 0, &item_deleted, client->GetVersion());
			client->QueuePacket(outapp);

			if(item_deleted)
				item = nullptr;

			//removed from bag send update
			if(old_inventory_id > 0 && item && item->details.inv_slot_id != old_inventory_id)
			{ 
				outapp = client->GetPlayer()->SendBagUpdate(old_inventory_id, client->GetVersion());
				if(outapp)
					client->QueuePacket(outapp);
			}

			if(item && item->details.inv_slot_id > 0 && item->details.inv_slot_id != old_inventory_id)
			{
				outapp = client->GetPlayer()->SendBagUpdate(item->details.inv_slot_id, client->GetVersion());

				if(outapp)
					client->QueuePacket(outapp);
			}
			
			client->GetPlayer()->CalculateApplyWeight();
			if(item) {
					if(!client->GetPlayer()->item_list.IsItemInSlotType(item, InventorySlotType::HOUSE_VAULT) && 
					!client->GetPlayer()->item_list.IsItemInSlotType(item, InventorySlotType::BASE_INVENTORY)) {
							broker.RemoveItem(client->GetPlayer()->GetCharacterID(), unique_id, charges);
					}
			}
			else {
				broker.RemoveItem(client->GetPlayer()->GetCharacterID(), unique_id, count);
			}
			client->OpenShopWindow(nullptr); // update the window if it is open
		}
		else if(sep->arg[1][0] && strncasecmp("equip", sep->arg[0], 5) == 0 && sep->IsNumber(1))
		{
				int16 index = atoi(sep->arg[1]);
				int8 slot_id = 255;
				int8 unk3 = 0;
				int8 appearance_equip = 0;
				
				if(sep->arg[2][0] && sep->IsNumber(2))
					slot_id = player->ConvertSlotFromClient(atoi(sep->arg[2]), client->GetVersion());
				if(sep->arg[3][0] && sep->IsNumber(3))
					unk3 = atoul(sep->arg[3]);
				if(sep->arg[4][0] && sep->IsNumber(4))
					appearance_equip = atoul(sep->arg[4]);

				vector<EQ2Packet*> packets = client->GetPlayer()->EquipItem(index, client->GetVersion(), appearance_equip, slot_id);
				EQ2Packet* outapp = 0;

				for(int32 i=0;i<packets.size();i++)
				{
					outapp = packets[i];
					if(outapp)
						client->QueuePacket(outapp);
				}

				client->GetPlayer()->UpdateWeapons();
				EQ2Packet* characterSheetPackets = client->GetPlayer()->GetPlayerInfo()->serialize(client->GetVersion());
				client->QueuePacket(characterSheetPackets);
				
				client->GetPlayer()->CalculateBonuses();
				client->OpenShopWindow(nullptr); // update the window if it is open
		}
		else if (sep->arg[1][0] && strncasecmp("unpack", sep->arg[0], 6) == 0 && sep->IsNumber(1))
		{
			if (client->GetPlayer()->EngagedInCombat())
				client->SimpleMessage(CHANNEL_COLOR_RED, "You may not unpack items while in combat.");
			else {
				int16 index = atoi(sep->arg[1]);
				Item* item = client->GetPlayer()->item_list.GetItemFromIndex(index);
				if (item) {
					if(item->IsItemLocked())
					{
						client->SimpleMessage(CHANNEL_COLOR_RED, "You cannot unpack the item in use.");
						return;
					}
					//	client->GetPlayer()->item_list.DestroyItem(index);
					if (item->item_sets.size() > 0) {
						for (int32 i = 0; i < item->item_sets.size(); i++) {
							ItemSet* set = item->item_sets[i];
							if (set->item_stack_size == 0)
								set->item_stack_size += 1;
							client->AddItem(set->item_id, set->item_stack_size);
						}
					}

				}
				client->RemoveItem(item, 1);

				client->OpenShopWindow(nullptr); // update the window if it is open
			}

		}
		else if(sep->arg[1][0] && strncasecmp("unequip", sep->arg[0], 7) == 0 && sep->IsNumber(1))
		{
			int16 index = player->ConvertSlotFromClient(atoi(sep->arg[1]), client->GetVersion());
			sint32 bag_id = -999;
			int8 to_slot = 255;

			if(sep->arg[3][0])
			{
				if(sep->IsNumber(2))
					bag_id = atol(sep->arg[2]);

				if(sep->IsNumber(3))
					to_slot = atoi(sep->arg[3]);
			}

			sint8 unk4 = 0;
			int8 appearance_equip = 0;
			if(sep->arg[4][0] && sep->IsNumber(4))
				unk4 = atoi(sep->arg[4]);
			if(sep->arg[5][0] && sep->IsNumber(5))
				appearance_equip = atoul(sep->arg[5]);

			vector<EQ2Packet*> packets = client->GetPlayer()->UnequipItem(index, bag_id, to_slot, client->GetVersion(), appearance_equip);
			EQ2Packet* outapp = 0;

			for(int32 i=0;i<packets.size();i++)
			{
				outapp = packets[i];

				if(outapp)
					client->QueuePacket(outapp);
			}

			client->UnequipItem(index, bag_id, to_slot, appearance_equip);
			client->GetPlayer()->CalculateBonuses();
			client->OpenShopWindow(nullptr); // update the window if it is open
		}
		else if(sep->arg[2][0] && strncasecmp("swap_equip", sep->arg[0], 10) == 0 && sep->IsNumber(1) && sep->IsNumber(2))
		{
			if(client->GetPlayer()->EngagedInCombat() && rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Player, AllowPlayerEquipCombat)->GetInt8() == 0) {
				client->SimpleMessage(CHANNEL_COLOR_RED, "You may not swap items while in combat.");
			}
			else {
				int16 index1 = client->GetPlayer()->ConvertSlotFromClient(atoi(sep->arg[1]), client->GetVersion());
				int16 index2 = client->GetPlayer()->ConvertSlotFromClient(atoi(sep->arg[2]), client->GetVersion());
				int8 type = 0;
				if(sep->IsNumber(3))
					type = atoul(sep->arg[3]); // type 0 is combat, 3 = appearance
				
				EQ2Packet* outapp = client->GetPlayer()->SwapEquippedItems(index1, index2, client->GetVersion(), type);

				if(outapp)
					client->QueuePacket(outapp);
				else
				{
					client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Unable to swap items");
					return;
				}
			}
		}
		else if (sep->arg[2][0] && strncasecmp("pop", sep->arg[0], 3) == 0 && sep->IsNumber(1) && sep->IsNumber(2)) 
		{
			sint16 to_slot = atoi(sep->arg[1]);
			sint32 bag_id = atoi(sep->arg[2]);
			Item* item = client->GetPlayer()->item_list.GetOverflowItem();
			if (item) {
				//auto inventory
				if (bag_id == 0 && to_slot == -1) 
				{ 
					if (!player->item_list.GetFirstFreeSlot(&bag_id, &to_slot)) 
					{
						client->SimpleMessage(CHANNEL_ERROR, "You do not have any free slots.");
						return;
					}
					// Set the slot for the item
					item->details.inv_slot_id = bag_id;
					item->details.slot_id = to_slot;
					// Flag the item so it gets saved in its new location
					item->save_needed = true;
					
					// Add the item to its new location
					if(player->item_list.AddItem(item)) {
						// Remove the item from the overflow list
						player->item_list.RemoveOverflowItem(item);
					}

					// Send the inventory update packet
					client->QueuePacket(player->item_list.serialize(player, client->GetVersion()));
					client->OpenShopWindow(nullptr); // update the window if it is open
					return;
				}
				else if (bag_id == InventorySlotType::BANK && to_slot == -1) {
					// Auto Bank
					if (!player->item_list.GetFirstFreeBankSlot(&bag_id, &to_slot)) {
						client->SimpleMessage(CHANNEL_STATUS, "You do not have any free bank slots.");
						return;
					}
					item->details.inv_slot_id = bag_id;
					item->details.slot_id = to_slot;
					item->save_needed = true;
					if(player->item_list.AddItem(item)) {
						player->item_list.RemoveOverflowItem(item);
					}
					client->QueuePacket(player->item_list.serialize(player, client->GetVersion()));
					client->OpenShopWindow(nullptr); // update the window if it is open
				}
				else if (bag_id == InventorySlotType::SHARED_BANK) {
					// Shared Bank
					if (!player->item_list.SharedBankAddAllowed(item)) {
						client->SimpleMessage(CHANNEL_STATUS, "That item (or an item inside) cannot be shared.");
						return;
					}
					Item* tmp_item = player->item_list.GetItem(bag_id, to_slot);
					if (tmp_item) {
						client->SimpleMessage(CHANNEL_STATUS, "You can not place an overflow item into an occupied slot");
						return;
					}
					else {
						item->details.inv_slot_id = bag_id;
						item->details.slot_id = to_slot;
						item->save_needed = true;
						if(player->item_list.AddItem(item)) {
						player->item_list.RemoveOverflowItem(item);
						}
						client->QueuePacket(player->item_list.serialize(player, client->GetVersion()));
						client->OpenShopWindow(nullptr); // update the window if it is open
						return;
					}
				}
				else {
					// Try to get an item from the given bag id and slot id
					Item* tmp_item = player->item_list.GetItem(bag_id, to_slot);
					// Check to see if we got an item, if we do send an error,
					// if we don't put the overflow item into this slot
					if (tmp_item) {
						client->SimpleMessage(CHANNEL_STATUS, "You can not place an overflow item into an occupied slot");
					}
					else {
						item->details.inv_slot_id = bag_id;
						item->details.slot_id = to_slot;
						item->save_needed = true;
						if(player->item_list.AddItem(item)) {
						player->item_list.RemoveOverflowItem(item);
						}
						client->QueuePacket(player->item_list.serialize(player, client->GetVersion()));
						client->OpenShopWindow(nullptr); // update the window if it is open
						return;
					}
				}
			}
		}
		else if(sep->arg[2][0] && strncasecmp("nosale", sep->arg[0], 6) == 0 && sep->IsNumber(1) && sep->IsNumber(2))
		{
			sint64 data = strtoull(sep->arg[1], NULL, 0);
			
			int32 character_item_id = (int32) (data >> 32);
			int32 item_id = (int32) (data & 0xffffffffL);

			int8 sale_setting = atoi(sep->arg[2]);
			Item* item = client->GetPlayer()->item_list.GetItemFromUniqueID(character_item_id);
			if(item)
			{
				item->no_sale = sale_setting;
				item->save_needed = true;
				client->SendSellMerchantList();
			}
		}
	}
	else
		client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Usage: /inventory {destroy|move|equip|unequip|swap_equip|pop} {item_id} [to_slot] [bag_id]");

}

/* 
	Function: Command_Languages()
	Purpose	: Show's languages the player knows
	Params	: 
	Dev		: Zcoretri
	Example	: 
*/ 
void Commands::Command_Languages(Client* client, Seperator* sep)
{
	list<Language*>* languages = client->GetPlayer()->GetPlayerLanguages()->GetAllLanguages();
	list<Language*>::iterator itr;
	Language* language;
	client->Message(CHANNEL_NARRATIVE, "You know the following languages:");

	for(itr = languages->begin(); itr != languages->end(); itr++)
	{
		language = *itr;
		client->Message(CHANNEL_NARRATIVE, "%s", language->GetName());
	}
}

/* 
	Function: Command_SetLanguage()
	Purpose	: Handles language commands
	Params	: Language ID
	Dev		: Zcoretri
	Example	: 
*/ 
void Commands::Command_SetLanguage(Client* client, Seperator* sep)
{
	Player* player = client->GetPlayer();

	if (sep && sep->arg[0])
	{
		if(!sep->IsNumber(0))
		{
			//String passed in
			const char* value = sep->arg[0];

			if(strncasecmp(value, "Common", strlen(value)) == 0)
			{
				database.SaveCharacterCurrentLang(0, client->GetCharacterID(), client);
				client->SendLanguagesUpdate(0);
				client->Message(CHANNEL_NARRATIVE, "You are now speaking %s", value);
			}
			else
			{
				if(player->HasLanguage(value))
				{
					Language* language = player->GetPlayerLanguages()->GetLanguageByName(value);
					database.SaveCharacterCurrentLang(language->GetID(), client->GetCharacterID(), client);
					client->SendLanguagesUpdate(language->GetID());
					client->Message(CHANNEL_NARRATIVE, "You are now speaking %s", language->GetName());
				}
				else
					client->Message(CHANNEL_NARRATIVE, "You do not know how to speak %s", value);
			}
		}
		else
		{
			//Number passed in
			int32 id = atoul(sep->arg[0]);

			if(player->HasLanguage(id))
			{
				Language* language = player->GetPlayerLanguages()->GetLanguage(id);
				database.SaveCharacterCurrentLang(id, client->GetCharacterID(), client);
				client->SendLanguagesUpdate(id);
				client->Message(CHANNEL_NARRATIVE, "You are now speaking %s", language->GetName());
			}
			else
			{
				Language* language = master_languages_list.GetLanguage(id);
              	if(language)
             	client->Message(CHANNEL_NARRATIVE, "You do not know how to speak %s", language->GetName());
			}
		}
	}
	else
	{
		//No value was passed in
		int32 id = database.GetCharacterCurrentLang(client->GetCharacterID(), player);

		if(id > 0)
		{
			Language* language = player->GetPlayerLanguages()->GetLanguage(id);
			client->Message(CHANNEL_NARRATIVE, "You are currently speaking %s ", language->GetName());
		}
		else
			client->Message(CHANNEL_NARRATIVE, "You are currently speaking Common");
	}
}

/* 
	Function: Command_LastName()
	Purpose	: Sets player surname
	Params	: Name text
	Dev		: theFoof
	Example	: 
*/ 
void Commands::Command_LastName(Client* client, Seperator* sep)
{
	if (!client)
		return;

	if (sep && sep->arg[0])
	{
		if (!client->GetPlayer()->get_character_flag(CF_ENABLE_CHANGE_LASTNAME)){
			client->Message(CHANNEL_COLOR_YELLOW, "You must be atleast level %i to change your last name.", rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Player, MinLastNameLevel)->GetInt8());
			return;
		}
		client->RemovePendingLastName();

		uchar* checkname = (uchar*)sep->arg[0];
		bool valid_name = true;
		for (int32 i = 0; i < strlen(sep->arg[0]); i++) {
			if (!alpha_check(checkname[i])) {
				valid_name = false;
				break;
			}
		}

		if (!valid_name) {
			client->Message(CHANNEL_COLOR_YELLOW, "Your last name can only contain letters.", rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Player, MinLastNameLevel)->GetInt8());
			return;
		}

		string last_name = (string)sep->arg[0];
		int8 max_length = rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Player, MaxLastNameLength)->GetInt8();
		int8 min_length = rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Player, MinLastNameLength)->GetInt8();
		if (last_name.length() <= max_length && last_name.length() >= min_length){
			client->SetPendingLastName(last_name);
			client->SendLastNameConfirmation();
		}
		else
			client->Message(CHANNEL_COLOR_YELLOW, "Your last name must be between %i and %i characters long.", min_length, max_length);
	}
}

/* 
	Function: Command_ConfirmLastName()
	Purpose	: Confirms setting of player surname
	Params	: Name text
	Dev		: theFoof
	Example	: 
*/ 
void Commands::Command_ConfirmLastName(Client* client, Seperator* sep)
{
	if (!client)
		return;

	string* name = client->GetPendingLastName();
	if (name){
		Player* player = client->GetPlayer();
		player->SetLastName(name->c_str(), false);
		client->SendTitleUpdate();
		player->SetCharSheetChanged(true);
		client->RemovePendingLastName();
	}
}

/* 
	Function: Command_Location()
	Purpose	: Display's Help for /location commands
	Params	: 
	Dev		: Scatman
	Example	: /location = show's help for command
*/ 
void Commands::Command_Location(Client* client)
{
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Valid /location commands are:");
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/location create [name] (include y).  Include y defaults to false");
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/location add [location id]");
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/location remove [location point id]");
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/location delete [location id]");
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/location list [locations|points|respawns] [location id if points used]");
}

/* 
	Function: Command_LocationAdd()
	Purpose	: Add's a location to an existing location config
	Params	: location_id
	Dev		: Scatman
	Example	: /location add {location_id}
*/ 
void Commands::Command_LocationAdd(Client* client, Seperator* sep)
{
	if (sep && sep->arg[0] && sep->IsNumber(0)) 
	{
		int32 location_id = atoul(sep->arg[0]);
		float x = client->GetPlayer()->GetX();
		float y = client->GetPlayer()->GetY();
		float z = client->GetPlayer()->GetZ();

		if (database.AddLocationPoint(location_id, x, y, z))
			client->Message(CHANNEL_COLOR_YELLOW, "Point (%f, %f, %f) was successfully added to location %u", x, y, z, location_id);
		else
			client->Message(CHANNEL_COLOR_YELLOW, "A location with ID %u does not exist.  Use /location create to create one", location_id);
	}
	else
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /location add [location id]");
}

/* 
	Function: Command_LocationCreate()
	Purpose	: Creates a new location config
	Params	: location name, 0/1 for include_y (optional)
	Dev		: Scatman 
	Example	: /location create Test 1 = creates a new location named Test with include_y True
*/ 
void Commands::Command_LocationCreate(Client* client, Seperator* sep)
{
	if (sep && sep->arg[0] && strlen(sep->arg[0]) > 0) 
	{
		const char* name = sep->arg[0];
		bool include_y = false;

		if (sep->arg[1] && sep->IsNumber(1) && atoi(sep->arg[1]) > 0)
			include_y = true;

		int32 location_id = database.CreateLocation(client->GetPlayer()->GetZone()->GetZoneID(), client->GetPlayer()->GetLocation(), name, include_y);

		if (location_id > 0)
			client->Message(CHANNEL_COLOR_YELLOW, "Location '%s' was successfully created with location id %u", name, location_id);
		else
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "There was an error creating the requested location");
	}
	else
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /location create [name] (include_y).  Include y defaults to false");
}

/* 
	Function: Command_LocationDelete()
	Purpose	: Delete's a location config and all it's location points
	Params	: location_id
	Dev		: Scatman
	Example	: /location delete {location_id}
*/ 
void Commands::Command_LocationDelete(Client* client, Seperator* sep)
{
	if (sep && sep->arg[0] && sep->IsNumber(0))
	{
		int32 location_id = atoul(sep->arg[0]);

		if (database.DeleteLocation(location_id))
			client->Message(CHANNEL_COLOR_YELLOW, "Location id %u and all its points were successfully deleted", location_id);
		else
			client->Message(CHANNEL_COLOR_YELLOW, "A location with ID %u does not exist", location_id);
	}
	else
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /location delete [location id]");
}

/* 
	Function: Command_LocationList()
	Purpose	: Display's a list of location points
	Params	: location_id
	Dev		: Scatman
	Example	: /location list {location_id}
*/ 
void Commands::Command_LocationList(Client* client, Seperator* sep)
{
	if (sep && sep->arg[0]) 
	{
		const char* option = sep->arg[0];

		if (strncmp(option, "locations", strlen(option)) == 0)
			database.ListLocations(client);
		else if (strncmp(option, "respawns", strlen(option)) == 0)
			client->GetPlayer()->GetZone()->SendRespawnTimerList(client);
		else if (strncmp(option, "points", strlen(option)) == 0 && sep->arg[1] && sep->IsNumber(1)) 
		{
			int32 location_id = atoul(sep->arg[1]);
			database.ListLocationPoints(client, location_id);
		}
		else
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Useage: /location list [locations|points|respawns] [location ID if points used]");
	}
	else
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Useage: /location list [locations|points|respawns] [location ID if points used]");
}

/* 
	Function: Command_LocationRemove()
	Purpose	: Removes a single location point from a location config
	Params	: location_point_id (gotten from /location list {id})
	Dev		: Scatman
	Example	: /location remove 1 = will remove location_point_id 1
*/ 
void Commands::Command_LocationRemove(Client* client, Seperator* sep)
{
	if (sep && sep->arg[0] && sep->IsNumber(0)) 
	{
		int32 location_point_id = atoul(sep->arg[0]);

		if (database.DeleteLocationPoint(location_point_id))
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Location point was successfully deleted");
		else
			client->Message(CHANNEL_COLOR_YELLOW, "Location point with ID %u does not exist", location_point_id);
	}
	else
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /location remove [location point id]");
}

/* 
	Function: Command_Merchant()
	Purpose	: Handles Merchant commands
	Params	: list
	Dev		: Scatman
	Example	: /merchant list
*/ 
void Commands::Command_Merchant(Client* client, Seperator* sep, int handler)
{
	if( handler == COMMAND_MERCHANT )
	{
		// /merchant command by itself shows help (to be extended)
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /merchant list [merchant description]");
		return;
	}
	else if( handler == COMMAND_MERCHANT_LIST )
	{
		// /merchant list command expects "description" param
		if (sep && sep->arg[0])
		{
			const char* merchant_description = sep->argplus[0];
			client->Message(CHANNEL_COLOR_YELLOW, "Listing merchants like '%s':", merchant_description);
			map<int32, MerchantInfo*>* merchant_info = world.GetMerchantInfo();
			map<int32, MerchantInfo*>::iterator itr;

			for (itr = merchant_info->begin(); itr != merchant_info->end(); itr++) 
			{
				string description = database.GetMerchantDescription(itr->first);

				if (ToLower(description).find(ToLower(string(merchant_description))) < 0xFFFFFFFF)
					client->Message(CHANNEL_COLOR_YELLOW, "Merchant ID: %u, Description: %s", itr->first, description.c_str());
			}
		}
		else // no description
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /merchant list [merchant description]");
	}
}

/* 
	Function: Command_Modify()
	Purpose	: to replace our other "set" commands
	Params	: System = the system to modify, Action = the action to perform, Target = what to change (target, or index)
	Dev		: John Adams
	Example	: /modify spell set name "Aegolism III"
*/ 
void Commands::Command_Modify(Client* client)
{
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /modify [system] [action] [field] [value] {target|id}");
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Systems: character, faction, guild, item, skill, spawn, spell, zone");
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Actions: set, create, delete, add, remove");
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Value  : field name in the table being modified");
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Target : modify current target, or specified {system} ID");
}


/* 
	Function: Command_ModifySpawn()
	Purpose	: replace "spawn set" commands
	Params	: Action	: the action to perform (or special handler)
			: Field		: the DB field to change
			: Value		: what to set the value to
			: Target	: what object to change (my target, or spawn_id)
	Dev		: John Adams
	Example	: /modify spawn set name "Lady Vox"
			:
	Note	: Special Handlers, like "zoneto" for Signs
			:	/modify spawn zoneto
			:	Will set a sign's zone x/y/z to my current coords
*/ 
void Commands::Command_ModifySpawn(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_MODIFY");

	// need at least 2 args for a valid command
	if( sep && sep->arg[1] )
	{
		// JA: just a quick implementation because I need it :)
		if (strcmp(sep->arg[0], "zoneto") == 0) 
		{
			if( sep->IsNumber(1) )
			{
				int32 spawn_id = atoul(sep->arg[1]);
				float x_coord = client->GetPlayer()->GetX();
				float y_coord = client->GetPlayer()->GetY();
				float z_coord = client->GetPlayer()->GetZ();
				float h_coord = client->GetPlayer()->GetHeading();

				database.SaveSignZoneToCoords(spawn_id, x_coord, y_coord, z_coord, h_coord);
			}
			else
				client->SimpleMessage(CHANNEL_COLOR_RED, "Usage: /modify spawn zoneto spawn_id - sets spawn_id to your coords");
		}
	}
	else
		Command_Modify(client);
}

/* 
	Function: Command_ModifyCharacter()
	Purpose	: to replace our other "set" commands
	Params	: Action	: Add, Remove
			: Field		: copper, silver, gold, plat
			: Value		: min 1, max unlimited
			: Target	: Self, Player
	Dev		: Cynnar
	Example	: /modify character add gold 50
			: /modify character remove silver 25
*/
void Commands::Command_ModifyCharacter(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_MODIFY");

	int64 value = 0;

	Player* player = client->GetPlayer();
	Client* targetClient = client;
	if (player->HasTarget() && player->GetTarget()->IsPlayer()) {
		player = (Player*)player->GetTarget();
		targetClient = player->GetClient();
	}

	// need at least 2 args for a valid command
	if( sep && sep->arg[1] )

	{
		if (strcmp(sep->arg[0], "add") == 0)
			
		{
			if (strcmp(sep->arg[1], "copper") == 0)
			{
				value = atoi64(sep->arg[2]);
				player->AddCoins(value);
				if (client->GetPlayer() == player)
				{
					client->Message(CHANNEL_COLOR_YELLOW, "You give yourself %llu copper coin%s", value, (value > 1 ? "s" : ""));
				}
				else {
					client->Message(CHANNEL_COLOR_YELLOW, "You give %s %llu copper coin%s", player->GetName(), value, (value > 1 ? "s" : ""));
					if(targetClient) {
						targetClient->Message(CHANNEL_COLOR_YELLOW, "%s gave you %llu copper coin%s", client->GetPlayer()->GetName(), value, (value > 1 ? "s" : ""));
					}
				}
			}

			else if (strcmp(sep->arg[1], "silver") == 0)
			{
				value = atoi64(sep->arg[2]) * 100;
				player->AddCoins(value);
				if (client->GetPlayer() == player)
				{
					client->Message(CHANNEL_COLOR_YELLOW, "You give yourself %llu silver coin%s", value / 100, (value > 1 ? "s" : ""));
				}
				else {
					client->Message(CHANNEL_COLOR_YELLOW, "You give %s %llu silver coin%s", player->GetName(), value / 100, (value > 1 ? "s" : ""));
					if(targetClient) {
						targetClient->Message(CHANNEL_COLOR_YELLOW, "%s gave you %llu silver coin%s", client->GetPlayer()->GetName(), value / 100, (value > 1 ? "s" : ""));
					}
				}
			}

			else if (strcmp(sep->arg[1], "gold") == 0)
			{
				value = atoi64(sep->arg[2]) * 10000;
				player->AddCoins(value);
				if (client->GetPlayer() == player)
				{
					client->Message(CHANNEL_COLOR_YELLOW, "You give yourself %llu gold coin%s", value / 10000, (value > 1 ? "s" : ""));
				}
				else {
					client->Message(CHANNEL_COLOR_YELLOW, "You give %s %llu gold coin%s", player->GetName(), value / 10000, (value > 1 ? "s" : ""));
					if(targetClient) {
						targetClient->Message(CHANNEL_COLOR_YELLOW, "%s gave you %llu gold coin%s", client->GetPlayer()->GetName(), value / 10000, (value > 1 ? "s" : ""));
					}
				}
			}

			else if (strcmp(sep->arg[1], "plat") == 0)
			{
				value = atoi64(sep->arg[2]) * 1000000;
				player->AddCoins(value);
				if (client->GetPlayer() == player)
				{
					client->Message(CHANNEL_COLOR_YELLOW, "You give yourself %llu platinum coin%s", value / 1000000, (value > 1 ? "s" : ""));
				}
				else {
					client->Message(CHANNEL_COLOR_YELLOW, "You give %s %llu platinum coin%s", player->GetName(), value / 1000000, (value > 1 ? "s" : ""));
					if(targetClient) {				
						targetClient->Message(CHANNEL_COLOR_YELLOW, "%s gave you %llu platinum coin%s", client->GetPlayer()->GetName(), value / 1000000, (value > 1 ? "s" : ""));
					}
				}
			}

			else
			{
				client->SimpleMessage(CHANNEL_COLOR_RED, "Usage: /modify character [action] [field] [value]");
				client->SimpleMessage(CHANNEL_COLOR_RED, "Actions: add, remove");
				client->SimpleMessage(CHANNEL_COLOR_RED, "Value  : copper, silver, gold, plat");
				client->SimpleMessage(CHANNEL_COLOR_RED, "Example: /modify character add copper 20");
			}
		}

		else if (strcmp(sep->arg[0], "remove") == 0)
		{
			if (strcmp(sep->arg[1], "copper") == 0)
			{
				value = atoi64(sep->arg[2]);
				player->RemoveCoins(value);
				if (client->GetPlayer() == player)
				{
					client->Message(CHANNEL_COLOR_YELLOW, "You take %llu copper coin%s from yourself", value, (value > 1 ? "s" : ""));
				}
				else {
					client->Message(CHANNEL_COLOR_YELLOW, "You take %llu copper coin%s from %s", value, (value > 1 ? "s" : ""), player->GetName());
					if(targetClient) {				
						targetClient->Message(CHANNEL_COLOR_YELLOW, "%s takes %llu copper coin%s from you", client->GetPlayer()->GetName(), value, (value > 1 ? "s" : ""));
					}
				}
			}

			else if (strcmp(sep->arg[1], "silver") == 0)
			{
				value = atoi64(sep->arg[2]) * 100;
				player->RemoveCoins(value);
				if (client->GetPlayer() == player)
				{
					client->Message(CHANNEL_COLOR_YELLOW, "You take %llu silver coin%s from yourself", value / 100, (value > 1 ? "s" : ""));
				}
				else {
					client->Message(CHANNEL_COLOR_YELLOW, "You take %llu silver coin%s from %s", value / 100, (value > 1 ? "s" : ""), player->GetName());
					if(targetClient) {				
						targetClient->Message(CHANNEL_COLOR_YELLOW, "%s takes %llu silver coin%s from you", client->GetPlayer()->GetName(), value / 100, (value > 1 ? "s" : ""));
					}
				}
			}

			else if (strcmp(sep->arg[1], "gold") == 0)
			{
				value = atoi64(sep->arg[2]) * 10000;
				player->RemoveCoins(value);
				if (client->GetPlayer() == player)
				{
					client->Message(CHANNEL_COLOR_YELLOW, "You take %llu gold coin%s from yourself", value / 10000, (value > 1 ? "s" : ""));
				}
				else {
					client->Message(CHANNEL_COLOR_YELLOW, "You take %llu gold coin%s from %s", value / 10000, (value > 1 ? "s" : ""), player->GetName());
					if(targetClient) {
						targetClient->Message(CHANNEL_COLOR_YELLOW, "%s takes %llu gold coin%s from you", client->GetPlayer()->GetName(), value / 10000, (value > 1 ? "s" : ""));
					}
				}
			}

			else if (strcmp(sep->arg[1], "plat") == 0)
			{
				value = atoi64(sep->arg[2]) * 1000000;
				player->RemoveCoins(value);
				if (client->GetPlayer() == player)
				{
					client->Message(CHANNEL_COLOR_YELLOW, "You take %llu platinum coin%s from yourself", value / 1000000, (value > 1 ? "s" : ""));
				}
				else {
					client->Message(CHANNEL_COLOR_YELLOW, "You take %llu platinum coin%s from %s", value / 1000000, (value > 1 ? "s" : ""), player->GetName());
					if(targetClient) {
						targetClient->Message(CHANNEL_COLOR_YELLOW, "%s takes %llu platinum coin%s from you", client->GetPlayer()->GetName(), value / 1000000, (value > 1 ? "s" : ""));
					}
				}
			}

			else
			{
				client->SimpleMessage(CHANNEL_COLOR_RED, "Example: /modify character remove gold 15");
				
			}
		}

		else if (strcmp(sep->arg[0], "set") == 0) {

			if (strcmp(sep->arg[1], "tslevel") == 0) {
				int8 level = atoi(sep->arg[2]);

				if (level > 0 && level < 256) {
					if (player) {
						if (client->GetPlayer() == player)
							client->ChangeTSLevel(player->GetTSLevel(), level);
						else if(targetClient)
							targetClient->ChangeTSLevel(player->GetTSLevel(), level);
					}
				}
				else
					client->SimpleMessage(CHANNEL_ERROR, "Level must be between 1 - 255");
			}

			else if (strcmp(sep->arg[1], "tsclass") == 0) {
				int8 tsclass = atoi(sep->arg[2]);

				player->SetTradeskillClass(tsclass);
				player->GetInfoStruct()->set_tradeskill_class1(classes.GetTSBaseClass(player->GetTradeskillClass()));
				player->GetInfoStruct()->set_tradeskill_class2(classes.GetSecondaryTSBaseClass(player->GetTradeskillClass()));
				player->GetInfoStruct()->set_tradeskill_class3(player->GetTradeskillClass());
				player->SetCharSheetChanged(true);
			}
			else if (strcmp(sep->arg[1], "gender") == 0) {
				int8 gender = atoi(sep->arg[2]);
				client->GetPlayer()->GetInfoStruct()->set_gender(gender);
				client->GetPlayer()->SetCharSheetChanged(true);
				client->UpdateTimeStampFlag ( GENDER_UPDATE_FLAG );
			}
		}
	}
	else
	{
		client->SimpleMessage(CHANNEL_COLOR_RED, "Usage: /modify character [action] [field] [value]");
		client->SimpleMessage(CHANNEL_COLOR_RED, "Actions: add, remove");
		client->SimpleMessage(CHANNEL_COLOR_RED, "Value  : copper, silver, gold, plat");
		client->SimpleMessage(CHANNEL_COLOR_RED, "Example: /modify character remove gold 15");
	}
		
}


void Commands::Command_ModifyFaction(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_MODIFY");

	// need at least 2 args for a valid command
	if( sep && sep->arg[1] )
	{
	}
	else
		Command_Modify(client);
}


void Commands::Command_ModifyGuild(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_MODIFY");

	// need at least 2 args for a valid command
	if( sep && sep->arg[1] )
	{
	}
	else
		Command_Modify(client);
}


void Commands::Command_ModifyItem(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_MODIFY");

	// need at least 2 args for a valid command
	if( sep && sep->arg[1] )
	{
	}
	else
		Command_Modify(client);
}

/* 
	Function: Command_ModifyQuest()
	Purpose	: to list players quest and completed quests
	Params	: Action	: list, completed
			: Target	: Self, Player
	Dev		: Cynnar
	Example	: /modify quest list
			: /modify quest completed
*/

void Commands::Command_ModifyQuest(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_MODIFY");

	int64 value = 0;

	Player* player = client->GetPlayer();
	Client* targetClient = client;
	if (player->HasTarget() && player->GetTarget()->IsPlayer()) {
		player = (Player*)player->GetTarget();
		targetClient = player->GetClient();
	}

	// need at least 2 args for a valid command

	if (sep && sep->arg[1])
	{

		if (strcmp(sep->arg[0], "list") == 0)
		{
			map<int32, Quest*>* quests = player->GetPlayerQuests();
			map<int32, Quest*>::iterator itr;
			client->Message(CHANNEL_COLOR_YELLOW, "%s's Quest List:.", client->GetPlayer()->GetName());
			if (quests->size() == 0)
				client->Message(CHANNEL_COLOR_YELLOW, "%s has no quests.", client->GetPlayer()->GetName());
			else
			{
				for (itr = quests->begin(); itr != quests->end(); itr++)
				{
					Quest* quest = itr->second;
					if(quest) {
						client->Message(CHANNEL_COLOR_YELLOW, "%u) %s", itr->first, quest->GetName());
					}
				}
			}

		}

		else if (strcmp(sep->arg[0], "completed") == 0)
		{

			map<int32, Quest*>* quests = player->GetCompletedPlayerQuests();
			map<int32, Quest*>::iterator itr;
			client->Message(CHANNEL_COLOR_YELLOW, "%s's Completed Quest List:.", client->GetPlayer()->GetName());
			if (quests->size() == 0)
				client->Message(CHANNEL_COLOR_YELLOW, "%s has no completed quests.", client->GetPlayer()->GetName());
			else
			{
				for (itr = quests->begin(); itr != quests->end(); itr++)
				{
					Quest* quest = itr->second;
					if(quest) {
						client->Message(CHANNEL_COLOR_YELLOW, "%u) %s", itr->first, quest->GetName());
					}
				}
			}

		}
		// Add in a progress step, and a LogWrite() for tracking GM Commands.
		// LogWrite(LUA__DEBUG, 0, "LUA", "Quest: %s, function: %s", quest->GetName(), function);
		else if (strcmp(sep->arg[0], "remove") == 0)
		{
			int32 quest_id = 0;

			if (sep && sep->arg[1] && sep->IsNumber(1))
				quest_id = atoul(sep->arg[1]);

			if (quest_id > 0)
			{
				if (lua_interface && client->GetPlayer()->player_quests.count(quest_id) > 0)
				{
					Quest* quest = client->GetPlayer()->player_quests[quest_id];
					if (quest)
						if (client->GetPlayer() == player)
						{
							client->Message(CHANNEL_COLOR_YELLOW, "The quest %s has been removed from your journal", quest->GetName());
						}
						else
						{
							client->Message(CHANNEL_COLOR_YELLOW, "You have removed the quest %s from %s's journal", quest->GetName(), player->GetName());
							if(targetClient) {
								targetClient->Message(CHANNEL_COLOR_YELLOW, "%s has removed the quest %s from your journal", client->GetPlayer()->GetName(), quest->GetName());
							}
						}
					LogWrite(COMMAND__INFO, 0, "GM Command", "%s removed the quest %s from %s", client->GetPlayer()->GetName(), quest->GetName(), player->GetName());
					lua_interface->CallQuestFunction(quest, "Deleted", client->GetPlayer());
				}
				client->RemovePlayerQuest(quest_id);
				client->GetCurrentZone()->SendQuestUpdates(client);
			}
		}

		else if (strcmp(sep->arg[0], "advance") == 0)
		{
			int32 quest_id = 0;
			int32 step = 0;

			if (sep && sep->arg[1] && sep->IsNumber(1))
			{
				quest_id = atoul(sep->arg[1]);
				Quest* quest = client->GetPlayer()->player_quests[quest_id];

				if(!quest) {
					client->Message(CHANNEL_COLOR_RED, "Quest not found!");
					return;
				}
				if (sep && sep->arg[2] && sep->IsNumber(1))
				{
					step = atoul(sep->arg[2]);

					if (quest_id > 0 && step > 0)
					{
						if (player && player->IsPlayer() && quest_id > 0 && step > 0 && (player->player_quests.count(quest_id) > 0))
						{
							if (client)
							{
								client->AddPendingQuestUpdate(quest_id, step);
								client->Message(CHANNEL_COLOR_YELLOW, "The quest %s has been advanced one step.", quest->GetName());
								LogWrite(COMMAND__INFO, 0, "GM Command", "%s advanced the quest %s one step", client->GetPlayer()->GetName(), quest->GetName());
							}
						}
					}
					else
					{
						client->Message(CHANNEL_COLOR_RED, "Quest ID and Step Number must be greater than 0!");
					}
				}
				else
				{
					client->Message(CHANNEL_COLOR_RED, "Step Number must be a number!");
				}
			}
			else
			{
				client->Message(CHANNEL_COLOR_RED, "Quest ID must be a number!");
			}
		}

		else
		{
			Command_Modify(client);
		}
	}

	else
	{
		client->SimpleMessage(CHANNEL_COLOR_RED, "Usage: /modify quest [action] [quest id] [step number]");
		client->SimpleMessage(CHANNEL_COLOR_RED, "Actions: list, completed, remove, advance");
		client->SimpleMessage(CHANNEL_COLOR_RED, "Example: /modify quest list");
		client->SimpleMessage(CHANNEL_COLOR_RED, "Example: /modify quest remove 156");
		client->SimpleMessage(CHANNEL_COLOR_RED, "Example: /modify quest advance 50 1");
	}

}


void Commands::Command_ModifySkill(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_MODIFY");

	// need at least 2 args for a valid command
	if( sep && sep->arg[1] )
	{
		if (strcmp(sep->arg[0], "add") == 0) {
			const char* skill_name = sep->argplus[1];
			Skill* skill = master_skill_list.GetSkillByName(skill_name);

			if (skill) {
				Player* player = 0;
				Client* to_client = 0;

				if (client->GetPlayer()->GetTarget() && client->GetPlayer()->GetTarget()->IsPlayer()) {
					player = (Player*)client->GetPlayer()->GetTarget();
					to_client = player->GetClient();
				}
				else {
					player = client->GetPlayer();
					to_client = client;
				}

				if (player)
				{
					if (!player->GetSkills()->HasSkill(skill->skill_id))
					{
						player->AddSkill(skill->skill_id, 1, player->GetLevel() * 5, true);
						if (to_client == client) {
							client->Message(CHANNEL_STATUS, "Added skill '%s'.", skill_name);
						}
						else {
							client->Message(CHANNEL_STATUS, "You gave skill '%s' to player '%s'.", skill_name, player->GetName());
							to_client->Message(CHANNEL_STATUS, "%s gave you skill '%s'.", client->GetPlayer()->GetName(), skill_name);
						}
					}
					else
						client->Message(CHANNEL_STATUS, "%s already has the skill '%s'.", to_client == client ? "You" : player->GetName(), skill_name);
				}
			}
			else
				client->Message(CHANNEL_STATUS, "The skill '%s' does not exist.", skill_name);
		}
		else if (strcmp(sep->arg[0], "remove") == 0) {
			const char* skill_name = sep->argplus[1];
			Skill* skill = master_skill_list.GetSkillByName(skill_name);

			if (skill) {
				Player* player = 0;
				Client* to_client = 0;

				if (client->GetPlayer()->GetTarget() && client->GetPlayer()->GetTarget()->IsPlayer()) {
					player = (Player*)client->GetPlayer()->GetTarget();
					to_client = player->GetClient();
				}
				else {
					player = client->GetPlayer();
					to_client = client;
				}

				if (player)
				{
					if (player->GetSkills()->HasSkill(skill->skill_id))
					{
						player->RemoveSkillFromDB(skill, true);
						if (client == to_client) {
							client->Message(CHANNEL_STATUS, "Removed skill '%s'.", skill_name);
						}
						else {
							client->Message(CHANNEL_STATUS, "Removed skill '%s' from player %s.", skill_name, player->GetName());
							if(to_client) {
								to_client->Message(CHANNEL_STATUS, "%s has removed skill '%s' from you.", client->GetPlayer()->GetName(), skill_name);
							}
						}
					}
					else {
						if (client == to_client)
							client->Message(CHANNEL_STATUS, "You do not have the skill '%s'.", skill_name);
						else
							client->Message(CHANNEL_STATUS, "Player '%s' does not have the skill '%s'.", player->GetName(), skill_name);
					}
				}
			}
			else
				client->Message(CHANNEL_STATUS, "The skill '%s' does not exist.", skill_name);
		}
		else if (strcmp(sep->arg[0], "set") == 0) {
			if (!sep->IsNumber(2)) {
				client->Message(CHANNEL_STATUS, "The last parameter must be a number.");
				return;
			}

			const char* skill_name = sep->arg[1];
			Skill* skill = master_skill_list.GetSkillByName(skill_name);
			if (skill) {
				int16 val = atoi(sep->arg[2]);
				Player* player = 0;
				Client* to_client = 0;

				if (client->GetPlayer()->GetTarget() && client->GetPlayer()->GetTarget()->IsPlayer()) {
					player = (Player*)client->GetPlayer()->GetTarget();
					to_client = player->GetClient();
				}
				else {
					player = client->GetPlayer();
					to_client = client;
				}

				if (player)
				{
					if (player->GetSkills()->HasSkill(skill->skill_id)) {
						player->GetSkills()->SetSkill(skill->skill_id, val);
						if (client != to_client)
							client->Message(CHANNEL_STATUS, "You set %s's '%s' skill to %i.", player->GetName(), skill_name, val);
						if(to_client) {
							to_client->Message(CHANNEL_STATUS, "Your '%s' skill has been set to %i.", skill_name, val);
						}
					}
					else {
						client->Message(CHANNEL_STATUS, "Target does not have the skill '%s'.", skill_name);
					}
				}
			}
			else {
				client->Message(CHANNEL_STATUS, "The skill '%s' does not exist.", skill_name);
			}
		}
		else {
			client->SimpleMessage(CHANNEL_STATUS, "Usage: /modify skill [action] [skill]");
			client->SimpleMessage(CHANNEL_STATUS, "Actions: add, remove, set");
			client->SimpleMessage(CHANNEL_STATUS, "Example: /modify skill add parry");
		}
	}
	else {
		client->SimpleMessage(CHANNEL_STATUS, "Usage: /modify skill [action] [skill]");
		client->SimpleMessage(CHANNEL_STATUS, "Actions: add, remove, set");
		client->SimpleMessage(CHANNEL_STATUS, "Example: /modify skill add parry");
	}
}


void Commands::Command_ModifySpell(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_MODIFY");

	// need at least 2 args for a valid command
	if( sep && sep->arg[1] )
	{
	}
	else
		Command_Modify(client);
}


void Commands::Command_ModifyZone(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_MODIFY");

	// need at least 2 args for a valid command
	if( sep && sep->arg[1] )
	{
	}
	else
		Command_Modify(client);
}


/* 
	Function: Command_MOTD()
	Purpose	: Displays server MOTD
	Params	: 
	Dev		: LethalEncounter
	Example	: /motd
*/ 
void Commands::Command_MOTD(Client* client)
{
	if (client)
		ClientPacketFunctions::SendMOTD(client);
}

/* 
	Function: Command_Pet()
	Purpose	: Handle Pet {Command} commands
	Params	: attack, backoff, preserve_master, preserve_self, follow, stay, getlost
	Dev		: 
	Example	: /pet preserve_master
*/ 
void Commands::Command_Pet(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_PET");
	//LogWrite(MISC__TODO, 1, "Command", "TODO-Command: Pet Commands");
	//client->Message(CHANNEL_COLOR_YELLOW, "Pets are not yet implemented.");


	if (!sep || !sep->arg[0])
		return; // should have sep->arg[0] filled

	if (strcmp(sep->arg[0], "hide") == 0) {
		// doing /pet hide will toggle the hide status on all the pets that can be hidden
		Entity* pet = client->GetPlayer()->GetDeityPet();
		if (pet) {
			if (pet->IsPrivateSpawn())
				client->GetPlayer()->HideDeityPet(false);
			else
				client->GetPlayer()->HideDeityPet(true);
		}

		pet = client->GetPlayer()->GetCosmeticPet();
		if (pet) {
			if (pet->IsPrivateSpawn())
				client->GetPlayer()->HideCosmeticPet(false);
			else
				client->GetPlayer()->HideCosmeticPet(true);
		}

		return;
	}

	// below is for all combat pets
	if (!client->GetPlayer()->HasPet()) {
		client->Message(CHANNEL_COLOR_YELLOW, "You do not have a pet.");
		return;
	}
	
	if (strcmp(sep->arg[0], "stay") == 0 || strcmp(sep->arg[0], "stayhere") == 0) {
		client->Message(CHANNEL_COLOR_YELLOW, "You command your pet to stay.");
		client->GetPlayer()->GetInfoStruct()->set_pet_movement(1);
		client->GetPlayer()->SetCharSheetChanged(true);
		if (client->GetPlayer()->GetPet())
			client->GetPlayer()->GetPet()->following = false;
		if (client->GetPlayer()->GetCharmedPet())
			client->GetPlayer()->GetCharmedPet()->following = false;
	}
	else if (strcmp(sep->arg[0], "follow") == 0 || strcmp(sep->arg[0], "followme") == 0) {
		client->Message(CHANNEL_COLOR_YELLOW, "You command your pet to follow.");
		client->GetPlayer()->GetInfoStruct()->set_pet_movement(2);
		client->GetPlayer()->SetCharSheetChanged(true);
	}
	else if (strcmp(sep->arg[0], "preserve_master") == 0 || strcmp(sep->arg[0], "guardme") == 0) {
		if (client->GetPlayer()->GetInfoStruct()->get_pet_behavior() & 1) {
			client->Message(CHANNEL_COLOR_YELLOW, "Your pet will no longer protect you.");
			client->GetPlayer()->GetInfoStruct()->set_pet_behavior(client->GetPlayer()->GetInfoStruct()->get_pet_behavior()-1);
		}
		else {
			client->Message(CHANNEL_COLOR_YELLOW, "You command your pet to protect you.");
			client->GetPlayer()->GetInfoStruct()->set_pet_behavior(client->GetPlayer()->GetInfoStruct()->get_pet_behavior()+1);
		}
		client->GetPlayer()->SetCharSheetChanged(true);
	}
	else if (strcmp(sep->arg[0], "preserve_self") == 0 || strcmp(sep->arg[0], "guardhere") == 0) { // guardhere might be accurate, diff logic
		if (client->GetPlayer()->GetInfoStruct()->get_pet_behavior() & 2) {
			client->Message(CHANNEL_COLOR_YELLOW, "Your pet will no longer protect itself.");
			client->GetPlayer()->GetInfoStruct()->set_pet_behavior(client->GetPlayer()->GetInfoStruct()->get_pet_behavior()-2);
		}
		else {
			client->Message(CHANNEL_COLOR_YELLOW, "You command your pet to protect itself.");
			client->GetPlayer()->GetInfoStruct()->set_pet_behavior(client->GetPlayer()->GetInfoStruct()->get_pet_behavior()+2);
		}
		client->GetPlayer()->SetCharSheetChanged(true);
	}
	else if (strcmp(sep->arg[0], "backoff") == 0) {
		client->Message(CHANNEL_COLOR_YELLOW, "You command your pet to back down.");
		if (client->GetPlayer()->GetPet()) {
			((NPC*)client->GetPlayer()->GetPet())->Brain()->ClearHate();
					
			client->GetPlayer()->GetPet()->SetFollowTarget(nullptr);
			client->GetPlayer()->GetZone()->movementMgr->StopNavigation(client->GetPlayer()->GetPet());
			client->GetPlayer()->GetPet()->ClearRunningLocations();
			client->GetPlayer()->GetPet()->StopMovement();
		}
		if (client->GetPlayer()->GetCharmedPet()) {
			((NPC*)client->GetPlayer()->GetCharmedPet())->Brain()->ClearHate();
					
			client->GetPlayer()->GetCharmedPet()->SetFollowTarget(nullptr);
			client->GetPlayer()->GetZone()->movementMgr->StopNavigation(client->GetPlayer()->GetCharmedPet());
			client->GetPlayer()->GetCharmedPet()->ClearRunningLocations();
			client->GetPlayer()->GetCharmedPet()->StopMovement();
		}
		client->GetPlayer()->GetInfoStruct()->set_pet_behavior(0);
		client->GetPlayer()->SetCharSheetChanged(true);
	}
	else if (strcmp(sep->arg[0], "attack") == 0) {
		if (client->GetPlayer()->HasTarget() && client->GetPlayer()->GetTarget()->IsEntity()) {
			if (client->GetPlayer()->AttackAllowed((Entity*)client->GetPlayer()->GetTarget())){
				client->Message(CHANNEL_COLOR_YELLOW, "You command your pet to attack your target.");
				if (client->GetPlayer()->GetPet())
					client->GetPlayer()->GetPet()->AddHate((Entity*)client->GetPlayer()->GetTarget(), 1, true);
				if (client->GetPlayer()->GetCharmedPet())
					client->GetPlayer()->GetCharmedPet()->AddHate((Entity*)client->GetPlayer()->GetTarget(), 1, true);
			}
			else
				client->Message(CHANNEL_COLOR_YELLOW, "You can not attack that.");
		}
		else
			client->Message(CHANNEL_COLOR_YELLOW, "You do not have a target.");

	}
	else if (strcmp(sep->arg[0], "getlost") == 0) {
		client->GetPlayer()->DismissPet((NPC*)client->GetPlayer()->GetPet());
		client->GetPlayer()->DismissPet((NPC*)client->GetPlayer()->GetCharmedPet());

		client->Message(CHANNEL_COLOR_YELLOW, "You tell your pet to get lost.");
	}
	else
		client->Message(CHANNEL_COLOR_YELLOW, "Unknown pet command %s.", sep->arg[0]);
	
}

/* 
	Function: Command_PetName()
	Purpose	: Pet your name (???)
	Params	: 
	Dev		: 
	Example	: 
*/ 
void Commands::Command_PetName(Client* client, Seperator* sep)
{
	if (sep && sep->arg[0]) {
		const char* pet_name = sep->argplus[0];
		client->SetPetName(pet_name);
	}
	else {
			client->GetPlayer()->GetInfoStruct()->set_pet_name("");
	}	
}

/* 
	Function: Command_NamePet()
	Purpose	: Name your pet
	Params	: 
	Dev		: 
	Example	: 
*/ 
void Commands::Command_NamePet(Client* client, Seperator* sep)
{
	Command_PetName(client, sep);
}

/* 
	Function: Command_Rename()
	Purpose	: Renames existing pet
	Params	: 
	Dev		: 
	Example	: 
*/ 
void Commands::Command_Rename(Client* client, Seperator* sep)
{
	Command_PetName(client, sep);
}

/* 
	Function: Command_ConfirmRename()
	Purpose	: Confirms renaming your pet
	Params	: 
	Dev		: 
	Example	: 
*/ 
void Commands::Command_ConfirmRename(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_CONFIRMRENAME");
	LogWrite(MISC__TODO, 1, "Command", "TODO-Command: Confirm Rename Pet Command");
	client->Message(CHANNEL_COLOR_YELLOW, "Pets are not yet implemented.");
}

/* 
	Function: Command_PetOptions()
	Purpose	: Sets various pet options
	Params	: 
	Dev		: 
	Example	: 
*/ 
void Commands::Command_PetOptions(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_PETOPTIONS");

	Player* player = client->GetPlayer();
	Spawn* target = player->GetTarget();
	if (!target)
		return;

	if (target == player->GetPet())
		client->SendPetOptionsWindow(player->GetPet()->GetName());
	else if (target == player->GetCharmedPet())
		client->SendPetOptionsWindow(player->GetCharmedPet()->GetName());
	else if (target == player->GetCosmeticPet())
		client->SendPetOptionsWindow(player->GetCosmeticPet()->GetName(), 0);
	else if (target == player->GetDeityPet())
		client->SendPetOptionsWindow(player->GetDeityPet()->GetName(), 0);
}

/* 
	Function: Command_Random()
	Purpose	: Handles /random dice roll in-game
	Params	: 1-100
	Dev		: Scatman
	Example	: /randon 1 100
*/ 
void Commands::Command_Random(Client* client, Seperator* sep)
{
	char message[256] = {0};

	if (sep)
	{
		if (sep->GetArgNumber() == 0 && sep->IsNumber(0))
			sprintf(message, "Random: %s rolls 1 to %i on the magic dice...and scores a %i!", client->GetPlayer()->GetName(), atoi(sep->arg[0]), MakeRandomInt(1, atoi(sep->arg[0])));
		else if (sep->GetArgNumber() > 0 && sep->IsNumber(0) && sep->IsNumber(1))
			sprintf(message, "Random: %s rolls from %i to %i on the magic dice...and scores a %i!", client->GetPlayer()->GetName(), atoi(sep->arg[0]), atoi(sep->arg[1]), MakeRandomInt(atoi(sep->arg[0]), atoi(sep->arg[1])));
		else
			sprintf(message, "Random: %s rolls from 1 to 100 on the magic dice...and scores a %i!", client->GetPlayer()->GetName(), MakeRandomInt(1, 100));
	}
	else
		sprintf(message, "Random: %s rolls from 1 to 100 on the magic dice...and scores a %i!", client->GetPlayer()->GetName(), MakeRandomInt(1, 100));
			
	client->GetPlayer()->GetZone()->HandleChatMessage(0, 0, CHANNEL_EMOTE, message);
}

/* 
	Function: Command_Randomize()
	Purpose	: Sets randomize (appearance) values for NPCs
	Params	: Attrib Name
	Dev		: Scatman
	Example	: /randomize gender 1  -- will randomize the NPCs gender (male/female)
*/ 
void Commands::Command_Randomize(Client* client, Seperator* sep)
{
	NPC* target = (NPC*)client->GetPlayer()->GetTarget();
	if (target) 
	{
		if (target->IsNPC()) 
		{
			if (sep && sep->arg[0] && !sep->IsNumber(0)) 
			{
				const char* value = sep->arg[0];
				if (strncasecmp(value, "show", strlen(value)) == 0) 
				{
					client->DisplayRandomizeFeatures(target->GetRandomize());
				}
				else
				{
					if (sep->arg[1] && sep->IsNumber(1)) 
					{
						int8 option = atoi(sep->arg[1]);
						if (option == 0 || option == 1) 
						{
							int32 feature = 0;
							char feature_text[32] = {0};
							if (strncasecmp(value, "gender", strlen(value)) == 0) {
								feature = RANDOMIZE_GENDER;
								strncpy(feature_text, "gender", sizeof(feature_text) - 1);
							}
							else if (strncasecmp(value, "race", strlen(value)) == 0) {
								feature = RANDOMIZE_RACE;
								strncpy(feature_text, "race", sizeof(feature_text) - 1);
							}
							else if (strncasecmp(value, "model", strlen(value)) == 0) {
								feature = RANDOMIZE_MODEL_TYPE;
								strncpy(feature_text, "model", sizeof(feature_text) - 1);
							}
							else if (strncasecmp(value, "facial_hair", strlen(value)) == 0) {
								feature = RANDOMIZE_FACIAL_HAIR_TYPE;
								strncpy(feature_text, "facial hair", sizeof(feature_text) - 1);
							}
							else if (strncasecmp(value, "hair", strlen(value)) == 0) {
								feature = RANDOMIZE_HAIR_TYPE;
								strncpy(feature_text, "hair", sizeof(feature_text) - 1);
							}
							else if (strncasecmp(value, "wing", strlen(value)) == 0) {
								feature = RANDOMIZE_WING_TYPE;
								strncpy(feature_text, "wings", sizeof(feature_text) - 1);
							}
							else if (strncasecmp(value, "cheek", strlen(value)) == 0) {
								feature = RANDOMIZE_CHEEK_TYPE;
								strncpy(feature_text, "cheeks", sizeof(feature_text) - 1);
							}
							else if (strncasecmp(value, "chin", strlen(value)) == 0) {
								feature = RANDOMIZE_CHIN_TYPE;
								strncpy(feature_text, "chin", sizeof(feature_text) - 1);
							}
							else if (strncasecmp(value, "ear", strlen(value)) == 0) {
								feature = RANDOMIZE_EAR_TYPE;
								strncpy(feature_text, "ears", sizeof(feature_text) - 1);
							}
							else if (strncasecmp(value, "eye_brow", strlen(value)) == 0) {
								feature = RANDOMIZE_EYE_BROW_TYPE;
								strncpy(feature_text, "eye brows", sizeof(feature_text) - 1);
							}
							else if (strncasecmp(value, "eye", strlen(value)) == 0) {
								feature = RANDOMIZE_EYE_TYPE;
								strncpy(feature_text, "eyes", sizeof(feature_text) - 1);
							}
							else if (strncasecmp(value, "lip", strlen(value)) == 0) {
								feature = RANDOMIZE_LIP_TYPE;
								strncpy(feature_text, "lips", sizeof(feature_text) - 1);
							}
							else if (strncasecmp(value, "nose", strlen(value)) == 0) {
								feature = RANDOMIZE_NOSE_TYPE;
								strncpy(feature_text, "nose", sizeof(feature_text) - 1);
							}
							else if (strncasecmp(value, "eye_color", strlen(value)) == 0) {
								feature = RANDOMIZE_EYE_COLOR;
								strncpy(feature_text, "eye color", sizeof(feature_text) - 1);
							}
							else if (strncasecmp(value, "hair_color", strlen(value)) == 0) {
								feature = RANDOMIZE_HAIR_TYPE_COLOR;
								strncpy(feature_text, "hair color", sizeof(feature_text) - 1);
							}
							else if (strncasecmp(value, "hair_color1", strlen(value)) == 0) {
								feature = RANDOMIZE_HAIR_COLOR1;
								strncpy(feature_text, "hair color1", sizeof(feature_text) - 1);
							}
							else if (strncasecmp(value, "hair_color2", strlen(value)) == 0) {
								feature = RANDOMIZE_HAIR_COLOR2;
								strncpy(feature_text, "hair color2", sizeof(feature_text) - 1);
							}
							else if (strncasecmp(value, "hair_highlight", strlen(value)) == 0) {
								feature = RANDOMIZE_HAIR_HIGHLIGHT;
								strncpy(feature_text, "hair highlights", sizeof(feature_text) - 1);
							}
							else if (strncasecmp(value, "facial_hair_color", strlen(value)) == 0) {
								feature = RANDOMIZE_HAIR_FACE_COLOR;
								strncpy(feature_text, "facial hair color", sizeof(feature_text) - 1);
							}
							else if (strncasecmp(value, "facial_hair_color_highlight", strlen(value)) == 0) {
								feature = RANDOMIZE_HAIR_FACE_HIGHLIGHT_COLOR;
								strncpy(feature_text, "facial hair color highlights", sizeof(feature_text) - 1);
							}
							else if (strncasecmp(value, "hair_color_highlight", strlen(value)) == 0) {
								feature = RANDOMIZE_HAIR_TYPE_HIGHLIGHT_COLOR;
								strncpy(feature_text, "hair color highlights", sizeof(feature_text) - 1);
							}
							else if (strncasecmp(value, "skin_color", strlen(value)) == 0) {
								feature = RANDOMIZE_SKIN_COLOR;
								strncpy(feature_text, "skin color", sizeof(feature_text) - 1);
							}
							else if (strncasecmp(value, "wing_color1", strlen(value)) == 0) {
								feature = RANDOMIZE_WING_COLOR1;
								strncpy(feature_text, "wing color1", sizeof(feature_text) - 1);
							}
							else if (strncasecmp(value, "wing_color2", strlen(value)) == 0) {
								feature = RANDOMIZE_WING_COLOR2;
								strncpy(feature_text, "wing color2", sizeof(feature_text) - 1);
							}
							else {
								client->Message(CHANNEL_COLOR_YELLOW, "'%s' is not a valid feature to randomize.", value);
								client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Valid features are: 'gender', 'race', 'model', 'hair', 'facial_hair', 'legs', 'wings', 'cheek', 'chin', 'ear', 'eye', 'eye_brow', 'lip', 'nose', 'eye_color', 'hair_color1', 'hair_color2', 'hair_highlight', 'facial_hair_color', 'facial_hair_color_highlight', 'hair_color', 'hair_color_highlight', 'skin_color', 'wing_color1', 'wing_color2'.");
							}

							if (feature > 0) {
								if (option == 1) {
									if (target->GetRandomize() & feature)
										client->Message(CHANNEL_COLOR_YELLOW, "'%s' is already set to randomly generate their %s.", target->GetName(), feature_text);
									else {
										target->AddRandomize(feature);
										((NPC*)client->GetCurrentZone()->GetSpawn(target->GetDatabaseID()))->AddRandomize(feature);
										database.UpdateRandomize(target->GetDatabaseID(), feature);
										client->Message(CHANNEL_COLOR_YELLOW, "'%s' will now generate their %s randomly.", target->GetName(), feature_text);
									}
								}
								else {
									if (target->GetRandomize() & feature) {
										target->AddRandomize(-feature);
										((NPC*)client->GetCurrentZone()->GetSpawn(target->GetDatabaseID()))->AddRandomize(-feature);
										database.UpdateRandomize(target->GetDatabaseID(), -feature);
										client->Message(CHANNEL_COLOR_YELLOW, "'%s' will no longer generate their %s randomly.", target->GetName(), feature_text);
									}
									else
										client->Message(CHANNEL_COLOR_YELLOW, "'%s' already does not randomly generate their %s.", target->GetName(), feature_text);
								}
							}
						}
						else
							client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You must specify either a 1(on) or 0(off) as the second parameter.");
					}
					else
						client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You must specify either a 1(on) or 0(off) as the second parameter.");
				}
			}
			else {
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /randomize [show | [feature [1|0]]");
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "'show' will display current configured features");
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Valid features are: 'gender', 'race', 'model', 'hair', 'facial_hair', 'legs', 'wings', 'cheek', 'chin', 'ear', 'eye', 'eye_brow', 'lip', 'nose', 'eye_color', 'hair_color1', 'hair_color2', 'hair_highlight', 'facial_hair_color', 'facial_hair_color_highlight', 'hair_color', 'hair_color_highlight', 'skin_color', 'wing_color1', 'wing_color2'.");
			}
		}
		else
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "The target you wish to randomize must be an NPC.");
	}
	else
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You must select the target you wish to randomize.");
}

/* 
	Function: Command_ShowCloak()
	Purpose	: Character Settings combo box for Show Cloak
	Params	: true/false
	Dev		: John Adams
	Example	: 
*/ 
void Commands::Command_ShowCloak(Client* client, Seperator* sep)
{
	Player* player = client->GetPlayer();

	if (sep && sep->arg[0])
	{
		const char* value = sep->arg[0];
		if (strncasecmp(value, "true", strlen(value)) == 0) 
		{
			player->set_character_flag(CF_SHOW_CLOAK);
			player->reset_character_flag(CF2_SHOW_RANGED);
		}
		else if (strncasecmp(value, "false", strlen(value)) == 0) 
			player->reset_character_flag(CF_SHOW_CLOAK);
		else
		{
			client->Message(CHANNEL_COLOR_YELLOW, "Not supposed to be here! Please /bug this: Error in %s (%u)", __FUNCTION__, __LINE__);
			LogWrite(COMMAND__WARNING, 0, "Command", "Not supposed to be here! Please /bug this: Error in %s (%u)", __FUNCTION__, __LINE__);
		}
	}
}

/* 
	Function: Command_ShowHelm()
	Purpose	: Character Settings combo box for Show Helm
	Params	: true/false
	Dev		: John Adams
	Example	: 
*/ 
void Commands::Command_ShowHelm(Client* client, Seperator* sep)
{
	Player* player = client->GetPlayer();
	
	if(client->GetVersion() <= 561) {
		return; // not allowed/supported
	}
	
	if (sep && sep->arg[0])
	{
		PrintSep(sep, "Command_ShowHelm");
		const char* value = sep->arg[0];
		if (strncasecmp(value, "true", strlen(value)) == 0) 
		{
			player->toggle_character_flag(CF_HIDE_HELM);
			player->toggle_character_flag(CF_HIDE_HOOD);
		}
		else if (strncasecmp(value, "false", strlen(value)) == 0) 
			player->toggle_character_flag(CF_HIDE_HELM);
		else
		{
			client->Message(CHANNEL_COLOR_YELLOW, "Not supposed to be here! Please /bug this: Error in %s (%u)", __FUNCTION__, __LINE__);
			LogWrite(COMMAND__WARNING, 0, "Command", "Not supposed to be here! Please /bug this: Error in %s (%u)", __FUNCTION__, __LINE__);
		}
	}
}

/* 
	Function: Command_ShowHood()
	Purpose	: Character Settings combo box for Show Hood
	Params	: true/false
	Dev		: John Adams
	Example	: 
*/ 
void Commands::Command_ShowHood(Client* client, Seperator* sep)
{
	Player* player = client->GetPlayer();

	if (sep && sep->arg[0])
	{
		PrintSep(sep, "Command_ShowHood");
		const char* value = sep->arg[0];

		if (strncasecmp(value, "true", strlen(value)) == 0) 
		{
			player->toggle_character_flag(CF_HIDE_HOOD);
			if(client->GetVersion() > 561) { // no hide helm support in DoF
				player->toggle_character_flag(CF_HIDE_HELM);
			}
		}
		else if (strncasecmp(value, "false", strlen(value)) == 0) {
			player->toggle_character_flag(CF_HIDE_HOOD);
		}
		else
		{
			client->Message(CHANNEL_COLOR_YELLOW, "Not supposed to be here! Please /bug this: Error in %s (%u)", __FUNCTION__, __LINE__);
			LogWrite(COMMAND__WARNING, 0, "Command", "Not supposed to be here! Please /bug this: Error in %s (%u)", __FUNCTION__, __LINE__);
		}
	}
}

/* 
	Function: Command_ShowHoodHelm()
	Purpose	: Character Settings combo box for Show Helm or Hood
	Params	: true/false
	Dev		: John Adams
	Example	: 
*/ 
void Commands::Command_ShowHoodHelm(Client* client, Seperator* sep)
{
	Player* player = client->GetPlayer();
	
	if(client->GetVersion() <= 561) {
		return; // not allowed/supported
	}
	
	if (sep && sep->arg[0])
	{
		PrintSep(sep, "Command_ShowHoodHelm");
		const char* value = sep->arg[0];
		if (strncasecmp(value, "true", strlen(value)) == 0) 
		{
			player->toggle_character_flag(CF_HIDE_HOOD);
			player->toggle_character_flag(CF_HIDE_HELM);
		}
		else if (strncasecmp(value, "false", strlen(value)) == 0) 
		{
			// don't think we ever wind up in here...
			player->toggle_character_flag(CF_HIDE_HOOD);
			player->toggle_character_flag(CF_HIDE_HELM);
		}
		else
		{
			client->Message(CHANNEL_COLOR_YELLOW, "Not supposed to be here! Please /bug this: Error in %s (%u)", __FUNCTION__, __LINE__);
			LogWrite(COMMAND__WARNING, 0, "Command", "Not supposed to be here! Please /bug this: Error in %s (%u)", __FUNCTION__, __LINE__);
		}
	}
}

/* 
	Function: Command_ShowRanged()
	Purpose	: Character Settings combo box for Show Ranged weapon
	Params	: true/false
	Dev		: John Adams
	Example	: 
*/ 
void Commands::Command_ShowRanged(Client* client, Seperator* sep)
{
	Player* player = client->GetPlayer();

	if (sep && sep->arg[0])
	{
		const char* value = sep->arg[0];
		if (strncasecmp(value, "true", strlen(value)) == 0) 
		{
			player->set_character_flag(CF2_SHOW_RANGED);
			player->reset_character_flag(CF_SHOW_CLOAK);
		}
		else if (strncasecmp(value, "false", strlen(value)) == 0) 
			player->reset_character_flag(CF2_SHOW_RANGED);
		else
		{
			client->Message(CHANNEL_COLOR_YELLOW, "Not supposed to be here! Please /bug this: Error in %s (%u)", __FUNCTION__, __LINE__);
			LogWrite(COMMAND__WARNING, 0, "Command", "Not supposed to be here! Please /bug this: Error in %s (%u)", __FUNCTION__, __LINE__);
		}
	}
}

/* 
	Function: 
	Purpose	: 
	Params	: 
	Dev		: 
	Example	: 
*/ 
void Commands::Command_Skills(Client* client, Seperator* sep, int handler)
{
	Player* player = 0;
	Client* to_client = 0;

	if(client->GetPlayer()->GetTarget() && client->GetPlayer()->GetTarget()->IsPlayer())
		to_client = ((Player*)client->GetPlayer()->GetTarget())->GetClient();

	switch(handler)
	{
		case COMMAND_SKILL:
			{
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /skill [add|remove|list] [skill name]");
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Target player to add/remove skills for that player.");
				break;
			}
		case COMMAND_SKILL_ADD:
			{
				if (sep && sep->arg[0]) 
				{
					const char* skill_name = sep->argplus[0];
					Skill* skill = master_skill_list.GetSkillByName(skill_name);

					if (skill && to_client && !(client->GetPlayer() == to_client->GetPlayer())) // add skill to your target, if target is not you
					{
						player = to_client->GetPlayer();

						if (player) 
						{
							if (!player->GetSkills()->HasSkill(skill->skill_id)) 
							{
								player->AddSkill(skill->skill_id, 1, player->GetLevel() * 5, true);
								client->Message(CHANNEL_COLOR_YELLOW, "You gave skill '%s' to player '%s'.", skill_name, player->GetName());
								to_client->Message(CHANNEL_COLOR_YELLOW, "%s gave you skill '%s'.", client->GetPlayer()->GetName(), skill_name);
							}
							else
								client->Message(CHANNEL_COLOR_YELLOW, "%s already has the skill '%s'.", player->GetName(), skill_name);
						}
					}
					else if (skill) // add skill to yourself
					{
						player = client->GetPlayer();

						if (player) 
						{
							if (!player->GetSkills()->HasSkill(skill->skill_id)) 
							{
								player->AddSkill(skill->skill_id, 1, player->GetLevel() * 5, true);
								client->Message(CHANNEL_COLOR_YELLOW, "Added skill '%s'.", skill_name);
							}
							else
								client->Message(CHANNEL_COLOR_YELLOW, "You already have the skill '%s'.", skill_name);
						}
					}
					else
						client->Message(CHANNEL_COLOR_YELLOW, "The skill '%s' does not exist.", skill_name);
				}
				else
				{
					client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /skill add [skill name]");
					client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Target player to give skill to that player.");
				}
				break;
			}
		case COMMAND_SKILL_REMOVE:
			{
				if (sep && sep->arg[0]) 
				{
					const char* skill_name = sep->argplus[0];
					Skill* skill = master_skill_list.GetSkillByName(skill_name);

					if (skill && to_client && !(client->GetPlayer() == to_client->GetPlayer())) // remove skill from your target, if target is not you
					{
						player = to_client->GetPlayer();

						if (player)
						{
							if (player->GetSkills()->HasSkill(skill->skill_id)) 
							{
								player->RemoveSkillFromDB(skill, true);
								client->Message(CHANNEL_COLOR_YELLOW, "Removed skill '%s' from player %s.", skill_name, player->GetName());
								to_client->Message(CHANNEL_COLOR_YELLOW, "%s has removed skill '%s' from you.", client->GetPlayer()->GetName(), skill_name);
							}
							else
								client->Message(CHANNEL_COLOR_YELLOW, "Player '%s' does not have the skill '%s'.", player->GetName(), skill_name);
						}
					}
					else if(skill) // remove skill from yourself
					{
						Player* player = client->GetPlayer();

						if (player) 
						{
							if (player->GetSkills()->HasSkill(skill->skill_id)) 
							{
								player->RemoveSkillFromDB(skill, true);
								client->Message(CHANNEL_COLOR_YELLOW, "Removed skill '%s'.", skill_name);
							}
							else
								client->Message(CHANNEL_COLOR_YELLOW, "You do not have the skill '%s'.", skill_name);
						}
					}
					else
						client->Message(CHANNEL_COLOR_YELLOW, "The skill '%s' does not exist.", skill_name);
				}
				else
				{
					client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /skill remove [skill name]");
					client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Target player to give skill to that player.");
				}
				break;
			}
		case COMMAND_SKILL_LIST:
			{
				if (sep && sep->arg[0]) 
				{
					const char* skill_name = sep->argplus[0];
					client->Message(CHANNEL_COLOR_YELLOW, "Listing skills like '%s':", skill_name);
					map<int32, Skill*>* skills = master_skill_list.GetAllSkills();
					map<int32, Skill*>::iterator itr;

					if (skills && skills->size() > 0) 
					{
						for (itr = skills->begin(); itr != skills->end(); itr++) 
						{
							Skill* skill = itr->second;
							string current_skill_name = ::ToLower(string(skill->name.data.c_str()));

							if (current_skill_name.find(::ToLower(string(skill_name))) < 0xFFFFFFFF)
								client->Message(CHANNEL_COLOR_YELLOW, "%s (%u)", skill->name.data.c_str(), skill->skill_id);
						}
					}
				}
				else
					client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /skill list [skill name]");
				break;
			}
	}
}

/* 
	Function: Command_SpawnTemplate()
	Purpose	: Create or Use defined Spawn Templates
	Params	: create, save, rempve, list
	Dev		: John Adams
	Example	: /spawn template create "Test"
*/ 
void Commands::Command_SpawnTemplate(Client* client, Seperator* sep)
{
	if (sep == NULL || sep->arg[0] == NULL) 
	{
		client->Message(CHANNEL_COLOR_YELLOW, "Examples:\n/spawn template save Test - saves a template of the targetted spawn as the name Test\n/spawn template list - shows a list of current templates\n/spawn template spawn [id|name] creates a new spawn based on template [id|name] at your current location.");
		return;
	} 

	// got params, continue
	const char * template_cmd = sep->arg[0];

	if (strncasecmp(template_cmd, "list", strlen(template_cmd)) == 0) 
	{
		if (!sep->IsSet(1))
		{
			client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Usage: /spawn template list [name|location_id]");
			client->SimpleMessage(CHANNEL_COLOR_YELLOW,"Examples:\n/spawn template list [name] - will show all templates containing [name].\n/spawn template list [location_id] - will show all templates that [location_id] is assigned to.");
			return;
		}

		map<int32, string>* template_names = 0;
		if (sep->IsNumber(1))
		{
			int32 location_id = 0;
			location_id = atoi(sep->arg[1]);
			template_names = database.GetSpawnTemplateListByID(location_id);
		}
		else
		{
			const char* name = 0;
			name = sep->argplus[1];
			template_names = database.GetSpawnTemplateListByName(name);
		}

		if(!template_names)
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "No templates found.");
		else
		{
			map<int32, string>::iterator itr;
			client->SimpleMessage(CHANNEL_COLOR_YELLOW," ID   Name: ");
			for(itr = template_names->begin(); itr != template_names->end(); itr++)
				client->Message(CHANNEL_COLOR_YELLOW,"%03lu %s", itr->first, itr->second.c_str());
			safe_delete(template_names);
		}
	}

	else if (strncasecmp(template_cmd, "save", strlen(template_cmd)) == 0) 
	{
		NPC* target = (NPC*)client->GetPlayer()->GetTarget();
		if ( target && (target->IsNPC() || target->IsObject() || target->IsSign() || target->IsWidget() || target->IsGroundSpawn()) ) 
		{
			if (sep && sep->arg[1][0] && !sep->IsNumber(1))
			{
				// first, lookup to see if template name already exists
				const char* name = 0;
				name = sep->argplus[1];
				map<int32, string>* template_names = database.GetSpawnTemplateListByName(name);
				if(!template_names)
				{
					int32 new_template_id = database.SaveSpawnTemplate(target->GetSpawnLocationID(), name);
					if( new_template_id > 0 )
						client->Message(CHANNEL_COLOR_YELLOW, "Spawn template '%s' added for spawn '%s' (%u) as TemplateID: %u", name, target->GetName(), target->GetSpawnLocationID(), new_template_id);
					else
						client->Message(CHANNEL_COLOR_RED, "ERROR: Failed to add new template '%s' for spawn '%s' (%u).", name, target->GetName(), target->GetSpawnLocationID());
				}
				else
					client->Message(CHANNEL_COLOR_RED, "ERROR: Failed to add new template '%s' - Already exists!", name);
			}
			else
				client->Message(CHANNEL_COLOR_RED, "ERROR: Saving a new spawn template requires a valid template name!");
		}
		else
			client->Message(CHANNEL_COLOR_RED, "ERROR: You must target the spawn you wish to save as a template!");
	}

	else if (strncasecmp(template_cmd, "remove", strlen(template_cmd)) == 0) 
	{
		if (sep && sep->arg[1][0] && sep->IsNumber(1))
		{
			int32 template_id = 0;
			template_id = atoi(sep->arg[1]);
			if (database.RemoveSpawnTemplate(template_id))
				client->Message(CHANNEL_COLOR_YELLOW, "Spawn template ID: %u successfully removed.", template_id);
			else
				client->Message(CHANNEL_COLOR_RED, "ERROR: Failed to remove spawn template ID: %u", template_id);
		}
		else
			client->Message(CHANNEL_COLOR_RED, "ERROR: Removing a spawn template requires a valid template ID!");
	}

	// Renamed create to spawn
	else if (strncasecmp(template_cmd, "spawn", strlen(template_cmd)) == 0) 
	{
		if (sep && sep->arg[1][0]) 
		{
			int32 new_location = 0;

			if (sep->IsNumber(1))
			{
				int32 template_id = 0;
				template_id = atoi(sep->arg[1]);

				new_location = database.CreateSpawnFromTemplateByID(client, template_id);
				if( new_location > 0 )
					client->Message(CHANNEL_COLOR_YELLOW, "New spawn location %u created from template ID: %u", new_location, template_id);
				else
					client->Message(CHANNEL_COLOR_RED, "ERROR: Failed to spawn the new spawn location from template ID: %u", template_id);
			}
			else
			{
				const char* name = 0;
				name = sep->argplus[1];

				new_location = database.CreateSpawnFromTemplateByName(client, name);
				if( new_location > 0 )
					client->Message(CHANNEL_COLOR_YELLOW, "New spawn location %u created from template: '%s'", new_location, name);
				else
					client->Message(CHANNEL_COLOR_RED, "ERROR: Failed to spawn the new spawn location from template: '%s'", name);
			}
		}
		else
			client->Message(CHANNEL_COLOR_RED, "ERROR: Spawning a new spawn location requires a valid template name or ID!");
	}

	else
		client->Message(CHANNEL_COLOR_RED, "ERROR: Unknown /spawn template command.");
}

void Commands::Command_Speed(Client* client, Seperator* sep) {
	if(sep && sep->arg[0][0] && sep->IsNumber(0)){
		float new_speed = atof(sep->arg[0]);
		if (new_speed > 0.0f)
		{
			client->GetPlayer()->SetSpeed(new_speed, true);
			client->GetPlayer()->SetCharSheetChanged(true);
			database.insertCharacterProperty(client, CHAR_PROPERTY_SPEED, sep->arg[0]);
			client->Message(CHANNEL_STATUS, "Setting speed to %.2f.", new_speed);
		}
		else
			client->Message(CHANNEL_STATUS, "Invalid speed provided %s.", sep->arg[0]);
	}
	else{
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /speed {new speed value}");
	}

}

/* 
	Function: Command_StationMarketPlace()
	Purpose	: just trying to eat the console spam for now
	Params	: 
	Dev		: John
*/ 
void Commands::Command_StationMarketPlace(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_SMP");
	// This will reduce the spam from once every 10 sec to once every 30 sec,
	// can't seem to reduce it any more get rid of it completely...
	if (sep && strcmp(sep->arg[0], "gkw") == 0) {
		PacketStruct* packet = configReader.getStruct("WS_MarketFundsUpdate", client->GetVersion());
		if (packet) {
			packet->setDataByName("account_id", client->GetAccountID());
			packet->setDataByName("character_id", client->GetCharacterID());
			packet->setDataByName("unknown1", 1, 5);
			packet->setDataByName("unknown1", 26, 6);
			packet->setDataByName("unknown2", 0xFFFFFFFF);
			packet->setDataByName("unknown3", 248, 1);
			client->QueuePacket(packet->serialize());
		}
		safe_delete(packet);
	}
}

/* 
	Function: Command_StopDrinking()
	Purpose	: Stops player from auto-consuming drink
	Params	: 
	Dev		: Zcoretri
	Example	: /stopdrinking
*/ 
void Commands::Command_StopDrinking(Client* client)
{
	client->GetPlayer()->reset_character_flag(CF_DRINK_AUTO_CONSUME);
	client->GetPlayer()->SetActiveDrinkUniqueID(0);
	client->Message(CHANNEL_COLOR_YELLOW,"You stop drinking your current drink.");
}

/* 
	Function: Command_StopEating()
	Purpose	: Stops player from auto-consuming food
	Params	: 
	Dev		: Zcoretri
	Example	: /stopeating
*/ 
void Commands::Command_StopEating(Client* client)
{
	client->GetPlayer()->reset_character_flag(CF_FOOD_AUTO_CONSUME);
	client->GetPlayer()->SetActiveFoodUniqueID(0);
	client->Message(CHANNEL_COLOR_YELLOW,"You stop eating your current food.");
}

/* 
	Function: Command_Title()
	Purpose	: Help for /title command
	Params	: n/a
	Dev		: Zcoretri
	Example	: /title
*/ 
void Commands::Command_Title(Client* client)
{
	client->Message(CHANNEL_COLOR_YELLOW, "Available subcommands: list, setprefix <index>, setsuffix <index>, fix");
}

/* 
	Function: Command_TitleList()
	Purpose	: List available titles for player
	Params	: n/a
	Dev		: Zcoretri
	Example	: /title list
*/ 
void Commands::Command_TitleList(Client* client)
{
	// must call release read lock before leaving function on GetPlayerTitles
	vector<Title*>* titles = client->GetPlayer()->GetPlayerTitles()->GetAllTitles();
	vector<Title*>::iterator itr;
	Title* title;
	sint32 i = 0;

	client->Message(CHANNEL_NARRATIVE, "Listing available titles:");
	for(itr = titles->begin(); itr != titles->end(); itr++)
	{
		title = *itr;
		client->Message(CHANNEL_NARRATIVE, "%i: type=[%s] title=[%s]", i, title->GetPrefix() ? "Prefix":"Suffix", title->GetName());
		i++;
	}

	client->GetPlayer()->GetPlayerTitles()->ReleaseReadLock();
}

/* 
	Function: Command_TitleSetPrefix()
	Purpose	: Set Prefix title for player
	Params	: Title ID
	Dev		: Zcoretri
	Example	: /title setprefix 1
*/ 
void Commands::Command_TitleSetPrefix(Client* client, Seperator* sep)
{
	if (sep && sep->arg[0] && sep->IsNumber(0))
	{
		sint32 index = atoul(sep->arg[0]);

		if(index > -1)
		{
			Title* title = client->GetPlayer()->GetPlayerTitles()->GetTitle(index);
			if(!title)
			{
				client->Message(CHANNEL_COLOR_RED, "Missing index %i to set title", index);
				return;
			}
			else if(!title->GetPrefix())
			{
				client->Message(CHANNEL_COLOR_RED, "%s is not a prefix.", title->GetName());
				return;
			}
		}
		else // make sure client doesn't pass some bogus negative index
			index = -1;

		database.SaveCharPrefixIndex(index, client->GetCharacterID());
		client->SendTitleUpdate();
	}
}

/* 
	Function: Command_TitleSetSuffix()
	Purpose	: Set Suffix title for player
	Params	: Title ID
	Dev		: Zcoretri
	Example	: /title setsuffix 1
*/ 
void Commands::Command_TitleSetSuffix(Client* client, Seperator* sep)
{
	if (sep && sep->arg[0] && sep->IsNumber(0))
	{
		sint32 index = atoul(sep->arg[0]);
		if(index > -1)
		{
			Title* title = client->GetPlayer()->GetPlayerTitles()->GetTitle(index);
			if(!title)
			{
				client->Message(CHANNEL_COLOR_RED, "Missing index %i to set title", index);
				return;
			}
			else if(title->GetPrefix())
			{
				client->Message(CHANNEL_COLOR_RED, "%s is not a suffix.", title->GetName());
				return;
			}
		}
		else // make sure client doesn't pass some bogus negative index
			index = -1;

		database.SaveCharSuffixIndex(index, client->GetCharacterID());
		client->SendTitleUpdate();
		client->GetPlayer()->info_changed = true;
		client->GetPlayer()->GetZone()->AddChangedSpawn(client->GetPlayer());
	}
}

/* 
	Function: Command_TitleFix()
	Purpose	: Fix title for player (???)
	Params	: 
	Dev		: Zcoretri
	Example	: 
*/ 
void Commands::Command_TitleFix(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_TITLE_FIX");
	LogWrite(MISC__TODO, 1, "Titles", "TODO-Command: TITLE_FIX");
}


/* 
	Function: Command_Toggle_Anonymous()
	Purpose	: Toggles player Anonymous
	Params	: 
	Dev		: paulgh
	Example	: /anon
*/ 
void Commands::Command_Toggle_Anonymous(Client* client)
{
	Player* player = client->GetPlayer();

	player->toggle_character_flag(CF_ANONYMOUS);
	client->Message(CHANNEL_COLOR_YELLOW,"You are %s anonymous.", player->get_character_flag(CF_ANONYMOUS)?"now":"no longer");

	if (player->get_character_flag(CF_ANONYMOUS))
		player->SetActivityStatus(player->GetActivityStatus() + ACTIVITY_STATUS_ANONYMOUS);
	else
		player->SetActivityStatus(player->GetActivityStatus() - ACTIVITY_STATUS_ANONYMOUS);
}

/* 
	Function: Command_Toggle_AutoConsume()
	Purpose	: Toggles player food/drink auto consume
	Params	: unknown
	Dev		: paulgh
	Example	: /set_auto_consume
*/ 
void Commands::Command_Toggle_AutoConsume(Client* client, Seperator* sep)
{
	Player* player = client->GetPlayer();

	if (sep && sep->arg[0])
		PrintSep(sep, "COMMAND_SET_AUTO_CONSUME");
	if (sep && sep->arg[0] && sep->IsNumber(0))
	{
		int8 slot = atoi(sep->arg[0]);
		int8 flag = atoi(sep->arg[1]);
		if (client->GetVersion() <= 373) {
			slot += 4;
		}
		else if (client->GetVersion() <= 561) {
			slot += 2;
		}
		if (slot == EQ2_FOOD_SLOT)
		{
			player->toggle_character_flag(CF_FOOD_AUTO_CONSUME);
			player->SetCharSheetChanged(true);
			client->QueuePacket(player->GetEquipmentList()->serialize(client->GetVersion(), player));
			if (flag == 1)
				client->Message(CHANNEL_NARRATIVE, "You decide to eat immediately whenever you become hungry.");
			else
			{
				client->Message(CHANNEL_NARRATIVE, "You decide to ignore the hunger.");
				return;
			}
		}
		else
		{
			player->toggle_character_flag(CF_DRINK_AUTO_CONSUME);
			player->SetCharSheetChanged(true);
			client->QueuePacket(player->GetEquipmentList()->serialize(client->GetVersion(), player));
			if (flag == 1)
				client->Message(CHANNEL_NARRATIVE, "You decide to drink immediately whenever you become thirsty.");
			else
			{
				client->Message(CHANNEL_NARRATIVE, "You decide to ignore the thirst.");
				return;
			}
		}

		if(!client->CheckConsumptionAllowed(slot, false))
			return;
		
		Item* item = player->GetEquipmentList()->GetItem(slot);
		if(item)
			client->ConsumeFoodDrink(item, slot);
	}
}

/* 
	Function: Command_Toggle_BonusXP()
	Purpose	: Toggles player Bonus XP
	Params	: 
	Dev		: John Adams
	Example	: /disable_char_bonus_exp
*/ 
void Commands::Command_Toggle_BonusXP(Client* client)
{
	Player* player = client->GetPlayer();

	player->toggle_character_flag(CF2_CHARACTER_BONUS_EXPERIENCE_ENABLED);
	player->SetCharSheetChanged(true);
}

/* 
	Function: Command_Toggle_CombatXP()
	Purpose	: Toggles player Adventure XP
	Params	: 
	Dev		: John Adams
	Example	: /disable_combat_exp
*/ 
void Commands::Command_Toggle_CombatXP(Client* client)
{
	Player* player = client->GetPlayer();

	player->toggle_character_flag(CF_COMBAT_EXPERIENCE_ENABLED);
	player->SetCharSheetChanged(true);
}

/* 
	Function: Command_Toggle_GMHide()
	Purpose	: Toggles hiding player GM status
	Params	: 
	Dev		: Scatman
	Example	: /gm_hide
*/ 
void Commands::Command_Toggle_GMHide(Client* client)
{
	Player* player = client->GetPlayer();

	player->toggle_character_flag(CF_HIDE_STATUS);
	client->Message(CHANNEL_COLOR_YELLOW,"You are %s hiding your GM status.", player->get_character_flag(CF_HIDE_STATUS)?"now":"no longer");
}

/* 
	Function: Command_Toggle_GMVanish()
	Purpose	: Toggles hiding GM players from /who searches
	Params	: 
	Dev		: Scatman
	Example	: /gm_vanish
*/ 
void Commands::Command_Toggle_GMVanish(Client* client)
{
	Player* player = client->GetPlayer();

	player->toggle_character_flag(CF_GM_HIDDEN);
	client->Message(CHANNEL_COLOR_YELLOW,"You are %s invisible to who queries.", player->get_character_flag(CF_GM_HIDDEN)?"now":"no longer");
}

/* 
	Function: Command_Toggle_Illusions()
	Purpose	: Toggles player illusion form
	Params	: not sure sep is needed, testing
	Dev		: paulgh
	Example	: /hide_illusions
*/ 
void Commands::Command_Toggle_Illusions(Client* client, Seperator* sep)
{
	if (sep && sep->arg[0])
		PrintSep(sep, "COMMAND_TOGGLE_ILLUSIONS");
	client->GetPlayer()->toggle_character_flag(CF_SHOW_ILLUSION);
}

/* 
	Function: Command_Toggle_LFG()
	Purpose	: Toggles player LFG Flag
	Params	: 
	Dev		: paulgh
	Example	: /lfg
*/ 
void Commands::Command_Toggle_LFG(Client* client)
{
	Player* player = client->GetPlayer();

	player->toggle_character_flag(CF_LFG);
	client->Message(CHANNEL_COLOR_YELLOW,"You are %s LFG.", player->get_character_flag(CF_LFG)?"now":"no longer");

	if (player->get_character_flag(CF_LFG))
		player->SetActivityStatus(player->GetActivityStatus() + ACTIVITY_STATUS_LFG);
	else
		player->SetActivityStatus(player->GetActivityStatus() - ACTIVITY_STATUS_LFG);
}

/* 
	Function: Command_Toggle_LFW()
	Purpose	: Toggles player LFW Flag
	Params	: 
	Dev		: paulgh
	Example	: /lfw
*/ 
void Commands::Command_Toggle_LFW(Client* client)
{
	Player* player = client->GetPlayer();

	player->toggle_character_flag(CF_LFW);
	client->Message(CHANNEL_COLOR_YELLOW,"You %s looking for work.", player->get_character_flag(CF_LFW)?"let others know you are":"stop");

	if (player->get_character_flag(CF_LFW))
		player->SetActivityStatus(player->GetActivityStatus() + ACTIVITY_STATUS_LFW);
	else
		player->SetActivityStatus(player->GetActivityStatus() - ACTIVITY_STATUS_LFW);
}

/* 
	Function: Command_Toggle_QuestXP()
	Purpose	: Toggles player Quest XP
	Params	: 
	Dev		: John Adams
	Example	: /disable_quest_exp
*/ 
void Commands::Command_Toggle_QuestXP(Client* client)
{
	Player* player = client->GetPlayer();

	player->toggle_character_flag(CF_QUEST_EXPERIENCE_ENABLED);
	player->SetCharSheetChanged(true);
}

/* 
	Function: Command_Toggle_Roleplaying()
	Purpose	: Toggles player Roleplaying flag
	Params	: 
	Dev		: paulgh
	Example	: /role
*/ 
void Commands::Command_Toggle_Roleplaying(Client* client)
{
	Player* player = client->GetPlayer();

	player->toggle_character_flag(CF_ROLEPLAYING);
	client->Message(CHANNEL_COLOR_YELLOW,"You are %s roleplaying.", player->get_character_flag(CF_ROLEPLAYING)?"now":"no longer");

	if (player->get_character_flag(CF_ROLEPLAYING))
		player->SetActivityStatus(player->GetActivityStatus() + ACTIVITY_STATUS_ROLEPLAYING);
	else
		player->SetActivityStatus(player->GetActivityStatus() - ACTIVITY_STATUS_ROLEPLAYING);
}

void Commands::Command_Toggle_Duels(Client* client)
{
	Player* player = client->GetPlayer();

	player->toggle_character_flag(CF_ALLOW_DUEL_INVITES);
	client->Message(CHANNEL_COLOR_YELLOW,"You are %s accepting duel invites.", player->get_character_flag(CF_ALLOW_DUEL_INVITES)?"now":"no longer");
}

void Commands::Command_Toggle_Trades(Client* client)
{
	Player* player = client->GetPlayer();

	player->toggle_character_flag(CF_ALLOW_TRADE_INVITES);
	client->Message(CHANNEL_COLOR_YELLOW,"You are %s accepting trade invites.", player->get_character_flag(CF_ALLOW_TRADE_INVITES)?"now":"no longer");
}

void Commands::Command_Toggle_Guilds(Client* client)
{
	Player* player = client->GetPlayer();

	player->toggle_character_flag(CF_ALLOW_GUILD_INVITES);
	client->Message(CHANNEL_COLOR_YELLOW,"You are %s accepting guild invites.", player->get_character_flag(CF_ALLOW_GUILD_INVITES)?"now":"no longer");
}

void Commands::Command_Toggle_Groups(Client* client)
{
	Player* player = client->GetPlayer();

	player->toggle_character_flag(CF_ALLOW_GROUP_INVITES);
	client->Message(CHANNEL_COLOR_YELLOW,"You are %s accepting group invites.", player->get_character_flag(CF_ALLOW_GROUP_INVITES)?"now":"no longer");
}

void Commands::Command_Toggle_Raids(Client* client)
{
	Player* player = client->GetPlayer();

	player->toggle_character_flag(CF_ALLOW_RAID_INVITES);
	client->Message(CHANNEL_COLOR_YELLOW,"You are %s accepting raid invites.", player->get_character_flag(CF_ALLOW_RAID_INVITES)?"now":"no longer");
}

void Commands::Command_Toggle_LON(Client* client)
{
	Player* player = client->GetPlayer();

	player->toggle_character_flag(CF2_ALLOW_LON_INVITES);
	client->Message(CHANNEL_COLOR_YELLOW,"You are %s accepting LoN invites.", player->get_character_flag(CF2_ALLOW_LON_INVITES)?"now":"no longer");
}

void Commands::Command_Toggle_VoiceChat(Client* client)
{
	Player* player = client->GetPlayer();

	player->toggle_character_flag(CF2_ALLOW_VOICE_INVITES);
	client->Message(CHANNEL_COLOR_YELLOW,"You are %s accepting voice chat invites.", player->get_character_flag(CF2_ALLOW_VOICE_INVITES)?"now":"no longer");
}

/* 
	Function: Command_TradeStart()
	Purpose	: Starts item/coin trade between players
	Params	: 
	Dev		: 
	Example	: 
*/ 
#include "../Trade.h"
void Commands::Command_TradeStart(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_START_TRADE");

	Entity* trader = client->GetPlayer();
	Entity* trader2 = 0;
	if (sep && sep->IsSet(0) && sep->IsNumber(0)) {
		Spawn* spawn = client->GetPlayer()->GetSpawnWithPlayerID(atoi(sep->arg[0]));
		if (spawn) {
			if (spawn->IsEntity())
				trader2 = (Entity*)spawn;
		}
	}
	else if (client->GetPlayer()->GetTarget()) {
		if (client->GetPlayer()->GetTarget()->IsEntity())
			trader2 = (Entity*)client->GetPlayer()->GetTarget();
	}

	// can only trade with player or bots
	if (trader && trader2) {
		if (trader == trader2) {
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You can't trade with yourself.");
		}
		else if (trader2->IsPlayer() || trader2->IsBot()) {
			LogWrite(PLAYER__ERROR, 0, "Trade", "creating trade");
			if (trader->trade)
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You are already trading.");
			else if (trader2->trade)
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Your target is already trading.");
			else {
				Trade* trade = new Trade(trader, trader2);
				trader->trade = trade;
				trader2->trade = trade;
			}
		}
		else {
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You can only trade with another player or a bot.");
		}
	}
	else
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Unable to find target");
}

/* 
	Function: Command_Track()
	Purpose	: Starts/Stops Tracking for a player
	Params	: 
	Dev		: Scatman
	Example	: /track
*/ 
void Commands::Command_Track(Client* client)
{
	if (!client->GetPlayer()->GetIsTracking())
		client->GetPlayer()->GetZone()->AddPlayerTracking(client->GetPlayer());
	else
		client->GetPlayer()->GetZone()->RemovePlayerTracking(client->GetPlayer(), TRACKING_STOP);
}

/* 
	Function: Command_TradeAccept()
	Purpose	: Accepts item/coin trade between players
	Params	: 
	Dev		: 
	Example	: 
*/ 
void Commands::Command_TradeAccept(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_ACCEPT_TRADE");
	Trade* trade = client->GetPlayer()->trade;
	if (trade) {
		bool trade_complete = trade->SetTradeAccepted(client->GetPlayer());
		if (trade_complete)
			safe_delete(trade);
	}
	else
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You are not currently trading.");
}

/* 
	Function: Command_TradeReject()
	Purpose	: Rejects item/coin trade between players
	Params	: 
	Dev		: 
	Example	: 
*/ 
void Commands::Command_TradeReject(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_REJECT_TRADE");
	Command_TradeCancel(client, sep);
}

/* 
	Function: Command_TradeCancel()
	Purpose	: Cancels item/coin trade between players
	Params	: 
	Dev		: 
	Example	: 
*/ 
void Commands::Command_TradeCancel(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_CANCEL_TRADE");
	Trade* trade = client->GetPlayer()->trade;
	if (trade) {
		trade->CancelTrade(client->GetPlayer());
		safe_delete(trade);
	}
	else
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You are not currently trading.");
}

/* 
	Function: Command_TradeSetCoin()
	Purpose	: Sets coin trade between players
	Params	: 
	Dev		: 
	Example	: 
*/ 
void Commands::Command_TradeSetCoin(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_SET_TRADE_COIN");
	LogWrite(MISC__TODO, 1, "Command", "TODO-Command: Set Trade Coin");
	client->Message(CHANNEL_COLOR_YELLOW, "You cannot trade with other players (Not Implemented)");
}

/* 
	Function: Command_TradeAddCoin()
	Purpose	: Adds coin to trade between players
	Params	: Passes "handler" through so we can process copper, silver, gold and plat in the same function
	Dev		: 
	Example	: 
*/ 
void Commands::Command_TradeAddCoin(Client* client, Seperator* sep, int handler)
{
	PrintSep(sep, "COMMAND_ADD_TRADE_{coin type}");
	Trade* trade = client->GetPlayer()->trade;
	if (trade) {
		if (sep && sep->IsSet(0) && sep->IsNumber(0)) {
			int32 amount = atoi(sep->arg[0]);
			int64 val = 0;
			switch (handler) {
			case COMMAND_ADD_TRADE_COPPER:
			{
				val = amount;
				trade->AddCoinToTrade(client->GetPlayer(), val);
				break;
			}

			case COMMAND_ADD_TRADE_SILVER:
			{
				val = amount * 100;
				trade->AddCoinToTrade(client->GetPlayer(), val);
				break;
			}

			case COMMAND_ADD_TRADE_GOLD:
			{
				val = amount * 10000;
				trade->AddCoinToTrade(client->GetPlayer(), val);
				break;
			}

			case COMMAND_ADD_TRADE_PLAT:
			{
				val = amount * 1000000;
				trade->AddCoinToTrade(client->GetPlayer(), val);
				break;
			}

			default:
			{
				LogWrite(COMMAND__ERROR, 0, "Command", "No coin type specified in func: '%s', line %u", __FUNCTION__, __LINE__);
				break;
			}
			}
		}
		else
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Invalid coin amount.");
	}
	else
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You are not currently trading.");
}

/* 
	Function: Command_TradeRemoveCoin()
	Purpose	: Removes coin from trade between players
	Params	: Passes "handler" through so we can process copper, silver, gold and plat in the same function
	Dev		: 
	Example	: 
*/ 
void Commands::Command_TradeRemoveCoin(Client* client, Seperator* sep, int handler)
{
	PrintSep(sep, "COMMAND_REMOVE_TRADE_{coin type}");

	Trade* trade = client->GetPlayer()->trade;
	if (trade) {
		if (sep && sep->IsSet(0) && sep->IsNumber(0)) {
			int32 amount = atoi(sep->arg[0]);
			int64 val = 0;
			switch (handler) {
			case COMMAND_REMOVE_TRADE_COPPER:
			{
				val = amount;
				trade->RemoveCoinFromTrade(client->GetPlayer(), val);
				break;
			}

			case COMMAND_REMOVE_TRADE_SILVER:
			{
				val = amount * 100;
				trade->RemoveCoinFromTrade(client->GetPlayer(), val);
				break;
			}

			case COMMAND_REMOVE_TRADE_GOLD:
			{
				val = amount * 10000;
				trade->RemoveCoinFromTrade(client->GetPlayer(), val);
				break;
			}

			case COMMAND_REMOVE_TRADE_PLAT:
			{
				val = amount * 1000000;
				trade->RemoveCoinFromTrade(client->GetPlayer(), val);
				break;
			}

			default:
			{
				LogWrite(COMMAND__ERROR, 0, "Command", "No coin type specified in func: '%s', line %u", __FUNCTION__, __LINE__);
				break;
			}
			}
		}
		else
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Invalid coin amount");
	}
	else
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You are not currently trading.");
}

/* 
	Function: Command_TradeAddItem()
	Purpose	: Adds item to trade between players
	Params	: 
	Dev		: 
	Example	: 
*/ 
void Commands::Command_TradeAddItem(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_ADD_TRADE_ITEM");
	/*
	arg[0] = item index
	arg[1] = slot
	arg[2] = quantity
	*/
	if (!client->GetPlayer()->trade) {
		LogWrite(PLAYER__ERROR, 0, "Trade", "Player is not currently trading.");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You are not currently trading.");
		return;
	}

	if (sep && sep->IsSet(0) && sep->IsNumber(0) && sep->IsSet(1) && sep->IsNumber(1) && sep->IsSet(2) && sep->IsNumber(2)) {
		Item* item = 0;
		int32 index = atoi(sep->arg[0]);
		item = client->GetPlayer()->GetPlayerItemList()->GetItemFromIndex(index);
		if (item) {
			if(item->IsItemLocked() || item->details.equip_slot_id) {
				client->SimpleMessage(CHANNEL_COLOR_RED, "You cannot trade an item currently in use.");
				return;
			}
			else if(item->details.inv_slot_id == InventorySlotType::BANK || item->details.inv_slot_id == InventorySlotType::SHARED_BANK) {
				client->SimpleMessage(CHANNEL_COLOR_RED, "You cannot trade an item in the bank.");
				return;
			}
			else if(client->GetPlayer()->item_list.IsItemInSlotType(item, InventorySlotType::HOUSE_VAULT)) {
				client->SimpleMessage(CHANNEL_COLOR_RED, "You cannot trade an item in the house vault.");
				return;
			}
			
			int8 result = client->GetPlayer()->trade->AddItemToTrade(client->GetPlayer(), item, atoi(sep->arg[2]), atoi(sep->arg[1]));
			if (result == 1)
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Item is already being traded.");
			else if (result == 2)
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You can't trade NO-TRADE items.");
			else if (result == 3)
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You can't trade HEIRLOOM items.");
			else if (result == 253)
				client->Message(CHANNEL_COLOR_YELLOW, "You do not have enough quantity...");
			else if (result == 254)
				client->Message(CHANNEL_COLOR_YELLOW, "You are trading with an older client with a %u trade slot restriction...", client->GetPlayer()->trade->MaxSlots());
			else if (result == 255)
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Unknown error trying to add the item to the trade...");
		}
		else {
			LogWrite(PLAYER__ERROR, 0, "Trade", "Unable to get an item for the player (%s) from the index (%u)", client->GetPlayer()->GetName(), index);
			client->Message(CHANNEL_ERROR, "Unable to find item at index %u", index);
		}
	}
	else
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Invalid item.");
}

/* 
	Function: Command_TradeRemoveItem()
	Purpose	: Removes item from trade between players
	Params	: 
	Dev		: 
	Example	: 
*/ 
void Commands::Command_TradeRemoveItem(Client* client, Seperator* sep)
{
	PrintSep(sep, "COMMAND_REMOVE_TRADE_ITEM");
	/*
	arg[0] = trade window slot
	*/

	Trade* trade = client->GetPlayer()->trade;
	if (trade) {
		if (sep && sep->IsSet(0) && sep->IsNumber(0)) {
			trade->RemoveItemFromTrade(client->GetPlayer(), atoi(sep->arg[0]));
		}
		else
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Invalid item.");
	}
	else
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You are not currently trading.");
}

void Commands::Command_TryOn(Client* client, Seperator* sep)
{
	Item *item = 0;
	sint32 crc;

	if (sep) {
		// 1096+ sends more info so need to do a version check so older clients (SF) still work
		if (client->GetVersion() < 1096) {
			if (sep->arg[1] && sep->IsNumber(1) && sep->arg[2] && sep->IsNumber(2)) 
			{
				item = master_item_list.GetItem(atoul(sep->arg[1]));
				crc = atol(sep->arg[2]);
			}
		}
		else {
			// From the broker and links in chat
			if (strcmp(sep->arg[0], "crc") == 0) {
				if (sep->IsNumber(2) && sep->IsNumber(3)) {
					item = master_item_list.GetItem(atoul(sep->arg[2]));
					crc = atol(sep->arg[3]);
				}
			}
			// From inventory
			if (strcmp(sep->arg[0], "dbid") == 0) {
				if (sep->IsNumber(2) && sep->IsNumber(4)) {
					item = master_item_list.GetItem(atoul(sep->arg[2]));
					crc = atol(sep->arg[4]);
				}
			}
		}
		if (item)
			client->ShowDressingRoom(item, crc);
	}
}

void Commands::Command_JoinChannel(Client * client, Seperator *sep) {
	const char *channel_name, *password = NULL;

	if (sep == NULL || !sep->IsSet(0)) {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /joinchannel <channel name> [password]");
		return;
	}

	channel_name = sep->arg[0];
	if (sep->IsSet(1))
		password = sep->arg[1];

	if (!chat.ChannelExists(channel_name) && !chat.CreateChannel(channel_name, password)) {
		client->Message(CHANNEL_COLOR_RED, "Unable to create channel '%s'.", channel_name);
		return;
	}

	if (chat.IsInChannel(client, channel_name)) {
		client->Message(CHANNEL_NARRATIVE, "You are already in '%s'.", channel_name);
		return;
	}

	if (chat.HasPassword(channel_name)) {
		if (password == NULL) {
			client->Message(CHANNEL_NARRATIVE, "Unable to join '%s': That channel is password protected.", channel_name);
			return;
		}
		if (!chat.PasswordMatches(channel_name, password)) {
			client->Message(CHANNEL_NARRATIVE, "Unable to join '%s': The password is not correc.t", channel_name);
			return;
		}
	}
	
	if (!chat.JoinChannel(client, channel_name))
		client->Message(CHANNEL_COLOR_RED, "There was an internal error preventing you from joining '%s'.", channel_name);
}

void Commands::Command_JoinChannelFromLoad(Client * client, Seperator *sep) {
	printf("ScatDebug: Received 'joinfromchannel', using the same function as 'joinchannel' (not sure what the difference is)\n");
	Command_JoinChannel(client, sep);
}

void Commands::Command_TellChannel(Client *client, Seperator *sep) {
	if (sep == NULL || !sep->IsSet(0) || !sep->IsSet(1)) {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /tellchannel <channel name> <message>");
		PrintSep(sep, "tellchannel");
		return;
	}

	chat.TellChannel(client, sep->arg[0], sep->argplus[1]);
}

void Commands::Command_Test(Client* client, EQ2_16BitString* command_parms) {
	Seperator* sep = new Seperator(command_parms->data.c_str(), ' ', 50, 500, true);
	if (sep->IsSet(0)) {
		if (atoi(sep->arg[0]) == 1) {
			PacketStruct* packet2 = configReader.getStruct("WS_SpellGainedMsg", client->GetVersion());
			if (packet2) {
				packet2->setDataByName("spell_type", 2);
				packet2->setDataByName("spell_id", 8308);
				packet2->setDataByName("spell_name", "Sprint");
				packet2->setDataByName("add_silently", 0);
				packet2->setDataByName("tier", 1);
				packet2->setDataByName("blah1", 1);
				packet2->setDataByName("blah2", 1);
				EQ2Packet* outapp = packet2->serialize();
				DumpPacket(outapp);
				client->QueuePacket(outapp);
				safe_delete(packet2);
			}
		}
		else if (atoi(sep->arg[0]) == 2) {
			PacketStruct* packet2 = configReader.getStruct("WS_GuildUpdate", client->GetVersion());
			if (packet2) {
				packet2->setDataByName("guild_name", "Test");
				packet2->setDataByName("guild_motd", "Test MOTD");
				packet2->setDataByName("guild_id", 1234);
				packet2->setDataByName("guild_level", 1);
				packet2->setDataByName("unknown", 2);
				packet2->setDataByName("unknown2", 3);
				packet2->setDataByName("exp_current", 1);
				packet2->setDataByName("exp_to_next_level", 4);
				EQ2Packet* outapp = packet2->serialize();
				DumpPacket(outapp);
				client->QueuePacket(outapp);
				safe_delete(packet2);
			}
		}
		else if (atoi(sep->arg[0]) == 3) {
			PacketStruct* packet2 = configReader.getStruct("WS_JoinGuildNotify", client->GetVersion());
			if (packet2) {
				packet2->setDataByName("guild_id", 1234);
				packet2->setDataByName("character_id", 1);
				packet2->setDataByName("account_id", 2);
				packet2->setDataByName("guild_level", 1);
				packet2->setDataByName("name", "Test");
				packet2->setDataByName("unknown2", 0);
				packet2->setDataByName("unknown3", 1);
				packet2->setDataByName("adventure_class", 6);
				packet2->setDataByName("adventure_level", 7);
				packet2->setDataByName("tradeskill_class", 4);
				packet2->setDataByName("tradeskill_level", 5);
				packet2->setDataByName("rank", 0);
				packet2->setDataByName("member_flags", 2);
				packet2->setDataByName("join_date", 1591112273);
				packet2->setDataByName("guild_status", 2);
				packet2->setDataByName("last_login", 1591132273);
				packet2->setDataByName("recruiter_id", 1);
				packet2->setDataByName("points", 2345);
				packet2->setDataByName("note", "note");
				packet2->setMediumStringByName("officer_note", "O note");
				packet2->setMediumStringByName("zone", "Blah");

				EQ2Packet* outapp = packet2->serialize();
				DumpPacket(outapp);
				client->QueuePacket(outapp);
				safe_delete(packet2);
			}
		}
		else if (atoi(sep->arg[0]) == 5) {
			int16 offset = atoi(sep->arg[1]);
			int32 value1 = atol(sep->arg[2]);
			EQ2Packet* outapp = client->GetPlayer()->GetPlayerInfo()->serialize(client->GetVersion(), offset, value1);
			client->QueuePacket(outapp);
		}
		else if (atoi(sep->arg[0]) == 6) {
			Spawn* spawn = client->GetPlayer()->GetTarget();
			if (!spawn)
				spawn = client->GetPlayer();
			else if (spawn->IsEntity())
				((Entity*)spawn)->SetSpeed(atof(sep->arg[4]));
			spawn->RunToLocation(atof(sep->arg[1]), atof(sep->arg[2]), atof(sep->arg[3]));
		}
		else if (atoi(sep->arg[0]) == 7) {
			int32 id = 0;
			Spawn* spawn = client->GetCurrentZone()->GetSpawn(7720001);
			if (spawn) {
				spawn->SetX(client->GetPlayer()->GetX() + .5, false);
				spawn->SetY(client->GetPlayer()->GetY(), false);
				spawn->SetZ(client->GetPlayer()->GetZ() + .5, false);
				float heading = client->GetPlayer()->GetHeading() + 180;
				if (heading > 360)
					heading -= 360;
				spawn->SetLevel(5);
				spawn->SetAdventureClass(4);
				spawn->SetHeading(heading, false);
				spawn->SetSpawnOrigX(spawn->GetX());
				spawn->SetSpawnOrigY(spawn->GetY());
				spawn->SetSpawnOrigZ(spawn->GetZ());
				spawn->SetSpawnOrigHeading(spawn->GetHeading());
				spawn->SetLocation(client->GetPlayer()->GetLocation());
				spawn->appearance.targetable = 1;
				if (spawn->IsNPC() && spawn->GetTotalHP() == 0) {
					spawn->SetTotalHP(spawn->GetLevel() * 15);
					spawn->SetHP(spawn->GetTotalHP());
				}
				if (spawn->GetTotalPower() == 0) {
					spawn->SetTotalPower(spawn->GetLevel() * 15);
					spawn->SetPower(spawn->GetTotalPower());
				}
				int16 offset = atoi(sep->arg[1]);
				int32 value1 = atol(sep->arg[2]);
				sprintf(spawn->appearance.name, "Offset %i", offset);
				EQ2Packet* ret = spawn->spawn_serialize(client->GetPlayer(), client->GetVersion(), offset, value1);
				DumpPacket(ret);
				client->QueuePacket(ret);
			}
		}
		else if (atoi(sep->arg[0]) == 8) {
			int32 id = atoi(sep->arg[1]);
			Spawn* spawn = client->GetCurrentZone()->GetSpawn(id);
			if (spawn) {
				spawn->SetX(client->GetPlayer()->GetX() + .5, false);
				spawn->SetY(client->GetPlayer()->GetY(), false);
				spawn->SetZ(client->GetPlayer()->GetZ() + .5, false);
				float heading = client->GetPlayer()->GetHeading() + 180;
				if (heading > 360)
					heading -= 360;
				spawn->SetLevel(5);
				spawn->SetAdventureClass(4);
				spawn->SetHeading(heading, false);
				spawn->SetSpawnOrigX(spawn->GetX());
				spawn->SetSpawnOrigY(spawn->GetY());
				spawn->SetSpawnOrigZ(spawn->GetZ());
				spawn->SetSpawnOrigHeading(spawn->GetHeading());
				spawn->SetLocation(client->GetPlayer()->GetLocation());
				spawn->appearance.targetable = 1;
				if (spawn->IsNPC() && spawn->GetTotalHP() == 0) {
					spawn->SetTotalHP(spawn->GetLevel() * 15);
					spawn->SetHP(spawn->GetTotalHP());
				}
				if (spawn->GetTotalPower() == 0) {
					spawn->SetTotalPower(spawn->GetLevel() * 15);
					spawn->SetPower(spawn->GetTotalPower());
				}
				int16 offset = atoi(sep->arg[2]);
				int16 offset2 = atoi(sep->arg[3]);
				int32 value1 = atol(sep->arg[4]);
				int16 offset3 = 0;
				int16 offset4 = 0;
				int32 value2 = 0;
				if (sep->IsSet(7)) {
					offset3 = atoi(sep->arg[5]);
					offset4 = atoi(sep->arg[6]);
					value2 = atol(sep->arg[7]);
				}
				sprintf(spawn->appearance.name, "Offset %i to %i", offset, offset2);
				spawn->AddPrimaryEntityCommand("attack", 10000, "attack","", 0, 0);
				EQ2Packet* ret = spawn->spawn_serialize(client->GetPlayer(), client->GetVersion(), offset, value1, offset2, offset3, offset4, value2);
				DumpPacket(ret);
				client->QueuePacket(ret);
			}
		}
		else if (atoi(sep->arg[0]) == 9) {		
			PacketStruct* packet2 = configReader.getStruct("WS_DeathWindow", client->GetVersion());
			if (packet2) {
				packet2->setArrayLengthByName("location_count", 1);
				packet2->setArrayDataByName("location_ida", 1234);
				packet2->setArrayDataByName("unknown2a", 3);
				packet2->setArrayDataByName("zone_name", "Queen's Colony");
				packet2->setArrayDataByName("location_name", "Myrrin's Tower");
				packet2->setArrayDataByName("distance", 134);
				EQ2Packet* app = packet2->serialize();
				if (sep->IsSet(2)) {
					int8 offset = atoi(sep->arg[1]);
					uchar* ptr2 = app->pBuffer;
					ptr2 += offset;
					if (sep->IsNumber(2)) {
						int32 value1 = atol(sep->arg[2]);
						if (value1 > 0xFFFF)
							memcpy(ptr2, (uchar*)&value1, 4);
						else if (value1 > 0xFF)
							memcpy(ptr2, (uchar*)&value1, 2);
						else
							memcpy(ptr2, (uchar*)&value1, 1);
					}
					else {
						int8 len = strlen(sep->arg[2]);
						memcpy(ptr2, (uchar*)&len, 1);
						ptr2 += 1;
						memcpy(ptr2, sep->arg[2], len);
					}
				}
				DumpPacket(app);
				client->QueuePacket(app);
				safe_delete(packet2);
			}
		}
		else if (atoi(sep->arg[0]) == 10) {
			PacketStruct* packet2 = configReader.getStruct("WS_QuestJournalUpdate", client->GetVersion());
			if (packet2) {
				packet2->setArrayLengthByName("num_quests", 1);
				packet2->setArrayDataByName("active", 1);
				packet2->setArrayDataByName("name", "Tasks aboard the Far Journey");
				packet2->setArrayDataByName("quest_type", "Hallmark");
				packet2->setArrayDataByName("quest_zone", "Hallmark");
				packet2->setArrayDataByName("journal_updated", 1);
				packet2->setArrayDataByName("quest_id", 524);
				packet2->setArrayDataByName("day", 19);
				packet2->setArrayDataByName("month", 6);
				packet2->setArrayDataByName("year", 20);
				packet2->setArrayDataByName("level", 2);
				packet2->setArrayDataByName("encounter_level", 4);
				packet2->setArrayDataByName("difficulty", 3);
				packet2->setArrayDataByName("visible", 1);
				packet2->setDataByName("visible_quest_id", 524);
				packet2->setDataByName("player_crc", 2900677088);
				packet2->setDataByName("player_name", "LethalEncounter");
				EQ2Packet* app = packet2->serialize();
				if (sep->IsSet(2)) {
					int8 offset = atoi(sep->arg[1]);
					uchar* ptr2 = app->pBuffer;
					ptr2 += offset;
					if (sep->IsNumber(2)) {
						int32 value1 = atol(sep->arg[2]);
						if (value1 > 0xFFFF)
							memcpy(ptr2, (uchar*)&value1, 4);
						else if (value1 > 0xFF)
							memcpy(ptr2, (uchar*)&value1, 2);
						else
							memcpy(ptr2, (uchar*)&value1, 1);
					}
					else {
						int8 len = strlen(sep->arg[2]);
						memcpy(ptr2, (uchar*)&len, 1);
						ptr2 += 1;
						memcpy(ptr2, sep->arg[2], len);
					}
				}
				DumpPacket(app);
				client->QueuePacket(app);
				safe_delete(packet2);
			}
		}
		else if (atoi(sep->arg[0]) == 11) {
			PacketStruct* packet2 = configReader.getStruct("WS_QuestJournalReply", client->GetVersion());
			if (packet2) {
				packet2->setDataByName("quest_id", 524);
				packet2->setDataByName("player_crc", 2900677088);
				packet2->setDataByName("name", "Tasks aboard the Far Journey");
				packet2->setDataByName("description", "I completed all the tasks assigned to me by Captain Varlos aboard the Far Journey");
				packet2->setDataByName("type", "Hallmark");
				packet2->setDataByName("complete_header", "To complete this quest, I must do the following tasks:");
				packet2->setDataByName("day", 19);
				packet2->setDataByName("month", 6);
				packet2->setDataByName("year", 20);
				packet2->setDataByName("level", 1);
				packet2->setDataByName("encounter_level", 1);
				packet2->setDataByName("difficulty", 1);
				packet2->setDataByName("time_obtained", Timer::GetUnixTimeStamp());
				//packet2->setDataByName("timer_start", Timer::GetUnixTimeStamp());
				//packet2->setDataByName("timer_duration", 300);
				//packet2->setDataByName("timer_running", 1);
				packet2->setArrayLengthByName("task_groups_completed", 9);
				packet2->setArrayLengthByName("num_task_groups", 10);
				packet2->setArrayDataByName("task_group", "I spoke to Waulon as Captain Varlos had asked of me.");
				packet2->setArrayDataByName("task_group", "I found Waulon's hat in one of the boxes.", 1);
				packet2->setArrayDataByName("task_group", "I returned Waulon's hat.", 2);
				packet2->setArrayDataByName("task_group", "I have spoken to Ingrid.", 3);
				packet2->setArrayDataByName("task_group", "I purchased a Shard of Luclin.", 4);
				packet2->setArrayDataByName("task_group", "I gave the Shard of Luclin to Ingrid.", 5);
				packet2->setArrayDataByName("task_group", "I have spoken to Captain Varlos.", 6);
				packet2->setArrayDataByName("task_group", "I killed the rats that Captain Varlos requested.", 7);
				packet2->setArrayDataByName("task_group", "Captain Varlos has ordered you to kill the escaped goblin.", 8);
				packet2->setArrayDataByName("task_group", "I killed the escaped goblin.", 9);
				/*packet2->setSubArrayLengthByName("num_tasks", 1);
				packet2->setSubArrayDataByName("task", "I need to talk to Garven Tralk");
				packet2->setSubArrayLengthByName("num_updates", 1);
				packet2->setSubArrayDataByName("update_currentval", 0);
				packet2->setSubArrayDataByName("update_maxval", 1);
				packet2->setSubArrayDataByName("icon", 11);
				*/
				packet2->setArrayDataByName("waypoint", 0xFFFFFFFF);
				packet2->setDataByName("journal_updated", 1);
				packet2->setDataByName("bullets", 1);
				EQ2Packet* app = packet2->serialize();
				if (sep->IsSet(2)) {
					int16 offset = atoi(sep->arg[1]);
					uchar* ptr2 = app->pBuffer;
					ptr2 += offset;
					if (sep->IsNumber(2)) {
						int32 value1 = atol(sep->arg[2]);
						if (value1 > 0xFFFF)
							memcpy(ptr2, (uchar*)&value1, 4);
						else if (value1 > 0xFF)
							memcpy(ptr2, (uchar*)&value1, 2);
						else
							memcpy(ptr2, (uchar*)&value1, 1);
					}
					else {
						int16 len = strlen(sep->arg[2]);
						memcpy(ptr2, (uchar*)&len, 2);
						ptr2 += 2;
						memcpy(ptr2, sep->arg[2], len);
					}
				}
				DumpPacket(app);
				client->QueuePacket(app);
				safe_delete(packet2);
			}
		}
		else if (atoi(sep->arg[0]) == 12) {
			PacketStruct* packet2 = configReader.getStruct("WS_QuestJournalReply", client->GetVersion());
			if (packet2) {
				packet2->setDataByName("quest_id", 5725);
				packet2->setDataByName("player_crc", 2900677088);
				packet2->setDataByName("name", "Archetype Selection");
				packet2->setDataByName("description", "I have reported my profession to Garven Tralk.");
				packet2->setDataByName("type", "Hallmark");
				packet2->setDataByName("complete_header", "To complete this quest, I must do the following tasks:");
				packet2->setDataByName("day", 19);
				packet2->setDataByName("month", 6);
				packet2->setDataByName("year", 20);
				packet2->setDataByName("level", 2);
				packet2->setDataByName("encounter_level", 4);
				packet2->setDataByName("difficulty", 3);
				packet2->setDataByName("time_obtained", Timer::GetUnixTimeStamp());
				packet2->setDataByName("timer_start", Timer::GetUnixTimeStamp());
				packet2->setDataByName("timer_duration", 300);
				packet2->setDataByName("timer_running", 1);
				packet2->setArrayLengthByName("task_groups_completed", 0);
				packet2->setArrayLengthByName("num_task_groups", 1);
				packet2->setArrayDataByName("task_group", "I need to talk to Garven Tralk");
				packet2->setSubArrayLengthByName("num_tasks", 1);
				packet2->setSubArrayDataByName("task", "I need to talk to Garven Tralk");
				packet2->setSubArrayLengthByName("num_updates", 1);
				packet2->setSubArrayDataByName("update_currentval", 0);
				packet2->setSubArrayDataByName("update_maxval", 1);
				packet2->setSubArrayDataByName("icon", 11);

				packet2->setArrayDataByName("waypoint", 0xFFFFFFFF);
				packet2->setDataByName("journal_updated", 1);
				packet2->setSubstructDataByName("reward_data", "unknown1", 255);
				packet2->setSubstructDataByName("reward_data", "reward", "Quest Reward!");
				packet2->setSubstructDataByName("reward_data", "unknown2", 0x3f);
				packet2->setSubstructDataByName("reward_data", "coin", 150);
				packet2->setSubstructDataByName("reward_data", "status_points", 5);
				packet2->setSubstructDataByName("reward_data", "exp_bonus", 10);
				packet2->setSubstructArrayLengthByName("reward_data", "num_rewards", 1);
				packet2->setSubstructArrayDataByName("reward_data", "reward_id", 123);
				Item* item = master_item_list.GetItem(152755);
				packet2->setArrayDataByName("reward_id", item->details.item_id);
				packet2->setItemArrayDataByName("item", item, client->GetPlayer(), 0, 0, client->GetClientItemPacketOffset());
				/*packet2->setSubstructDataByName("item", "unique_id", 567);
				packet2->setSubstructDataByName("item", "broker_item_id", 0xFFFFFFFFFFFFFFFF);
				packet2->setSubstructDataByName("item", "icon", 0xe7);
				packet2->setSubstructDataByName("item", "tier", 4);
				packet2->setSubstructDataByName("item", "flags", 0x60);
				packet2->setSubstructArrayLengthByName("item", "stat_count", 1);
				packet2->setSubstructDataByName("item", "stat_type", 1);
				packet2->setSubstructDataByName("item", "stat_subtype", 2);
				packet2->setSubstructDataByName("item", "value", 3);
				packet2->setSubstructDataByName("item", "condition", 100);
				packet2->setSubstructDataByName("item", "weight", 1);
				packet2->setSubstructDataByName("item", "skill_req1", 0xacafa99e);
				packet2->setSubstructDataByName("item", "skill_req2", 0xacafa99e);
				packet2->setSubstructDataByName("item", "skill_min", 1);
				packet2->setSubstructArrayLengthByName("item", "class_count", 3);
				packet2->setSubstructDataByName("item", "adventure_class", 1);
				packet2->setSubstructDataByName("item", "adventure_class", 11, 0, 1);
				packet2->setSubstructDataByName("item", "adventure_class", 0x1f, 0, 2);
				packet2->setSubstructDataByName("item", "tradeskill_class", 255);
				packet2->setSubstructDataByName("item", "tradeskill_class", 255, 0, 1);
				packet2->setSubstructDataByName("item", "tradeskill_class", 255, 0, 2);
				packet2->setSubstructDataByName("item", "level", 0x1e);
				packet2->setSubstructDataByName("item", "level", 100, 0, 1);
				packet2->setSubstructDataByName("item", "level", 100, 0, 2);
				packet2->setSubstructDataByName("item_footer", "name", "Footman Gloves");*/
				//packet2->PrintPacket();
				EQ2Packet* app = packet2->serialize();
				if (sep->IsSet(2)) {
					int16 offset = atoi(sep->arg[1]);
					uchar* ptr2 = app->pBuffer;
					ptr2 += offset;
					if (sep->IsNumber(2)) {
						int32 value1 = atol(sep->arg[2]);
						if (value1 > 0xFFFF)
							memcpy(ptr2, (uchar*)&value1, 4);
						else if (value1 > 0xFF)
							memcpy(ptr2, (uchar*)&value1, 2);
						else
							memcpy(ptr2, (uchar*)&value1, 1);
					}
					else {
						int16 len = strlen(sep->arg[2]);
						memcpy(ptr2, (uchar*)&len, 2);
						ptr2 += 2;
						memcpy(ptr2, sep->arg[2], len);
					}
				}
				DumpPacket(app);
				client->QueuePacket(app);
				safe_delete(packet2);
			}
		}
		else if (atoi(sep->arg[0]) == 13) {
			PacketStruct* packet2 = configReader.getStruct("WS_OnScreenMsg", client->GetVersion());
			if (packet2 && sep->IsSet(7)) {
				packet2->setDataByName("unknown", atoi(sep->arg[1]));
				char blah[128];
				sprintf(blah, "\\#6EFF6EYou get better at \12\\#C8FFC8%s\\#6EFF6E! (7/15)", sep->arg[2]);
				packet2->setDataByName("text", blah);
				packet2->setDataByName("message_type", sep->arg[3]);
				packet2->setDataByName("size", atof(sep->arg[4]));
				packet2->setDataByName("red", atoi(sep->arg[5]));
				packet2->setDataByName("green", atoi(sep->arg[6]));
				packet2->setDataByName("blue", atoi(sep->arg[7]));
				EQ2Packet* app = packet2->serialize();
				DumpPacket(app);
				client->QueuePacket(app);
				safe_delete(packet2);
			}
		}
		else if (atoi(sep->arg[0]) == 14) {
			PacketStruct* packet2 = configReader.getStruct("WS_InstructionWindow", client->GetVersion());
			if (packet2 && sep->IsSet(3)) {
				packet2->setDataByName("open_seconds_min", atof(sep->arg[1]));
				packet2->setDataByName("open_seconds_max", atof(sep->arg[2]));
				packet2->setDataByName("voice_sync", atoi(sep->arg[3]));
				packet2->setDataByName("text", "Welcome to Norrath, the world of EverQuest II. Left click on the help button at any time for more detailed help and information.");
				packet2->setDataByName("voice", "voiceover/english/narrator/boat_06p_tutorial02/narrator_001_63779ca0.mp3");
				packet2->setArrayLengthByName("num_goals", 1);
				//packet2->setArrayDataByName("goal_text", )
				packet2->setSubArrayLengthByName("num_tasks", 1);
				packet2->setSubArrayDataByName("task_text", "continue");
				packet2->setDataByName("complete_sound", "click");
				packet2->setDataByName("signal", "introduction");
				packet2->setDataByName("voice_key1", 0xcda65173);
				packet2->setDataByName("voice_key2", 0x984bfc6d);
				EQ2Packet* app = packet2->serialize();
				DumpPacket(app);
				client->QueuePacket(app);
				safe_delete(packet2);
				packet2 = configReader.getStruct("WS_ShowWindow", client->GetVersion());
				packet2->setDataByName("window", "MainHUD.StartMenu");
				packet2->setDataByName("show", 1);
				app = packet2->serialize();
				DumpPacket(app);
				client->QueuePacket(app);
				safe_delete(packet2);

				packet2 = configReader.getStruct("WS_FlashWindow", client->GetVersion());
				packet2->setDataByName("window", "MainHUD.StartMenu.help");
				packet2->setDataByName("flash_seconds", 10);
				app = packet2->serialize();
				DumpPacket(app);
				client->QueuePacket(app);
				safe_delete(packet2);
			}
		}
		else if (atoi(sep->arg[0]) == 15) {
			PacketStruct* packet2 = configReader.getStruct("WS_UpdateLoot", client->GetVersion());
			if (packet2) {
				packet2->setArrayLengthByName("loot_count", 1);
				packet2->setArrayDataByName("name", "Test");
				packet2->setArrayDataByName("item_id", 1234);
				packet2->setArrayDataByName("count", 1);
				packet2->setArrayDataByName("icon", 258);
				packet2->setArrayDataByName("ability_id", 0xFFFFFFFF);
				Spawn* spawn = client->GetPlayer()->GetTarget();
				if (spawn)
					packet2->setDataByName("object_id", client->GetPlayer()->GetIDWithPlayerSpawn(spawn));
				packet2->setDataByName("display", 1);
				packet2->setDataByName("loot_type", 1);
				packet2->setDataByName("lotto_timeout", 60);
				EQ2Packet* app = packet2->serialize();
				DumpPacket(app);
				client->QueuePacket(app);
				safe_delete(packet2);
			}
		}
		else if (atoi(sep->arg[0]) == 16 && sep->IsNumber(1)) {
			char blah[32];
			sprintf(blah, "Testing: %i", atoi(sep->arg[1]));
			client->SimpleMessage(atoi(sep->arg[1]), blah);
		}
		else if (atoi(sep->arg[0]) == 17 && sep->IsNumber(2)) {
			if (sep->IsSet(2)) {
				int16 offset = atoi(sep->arg[1]);
				if (sep->IsNumber(2)) {
					int32 value1 = atol(sep->arg[2]);
					client->QueuePacket(client->GetPlayer()->GetPlayerInfo()->serialize(client->GetVersion(), offset, value1));
					cout << "Sent" << endl;
				}
			}
		}
		else if (atoi(sep->arg[0]) == 18) {
			PacketStruct* packet2 = configReader.getStruct("WS_QuestRewardPackMsg", client->GetVersion());
			if (packet2) {
				packet2->setSubstructDataByName("reward_data", "unknown1", 255);
				packet2->setSubstructDataByName("reward_data", "reward", "Quest Reward!");
				packet2->setSubstructDataByName("reward_data", "max_coin", 0);
				packet2->setSubstructDataByName("reward_data", "text", "Some custom text to mess things up?");
				packet2->setSubstructDataByName("reward_data", "status_points", 5);
				//packet2->setSubstructDataByName("reward_data", "exp_bonus", 10);
				packet2->setSubstructArrayLengthByName("reward_data", "num_rewards", 1);
				Item* item = new Item(master_item_list.GetItem(9357));
				packet2->setArrayDataByName("reward_id", item->details.item_id);
				//item->stack_count = 20;
				packet2->setItemArrayDataByName("item", item, client->GetPlayer(), 0, 0, client->GetClientItemPacketOffset());
				safe_delete(item);
				/*item = new Item(master_item_list.GetItem(36685));
				item->stack_count = 20;
				packet2->setArrayDataByName("reward_id", item->details.item_id);
				packet2->setItemArrayDataByName("item", item, client->GetPlayer(), 1, 0, -1);
				safe_delete(item);
				item = master_item_list.GetItem(1414);
				packet2->setArrayDataByName("reward_id", item->details.item_id);
				packet2->setItemArrayDataByName("item", item, client->GetPlayer(), 2, 0, -1);
				item = master_item_list.GetItem(75057);
				packet2->setArrayDataByName("reward_id", item->details.item_id);
				packet2->setItemArrayDataByName("item", item, client->GetPlayer(), 3, 0, -1);*/
				EQ2Packet* app = packet2->serialize();
				if (sep->IsSet(2)) {
					int16 offset = atoi(sep->arg[1]);
					uchar* ptr2 = app->pBuffer;
					ptr2 += offset;
					if (sep->IsNumber(2)) {
						int32 value1 = atol(sep->arg[2]);
						if (value1 > 0xFFFF)
							memcpy(ptr2, (uchar*)&value1, 4);
						else if (value1 > 0xFF)
							memcpy(ptr2, (uchar*)&value1, 2);
						else
							memcpy(ptr2, (uchar*)&value1, 1);
					}
					else {
						int16 len = strlen(sep->arg[2]);
						memcpy(ptr2, (uchar*)&len, 2);
						ptr2 += 2;
						memcpy(ptr2, sep->arg[2], len);
					}
				}
				DumpPacket(app);
				client->QueuePacket(app);
				safe_delete(packet2);
			}
		}
		else if (atoi(sep->arg[0]) == 19) {
			PacketStruct* packet2 = configReader.getStruct("WS_UpdateLoot", client->GetVersion());
			Spawn* spawn = client->GetPlayer()->GetTarget();
			if (packet2 && spawn) {
				Item* item = master_item_list.GetItem(130053);
				packet2->setArrayLengthByName("loot_count", 1);
				packet2->setArrayDataByName("loot_id", item->details.item_id);
				packet2->setArrayDataByName("unknown2", 1);
				packet2->setItemArrayDataByName("item", item, client->GetPlayer(), 0, 0, 2, true);
				packet2->setDataByName("display", 1);
				packet2->setDataByName("unknown2b", 1);
				packet2->setDataByName("unknown3", 0x3c);
				packet2->setDataByName("spawn_id", client->GetPlayer()->GetIDWithPlayerSpawn(spawn));
				packet2->PrintPacket();
				EQ2Packet* app = packet2->serialize();
				if (sep->IsSet(2)) {
					int16 offset = atoi(sep->arg[1]);
					int16 offset2 = 0;
					int32 value1 = 0;
					if (sep->IsSet(3)) {
						offset2 = atoi(sep->arg[2]);
						value1 = atol(sep->arg[3]);
					}
					else
						value1 = atol(sep->arg[2]);
					int16 offset3 = 0;
					int16 offset4 = 0;
					int32 value2 = 0;
					if (sep->IsSet(6)) {
						offset3 = atoi(sep->arg[4]);
						offset4 = atoi(sep->arg[5]);
						value2 = atol(sep->arg[6]);
					}
					offset--;
					if (offset2 > 0 && offset2 >= offset) {
						offset2--;
						uchar* ptr2 = app->pBuffer;
						ptr2 += offset;
						for (int i = offset; i <= offset2; i++) {
							if (value1 > 0xFFFF) {
								memcpy(ptr2, (uchar*)&value1, 4);
								i += 3;
								ptr2 += 3;
							}
							else if (value1 > 0xFF) {
								memcpy(ptr2, (uchar*)&value1, 2);
								i++;
								ptr2++;
							}
							else
								memcpy(ptr2, (uchar*)&value1, 1);
							ptr2++;
						}
					}
					if (offset4 > 0 && offset4 >= offset3) {
						offset3--;
						offset4--;
						uchar* ptr2 = app->pBuffer;
						ptr2 += offset3;
						for (int i = offset3; i <= offset4; i++) {
							if (value2 > 0xFFFF) {
								memcpy(ptr2, (uchar*)&value2, 4);
								i += 3;
								ptr2 += 3;
							}
							else if (value2 > 0xFF) {
								memcpy(ptr2, (uchar*)&value2, 2);
								i++;
								ptr2++;
							}
							else
								memcpy(ptr2, (uchar*)&value2, 1);
							ptr2++;
						}
					}
				}
				DumpPacket(app);
				client->QueuePacket(app);
				safe_delete(packet2);
			}
		}
		else if (atoi(sep->arg[0]) == 21) {
			PacketStruct* packet2 = configReader.getStruct("WS_OfferQuest", client->GetVersion());
			if (packet2) {
				packet2->setDataByName("unknown0", 255);
				packet2->setDataByName("reward", "New Quest");
				packet2->setDataByName("title", "Title");
				packet2->setDataByName("description", "description");
				packet2->setDataByName("quest_difficulty", 3);
				packet2->setDataByName("unknown1", 5);
				packet2->setDataByName("level", 3);
				packet2->setDataByName("coin", 150);
				packet2->setDataByName("status_points", 5);
				packet2->setDataByName("exp_bonus", 10);
				packet2->setArrayLengthByName("num_rewards", 1);
				Item* item = new Item(master_item_list.GetItem(36212));
				packet2->setArrayDataByName("reward_id", item->details.item_id);
				item->stack_count = 20;
				packet2->setItemArrayDataByName("item", item, client->GetPlayer(), 0, 0, client->GetClientItemPacketOffset());
				safe_delete(item);
				char accept[35] = { 0 };
				char decline[35] = { 0 };
				sprintf(accept, "q_accept_pending_quest %u", 0);
				sprintf(decline, "q_deny_pending_quest %u", 0);
				packet2->setDataByName("accept_command", accept);
				packet2->setDataByName("decline_command", decline);
				EQ2Packet* app = packet2->serialize();
				if (sep->IsSet(2)) {
					int16 offset = atoi(sep->arg[1]);
					uchar* ptr2 = app->pBuffer;
					ptr2 += offset;
					if (sep->IsNumber(2)) {
						int32 value1 = atol(sep->arg[2]);
						if (value1 > 0xFFFF)
							memcpy(ptr2, (uchar*)&value1, 4);
						else if (value1 > 0xFF)
							memcpy(ptr2, (uchar*)&value1, 2);
						else
							memcpy(ptr2, (uchar*)&value1, 1);
					}
					DumpPacket(app);
					client->QueuePacket(app);
					safe_delete(packet2);
				}
			}
		}
		else if (atoi(sep->arg[0]) == 22) { //same as 21, but 8bit string
			PacketStruct* packet2 = configReader.getStruct("WS_OfferQuest", client->GetVersion());
			if (packet2) {
				packet2->setDataByName("unknown0", 255);
				packet2->setDataByName("reward", "New Quest");
				packet2->setDataByName("title", "Title");
				packet2->setDataByName("description", "description");
				packet2->setDataByName("quest_difficulty", 3);
				packet2->setDataByName("unknown1", 5);
				packet2->setDataByName("level", 3);
				packet2->setDataByName("coin", 150);
				packet2->setDataByName("status_points", 5);
				packet2->setDataByName("exp_bonus", 10);
				packet2->setArrayLengthByName("num_rewards", 1);
				Item* item = new Item(master_item_list.GetItem(36212));
				packet2->setArrayDataByName("reward_id", item->details.item_id);
				item->stack_count = 20;
				packet2->setItemArrayDataByName("item", item, client->GetPlayer(), 0, 0, client->GetClientItemPacketOffset());
				safe_delete(item);
				char accept[35] = { 0 };
				char decline[35] = { 0 };
				sprintf(accept, "q_accept_pending_quest %u", 0);
				sprintf(decline, "q_deny_pending_quest %u", 0);
				packet2->setDataByName("accept_command", accept);
				packet2->setDataByName("decline_command", decline);
				EQ2Packet* app = packet2->serialize();
				if (sep->IsSet(2)) {
					int16 offset = atoi(sep->arg[1]);
					uchar* ptr2 = app->pBuffer;
					ptr2 += offset;
					if (sep->IsNumber(2)) {
						int32 value1 = atol(sep->arg[2]);
						if (value1 > 0xFFFF)
							memcpy(ptr2, (uchar*)&value1, 4);
						else if (value1 > 0xFF)
							memcpy(ptr2, (uchar*)&value1, 2);
						else
							memcpy(ptr2, (uchar*)&value1, 1);
					}
					else {
						int8 len = strlen(sep->arg[2]);
						memcpy(ptr2, (uchar*)&len, 1);
						ptr2 += 1;
						memcpy(ptr2, sep->arg[2], len);
					}
				}
				DumpPacket(app);
				client->QueuePacket(app);
				safe_delete(packet2);
			}
		}	
		else if (atoi(sep->arg[0]) == 23) {
			if (client->GetPlayer()->GetTarget()) {
				PacketStruct* packet2 = configReader.getStruct("WS_UpdateMerchant", client->GetVersion());
				if (packet2) {
					Spawn* target = client->GetPlayer()->GetTarget();
					packet2->setDataByName("spawn_id", client->GetPlayer()->GetIDWithPlayerSpawn(target));
					Item* item = new Item(master_item_list.GetItem(12565));
					int8 i = 0;
					packet2->setArrayLengthByName("num_items", 1);
					packet2->setArrayDataByName("item_name", item->name.c_str(), i);
					packet2->setArrayDataByName("item_id", item->details.item_id, i);
					packet2->setArrayDataByName("stack_size", item->stack_count, i);
					packet2->setArrayDataByName("icon", item->GetIcon(client->GetVersion()), i);
					int8 tmp_level = 0;
					if (item->generic_info.adventure_default_level > 0)
						tmp_level = item->generic_info.adventure_default_level;
					else
						tmp_level = item->generic_info.tradeskill_default_level;
					packet2->setArrayDataByName("level", tmp_level, i);
					packet2->setArrayDataByName("tier", item->details.tier, i);
					packet2->setArrayDataByName("item_id2", item->details.item_id, i);
					int8 item_difficulty = client->GetPlayer()->GetArrowColor(tmp_level);
					if (item_difficulty != ARROW_COLOR_WHITE && item_difficulty != ARROW_COLOR_RED && item_difficulty != ARROW_COLOR_GRAY)
						item_difficulty = ARROW_COLOR_WHITE;
					item_difficulty -= 6;
					if (item_difficulty < 0)
						item_difficulty *= -1;
					packet2->setArrayDataByName("item_difficulty", item_difficulty, i);
					packet2->setArrayDataByName("quantity", 2, i);
					packet2->setArrayDataByName("unknown5", 255, i);
					packet2->setArrayDataByName("stack_size2", item->stack_count, i);
					packet2->setArrayDataByName("description", item->description.c_str(), i);
					packet2->setDataByName("type", 2);
					EQ2Packet* app = packet2->serialize();
					if (sep->IsSet(2)) {
						int16 offset = atoi(sep->arg[1]);
						uchar* ptr2 = app->pBuffer;
						ptr2 += offset;
						if (sep->IsNumber(2)) {
							int32 value1 = atol(sep->arg[2]);
							if (value1 > 0xFFFF)
								memcpy(ptr2, (uchar*)&value1, 4);
							else if (value1 > 0xFF)
								memcpy(ptr2, (uchar*)&value1, 2);
							else
								memcpy(ptr2, (uchar*)&value1, 1);
						}
						else {
							int8 len = strlen(sep->arg[2]);
							memcpy(ptr2, (uchar*)&len, 1);
							ptr2 += 1;
							memcpy(ptr2, sep->arg[2], len);
						}
					}
					DumpPacket(app);
					client->QueuePacket(app);
					safe_delete(packet2);
				}
			}
		}
		else if (atoi(sep->arg[0]) == 24) {
			PacketStruct* packet2 = configReader.getStruct("WS_EnableGameEvent", client->GetVersion());
			if (packet2) {
				if (sep->IsSet(2)) {
					packet2->setDataByName("event_name", sep->arg[1]);
					packet2->setDataByName("enabled", atoi(sep->arg[2]));					
					EQ2Packet* app = packet2->serialize();
					DumpPacket(app);
					client->QueuePacket(app);
					safe_delete(packet2);
				}
			}
		}
		else if (atoi(sep->arg[0]) == 25) {
			if (sep->IsSet(1)) {
				Widget* new_spawn = new Widget();
				int32 id = atoul(sep->arg[1]);
				new_spawn->SetWidgetID(id);
				EQ2Packet* ret = new_spawn->serialize(client->GetPlayer(), client->GetVersion());
				client->QueuePacket(ret);
				int8 index = client->GetPlayer()->GetIndexForSpawn(new_spawn);
				PacketStruct* packet2 = configReader.getStruct("WS_DestroyGhostCmd", client->GetVersion());
				if (packet2) {
					packet2->setDataByName("spawn_index", index);
					packet2->setDataByName("delete", 1);
					EQ2Packet* app = packet2->serialize();
					client->QueuePacket(app);
					safe_delete(packet2);
				}
			}
		}
		else if (atoi(sep->arg[0]) == 26) {
			if (sep->IsSet(4)) {
				Widget* new_spawn = new Widget();
				int32 id = atoul(sep->arg[1]);
				new_spawn->SetWidgetID(id);
				float x = atof(sep->arg[2]);
				float y = atof(sep->arg[3]);
				float z = atof(sep->arg[4]);
				new_spawn->SetLocation(client->GetPlayer()->GetLocation());
				new_spawn->SetWidgetX(x);
				new_spawn->SetWidgetY(y);
				new_spawn->SetWidgetZ(z);
				new_spawn->SetX(x);
				new_spawn->SetY(y);
				new_spawn->SetZ(z);
				EQ2Packet* ret = new_spawn->serialize(client->GetPlayer(), client->GetVersion());
				client->QueuePacket(ret);
			}
		}
		else if (atoi(sep->arg[0]) == 27) {
			Spawn* target = client->GetPlayer()->GetTarget();
			PacketStruct* packet2 = configReader.getStruct("WS_HearSimpleDamage", client->GetVersion());
			if (packet2 && target && sep->IsSet(4)) {
				client->GetPlayer()->GetZone()->SendDamagePacket(client->GetPlayer(), target, atoul(sep->arg[1]), atoul(sep->arg[2]), atoul(sep->arg[3]), atoul(sep->arg[4]), sep->arg[5] != nullptr ? sep->arg[5] : "");
			}
		}
		else if (atoi(sep->arg[0]) == 28 && sep->IsNumber(1)) {
			World::newValue = strtoull(sep->arg[1], NULL, 0);
		}
		else if (atoi(sep->arg[0]) == 29 && sep->IsNumber(1)) {
			client->SendHearCast(client->GetPlayer(), client->GetPlayer()->GetTarget() ? client->GetPlayer()->GetTarget() : client->GetPlayer(),
								 strtoull(sep->arg[1], NULL, 0), atoul(sep->arg[2]));
		}
		else if (atoi(sep->arg[0]) == 30) {
			PacketStruct* packet = configReader.getStruct("WS_UpdateSkillBook", client->GetVersion());
			if (packet) {
				packet->setDataByName("unknown", World::newValue);
				EQ2Packet* outapp = packet->serialize();
				DumpPacket(outapp);
				client->QueuePacket(outapp);
				safe_delete(packet);
			}
		}
		else if (atoi(sep->arg[0]) == 31) {
			client->SendRecipeList();
		}
		else if (atoi(sep->arg[0]) == 32 && sep->IsNumber(1) && sep->IsNumber(2)) {
			if(client->GetVersion() <= 561) {
				int32 param = atoul(sep->arg[1]);
				int32 paramvalue = atoul(sep->arg[2]);
				client->Message(CHANNEL_COLOR_YELLOW, "Send control flag param %u param value %u", param, paramvalue);
				ClientPacketFunctions::SendServerControlFlagsClassic(client, param, paramvalue);
			}
			else if(sep->IsNumber(3)) {
				int8 param1 = atoul(sep->arg[1]);
				int8 param2 = atoul(sep->arg[2]);
				int8 paramval = atoul(sep->arg[3]);
				client->Message(CHANNEL_COLOR_YELLOW, "Send control flag param1 %u param2 %u param value %u", param1, param2, paramval);
				ClientPacketFunctions::SendServerControlFlags(client, param1, param2, paramval);
			}
		}
		else if (atoi(sep->arg[0]) == 33 && sep->IsNumber(1) && sep->IsNumber(2)) {
			client->GetCurrentZone()->SendHealPacket(client->GetPlayer(), client->GetPlayer()->GetTarget() ? client->GetPlayer()->GetTarget() : client->GetPlayer(),
													 atoul(sep->arg[1]), atoul(sep->arg[2]), "TestSpell");
		}
		else if(atoi(sep->arg[0]) == 34 && sep->IsNumber(1) && sep->IsNumber(2)) {
			PacketStruct* packet = configReader.getStruct("WS_QuestRewardPackMsg", client->GetVersion());
			packet->setSubstructDataByName("reward_data", "unknown1", atoi(sep->arg[1]));
			Item* item = master_item_list.GetItem(atoul(sep->arg[2]));
			if(item) {
				packet->setSubstructArrayLengthByName("reward_data", "num_select_rewards", 1);
				packet->setArrayDataByName("select_reward_id", item->details.item_id, 0);
				packet->setItemArrayDataByName("select_item", item, client->GetPlayer(), 0, 0, World::newValue - 2);
			}

			client->QueuePacket(packet->serialize());
			safe_delete(packet);
		}
		else if(atoi(sep->arg[0]) == 35) {
			if(client->GetPlayer()->GetGroupMemberInfo() && client->GetPlayer()->GetTarget() && client->GetPlayer()->GetTarget()->IsEntity()) {
				Entity* target = (Entity*)client->GetPlayer()->GetTarget();
				if(target->GetGroupMemberInfo()) {
					PlayerGroup* group = world.GetGroupManager()->GetGroup(client->GetPlayer()->GetGroupMemberInfo()->group_id);
					PlayerGroup* group2 = world.GetGroupManager()->GetGroup(target->GetGroupMemberInfo()->group_id);
					if(group && group2) {
						group->AddGroupToRaid(group2->GetID());
						group2->AddGroupToRaid(group->GetID());
					}
				}
			}
		}
		else if(atoi(sep->arg[0]) == 36) {
			EQ2Packet* packet = client->GetPlayer()->GetRaidUpdatePacket(client->GetVersion());
			if(packet) {
				client->QueuePacket(packet);
			}
		}
		else if(atoi(sep->arg[0]) == 37) {
			Guild* guild = client->GetPlayer()->GetGuild();
			if(guild)
				guild->SendGuildMemberList();
		
		}
		else if(atoi(sep->arg[0]) == 38) {
			client->GetPlayer()->GetZone()->SendFlightPathsPackets(client);
		}
		else if(atoi(sep->arg[0]) == 39) {
			client->OpenShopWindow(nullptr, true);
		}
		else if(atoi(sep->arg[0]) == 40) {
			
			PacketStruct* packet2 = configReader.getStruct("WS_HouseStoreLog", client->GetVersion());
			if (packet2) {
				packet2->setDataByName("data", sep->arg[1]);
				packet2->setDataByName("coin_gain_session", atoul(sep->arg[2]));
				packet2->setDataByName("coin_gain_alltime", atoul(sep->arg[3]));
				packet2->setDataByName("sales_log_open", atoi(sep->arg[4]));
				EQ2Packet* outapp = packet2->serialize();
				DumpPacket(outapp);
				client->QueuePacket(outapp);
				safe_delete(packet2);
			}
		}
	}
	else {
			PacketStruct* packet2 = configReader.getStruct("WS_ExaminePartialSpellInfo", client->GetVersion());
			if (packet2) {
				packet2->setSubstructDataByName("info_header", "show_name", 1);
				packet2->setSubstructDataByName("info_header", "unknown", 7);
				packet2->setSubstructDataByName("info_header", "unknown2", 1);
				packet2->setSubstructDataByName("info_header", "unknown3", 1);
				packet2->setSubstructDataByName("info_header", "unknown4", 1);
				packet2->setSubstructDataByName("info_header", "unknown5", 1);
				packet2->setSubstructDataByName("info_header", "unknown6", "Testing");
				packet2->setSubstructDataByName("info_header", "unknown7", "Testing");
				packet2->setSubstructDataByName("info_header", "unknown8", 66);
				packet2->setSubstructDataByName("info_header", "title", "Blah Title Blah");
				packet2->setSubstructDataByName("info_header", "title_text", "Blah Blah");
				packet2->setSubstructDataByName("info_header", "title_text2", "Blah Blah2");
				packet2->setSubstructDataByName("info_header", "show_popup", 1);
				packet2->setSubstructDataByName("info_header", "packettype", 3);
				packet2->setSubstructDataByName("spell_info", "skill_name", "Testing");
				packet2->setSubstructDataByName("spell_info", "level", 19);
				packet2->setSubstructDataByName("spell_info", "tier", 1);
				packet2->setSubstructDataByName("spell_info", "health_cost", 5);
				packet2->setSubstructDataByName("spell_info", "min_class_skill_req", 3); 
				packet2->setSubstructDataByName("spell_info", "mana_cost", 6);
				packet2->setSubstructDataByName("spell_info", "req_concentration", 7);
				packet2->setSubstructDataByName("spell_info", "cast_time", 200);
				packet2->setSubstructDataByName("spell_info", "recovery", 220);
				packet2->setSubstructDataByName("spell_info", "recast", 280);
				packet2->setSubstructDataByName("spell_info", "beneficial", 1);
				packet2->setSubstructDataByName("spell_info", "maintained", 1);
				packet2->setSubstructDataByName("spell_info", "spell_book_type", 1);
				packet2->setSubstructDataByName("spell_info", "quality", 3);

				packet2->setSubstructDataByName("spell_info", "test_1a", 1);
				packet2->setSubstructDataByName("spell_info", "test_1b", 1);
				packet2->setSubstructDataByName("spell_info", "test_1c", 1);
				packet2->setSubstructDataByName("spell_info", "test_1d", 1);
				packet2->setSubstructDataByName("spell_info", "test_2a", 2);
				packet2->setSubstructDataByName("spell_info", "test_2b", 2);
				packet2->setSubstructDataByName("spell_info", "test_2c", 2);
				packet2->setSubstructDataByName("spell_info", "test_2d", 2);
				packet2->setSubstructDataByName("spell_info", "test_3", 3);
				packet2->setSubstructDataByName("spell_info", "test_4", 4);
				packet2->setSubstructDataByName("spell_info", "test_5", 5);
				packet2->setSubstructDataByName("spell_info", "test_6", 6);
				packet2->setSubstructDataByName("spell_info", "min_class_skill_req", 1);
				packet2->setSubstructDataByName("spell_info", "min_class_skill_rec", 30123);
				packet2->setSubstructDataByName("spell_info", "num_reagents", 1);
				packet2->setSubstructArrayLengthByName("spell_info", "num_reagents", 1);
				packet2->setArrayDataByName("reagent", "Alcohol");
				packet2->setArrayDataByName("consumed", "123");
				packet2->setSubstructDataByName("spell_info", "class_skill", 52);
				packet2->setSubstructDataByName("spell_info", "id", 8308);
				packet2->setSubstructDataByName("spell_info", "icon", 303);
				packet2->setSubstructDataByName("spell_info", "icon2", 0xFFFF);
				packet2->setSubstructDataByName("spell_info", "icontype", 317);
				packet2->setSubstructDataByName("spell_info", "type", 2);
				packet2->setSubstructDataByName("spell_info", "spell_text_color", 255);
				packet2->setSubstructDataByName("spell_info", "duration1", 600);
				packet2->setSubstructDataByName("spell_info", "duration2", 600);
				packet2->setSubstructDataByName("spell_info", "name", "Sprint");
				packet2->setSubstructDataByName("spell_info", "description", "Test description");
				EQ2Packet* outapp = packet2->serialize();
				DumpPacket(outapp);
				client->QueuePacket(outapp);
				safe_delete(packet2);
			}
		}
	return;
	PacketStruct* p = configReader.getStruct("WS_EqTargetItemCmd", client->GetVersion());
	if (!p) return;

	//Seperator* sep2 = new Seperator(command_parms->data.c_str(), ' ', 50, 500, true);
	//client->GetCurrentZone()->SendSpellFailedPacket(client, atoi(sep2->arg[0]));
		//}













	/*Seperator* sep2 = new Seperator(command_parms->data.c_str(), ' ', 50, 500, true);
	if (sep2 && sep2->arg[0] && sep2->IsNumber(0)) {
		client->SetPendingFlightPath(atoi(sep2->arg[0]));
		PacketStruct* packet = configReader.getStruct("WS_ReadyForTakeOff", client->GetVersion());
		if (packet) {
			client->QueuePacket(packet->serialize());
			safe_delete(packet);
		}
	}*/

	/*if (client->GetCurrentZone()->Grid != nullptr) {
		int32 numFaces = 0;
		int32 numGrids = client->GetPlayer()->Cell_Info.CurrentCell->FaceList.size();
		client->Message(CHANNEL_COLOR_YELLOW, "Num grids in cell: %u", numGrids);

		map<int32, vector<Face*> >::iterator itr;
		for (itr = client->GetPlayer()->Cell_Info.CurrentCell->FaceList.begin(); itr != client->GetPlayer()->Cell_Info.CurrentCell->FaceList.end(); itr++)
			numFaces += (*itr).second.size();

		client->Message(CHANNEL_COLOR_YELLOW, "Num faces in cell: %u", numFaces);
	}*/

	//uchar blah[] = {
		// 1208 - OP_EQUpdateStoreCmd
		// /*0x00,0x3A,*/0x2B,0x00,0x00,0x00,0xFF,0x78,0x02,0x53,0x2C,0x33,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00

		// /*0x58,*/0x47,0x35,0xD3,0x45,0x42,0x42,0x51,0x00,0x40,0xD1,0xE7,0x57,0xB1,0x51,0xB1,0xBB,0xBB,0xB1,0x3B,0xB0,0xBB,0xB0,0x3B,0x59,0x85,0x5B,0x47,0x1D,0x9C,0x3B,0x3A,0x1B,0xB8,0xD9,0x64,0x14,0x85,0x9F,0x4C,0xC8,0x09,0x21,0xA4,0xB3,0x7F,0x45,0x90,0x0B,0x79,0x90,0x0F,0x31,0x28,0x80,0x42,0x28,0x82,0x62,0x28,0x81,0x52,0x28,0x83,0x38,0x94,0x43,0x05,0x54,0x42,0x02,0xAA,0xA0,0x1A,0x6A,0xA0,0x16,0xEA,0xA0,0x1E,0x1A,0xA0,0x11,0x9A,0xA0,0x19,0x5A,0xA0,0x15,0xDA,0xA0,0x1D,0x3A,0xA0,0x13,0xBA,0xA0,0x1B,0x7A,0xA0,0x17,0xFA,0xA0,0x1F,0x06,0x60,0x10,0x86,0x60,0x18,0x46,0x60,0x14,0xC6,0x60,0x1C,0x26,0x20,0x09,0x93,0x30,0x05,0xD3,0x30,0x03,0xB3,0x30,0x07,0xF3,0xB0,0x00,0x8B,0xB0,0x04,0xCB,0xB0,0x02,0xAB,0xB0,0x06,0xEB,0xB0,0x01,0x29,0xD8,0x84,0x2D,0xD8,0x86,0x1D,0xD8,0x85,0x3D,0xD8,0x87,0x03,0x38,0x84,0x23,0x38,0x86,0x13,0x38,0x85,0x33,0x38,0x87,0x0B,0xB8,0x84,0x34,0x5C,0xC1,0x35,0xDC,0xC0,0x2D,0xDC,0xC1,0x3D,0x3C,0xC0,0x23,0x3C,0xC1,0x33,0xBC,0xC0,0x2B,0xBC,0xC1,0x3B,0x7C,0xC0,0x27,0x7C,0xC1,0x37,0x64,0xE0,0xFF,0xD3,0xF0,0x0B,0x29,0x4A,0xD8,0x68

		// 1193 - -- OP_ClientCmdMsg::OP_EqUpdateMerchantCmd --
		// /*0x00,0x3A,*/0x38,0x00,0x00,0x00,0xFF,0x77,0x02,0xA6,0xAA,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x3F,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x3F,0x00,0x00,0x00,0x00,0xA8,0x55,0xEC,0x8F,0x7F,0x70,0x31,0x08,0x40,0x71,0x3A,0x8B,0xB4,0x55,0xEC,0x8F,0x00
	/*};

	Seperator* sep2 = new Seperator(command_parms->data.c_str(), ' ', 50, 500, true);
	int8 val = 0;
	int16 pos = 0;
	int idx = 0;
	while(sep2 && sep2->arg[idx+1] && sep2->IsNumber(idx) && sep2->IsNumber(idx+1)){
		pos = atoi(sep2->arg[idx]);
		val = atoi(sep2->arg[idx+1]);
		memset(blah+pos, val, 1);
		idx+=2;
	}

	DumpPacket(blah, sizeof(blah));
	client->QueuePacket(new EQ2Packet(OP_GroupCreatedMsg , blah, sizeof(blah)));*/

	if (client->GetPlayer()->GetTarget()) {
		PacketStruct* packet = configReader.getStruct("WS_SetControlGhost", client->GetVersion());
		if (packet) {
			packet->setDataByName("spawn_id", client->GetPlayer()->GetIDWithPlayerSpawn(client->GetPlayer()->GetTarget()));
			packet->setDataByName("size", 0);
			packet->setDataByName("unknown2", 0);
			EQ2Packet* app = packet->serialize();
			client->QueuePacket(app);
			safe_delete(packet);
		}

		PacketStruct* set_pov = configReader.getStruct("WS_SetPOVGhostCmd", client->GetVersion());
		if (set_pov) {
			set_pov->setDataByName("spawn_id", client->GetPlayer()->GetIDWithPlayerSpawn(client->GetPlayer()->GetTarget()));
			EQ2Packet* app_pov = set_pov->serialize();
			client->QueuePacket(app_pov);
			safe_delete(set_pov);
		}
	}

	Seperator* sep2 = new Seperator(command_parms->data.c_str(), ' ', 50, 500, true);

	PacketStruct* packet = configReader.getStruct("WS_OpenCharCust", client->GetVersion());
	if (packet) {


		if (sep2 && sep2->arg[0] && sep2->IsNumber(0))
			packet->setDataByName("Type", atoi(sep2->arg[0]));
		else
			packet->setDataByName("Type", 2);

		if (sep2 && sep2->arg[1] && sep2->IsNumber(1))
			packet->setDataByName("race_id", atoi(sep2->arg[1]));
		else
			packet->setDataByName("race_id", 0);

		if (sep2 && sep2->arg[2] && sep2->IsNumber(2))
			packet->setDataByName("gender", atoi(sep2->arg[2]));
		else
			packet->setDataByName("gender", 2);

		if (sep2 && sep2->arg[3] && sep2->IsNumber(3))
			packet->setDataByName("unknown", atoi(sep2->arg[3]), 0);
		else
			packet->setDataByName("unknown", 0, 0);

		if (sep2 && sep2->arg[4] && sep2->IsNumber(4))
			packet->setDataByName("unknown", atoi(sep2->arg[4]), 1);
		else
			packet->setDataByName("unknown", 0, 1);

		if (sep2 && sep2->arg[5] && sep2->IsNumber(5))
			packet->setDataByName("unknown", atoi(sep2->arg[5]), 2);
		else
			packet->setDataByName("unknown", 0, 2);


		client->QueuePacket(packet->serialize());
	}
	safe_delete(packet);
	safe_delete(sep2);
}

void Commands::Command_LeaveChannel(Client *client, Seperator *sep) {
	const char *channel_name;

	if (sep == NULL || !sep->IsSet(0)) {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /leavechat <channel name>");
		PrintSep(sep);
		return;
	}

	channel_name = sep->arg[0];

	if (!chat.IsInChannel(client, channel_name))
		client->Message(CHANNEL_NARRATIVE, "Unable to leave '%s': You are not in the channel.", channel_name);
	else if (!chat.LeaveChannel(client, channel_name))
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "There was an internal error preventing you from leaving that channel MUAHAHA");
}

/* 
	Function: 
	Purpose	: 
	Params	: 
	Dev		: 
	Example	: 
*/ 
void Commands::Command_WeaponStats(Client* client)
{
	Player* player = client->GetPlayer();
	Spawn* target = player->GetTarget();
	
	Item* primary = player->GetEquipmentList()->GetItem(EQ2_PRIMARY_SLOT);
	Item* secondary = player->GetEquipmentList()->GetItem(EQ2_SECONDARY_SLOT);
	Item* ranged = player->GetEquipmentList()->GetItem(EQ2_RANGE_SLOT);
	const char* charName = player->GetName();
	if(target && target->IsEntity()) {
		primary = ((Entity*)target)->GetEquipmentList()->GetItem(EQ2_PRIMARY_SLOT);
		secondary = ((Entity*)target)->GetEquipmentList()->GetItem(EQ2_SECONDARY_SLOT);
		ranged = ((Entity*)target)->GetEquipmentList()->GetItem(EQ2_RANGE_SLOT);
		charName = target->GetName();
	}
	else {
		target = nullptr;
	}
	client->Message(0, "WeaponStats for %s", charName);
	client->SimpleMessage(0, "Primary:");
	if (primary) {
		client->Message(0, "Name: %s", primary->name.c_str());
		if(primary->weapon_info) {
			client->Message(0, "Base Damage: %u - %u", primary->weapon_info->damage_low3, primary->weapon_info->damage_high3);
			client->Message(0, "Mastery Damage: %u - %u", primary->weapon_info->damage_low2, primary->weapon_info->damage_high2);
			client->Message(0, "Damage: %u - %u", primary->weapon_info->damage_low1, primary->weapon_info->damage_high1);
		}
		else {
			client->Message(0, "WARNING: WeaponInfo not assigned to primary item");
		}
		
		client->Message(0, "Actual Damage: %u - %u", target ? ((Entity*)target)->GetPrimaryWeaponMinDamage() : player->GetPrimaryWeaponMinDamage(),
													target ? ((Entity*)target)->GetPrimaryWeaponMaxDamage() : player->GetPrimaryWeaponMaxDamage());
		client->Message(0, "Actual Delay: %u", target ? ((Entity*)target)->GetPrimaryWeaponDelay() : player->GetPrimaryWeaponDelay());
		client->Message(0, "Proc Percent: %d%%", 0);
		client->Message(0, "Procs Per Minute: %d", 0);
	}
	else {
		client->SimpleMessage(0, "Name: fist");
		client->Message(0, "Base Damage: %u - %u", target ? ((Entity*)target)->GetPrimaryWeaponMinDamage() : player->GetPrimaryWeaponMinDamage(),
												   target ? ((Entity*)target)->GetPrimaryWeaponMaxDamage() : player->GetPrimaryWeaponMaxDamage());
		client->Message(0, "Actual Damage: %u - %u", target ? ((Entity*)target)->GetPrimaryWeaponMinDamage() : player->GetPrimaryWeaponMinDamage(), 
													 target ? ((Entity*)target)->GetPrimaryWeaponMaxDamage() : player->GetPrimaryWeaponMaxDamage());
		client->Message(0, "Actual Delay: %d", target ? ((Entity*)target)->GetPrimaryWeaponDelay() : player->GetPrimaryWeaponDelay() * 0.1);
		client->Message(0, "Proc Percent: %d%%", 0);
		client->Message(0, "Procs Per Minute: %d", 0);
	}
	client->SimpleMessage(0, " ");
	client->SimpleMessage(0, " ");
	if (secondary) {
		client->SimpleMessage(0, "Secondary:");
		client->Message(0, "Name: %s", secondary->name.c_str());
		if(secondary->weapon_info) {
			client->Message(0, "Base Damage: %u - %u", secondary->weapon_info->damage_low3, secondary->weapon_info->damage_high3);
			client->Message(0, "Mastery Damage: %u - %u", secondary->weapon_info->damage_low2, secondary->weapon_info->damage_high2);
			client->Message(0, "Damage: %u - %u", secondary->weapon_info->damage_low1, secondary->weapon_info->damage_high1);
		}
		else {
			client->Message(0, "WARNING: WeaponInfo not assigned to secondary item");
		}
		client->Message(0, "Actual Damage: %u - %u", target ? ((Entity*)target)->GetSecondaryWeaponMinDamage() : player->GetSecondaryWeaponMinDamage(), 
													 target ? ((Entity*)target)->GetSecondaryWeaponMaxDamage() : player->GetSecondaryWeaponMaxDamage());
		client->Message(0, "Actual Delay: %d", target ? ((Entity*)target)->GetSecondaryWeaponDelay() : player->GetSecondaryWeaponDelay() * 0.1);
		client->Message(0, "Proc Percent: %d%%", 0);
		client->Message(0, "Procs Per Minute: %d", 0);
		client->SimpleMessage(0, " ");
		client->SimpleMessage(0, " ");
	}
	client->SimpleMessage(0, "Ranged:");
	if (ranged) {
		client->Message(0, "Name: %s", ranged->name.c_str());
		
		if(ranged->ranged_info) {
			client->Message(0, "Base Damage: %u - %u", ranged->ranged_info->weapon_info.damage_low3, ranged->ranged_info->weapon_info.damage_high3);
			client->Message(0, "Mastery Damage: %u - %u", ranged->ranged_info->weapon_info.damage_low2, ranged->ranged_info->weapon_info.damage_high2);
			client->Message(0, "Damage: %u - %u", ranged->ranged_info->weapon_info.damage_low1, ranged->ranged_info->weapon_info.damage_high1);
		}
		else {
			client->Message(0, "WARNING: RangedInfo not assigned to ranged item");
		}
		client->Message(0, "Actual Damage: %u - %u", target ? ((Entity*)target)->GetRangedWeaponMinDamage() : player->GetRangedWeaponMinDamage(), 
													 target ? ((Entity*)target)->GetRangedWeaponMaxDamage() : player->GetRangedWeaponMaxDamage());
		client->Message(0, "Actual Delay: %d",  target ? ((Entity*)target)->GetRangeWeaponDelay() : player->GetRangeWeaponDelay() * 0.1);
		client->Message(0, "Proc Percent: %d%%", 0);
		client->Message(0, "Procs Per Minute: %d", 0);
	}
	else
		client->SimpleMessage(0, "None");

}

void Commands::Command_WhoChannel(Client *client, Seperator *sep) {
	const char *channel_name;

	if (sep == NULL || !sep->IsSet(0)) {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /chatwho <channel name>");
		return;
	}

	channel_name = sep->arg[0];

	if (!chat.ChannelExists(channel_name))
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "That channel does not exist!");
	else
		chat.SendChannelUserList(client, channel_name);
}

void Commands::Command_ZoneSafeCoords(Client *client, Seperator *sep) 
{
	ZoneServer* zone = 0;
	int32 zone_id = client->GetPlayer()->GetZone()->GetZoneID();

	if (zone_id > 0)
	{
		ZoneChangeDetails zone_details;
		if(zone_list.GetZone(&zone_details, zone_id, "", false, false, false)) {
			zone = (ZoneServer*)zone_details.zonePtr;
		}
		if (zone)
		{
			zone->SetSafeX(client->GetPlayer()->GetX());
			zone->SetSafeY(client->GetPlayer()->GetY());
			zone->SetSafeZ(client->GetPlayer()->GetZ());
			zone->SetSafeHeading(client->GetPlayer()->GetHeading());
			if( database.SaveZoneSafeCoords(zone_id, zone->GetSafeX(), zone->GetSafeY(), zone->GetSafeZ(), zone->GetSafeHeading()) )
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Zone safe coordinates updated!");
			else
				client->SimpleMessage(CHANNEL_COLOR_RED, "FAILED to update zone safe coordinates!");
		}
	}
}

/* 
	Function: 
	Purpose	: 
	Params	: 
	Dev		: 
	Example	: 
*/ 
void Commands::Command_ZoneDetails(Client* client, Seperator* sep)
{
	ZoneInfo* zone_info = new ZoneInfo;

	if (sep && sep->arg[0]) 
	{
		if (sep->IsNumber(0))
			zone_info->id = atoi(sep->arg[0]);
		else
			zone_info->id = database.GetZoneID(sep->arg[0]);

		if (zone_info->id > 0) 
		{
			database.LoadZoneInfo(zone_info);
			client->Message(CHANNEL_COLOR_YELLOW, "id: %u, name: %s, file: %s, description: %s", zone_info->id, zone_info->name, zone_info->file, zone_info->description);
			client->Message(CHANNEL_COLOR_YELLOW, "safe_x: %f, safe_y: %f, safe_z: %f, underworld: %f", zone_info->safe_x, zone_info->safe_y, zone_info->safe_z, zone_info->underworld);
			client->Message(CHANNEL_COLOR_YELLOW, "min_status: %u, min_level: %u, max_level: %u, xp_modifier: %u", zone_info->min_status, zone_info->min_level, zone_info->max_level, zone_info->xp_modifier);
			client->Message(CHANNEL_COLOR_YELLOW, "instance_type: %u, shutdown_timer: %u, ruleset_id: %u", zone_info->instance_type, zone_info->shutdown_timer, zone_info->ruleset_id);
			client->Message(CHANNEL_COLOR_YELLOW, "default_reenter_time: %u, default_reset_time: %u, default_lockout_time: %u", zone_info->default_reenter_time, zone_info->default_reenter_time, zone_info->default_lockout_time);
			client->Message(CHANNEL_COLOR_YELLOW, "force_group_to_zone: %u, expansion_id: %u, min_version: %u", zone_info->force_group_to_zone, zone_info->expansion_id, zone_info->min_version);
			client->Message(CHANNEL_COLOR_YELLOW, "always_loaded: %u, city_zone: %u, start_zone: %u, weather_allowed: %u", zone_info->always_loaded, zone_info->city_zone, zone_info->start_zone, zone_info->weather_allowed);
			client->Message(CHANNEL_COLOR_YELLOW, "zone_type: %s, sky_file: %s", zone_info->zone_type, zone_info->sky_file);
			client->Message(CHANNEL_COLOR_YELLOW, "lua_script: %s", zone_info->lua_script);
			client->Message(CHANNEL_COLOR_YELLOW, "zone_motd: %s", zone_info->zone_motd);
		}
		else
			client->Message(CHANNEL_COLOR_YELLOW, "The zone name or ID '%s' does not exist.", sep->arg[0]);
	}
	else
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /zone details [zone id|zone name]");

	safe_delete(zone_info);
}

/* 
	Function: 
	Purpose	: 
	Params	: 
	Dev		: 
	Example	: 
*/ 
void Commands::Command_ZoneSet(Client* client, Seperator* sep)
{
	if (sep && sep->arg[0] && sep->arg[1] && sep->arg[2]) 
	{
		ZoneServer* zone = 0;
		int32 zone_id = 0;
		ZoneChangeDetails zone_details;
		if (sep->IsNumber(0) && atoi(sep->arg[0]) > 0) 
		{
			zone_id = atoul(sep->arg[0]);
			if(zone_list.GetZone(&zone_details, zone_id, "", false, false, false, false)) {
				zone = (ZoneServer*)zone_details.zonePtr;
			}
		}
		else 
		{
			zone_id = database.GetZoneID(sep->arg[0]);

			if(zone_list.GetZone(&zone_details, zone_id, "", false, false, false, false)) {
				zone = (ZoneServer*)zone_details.zonePtr;
			}
		}

		if (zone_id > 0) 
		{
			if (zone_set_values.count(string(sep->arg[1])) > 0)
				SetZoneCommand(client, zone_id, zone, zone_set_values[sep->arg[1]], sep->arg[2]);
			else
				client->Message(CHANNEL_COLOR_YELLOW, "The attribute '%s' is not valid.", sep->arg[1]);
		}
		else
			client->Message(CHANNEL_COLOR_YELLOW, "The zone name or ID '%s' does not exist.", sep->arg[0]);
	}
	else
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /zone set [zone id|zone name] [attribute] [value]");
}

void Commands::Command_Rain(Client* client, Seperator* sep) {
	if (sep == NULL || !sep->IsSet(0) || !sep->IsNumber(0)) {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /rain <float>");
		return;
	}

	client->Message(CHANNEL_COLOR_YELLOW,"Setting rain to %.2f", atof(sep->arg[0]));
	client->GetCurrentZone()->SetRain(atof(sep->arg[0]));
	client->GetCurrentZone()->SetCurrentWeather(atof(sep->arg[0]));
}

void Commands::Command_Wind(Client* client, Seperator* sep) {
	if (sep == NULL || !sep->IsSet(0) || !sep->IsNumber(0)) {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /wind <float>");
		return;
	}

	client->Message(CHANNEL_COLOR_YELLOW, "Setting wind to %.2f", atof(sep->arg[0]));
	client->GetCurrentZone()->SetWind(atof(sep->arg[0]));
}


void Commands::Command_SendMerchantWindow(Client* client, Seperator* sep, bool sell) {
	Spawn* spawn = client->GetPlayer()->GetTarget();
	if(spawn)
		client->SendMerchantWindow(spawn, sell);
}


void Commands::Command_Weather(Client* client, Seperator* sep) 
{
	//PrintSep(sep, "Weather");

	if( sep && sep->arg[0] )
	{
		ZoneServer* zsZone = client->GetCurrentZone();
		const char* value = sep->arg[0];

		// process single-param commands first
		if( strncasecmp(value, "details", strlen(value)) == 0 ) 
		{
			client->Message(CHANNEL_COLOR_YELLOW, "Weather Details for zone %s: ", zsZone->GetZoneName());
			client->Message(CHANNEL_COLOR_YELLOW, "enabled: %i, allowed: %i, type: %i, frequency: %u", zsZone->isWeatherEnabled(), zsZone->isWeatherAllowed(), zsZone->GetWeatherType(), zsZone->GetWeatherFrequency());
			client->Message(CHANNEL_COLOR_YELLOW, "severity: %.2f = %.2f, current: %.2f", zsZone->GetWeatherMinSeverity(), zsZone->GetWeatherMaxSeverity(), zsZone->GetCurrentWeather());
			client->Message(CHANNEL_COLOR_YELLOW, "pattern: %i, chance: %i, amount: %.2f, offset: %.2f", zsZone->GetWeatherPattern(), zsZone->GetWeatherChance(), zsZone->GetWeatherChangeAmount(), zsZone->GetWeatherDynamicOffset());
		}
		else if( strncasecmp(value, "process", strlen(value)) == 0 ) 
		{
			zsZone->SetWeatherLastChangedTime(Timer::GetUnixTimeStamp() - zsZone->GetWeatherFrequency());
			zsZone->ProcessWeather();
		}
		else if( strncasecmp(value, "reset", strlen(value)) == 0 ) 
		{
			zsZone->SetWeatherType(rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Zone, WeatherType)->GetInt8());
			zsZone->SetWeatherFrequency(rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Zone, WeatherChangeFrequency)->GetInt32());
			zsZone->SetWeatherMinSeverity(rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Zone, MinWeatherSeverity)->GetFloat());
			zsZone->SetWeatherMaxSeverity(rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Zone, MaxWeatherSeverity)->GetFloat());
			zsZone->SetCurrentWeather(zsZone->GetWeatherMinSeverity());
			zsZone->SetWeatherPattern(1);
			zsZone->SetWeatherChance(rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Zone, WeatherChangeChance)->GetInt8());
			zsZone->SetWeatherChangeAmount(rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Zone, WeatherChangePerInterval)->GetFloat());
			zsZone->SetWeatherDynamicOffset(rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Zone, WeatherDynamicMaxOffset)->GetFloat());
			zsZone->SetWeatherLastChangedTime(Timer::GetUnixTimeStamp() - zsZone->GetWeatherFrequency());
			zsZone->ProcessWeather();
		}

		// process commands with params
		if( sep->arg[1] )
		{
			if( strncasecmp(value, "enable", strlen(value)) == 0 && sep->IsNumber(1) ) 
				zsZone->SetWeatherEnabled( ( atoi(sep->arg[1]) == 1 ) ? true : false );
			else if( strncasecmp(value, "type", strlen(value)) == 0 && sep->IsNumber(1) && (atoi(sep->arg[1]) >= 0 && atoi(sep->arg[1]) <= 3) ) 
				zsZone->SetWeatherType(atoi(sep->arg[1]));
			else if( strncasecmp(value, "frequency", strlen(value)) == 0 && sep->IsNumber(1)) 
				zsZone->SetWeatherFrequency( atoul(sep->arg[1]) );
			else if( strncasecmp(value, "range", strlen(value)) == 0 && sep->IsNumber(1) && sep->IsNumber(2) ) {
				zsZone->SetWeatherMinSeverity(atof(sep->arg[1]));
				zsZone->SetWeatherMaxSeverity(atof(sep->arg[2]));
				zsZone->SetRain(zsZone->GetWeatherMinSeverity());
			}
			else if( strncasecmp(value, "current", strlen(value)) == 0 && sep->IsNumber(1) ) {
				zsZone->SetCurrentWeather(atof(sep->arg[1]));
				zsZone->SetRain(zsZone->GetCurrentWeather());
			}
			else if( strncasecmp(value, "pattern", strlen(value)) == 0 && sep->IsNumber(1) && (atoi(sep->arg[1]) >= 0 && atoi(sep->arg[1]) <= 2) )
				zsZone->SetWeatherPattern( atoi(sep->arg[1]) );
			else if( strncasecmp(value, "chance", strlen(value)) == 0 && sep->IsNumber(1) && (atoi(sep->arg[1]) >= 0 && atoi(sep->arg[1]) <= 100) )
				zsZone->SetWeatherChance( atoi(sep->arg[1]) );
			else if( strncasecmp(value, "amount", strlen(value)) == 0 && sep->IsNumber(1) && (atoi(sep->arg[1]) >= 0 && atoi(sep->arg[1]) <= 1) )
				zsZone->SetWeatherChangeAmount( atof(sep->arg[1]) );
			else if( strncasecmp(value, "offset", strlen(value)) == 0 && sep->IsNumber(1) && (atoi(sep->arg[1]) >= 0 && atoi(sep->arg[1]) <= 1) )
				zsZone->SetWeatherDynamicOffset( atof(sep->arg[1]) );
		}
	}
	else
	{
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /weather [command] [param]");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Commands: enable (0|1), type (0-3), frequency (sec), range|current (0.0 - 1.0)");
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Commands: details, process, pattern (0|1), chance (%), amount (float), offset (float)");
		return;
	}

}

void Commands::Command_Select(Client* client, Seperator* sep) {
	if (sep && sep->arg[0]) {
		Spawn* spawn = client->GetPlayer()->GetTarget();
		int32 spawn_id = client->dialog_manager.getAcceptValue("select");
		if(spawn && spawn_id == spawn->GetID()) {
			client->GetCurrentZone()->CallSpawnScript(spawn, SPAWN_SCRIPT_CASTED_ON, client->GetPlayer(), sep->arg[0] ? sep->argplus[0] : "");
		}
		else if (spawn && sep->arg[1] && strlen(sep->arg[1]) > 0)
			client->GetCurrentZone()->CallSpawnScript(spawn, SPAWN_SCRIPT_CUSTOM, client->GetPlayer(), sep->arg[1]);
	}
}

/* 
	Function: Command_ConsumeFood()
	Purpose	: Consume Food/Drink and apply mods
	Params	: Slot ID - EQ2_FOOD_SLOT 22, EQ2_DRINK_SLOT 23
	Dev		: Zcoretri
	Example	: /consume_food 22
*/ 
void Commands::Command_ConsumeFood(Client* client, Seperator* sep) {
	if (sep && sep->arg[0] && sep->IsNumber(0))
	{
		Player* player = client->GetPlayer();
		int32 slot = atoul(sep->arg[0]);
		if (client->GetVersion() <= 373) {
			if(client->GetVersion() <= 561) {
				if(slot <= 255) {
					slot = 255 - slot;
				}
				else {
					if(slot == 256) { // first "new" item to inventory is assigned index 256 by client
						slot = client->GetPlayer()->item_list.GetFirstNewItem();
					}
					else {
						// otherwise the slot has to be mapped out depending on the amount of new items + index sent in
						slot = client->GetPlayer()->item_list.GetNewItemByIndex((int16)slot - 255);
					}
				}
			}
			
			Item* item = client->GetPlayer()->item_list.GetItemFromIndex(slot);
			if(item && item->IsFood()) {
				if(client->CheckConsumptionAllowed(slot)) {
					if(item->IsFoodFood())
						slot = EQ2_FOOD_SLOT;
					else if(item->IsFoodDrink())
						slot = EQ2_DRINK_SLOT;
					else
						return; // not valid item
					
					client->ConsumeFoodDrink(item, slot);
				}
			}
		}
		else {
				if (client->GetVersion() > 373 && client->GetVersion() <= 561) {
					slot += 2;
				}
				Item* item = player->GetEquipmentList()->GetItem(slot);
				
				if(client->CheckConsumptionAllowed(slot)) {
					client->ConsumeFoodDrink(item, slot);
				}
		}
	}
}

void Commands::Command_Aquaman(Client* client, Seperator* sep) {
	if (sep && sep->arg[0] && sep->IsNumber(0)) {
		if (atoi(sep->arg[0]) == 1) {
			client->GetPlayer()->GetInfoStruct()->set_vision(4);
			client->GetPlayer()->GetInfoStruct()->set_breathe_underwater(1);
			client->GetPlayer()->SetCharSheetChanged(true);
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Maybe you ought to stick to the shallow end until you know how to swim.");
		}
		else {
			client->GetPlayer()->GetInfoStruct()->set_vision(0);
			client->GetPlayer()->GetInfoStruct()->set_breathe_underwater(0);
			client->GetPlayer()->SetCharSheetChanged(true);
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Aquaman mode turned off.");
		}
	}
	else
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage: /aquaman [0|1]");
}

void Commands::Command_ReportBug(Client* client, Seperator* sep) 
{
	if(sep)
	{ 
		string data;

		if(sep->arg[0]){
			data = string(sep->arg[0]);
		}

		for(int i=1;i<sep->GetMaxArgNum();i++){
			if(sep->arg[i])
				data.append(" ").append(sep->arg[i]);
		}

		if(!sep->IsSet(7)){
        data.append(" ").append(std::to_string(client->GetVersion())).append("\a");
		}
	
		const char* target_name = 0;
		int32 spawn_id = 0;

		if(client->GetPlayer()->GetTarget())
		{
			target_name = client->GetPlayer()->GetTarget()->GetName();
			spawn_id = client->GetPlayer()->GetTarget()->GetDatabaseID();
		}
		else
			target_name = "N/A";

		LogWrite(COMMAND__DEBUG, 1, "Command", "%s", data.c_str());

		if(world.ReportBug(data, client->GetPlayer()->GetName(), client->GetAccountID(), target_name, spawn_id, client->GetCurrentZone()->GetZoneID()))
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Successfully submitted bug.");
		else
			client->SimpleMessage(CHANNEL_COLOR_RED, "Error submitting bug.");
	}
}

void Commands::Command_Attune_Inv(Client* client, Seperator* sep) {
	PrintSep(sep, "Command_Attune_Inv");
	if (sep && sep->arg[0] && sep->IsNumber(0)) {
		// Get the item index from the first parameter
		int16 index = atoi(sep->arg[0]);
		// Check to see if this is a valid item index for the player, if not exit out of the function
		if (client->GetPlayer()->item_list.indexed_items.count(index) == 0) {
			LogWrite(ITEM__DEBUG, 0, "Items", "%s has no item with an index of %i", client->GetPlayer()->GetName(), index);
			return;
		}

		// Get the item
		Item* item = client->GetPlayer()->item_list.indexed_items[index];
		if(item) {
			// Valid item lets check to make sure this item is attunable, if not return out
			if (!item->CheckFlag(ATTUNEABLE)) {
				LogWrite(ITEM__DEBUG, 0, "Items", "attune_inv called for an item that is not attunable (%s)", item->name.c_str());
				return;
			}

			// Remove the attunable flag
			item->generic_info.item_flags -= ATTUNEABLE;
			// Set the attuned flag
			item->generic_info.item_flags += ATTUNED;
			// Flag this item for saving
			item->save_needed = true;

			client->QueuePacket(item->serialize(client->GetVersion(), false, client->GetPlayer()));

			vector<EQ2Packet*> packets = client->GetPlayer()->EquipItem(index, client->GetVersion(), 0, -1); // appearance type??
			EQ2Packet* outapp = 0;

			for (int32 i=0;i<packets.size();i++) {
				outapp = packets[i];
				if(outapp)
					client->QueuePacket(outapp);
			}
			
			client->GetPlayer()->CalculateBonuses();
		}

	}
}

void Commands::Command_Reset_Zone_Timer(Client* client, Seperator* sep) {
	PrintSep(sep, "Command_Reset_Zone_Timer");
	/*if (sep && sep->arg[0] && sep->IsNumber(0)) {
		int32 db_id = atoul(sep->arg[0]);
		InstanceData* data = client->GetPlayer()->GetCharacterInstances().FindInstanceByDBID(db_id);
		if (data) {
			// TODO: add a check to timers to ensure it can be reset

			// Delete the character from the instance
			database.DeleteCharacterFromInstance(client->GetPlayer()->GetCharacterID(), data->instance_id);
			data->instance_id = 0;

			// Update the success time and set to 0 so the player can enter it again
			database.UpdateCharacterInstance(client->GetPlayer()->GetCharacterID(), data->zone_name, 0, 1, 0);
			data->last_success_timestamp = 0;
		}
	}*/
}

void Commands::Command_Player(Client* client, Seperator* sep) {
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, " -- /player syntax --");
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/player coins");
}

void Commands::Command_Player_Coins(Client* client, Seperator* sep) {
	// /player coins add 10
	// /player coins add plat 10
	Player* player = client->GetPlayer();
	Client* targetClient = client;
	if (player->HasTarget() && player->GetTarget()->IsPlayer()) {
		player = (Player*)player->GetTarget();
		targetClient = player->GetClient();
	}

	if (sep && sep->arg[0] && sep->arg[1]) {
		const char* action = sep->arg[0];
		int64 value = 0;

		if (strncasecmp(action, "add", strlen(action)) == 0) {
			if (sep->IsNumber(1)) {
				value = atoi64(sep->arg[1]);
				player->AddCoins(value);
				
				if (client->GetPlayer() == player)
					client->Message(CHANNEL_COLOR_YELLOW, "You give yourself %llu coin%s", value, (value > 1 ? "s" : ""));
				else {
					client->Message(CHANNEL_COLOR_YELLOW, "You give %s %llu coin%s", player->GetName(), value, (value > 1 ? "s" : ""));
					if(targetClient) {
						targetClient->Message(CHANNEL_COLOR_YELLOW, "%s gave you %llu coin%s", client->GetPlayer()->GetName(), value, (value > 1 ? "s" : ""));
					}
				}

				return;
			}
			else if (sep->arg[2] && sep->IsNumber(2)) {
				const char* type = sep->arg[1];
				if (strncasecmp(type, "copper", strlen(type)) == 0) {
					value = atoi64(sep->arg[2]);
				}
				else if (strncasecmp(type, "silver", strlen(type)) == 0) {
					value = atoi64(sep->arg[2]) * 100;
				}
				else if (strncasecmp(type, "gold", strlen(type)) == 0) {
					value = atoi64(sep->arg[2]) * 10000;
				}
				else if (strncasecmp(type, "plat", strlen(type)) == 0) {
					value = atoi64(sep->arg[2]) * 1000000;
				}
				player->AddCoins(value);
				
				if (client->GetPlayer() == player)
					client->Message(CHANNEL_COLOR_YELLOW, "You give yourself %llu coin%s", value, (value > 1 ? "s" : ""));
				else {
					client->Message(CHANNEL_COLOR_YELLOW, "You give %s %llu coin%s", player->GetName(), value, (value > 1 ? "s" : ""));
					if(targetClient) {
						targetClient->Message(CHANNEL_COLOR_YELLOW, "%s gave you %llu coin%s", client->GetPlayer()->GetName(), value, (value > 1 ? "s" : ""));
					}
				}
				return;
			}
		}
	}

	client->SimpleMessage(CHANNEL_COLOR_YELLOW, " -- /player coins syntax --");
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/player coins add [value] - adds the given number of coins to the player");
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/player coins add copper [value] - adds the given amount of copper to the player");
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/player coins add silver [value] - adds the given amount of silver to the player");
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/player coins add gold [value] - adds the given amount of gold to the player");
	client->SimpleMessage(CHANNEL_COLOR_YELLOW, "/player coins add plat [value] - adds the given amount of platinum to the player");
}

void Commands::Command_AchievementAdd(Client* client, Seperator* sep) {
	PrintSep(sep, "ACHIEVEMENT_ADD");
	if (sep && sep->IsSet(0)) {
		int32 spell_id = atoul(sep->arg[1]);
		int8 spell_tier = 0;
		spell_tier = client->GetPlayer()->GetSpellTier(spell_id);
		AltAdvanceData* data = master_aa_list.GetAltAdvancement(spell_id);
		// addspellbookentry here
		if (spell_tier >= data->maxRank) {
			return;
		}
		if (!spell_tier) {
			spell_tier = 1;
		}
		if (!client->GetPlayer()->HasSpell(spell_id, 0, true))
		{
			Spell* spell = master_spell_list.GetSpell(spell_id, spell_tier);
			client->GetPlayer()->AddSpellBookEntry(spell_id, 1, client->GetPlayer()->GetFreeSpellBookSlot(spell->GetSpellData()->spell_book_type), spell->GetSpellData()->spell_book_type, spell->GetSpellData()->linked_timer, true);
			client->GetPlayer()->UnlockSpell(spell);
			client->SendSpellUpdate(spell);
		}
		else
		{
			Spell* spell = master_spell_list.GetSpell(spell_id, spell_tier + 1);
			int8 old_slot = client->GetPlayer()->GetSpellSlot(spell->GetSpellID());
			client->GetPlayer()->RemoveSpellBookEntry(spell->GetSpellID());
			client->GetPlayer()->AddSpellBookEntry(spell->GetSpellID(), spell->GetSpellTier(), old_slot, spell->GetSpellData()->spell_book_type, spell->GetSpellData()->linked_timer, true);
			client->GetPlayer()->UnlockSpell(spell);
			client->SendSpellUpdate(spell);
		}


		// cast spell here
		if (!spell_tier)
			spell_tier = 1;
		Spell* spell = master_spell_list.GetSpell(spell_id, spell_tier);
		if (spell) {
			client->GetCurrentZone()->ProcessSpell(spell, client->GetPlayer(), client->GetPlayer());
		}
	}
	else {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage:  /useability {spell_id} [spell_tier]");
	}
	if(client->GetVersion() > 561)
		master_aa_list.DisplayAA(client, 0, 0);
}

void Commands::Command_Editor(Client* client, Seperator* sep) {
	PacketStruct* packet = configReader.getStruct("WS_ChoiceWindow", client->GetVersion());
	if (packet) {
		string url = string(rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_World, EditorURL)->GetString());

		if (rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_World, EditorOfficialServer)->GetBool()) {
			char command[255];
			url = "browser " + url;
			int32 spawn_id = 0;
			int32 zone_id = 0;
			string type;

			if (client->GetCurrentZone())
				zone_id = client->GetCurrentZone()->GetZoneID();

			if (client->GetPlayer()->GetTarget()) {
				Spawn* target = client->GetPlayer()->GetTarget();
				if (target->IsWidget())
					type = "widgets";
				else if (target->IsSign())
					type = "signs";
				else if (target->IsObject())
					type = "objects";
				else if (target->IsNPC())
					type = "npcs";
				else if (target->IsGroundSpawn())
					type = "ground";
				else
					type = "spawn";

				spawn_id = client->GetPlayer()->GetTarget()->GetDatabaseID();
			}

			sprintf(command, url.c_str(), zone_id, type.c_str(), spawn_id);

			packet->setDataByName("accept_command", command);
		}
		else if (rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_World, EditorIncludeID)->GetBool()) {
			char command[255];
			url = "browser " + url;
			if (client->GetPlayer()->GetTarget())
				sprintf(command, url.c_str(), client->GetPlayer()->GetTarget()->GetDatabaseID());

			packet->setDataByName("accept_command", command);
		}
		else {
			string command = "browser " + url;
			packet->setDataByName("accept_command", command.c_str());
		}

		packet->setDataByName("text", "Open the web editor?");
		packet->setDataByName("accept_text", "Open");
		
		packet->setDataByName("cancel_text", "Cancel");
		// No clue if we even need the following 2 unknowns, just added them so the packet matches what live sends
		packet->setDataByName("unknown2", 50);
		packet->setDataByName("unknown4", 1);

		client->QueuePacket(packet->serialize());
		safe_delete(packet);
	}
}

void Commands::Command_AcceptResurrection(Client* client, Seperator* sep) {
	if(!client || !sep || client->GetPlayer()->GetID() != atoul(sep->arg[0]))
		return;
	client->GetResurrectMutex()->writelock(__FUNCTION__, __LINE__);
	if(client->GetCurrentRez()->active)
		client->AcceptResurrection();
	client->GetResurrectMutex()->releasewritelock(__FUNCTION__, __LINE__);
}

void Commands::Command_DeclineResurrection(Client* client, Seperator* sep) {
	if(!client || !sep || client->GetPlayer()->GetID() != atoul(sep->arg[0]))
		return;
	client->GetResurrectMutex()->writelock(__FUNCTION__, __LINE__);
	if(client->GetCurrentRez()->active)
		client->GetCurrentRez()->should_delete = true;
	client->GetResurrectMutex()->releasewritelock(__FUNCTION__, __LINE__);
}
void Commands::Switch_AA_Profile(Client* client, Seperator* sep) {
	PrintSep(sep, "Switch_AA_Profile");
		if (sep && sep->IsSet(0)) {
		string type = sep->arg[0];
		int8 newtemplate = atoul(sep->arg[1]);
		if(client->GetVersion() > 561)
			master_aa_list.DisplayAA(client, newtemplate, 1);
	}
}
void Commands::Get_AA_Xml(Client* client, Seperator* sep) {
	PrintSep(sep, "Get_AA_Xml");
	if (sep && sep->IsSet(0)) {
		string tabnum = sep->arg[0];
		string spellid = sep->arg[1];



	}
}
void Commands::Add_AA(Client* client, Seperator* sep) {
	PrintSep(sep, "Add_AA");
	if (sep && sep->IsSet(0)) {
		int32 spell_id = atoul(sep->arg[1]);
		int8 spell_tier = 0;
		spell_tier = client->GetPlayer()->GetSpellTier(spell_id);
		AltAdvanceData* data = master_aa_list.GetAltAdvancement(spell_id);
		// addspellbookentry here
		if(!data) {
		LogWrite(COMMAND__ERROR, 0, "Command", "Error in Add_AA no data for spell_id %u spell_tier %u", spell_id, spell_tier);
			return;
		}
		if (spell_tier >= data->maxRank) {
		LogWrite(COMMAND__ERROR, 0, "Command", "Error in Add_AA spell_tier %u >= maxRank %u", spell_tier, data->maxRank);
			return;
		}
		if (!spell_tier) {
			spell_tier = 1;
		}
		if (!client->GetPlayer()->HasSpell(spell_id, 0, true))
		{
			Spell* spell = master_spell_list.GetSpell(spell_id, spell_tier);
			if(spell)
			{
				client->GetPlayer()->AddSpellBookEntry(spell_id, 1, client->GetPlayer()->GetFreeSpellBookSlot(spell->GetSpellData()->spell_book_type), spell->GetSpellData()->spell_book_type, spell->GetSpellData()->linked_timer, true);
				client->GetPlayer()->UnlockSpell(spell);
				client->SendSpellUpdate(spell);
			}
		}
		else
		{
			Spell* spell = master_spell_list.GetSpell(spell_id, spell_tier + 1 );
			if(spell)
			{
				int8 old_slot = client->GetPlayer()->GetSpellSlot(spell->GetSpellID());
				client->GetPlayer()->RemoveSpellBookEntry(spell->GetSpellID());
				client->GetPlayer()->AddSpellBookEntry(spell->GetSpellID(), spell->GetSpellTier(), old_slot, spell->GetSpellData()->spell_book_type, spell->GetSpellData()->linked_timer, true);
				client->GetPlayer()->UnlockSpell(spell);
				client->SendSpellUpdate(spell);
			}
		}
		
		
		// cast spell here
		if (!spell_tier)
			spell_tier = 1;
		Spell* spell = master_spell_list.GetSpell(spell_id, spell_tier);
		if (spell) {
			client->GetCurrentZone()->ProcessSpell(spell, client->GetPlayer(), client->GetPlayer());
		}
	}
	else {
		client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Usage:  /useability {spell_id} [spell_tier]");
	}
	if(client->GetVersion() > 561)
		master_aa_list.DisplayAA(client, 0, 0);
}
void Commands::Commit_AA_Profile(Client* client, Seperator* sep) {
	PrintSep(sep, "Commit_AA_Profile");
	if (sep && sep->IsSet(0)) {


	}
}
void Commands::Begin_AA_Profile(Client* client, Seperator* sep) {
	PrintSep(sep, "Begin_AA_Profile");
	if (sep && sep->IsSet(0)) {
		if(client->GetVersion() > 561)
			master_aa_list.DisplayAA(client, 100, 2);
	}
}
void Commands::Back_AA(Client* client, Seperator* sep) {
	if (sep && sep->IsSet(0)) {
		PrintSep(sep, "Back_AA");

	}
}
void Commands::Remove_AA(Client* client, Seperator* sep) {
	if (sep && sep->IsSet(0)) {
		PrintSep(sep, "Remove_AA");

	}
}

void Commands::Cancel_AA_Profile(Client* client, Seperator* sep) {
	MasterAAList master_aa_list;
	PrintSep(sep, "Cancel_AA_Profile");	
	if(client->GetVersion() > 561)
		master_aa_list.DisplayAA(client, 0, 0);

}
void Commands::Save_AA_Profile(Client* client, Seperator* sep) {
	if (sep && sep->IsSet(0)) {
		PrintSep(sep, "Save_AA_Profile");

	}
}

void Commands::Command_TargetItem(Client* client, Seperator* sep) {
	if (!sep || sep->GetArgNumber() < 1 || !client->GetPlayer()) return;

	if (!sep->IsNumber(0)) return;

	int32 request_id = atoul(sep->arg[0]);

	if (!sep->IsNumber(1)) return;

	sint32 item_id = atoi(sep->arg[1]);

	if (client->IsCurrentTransmuteID(request_id)) {
		Transmute::HandleItemResponse(client, client->GetPlayer(), request_id, reinterpret_cast<int32&>(item_id));
	}
	else if (client->IsCurrentTransmuteID(item_id)) {
		if (!sep->IsSet(2)) return;

		if (sep->IsNumber(2) && atoi(sep->arg[2]) == 1) {
			Transmute::HandleConfirmResponse(client, client->GetPlayer(), reinterpret_cast<int32&>(item_id));
		}
	}
}

void Commands::Command_FindSpawn(Client* client, Seperator* sep) {
	if(sep)
		client->GetCurrentZone()->FindSpawn(client, (char*)sep->argplus[0]);
}

void Commands::Command_MoveCharacter(Client* client, Seperator* sep) {
	if(sep && sep->arg[0][0] && sep->arg[1][0])
	{
		char* name = sep->arg[0];
		char* zoneName = sep->arg[1];
		int32 zone_duplicating_id = 0;
		if(sep->arg[2][0])
			zone_duplicating_id = atoul(sep->arg[2]);
		
		char query[256];
		snprintf(query, 256, "UPDATE characters c, zones z set c.x = z.safe_x, c.y = z.safe_y, c.z = z.safe_z, c.heading = z.safe_heading, c.current_zone_id = z.id, c.zone_duplicating_id = %u where c.name = '%s' and z.name='%s'", zone_duplicating_id, name, zoneName);
		if (database.RunQuery(query, strnlen(query, 256)))
		{
			client->Message(CHANNEL_COLOR_YELLOW, "Ran query:%s", query);
		}
		else
			client->Message(CHANNEL_COLOR_RED, "Query FAILED to run: %s", query);
	}
}

void Commands::Command_Mood(Client* client, Seperator* sep) {
Player* player = client->GetPlayer();

	if( sep && sep->arg[0] )
	{
		const char* value = sep->arg[0];
		InfoStruct* info = player->GetInfoStruct();
		int32 cid = client->GetCharacterID();
		char* characterName = database.GetCharacterName(cid);
		char tmp[1024]; // our emote string "xyz appears zyx"
		//char properties vals
		char* pname = "mood";
		char* pval; // mood value
		bool pt; //used to verify return from DB.

		//This should never be seen.
		sprintf(tmp, " ");
		if( strncasecmp(value, "angry", strlen(value)) == 0 ) 
		{
			sprintf(tmp, "%s appears angry", characterName);
			pval = "11852";
			player->SetMoodState(11852, 1);
			info->set_mood(11852);
			pt = database.insertCharacterProperty(client, pname, pval);
		}
		else if( strncasecmp(value, "afraid", strlen(value)) == 0 ) 
		{
			sprintf(tmp, "%s appears afraid", characterName);
			pval = "11851";
			player->SetMoodState(11851, 1);
			info->set_mood(11851);
			pt = database.insertCharacterProperty(client, pname, pval);
		} 
		else if( strncasecmp(value, "happy", strlen(value)) == 0 ) 
		{
			sprintf(tmp, "%s appears happy", characterName);
			pval = "11854";
			player->SetMoodState(11854, 1);
			info->set_mood(11854);
			pt = database.insertCharacterProperty(client, pname, pval);
		} 
		else if( strncasecmp(value, "sad", strlen(value)) == 0 ) {
			sprintf(tmp, "%s appears sad", characterName);
			pval = "11856";
			player->SetMoodState(11856, 1);
			info->set_mood(11856);
			pt = database.insertCharacterProperty(client, pname, pval);
		}
		else if( strncasecmp(value, "tired", strlen(value)) == 0 ) 
		{
			sprintf(tmp, "%s appears tired", characterName);
			pval = "11857";
			player->SetMoodState(11857, 1);
			info->set_mood(11857);
			pt = database.insertCharacterProperty(client, pname, pval);
		}
		else if( strncasecmp(value, "none", strlen(value)) == 0 ) 
		{
			//using 11855 mood_idle for none, I assume thats what its for?
			pval = "11855";
			player->SetMoodState(11855, 1);
			info->set_mood(11855);
			pt = database.insertCharacterProperty(client, pname, pval);
			//return since we have nothing left to do. No emote for none.
			return;
		}else{
			client->SimpleMessage(CHANNEL_NARRATIVE, "Listing Available Moods:");
			client->SimpleMessage(CHANNEL_NARRATIVE, "none");
			client->SimpleMessage(CHANNEL_NARRATIVE, "afraid");
			client->SimpleMessage(CHANNEL_NARRATIVE, "angry");
			client->SimpleMessage(CHANNEL_NARRATIVE, "happy");
			client->SimpleMessage(CHANNEL_NARRATIVE, "sad");
			client->SimpleMessage(CHANNEL_NARRATIVE, "tired");
			return;
			}
			
		client->GetPlayer()->GetZone()->HandleChatMessage(0, 0, CHANNEL_EMOTE, tmp);
		return;
	}
	
	client->SimpleMessage(CHANNEL_NARRATIVE, "Listing Available Moods:");
	client->SimpleMessage(CHANNEL_NARRATIVE, "none");
	client->SimpleMessage(CHANNEL_NARRATIVE, "afraid");
	client->SimpleMessage(CHANNEL_NARRATIVE, "angry");
	client->SimpleMessage(CHANNEL_NARRATIVE, "happy");
	client->SimpleMessage(CHANNEL_NARRATIVE, "sad");
	client->SimpleMessage(CHANNEL_NARRATIVE, "tired");
	return;
}

/* 
	Function: Command_CancelEffect()
	Purpose	: Cancels (good) effect spells
	Example	: /cancel_effect spell_id - would cancel the spell with the <spell_id> value in spell effects list
*/ 
void Commands::Command_CancelEffect(Client* client, Seperator* sep)
{
	if (sep && sep->arg[0] && sep->IsNumber(0)) 
	{
		int32 spell_id = atoul(sep->arg[0]);
		
		SpellEffects* effect = client->GetPlayer()->GetSpellEffect(spell_id);
		if(!effect || effect->spell->spell->GetSpellData()->det_type) {
			return;
		}
		
		MaintainedEffects* meffect = effect->caster->GetMaintainedSpell(spell_id);
		
		if (!meffect || !meffect->spell || !meffect->spell->caster || !meffect->spell->caster->GetZone() ||
		!meffect->spell->caster->GetZone()->GetSpellProcess()->DeleteCasterSpell(meffect->spell, "canceled", false, client->GetPlayer()))
			client->Message(CHANNEL_COLOR_RED, "The spell effect could not be cancelled.");
	}
}


/* 
	Function: Command_CurePlayer()
	Purpose	: Identifies spell to cast for cure based on type
	Example	: /cureplayer ??
	https://eq2.fandom.com/wiki/Update:58
	New command /cureplayer [playername|group or raid position][trauma|arcane|noxious|elemental|curse] optional [spell|potion]

    Example: /cureplayer g0 noxious spell
        Will attempt to cure yourself of a noxious detriment with only spells and without using potions (even if you have them).
    Example: /cureplayer r4 noxious
        Will attempt to cure the character in raid slot 4 of a noxious detriment using a spell or potion (whichever is available).
*/ 
void Commands::Command_CurePlayer(Client* client, Seperator* sep)
{
	Entity* target = nullptr;
	bool use_spells = true;
	bool use_potions = true;
	if (sep && sep->arg[0] && sep->arg[1]) {
		if(sep->arg[2]) {
			std::string type(sep->arg[2]);
			boost::algorithm::to_lower(type);
			if(type == "potion")
				use_spells = false;
			else if(type == "spell")
				use_potions = false;
		}
		
		if(strlen(sep->arg[0]) > 1 && isdigit(sep->arg[0][1])) {
			int32 mapped_position = (int32)(sep->arg[0][1]) - 48;
			
			// TODO: RAID Support ('r' argument)
			if(sep->arg[0][0] == 'g' && !mapped_position) {
				target = (Entity*)client->GetPlayer();
			}
			else if(sep->arg[0][0] == 'r') {
				std::vector<int32> raidGroups;
				GroupMemberInfo* gmi = client->GetPlayer()->GetGroupMemberInfo();
				if(gmi)
					world.GetGroupManager()->GetRaidGroups(gmi->group_id, &raidGroups);
				if(raidGroups.size() < 1) {
					if (gmi && gmi->group_id) {
						raidGroups.push_back(gmi->group_id);
					}
				}
				int8 group_idx = mapped_position / 6;
				if(group_idx < raidGroups.size()) {
					PlayerGroup* group = world.GetGroupManager()->GetGroup(raidGroups.at(group_idx));
					if(group) {
						int8 actual_idx = mapped_position - (group_idx * 6);
						target = group->GetGroupMemberByPosition(client->GetPlayer(), actual_idx);
					}
				}
			}
			else {
				GroupMemberInfo* gmi = client->GetPlayer()->GetGroupMemberInfo();
				if (gmi && gmi->group_id) {
					PlayerGroup* group = world.GetGroupManager()->GetGroup(gmi->group_id);
					if(group) {
						target = group->GetGroupMemberByPosition(client->GetPlayer(), mapped_position);
					}
				}
			}
		}
		else {
				Client* target_client = zone_list.GetClientByCharName(sep->arg[0]);
				if(target_client && target_client->GetPlayer() && target_client->GetPlayer()->GetZone() == client->GetPlayer()->GetZone()) {
					target = (Entity*)target_client->GetPlayer();
				}
		}
		
		ItemEffectType type = EFFECT_CURE_TYPE_ALL;
		bool successful_spell = false;
		bool successful_potion = false;
		if(target) {
			SpellBookEntry* entry = nullptr;
			std::string str(sep->arg[1]);
			boost::algorithm::to_lower(str);
					if(str == "arcane") {
						type = EFFECT_CURE_TYPE_ARCANE;
						// cure arcane spell missing in DB?
						if(use_spells && rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Spells, CureArcaneSpellID)->GetInt32()) {
							entry = client->GetPlayer()->GetSpellBookSpell(rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Spells, CureArcaneSpellID)->GetInt32()); // cure noxious
						}
					}
					else if(str == "trauma") {
						type = EFFECT_CURE_TYPE_TRAUMA;
						// cure trauma spell missing in DB?
						if(use_spells && rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Spells, CureTraumaSpellID)->GetInt32()) {
							entry = client->GetPlayer()->GetSpellBookSpell(rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Spells, CureTraumaSpellID)->GetInt32()); // cure noxious
						}
					}
					else if(str == "noxious") {
						type = EFFECT_CURE_TYPE_NOXIOUS;
						if(use_spells && rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Spells, CureNoxiousSpellID)->GetInt32()) {
							entry = client->GetPlayer()->GetSpellBookSpell(rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Spells, CureNoxiousSpellID)->GetInt32()); // cure noxious
						}
					}
					else if(str == "curse") {
						type = EFFECT_CURE_TYPE_CURSE;
						if(use_spells && rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Spells, CureCurseSpellID)->GetInt32()) {
							entry = client->GetPlayer()->GetSpellBookSpell(rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Spells, CureCurseSpellID)->GetInt32()); // cure curse
						}
					}
					else if(str == "magic") {
						type = EFFECT_CURE_TYPE_MAGIC;
						if(use_spells && rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Spells, CureMagicSpellID)->GetInt32()) {
							entry = client->GetPlayer()->GetSpellBookSpell(rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Spells, CureMagicSpellID)->GetInt32()); // cure magic
						}
					}
				
				if(use_spells) {
					// check if any of the specific cure types are available, if not then check the base cures
					if(!entry && rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Spells, CureSpellID)->GetInt32()) {
						entry = client->GetPlayer()->GetSpellBookSpell(rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Spells, CureSpellID)->GetInt32()); // cure
					}
					
					if(entry && entry->spell_id) {
						Spell* spell = master_spell_list.GetSpell(entry->spell_id, entry->tier);
						target->GetZone()->ProcessSpell(spell, (Entity*)client->GetPlayer(), target, true, false);
						successful_spell = true;
					}
				}
		}
		
		if(!successful_spell && use_potions && target && type != NO_EFFECT_TYPE) {
			vector<Item*>* potential_items = client->GetItemsByEffectType(type, EFFECT_CURE_TYPE_ALL);
			if (potential_items && potential_items->size() > 0) {
				vector<Item*>::iterator itr;
				for (itr = potential_items->begin(); itr != potential_items->end(); itr++)
				{
					Item* item = *itr;
					if(client->UseItem(item, target)) {
						successful_potion = true;
						break;
					}
				}
			}
			safe_delete(potential_items);
		}
	}
}


/* 
	Function: Command_ShareQuest()
	Purpose	: Share quest with the group
	Example	: /share_quest [quest_id]
*/ 
void Commands::Command_ShareQuest(Client* client, Seperator* sep)
{
	if (sep && sep->arg[0] && sep->IsNumber(0)) {
		int32 quest_id = atoul(sep->arg[0]);
		
		bool hasQuest = client->GetPlayer()->HasAnyQuest(quest_id);
		if(hasQuest) {
		Quest* quest = master_quest_list.GetQuest(quest_id, false);
			if(quest) {
				GroupMemberInfo* gmi = client->GetPlayer()->GetGroupMemberInfo();
				if (gmi && gmi->group_id) {
					PlayerGroup* group = world.GetGroupManager()->GetGroup(gmi->group_id);
					if (group) {
						group->ShareQuestWithGroup(client, quest);
					}
				}
			}
		}
		else {
			client->SimpleMessage(CHANNEL_COLOR_RED, "Cannot find quest.");
		}
	}
}



/* 
	Function: Command_Yell()
	Purpose	: Yell to break an encounter
	Example	: /yell
	* Uses self target, encounter target or no target
*/ 
void Commands::Command_Yell(Client* client, Seperator* sep) {
	Entity* player = (Entity*)client->GetPlayer();
	Spawn* res = nullptr;
	Spawn* prev = nullptr;
	bool cycleAll = false;
	if (player->GetTarget() == player) {
		cycleAll = true; // self target breaks all encounters
	}
	else if (player->GetTarget()) {
		res = player->GetTarget(); // selected target other than self only dis-engages that encounter
	}

	if (res && !client->GetPlayer()->IsEngagedBySpawnID(res->GetID()))
		return;

	bool groupPermissionYell = true;

	GroupMemberInfo* gmi = client->GetPlayer()->GetGroupMemberInfo();
	// If the player has a group and has a target
	if (gmi) {
		deque<GroupMemberInfo*>::iterator itr;

		world.GetGroupManager()->GroupLock(__FUNCTION__, __LINE__);

		PlayerGroup* group = world.GetGroupManager()->GetGroup(gmi->group_id);
		if (group && !group->GetGroupOptions()->default_yell && !gmi->leader) { // default_yell_method = 0 means leader only
			groupPermissionYell = false;
		}

		world.GetGroupManager()->ReleaseGroupLock(__FUNCTION__, __LINE__);
	}

	if (!groupPermissionYell) {
		LogWrite(COMMAND__ERROR, 0, "Command", "%s permission to yell denied due to group yell method set to leader only", client->GetPlayer()->GetName());
		return;
	}

	bool alreadyYelled = false;
	do {
		if (!res && player->IsEngagedInEncounter(&res)) { // no target is set, dis-engage top of hated by list

		}
		if (!res || prev == res) {
			return;
		}

		if (res->IsNPC() && ((NPC*)res)->Brain()) {
			if (!alreadyYelled) {
				client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You yell for help!");
				string yellMsg = std::string(client->GetPlayer()->GetName()) + " yelled for help!";
				client->GetPlayer()->GetZone()->SimpleMessage(CHANNEL_COLOR_RED, yellMsg.c_str(), client->GetPlayer(), 35.0f, false);
				client->GetPlayer()->GetZone()->SendYellPacket(client->GetPlayer());
			}
			alreadyYelled = true;

			NPC* npc = (NPC*)res;
			npc->Brain()->ClearEncounter();
			npc->SetLockedNoLoot(ENCOUNTER_STATE_BROKEN);
			npc->UpdateEncounterState(ENCOUNTER_STATE_BROKEN);
		}
		prev = res;
		res = nullptr;
	} while (cycleAll);

	if (!player->IsEngagedInEncounter()) {
		if (player->GetInfoStruct()->get_engaged_encounter()) {
			player->GetInfoStruct()->set_engaged_encounter(0);
			player->SetRegenValues((player->GetInfoStruct()->get_effective_level() > 0) ? player->GetInfoStruct()->get_effective_level() : player->GetLevel());
			client->GetPlayer()->SetCharSheetChanged(true);
			player->info_changed = true;
		}
	}
}


/* 
	Function: Command_SetAutoLootMode()
	Purpose	: Set player auto loot mode (0 = disabled, 1 = need/lotto, 2 = decline).
	Example	: /setautolootmode [mode]
*/ 
void Commands::Command_SetAutoLootMode(Client* client, Seperator* sep) {
	if (sep && sep->IsNumber(0)) {
		int8 mode = atoul(sep->arg[0]);
		switch (mode) {
		case AutoLootMode::METHOD_DISABLED: {
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Disabled auto loot mode");
			break;
		}
		case AutoLootMode::METHOD_ACCEPT: {
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Enabled auto loot mode for need and lotto.");
			break;
		}
		default: {
			mode = AutoLootMode::METHOD_DECLINE;
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "Enabled auto loot mode to decline need and lotto.");
			break;
		}
		}
		client->GetPlayer()->GetInfoStruct()->set_group_auto_loot_method(mode);
		database.insertCharacterProperty(client, CHAR_PROPERTY_AUTOLOOTMETHOD, (char*)std::to_string(mode).c_str());
		client->SendDefaultGroupOptions();
	}
}

/*
	Function: Command_AutoAttack()
	Purpose	: Attack / Auto Attack
	Example	: /attack, /autoattack type
*/ 
void Commands::Command_AutoAttack(Client* client, Seperator* sep) {
	int8 type = 1;
	bool update = false;
	Player* player = client->GetPlayer();
	if(!player)
		return;
	bool incombat = player->EngagedInCombat();
	if(sep && sep->arg[0] && sep->IsNumber(0))
		type = atoi(sep->arg[0]);
	if(!client->GetPlayer()->Alive()){
		client->SimpleMessage(CHANNEL_COLOR_RED,"You cannot do that right now.");
		return;
	}
	if(type == 0){
		if(incombat)
			client->SimpleMessage(CHANNEL_GENERAL_COMBAT, "You stop fighting.");
			player->StopCombat(type);
			update = true;
	}
	else {
		if(type == 2){
			player->InCombat(false);
			if(incombat && player->GetRangeAttack()){
				player->StopCombat(type);
				client->SimpleMessage(CHANNEL_GENERAL_COMBAT, "You stop fighting.");
				update = true;
			}
			else{
				player->SetRangeAttack(true);
				player->InCombat(true, true);
				client->SimpleMessage(CHANNEL_GENERAL_COMBAT, "You start fighting.");
				update = true;
			}
		}
		else {
			player->InCombat(false, true);
			player->SetRangeAttack(false);
			player->InCombat(true);
			if(!incombat) {
				client->SimpleMessage(CHANNEL_GENERAL_COMBAT, "You start fighting.");
				update = true;
			}
		}
		/*else
			client->SimpleMessage(CHANNEL_COLOR_YELLOW, "You cannot attack that!");*/
	}
	
	if(update) {
		player->SetCharSheetChanged(true);
	}
}
	
/* 
	Function: Command_Assist()
	Purpose	: Assist target
	Example	: /assist [name]
	* Uses target or character name
*/ 
void Commands::Command_Assist(Client* client, Seperator* sep) {
	Entity* player = (Entity*)client->GetPlayer();
	Spawn* res = nullptr;
	if(sep && sep->arg[0]) {
		if(!stricmp(sep->arg[0], "on")) {
			database.insertCharacterProperty(client, CHAR_PROPERTY_ASSISTAUTOATTACK, "1");
			return;
		}
		else if(!stricmp(sep->arg[0], "off")) {
			database.insertCharacterProperty(client, CHAR_PROPERTY_ASSISTAUTOATTACK, "0");
			return;
		}
		Client* otherClient = client->GetPlayer()->GetZone()->GetClientByName(sep->arg[0]);
		if(otherClient) {
			res = otherClient->GetPlayer();
		}
	}
	
	if (player->GetTarget()) {
		res = player->GetTarget(); // selected target other than self only dis-engages that encounter
	}
	if(res && res->GetTarget()) {
		res = res->GetTarget();
	}
	
	if(res) {
		client->TargetSpawn(res);
		
		if(client->GetPlayer()->GetInfoStruct()->get_assist_auto_attack() && !player->EngagedInCombat()) {
			Command_AutoAttack(client, nullptr);
		}
	}
}

/* 
	Function: Command_Target()
	Purpose	: Target spawn/player
	Example	: /target [name]
	* Uses target or character name
*/ 
void Commands::Command_Target(Client* client, Seperator* sep) {
	Entity* player = (Entity*)client->GetPlayer();
	Spawn* res = nullptr;
	if(sep && sep->arg[0] && player->GetZone()) {
		float max_distance = rule_manager.GetZoneRule(client->GetCurrentZoneID(), R_Player, MaxTargetCommandDistance)->GetFloat();
		
		if(max_distance < 1.0f) {
			max_distance = 10.0f;
		}
		
		player->GetZone()->SetPlayerTargetByName(client, (char*)sep->argplus[0], max_distance);
	}
}


/* 
	Function: Command_Target_Pet()
	Purpose	: Target pet
	Example	: /target_pet
	* Use to target pet
*/ 
void Commands::Command_Target_Pet(Client* client, Seperator* sep) {
	Entity* player = (Entity*)client->GetPlayer();
	Spawn* res = client->GetPlayer()->GetPet();

	if(res) {
		client->TargetSpawn(res);
	}
}


/* 
	Function: Command_WhoGroup()
	Purpose	: Lists all members of current group
	Example	: /whogroup
*/ 
void Commands::Command_WhoGroup(Client* client, Seperator* sep) {
	Entity* player = (Entity*)client->GetPlayer();
	if(player->GetGroupMemberInfo()) {
		world.GetGroupManager()->SendWhoGroupMembers(client, player->GetGroupMemberInfo()->group_id);
	}
	else {
		client->SimpleMessage(CHANNEL_COLOR_RED, "You are not currently in a group.");
	}
}

/* 
	Function: Command_WhoRaid()
	Purpose	: Lists all members of raid
	Example	: /whoraid
*/ 
void Commands::Command_WhoRaid(Client* client, Seperator* sep) {
	Entity* player = (Entity*)client->GetPlayer();
	if(player->GetGroupMemberInfo()) {
		world.GetGroupManager()->SendWhoRaidMembers(client, player->GetGroupMemberInfo()->group_id);
	}
	else {
		client->SimpleMessage(CHANNEL_COLOR_RED, "You are not currently in a group or raid.");
	}
}

/* 
	Function: Command_RaidInvite()
	Purpose	: Invites a group to the raid
	Example	: /raidinvite
*/ 
void Commands::Command_RaidInvite(Client* client, Seperator* sep) {
	Entity* target = nullptr;
	if( sep && sep->arg[0] ) {
		Client* target_client = zone_list.GetClientByCharName(sep->arg[0]);
		if(target_client)
			target = (Entity*)target_client->GetPlayer();
	}
	if(!target) {
		if(client->GetPlayer()->GetTarget() && client->GetPlayer()->GetTarget()->IsEntity())
			target = (Entity*)client->GetPlayer()->GetTarget();
	}
	world.GetGroupManager()->SendRaidInvite(client, target);
}

/* 
	Function: Command_Raid_Looter()
	Purpose	: Adds a looter to the raid loot list
	Example	: /raid_looter <name>
*/ 
void Commands::Command_Raid_Looter(Client* client, Seperator* sep) {
	if(!client->GetPlayer()->GetGroupMemberInfo() || client->GetPlayer()->GetGroupMemberInfo()->leader)
		return;
	Entity* target = nullptr;
	if( sep && sep->arg[0] ) {
		Client* target_client = zone_list.GetClientByCharName(sep->arg[0]);
		if(target_client)
			target = (Entity*)target_client->GetPlayer();
	}
	if(!target) {
		if(client->GetPlayer()->GetTarget() && client->GetPlayer()->GetTarget()->IsEntity())
			target = (Entity*)client->GetPlayer()->GetTarget();
	}
	
	bool isLeaderRaid = world.GetGroupManager()->IsInRaidGroup(client->GetPlayer()->GetGroupMemberInfo()->group_id, client->GetPlayer()->GetGroupMemberInfo()->group_id, true);
	if(isLeaderRaid && target && target->IsEntity()) {
		if(((Entity*)target)->GetGroupMemberInfo() && world.GetGroupManager()->IsInRaidGroup(client->GetPlayer()->GetGroupMemberInfo()->group_id, ((Entity*)target)->GetGroupMemberInfo()->group_id, false)) {
			if(((Entity*)target)->GetGroupMemberInfo()->is_raid_looter) {
				client->Message(CHANNEL_COLOR_YELLOW, "%s removed as a raid looter.", target->GetName());
				((Entity*)target)->GetGroupMemberInfo()->is_raid_looter = false;
			}
			else {
				client->Message(CHANNEL_COLOR_YELLOW, "%s added as a raid looter.", target->GetName());
				((Entity*)target)->GetGroupMemberInfo()->is_raid_looter = true;
			}
		}
	}
}

/* 
	Function: Command_KickFromGroup()
	Purpose	: Kick a player from a group
	Example	: /kickfromgroup <name>
*/ 
void Commands::Command_KickFromGroup(Client* client, Seperator* sep) {
	Entity* target = nullptr;
	Client* target_client = nullptr;
	if( sep && sep->arg[0] ) {
		target_client = zone_list.GetClientByCharName(sep->arg[0]);
		if(target_client) {
			target = target_client->GetPlayer();
		}
	}
	if(!target) {
		if(client->GetPlayer()->GetTarget() && client->GetPlayer()->GetTarget()->IsEntity())
			target = (Entity*)client->GetPlayer()->GetTarget();
		
		if(target && target->IsPlayer())
			target_client = ((Player*)target)->GetClient();
	}
	GroupMemberInfo* gmi = client->GetPlayer()->GetGroupMemberInfo();

	if (gmi && gmi->leader && target && target->GetGroupMemberInfo() && gmi->group_id == target->GetGroupMemberInfo()->group_id) {
		int32 group_id = gmi->group_id;
		world.GetGroupManager()->RemoveGroupMember(group_id, target);
		if (!world.GetGroupManager()->IsGroupIDValid(group_id)) {
			// leader->Message(CHANNEL_COLOR_GROUP, "%s has left the group.", client->GetPlayer()->GetName());
		}
		else {
			world.GetGroupManager()->GroupMessage(group_id, "%s has been removed from the group.", target->GetName());
		}

		if(target_client)
			target_client->SimpleMessage(CHANNEL_GROUP_CHAT, "You have been kicked from the group");
	}
}

/* 
	Function: Command_KickFromRaid()
	Purpose	: Kick a group from a raid
	Example	: /kickfromraid <name>
*/ 
void Commands::Command_KickFromRaid(Client* client, Seperator* sep) {
	Entity* target = nullptr;
	Client* target_client = nullptr;
	if( sep && sep->arg[0] ) {
		target_client = zone_list.GetClientByCharName(sep->arg[0]);
		if(target_client) {
			target = target_client->GetPlayer();
		}
	}
	if(!target) {
		if(client->GetPlayer()->GetTarget() && client->GetPlayer()->GetTarget()->IsEntity())
			target = (Entity*)client->GetPlayer()->GetTarget();
		
		if(target && target->IsPlayer())
			target_client = ((Player*)target)->GetClient();
	}
	GroupMemberInfo* gmi = client->GetPlayer()->GetGroupMemberInfo();
	if(gmi && gmi->leader && target && target->GetGroupMemberInfo() && world.GetGroupManager()->IsInRaidGroup(gmi->group_id, target->GetGroupMemberInfo()->group_id, false) && 
		world.GetGroupManager()->IsInRaidGroup(gmi->group_id, gmi->group_id, true)) {
		GroupOptions goptions;
		world.GetGroupManager()->GetDefaultGroupOptions(gmi->group_id, &goptions);
		world.GetGroupManager()->RemoveGroupFromRaid(gmi->group_id, target->GetGroupMemberInfo()->group_id);
		std::vector<int32> raidGroups;
		world.GetGroupManager()->GetRaidGroups(gmi->group_id, &raidGroups);
		peer_manager.sendPeersNewGroupRequest("", 0, gmi->group_id, "", "", &goptions, "", &raidGroups, true);
		std::vector<int32> emptyRaid;
		peer_manager.sendPeersNewGroupRequest("", 0, target->GetGroupMemberInfo()->group_id, "", "", &goptions, "", &emptyRaid, true);
	}
}

/* 
	Function: Command_LeaveRaid()
	Purpose	: Leave a raid
	Example	: /leaveraid
*/ 
void Commands::Command_LeaveRaid(Client* client, Seperator* sep) {
	GroupMemberInfo* gmi = client->GetPlayer()->GetGroupMemberInfo();
	int32 orig_group_id = 0;
	if(gmi && gmi->leader && world.GetGroupManager()->IsInRaidGroup(gmi->group_id, gmi->group_id, false)) {
		orig_group_id = gmi->group_id;
		GroupOptions goptions;
		world.GetGroupManager()->GetDefaultGroupOptions(gmi->group_id, &goptions);
		std::vector<int32> raidGroups;
		world.GetGroupManager()->GetRaidGroups(gmi->group_id, &raidGroups);
		std::vector<int32>::iterator cur_group_itr = std::find(raidGroups.begin(), raidGroups.end(), gmi->group_id);
		if(cur_group_itr != raidGroups.end())
			raidGroups.erase(cur_group_itr);
		
		bool sendEmpty = false;
		std::vector<int32> emptyRaid;
		if(raidGroups.size() < 2) {
			sendEmpty = true;
		}
		
		for(cur_group_itr = raidGroups.begin(); cur_group_itr != raidGroups.end(); cur_group_itr++) {
			if(sendEmpty) {
				world.GetGroupManager()->ClearGroupRaid((*cur_group_itr));
				peer_manager.sendPeersNewGroupRequest("", 0, (*cur_group_itr), "", "", &goptions, "", &emptyRaid, true);
				world.GetGroupManager()->SendGroupUpdate((*cur_group_itr), nullptr, true);
			}
			else {
				world.GetGroupManager()->ReplaceRaidGroups((*cur_group_itr), &raidGroups);
			}
		}
		
		if(!sendEmpty) {
			peer_manager.sendPeersNewGroupRequest("", 0, orig_group_id, "", "", &goptions, "", &raidGroups, true);
		}
	
		world.GetGroupManager()->ClearGroupRaid(orig_group_id);
		world.GetGroupManager()->SendGroupUpdate(orig_group_id, nullptr, true);
		
		peer_manager.sendPeersNewGroupRequest("", 0, orig_group_id, "", "", &goptions, "", &emptyRaid, true);
	}
}

/* 
	Function: Command_Split()
	Purpose	: split coin to group
	Example	: /split {plat} {gold} {silver} {copper}
*/ 
void Commands::Command_Split(Client* client, Seperator* sep) {
 // int32 item_id = atoul(sep->arg[0,1,2,3]);
	int32 plat = 0, gold = 0, silver = 0, copper = 0;
	if(sep->IsNumber(0))
		plat = atoul(sep->arg[0]);
	if(sep->IsNumber(1))
		gold = atoul(sep->arg[1]);
	if(sep->IsNumber(2))
		silver = atoul(sep->arg[2]);
	if(sep->IsNumber(3))
		copper = atoul(sep->arg[3]);
	
	world.GetGroupManager()->SplitWithGroupOrRaid(client, plat, gold, silver, copper);
}

/* 
	Function: Command_RaidSay()
	Purpose	: Speak to raid members
	Example	: /raidsay {message}, /rsay {message}
*/ 
void Commands::Command_RaidSay(Client* client, Seperator* sep) {
	GroupMemberInfo* gmi = client->GetPlayer()->GetGroupMemberInfo();
	if(sep && sep->arg[0] && gmi) {
		world.GetGroupManager()->GroupLock(__FUNCTION__, __LINE__);
		int32 spawn_group_id = gmi->group_id;
		PlayerGroup* spawn_group = world.GetGroupManager()->GetGroup(spawn_group_id);
		bool israidgroup = (spawn_group && spawn_group->IsGroupRaid());
		world.GetGroupManager()->ReleaseGroupLock(__FUNCTION__, __LINE__);
		if(israidgroup) {
			world.GetGroupManager()->GroupChatMessage(gmi->group_id, client->GetPlayer(), client->GetPlayer()->GetCurrentLanguage(), sep->argplus[0], CHANNEL_RAID_SAY);
			peer_manager.SendPeersChannelMessage(gmi->group_id, std::string(client->GetPlayer()->GetName()), std::string(sep->argplus[0]), CHANNEL_RAID_SAY, client->GetPlayer()->GetCurrentLanguage());
		}
	}
}

/* 
	Function: Command_ReloadZoneInfo()
	Purpose	: Clears ZoneInfoMemory used for database.LoadZoneInfo
	Example	: /reload zoneinfo
*/ 
void Commands::Command_ReloadZoneInfo(Client* client, Seperator* sep) {
	world.ClearZoneInfoCache();
}

// somewhere in your command‐handler:
void Commands::Command_SetLocationEntry(Client* client, Seperator* sep) {
	if(client->GetCurrentZone()->GetInstanceType() == PERSONAL_HOUSE_INSTANCE) {
		client->Message(CHANNEL_COLOR_YELLOW, "Use in a non house zone.");
		return;
	}
	
	if (!sep->IsSet(1) || (sep->IsNumber(0))) {
		client->Message(CHANNEL_COLOR_YELLOW, "Usage: /sle <column_name> <new_value>");
		return;
	}
	
	Spawn* target = client->GetPlayer()->GetTarget();
	
	if(!target) {
		client->Message(CHANNEL_COLOR_RED, "Target missing for set location entry, /sle <column_name> <new_value>");
		return;
	}

	// GetSpawnEntryID for spawn_location_entry to set the spawnpercentage
	int32 spawnEntryID = target->GetSpawnEntryID();
	int32 spawnPlacementID = target->GetSpawnLocationPlacementID();
	int32 spawnLocationID = target->GetSpawnLocationID();
	int32 dbID = target->GetDatabaseID();
	if (spawnPlacementID == 0 || spawnLocationID == 0 || spawnEntryID == 0 || dbID == 0) {
		client->Message(CHANNEL_COLOR_RED, "Error: no valid spawn entry selected.");
		return;
	}

	// 2) Whitelist the allowed columns
	static const std::unordered_set<std::string> allowed = {
		"x",
		"y",
		"z",
		"x_offset",
		"y_offset",
		"z_offset",
		"heading",
		"pitch",
		"roll",
		"respawn",
		"respawn_offset_low",
		"respawn_offset_high",
		"duplicated_spawn",
		"expire_timer",
		"expire_offset",
		"grid_id",
		"processed",
		"instance_id",
		"lvl_override",
		"hp_override",
		"mp_override",
		"str_override",
		"sta_override",
		"wis_override",
		"int_override",
		"agi_override",
		"heat_override",
		"cold_override",
		"magic_override",
		"mental_override",
		"divine_override",
		"disease_override",
		"poison_override",
		"difficulty_override",
		"spawnpercentage",
		"condition"
	};


	const std::string& field = std::string(sep->arg[0]);
	if (!allowed.count(field)) {
		client->Message(CHANNEL_COLOR_RED, "Error: column '%s' is not modifiable.", field.c_str());
		return;
	}
	
	const std::string& val = std::string(sep->arg[1]);

	Query query;	
	if(field == "spawnpercentage" || field == "condition") {
				query.AddQueryAsync(0, 
							&database,
							Q_UPDATE,
							// we embed the whitelisted field name directly in the format
							"UPDATE spawn_location_entry "
							"SET %s=%s "
							"WHERE id=%u and spawn_location_id=%u and spawn_id=%u ",
							field.c_str(),
							val.c_str(),
							spawnEntryID,
							spawnLocationID,
							dbID
		);
	}
	else {
		query.AddQueryAsync(0, 
							&database,
							Q_UPDATE,
							// we embed the whitelisted field name directly in the format
							"UPDATE spawn_location_placement "
							"SET %s=%s "
							"WHERE id=%u and spawn_location_id=%u ",
							field.c_str(),
							val.c_str(),
							spawnPlacementID,
							spawnLocationID
		);
	}

	client->Message(CHANNEL_COLOR_YELLOW, "Modified %s to %s for row entry id %u, spawn placement id %u, related to location id %u and spawn database id %u.", 
					field.c_str(), val.c_str(), spawnEntryID, spawnPlacementID, spawnLocationID, dbID);
}



void Commands::Command_StoreListItem(Client* client, Seperator* sep) {
	if(!client->GetShopWindowStatus()) {
		client->Message(CHANNEL_COLOR_RED, "Shop not available.");
		return;
	}
	
	if(sep && sep->arg[0]) {
		auto info = broker.GetSellerInfo(client->GetPlayer()->GetCharacterID());
		if(!info) {
			client->Message(CHANNEL_COLOR_RED, "Player %u is not in the broker database.", client->GetPlayer()->GetCharacterID());
		}
		else if(sep->IsNumber(0)) {
			int64 unique_id = atoll(sep->arg[0]);
			if(client->GetPlayer()->item_list.CanStoreSellItem(unique_id, true)) {
				Item* item = client->GetPlayer()->item_list.GetVaultItemFromUniqueID(unique_id, true);
				if(item) {
					bool isInv = !client->GetPlayer()->item_list.IsItemInSlotType(item, InventorySlotType::HOUSE_VAULT);
					int64 cost = broker.GetSalePrice(client->GetPlayer()->GetCharacterID(), item->details.unique_id);
					client->AddItemSale(item->details.unique_id, item->details.item_id, cost, item->details.inv_slot_id, item->details.slot_id, item->details.count, isInv, true, item->creator);
				}
				else
					client->Message(CHANNEL_COLOR_RED, "Broker issue, cannot find item %u.", unique_id);

				client->SetItemSaleStatus(unique_id, true);				
				client->GetPlayer()->item_list.SetVaultItemLockUniqueID(client, unique_id, true, false);
				client->SetSellerStatus();
			}
		}
		else {
			client->Message(CHANNEL_COLOR_RED, "Invalid arguments for /store_list_item unique_id.");

		}
	}
}
void Commands::Command_StoreSetPrice(Client* client, Seperator* sep) {
	if(!client->GetShopWindowStatus()) {
		client->Message(CHANNEL_COLOR_RED, "Shop not available.");
		return;
	}
	
	if(sep && sep->arg[0]) {
		auto info = broker.GetSellerInfo(client->GetPlayer()->GetCharacterID());
		int64 unique_id = atoll(sep->arg[0]);
		if(!info) {
			client->Message(CHANNEL_COLOR_RED, "Player %u is not in the broker database.", client->GetPlayer()->GetCharacterID());
		}
		else if(info->sell_from_inventory && broker.IsItemFromInventory(client->GetPlayer()->GetCharacterID(), unique_id)) {
			client->Message(CHANNEL_COLOR_RED, "You cannot change the price while selling.");
		}
		else if(info->sell_from_inventory && !broker.IsItemFromInventory(client->GetPlayer()->GetCharacterID(), unique_id) && broker.IsItemForSale(client->GetPlayer()->GetCharacterID(), unique_id)) {
			client->Message(CHANNEL_COLOR_RED, "You cannot change the price while selling.");
		}
		else if(sep->IsNumber(1) && sep->IsNumber(2) && sep->IsNumber(3) && sep->IsNumber(4)) {
			int32 plat = atoul(sep->arg[1]);
			int32 gold = atoul(sep->arg[2]);
			int32 silver = atoul(sep->arg[3]);
			int32 copper = atoul(sep->arg[4]);
			int64 price = plat * 1000000 + gold * 10000 + silver * 100 + copper;
			
			LogWrite(PLAYER__INFO, 5, "Broker",
			  "--StoreSetPrice: %u (%u), cost=%u",
			  client->GetPlayer()->GetCharacterID(), unique_id, price
			);
			client->SetItemSaleCost(unique_id, plat, gold, silver, copper);
		}
		else {
			client->Message(CHANNEL_COLOR_RED, "Invalid arguments for /store_set_price unique_id platinum gold silver copper.");
		}
	}
}
void Commands::Command_StoreSetPriceLocal(Client* client, Seperator* sep) {
	if(!client->GetShopWindowStatus()) {
		client->Message(CHANNEL_COLOR_RED, "Shop not available.");
		return;
	}
	
	if(sep && sep->arg[0]) {
		auto info = broker.GetSellerInfo(client->GetPlayer()->GetCharacterID());
		if(!info) {
			client->Message(CHANNEL_COLOR_RED, "Player %u is not in the broker database.", client->GetPlayer()->GetCharacterID());
		}
		else if(info->sell_from_inventory) {
			client->Message(CHANNEL_COLOR_RED, "You cannot change the price while selling.");
		}
		else if(sep->IsNumber(0) && sep->IsNumber(1) && sep->IsNumber(2) && sep->IsNumber(3) && sep->IsNumber(4)) {
			int64 unique_id = atoll(sep->arg[0]);
			int32 plat = atoul(sep->arg[1]);
			int32 gold = atoul(sep->arg[2]);
			int32 silver = atoul(sep->arg[3]);
			int32 copper = atoul(sep->arg[4]);
			int64 price = plat * 1000000 + gold * 10000 + silver * 100 + copper;
			LogWrite(PLAYER__INFO, 5, "Broker",
			  "--StoreSetLocalPrice: %u (%u), cost=%u",
			  client->GetPlayer()->GetCharacterID(), unique_id, price
			);
			client->SetItemSaleCost(unique_id, plat, gold, silver, copper);
		}
		else {
			client->Message(CHANNEL_COLOR_RED, "Invalid arguments for /store_set_price_local unique_id platinum gold silver copper.");
		}
	}
	
}
void Commands::Command_StoreStartSelling(Client* client, Seperator* sep) {
	if(!client->GetShopWindowStatus()) {
		client->Message(CHANNEL_COLOR_RED, "Shop not available.");
		return;
	}
	
	broker.AddSeller(client->GetPlayer()->GetCharacterID(), std::string(client->GetPlayer()->GetName()), client->GetPlayer()->GetPlayerInfo()->GetHouseZoneID(), true, true);
	client->OpenShopWindow(nullptr);
	broker.LockActiveItemsForClient(client);
}

void Commands::Command_StoreStopSelling(Client* client, Seperator* sep) {
	if(!client->GetShopWindowStatus()) {
		client->Message(CHANNEL_COLOR_RED, "Shop not available.");
		return;
	}
	
	broker.AddSeller(client->GetPlayer()->GetCharacterID(), std::string(client->GetPlayer()->GetName()), client->GetPlayer()->GetPlayerInfo()->GetHouseZoneID(), true, false);
	client->OpenShopWindow(nullptr);
	broker.LockActiveItemsForClient(client);
}

void Commands::Command_StoreUnlistItem(Client* client, Seperator* sep) {
	if(!client->GetShopWindowStatus()) {
		client->Message(CHANNEL_COLOR_RED, "Shop not available.");
		return;
	}
	if(sep && sep->arg[0]) {
		auto info = broker.GetSellerInfo(client->GetPlayer()->GetCharacterID());
		if(!info) {
			client->Message(CHANNEL_COLOR_RED, "Player %u is not in the broker database.", client->GetPlayer()->GetCharacterID());
		}
		else if(sep->IsNumber(0)) {
			int64 unique_id = atoll(sep->arg[0]);
			client->SetItemSaleStatus(unique_id, false);
			client->SetSellerStatus();
			client->GetPlayer()->item_list.SetVaultItemLockUniqueID(client, unique_id, false, false);
		}
		else {
			client->Message(CHANNEL_COLOR_RED, "Invalid arguments for /store_unlist_item unique_id.");

		}
	}
}

void Commands::Command_CloseStoreKeepSelling(Client* client, Seperator* sep) {
	if(!client->GetShopWindowStatus()) {
		client->Message(CHANNEL_COLOR_RED, "Shop not available.");
		return;
	}
	auto info = broker.GetSellerInfo(client->GetPlayer()->GetCharacterID());
	client->SetShopWindowStatus(false);
	if(!info) {
		client->Message(CHANNEL_COLOR_RED, "Player %u is not in the broker database.", client->GetPlayer()->GetCharacterID());
	}
	else {
		bool itemsSelling = broker.IsSellingItems(client->GetPlayer()->GetCharacterID());
		broker.AddSeller(client->GetPlayer()->GetCharacterID(), std::string(client->GetPlayer()->GetName()), client->GetPlayer()->GetPlayerInfo()->GetHouseZoneID(), itemsSelling, true);
	}
	broker.LockActiveItemsForClient(client);
}

void Commands::Command_CancelStore(Client* client, Seperator* sep) {
	if(!client->GetShopWindowStatus()) {
		client->Message(CHANNEL_COLOR_RED, "Shop not available.");
		return;
	}
	auto info = broker.GetSellerInfo(client->GetPlayer()->GetCharacterID());
	client->SetShopWindowStatus(false);
	if(!info) {
		client->Message(CHANNEL_COLOR_RED, "Player %u is not in the broker database.", client->GetPlayer()->GetCharacterID());
	}
	else {
		bool itemsSelling = broker.IsSellingItems(client->GetPlayer()->GetCharacterID(), true);
		broker.AddSeller(client->GetPlayer()->GetCharacterID(), std::string(client->GetPlayer()->GetName()), client->GetPlayer()->GetPlayerInfo()->GetHouseZoneID(), itemsSelling, false);
	}
	broker.LockActiveItemsForClient(client);
}