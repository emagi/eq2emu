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
#pragma once
#include "client.h"

struct HouseZone;
struct PlayerHouse;
struct HeroicOP;

class ClientPacketFunctions
{
public:
	static void SendFinishedEntitiesList ( Client* client );

	static void SendLoginDenied ( Client* client );
	static void SendLoginAccepted ( Client* client );

	static void SendCommandList ( Client* client );

	static void SendGameWorldTime ( Client* client );

	static void SendCharacterData ( Client* client );
	static void SendCharacterSheet ( Client* client );
	static void SendSkillBook ( Client* client );
	static void SendTraitList ( Client* client );
	static void SendAbilities ( Client* client );

	static void SendCommandNamePacket ( Client* client );

	static void SendQuickBarInit ( Client* client );

	static void SendMOTD ( Client* client );

	static void SendCharacterMacros(Client* client);

	static void SendUpdateSpellBook ( Client* client );

	static void SendSkillSlotMappings(Client* client);

	static void SendRestartZoneMsg(Client* client);

	static void SendServerControlFlags(Client* client, int8 param, int8 param_val, int8 value);

	static void SendServerControlFlagsClassic(Client* client, int32 param, int32 value);

	static void SendInstanceList(Client* client);

	static void SendZoneChange(Client* client, char* zone_ip, int16 zone_port, int32 key);

	static void SendStateCommand(Client* client, int32 spawn_id, int32 state);

	static void SendFlyMode(Client* client, int8 flymode, bool updateCharProperty=true);

	/* Tradeskills (/Tradeskills/TradeskillsPackets.cpp) */
	static void SendCreateFromRecipe(Client* client, int32 recipeID);
	static void SendItemCreationUI(Client* client, Recipe* recipe);
	static void StopCrafting(Client* client);
	static void CounterReaction(Client* client, bool countered);

	static void SendAchievementList(Client* client);

	/* Housing (/Housing/HousingPackets.cpp) */
	static void SendHousePurchase(Client* client, HouseZone* hz, int32 spawnID);
	static void SendHousingList(Client* client);
	static void SendBaseHouseWindow(Client* client, HouseZone* hz, PlayerHouse* ph, int32 spawnID);
	static void SendHouseVisitWindow(Client* client, vector<PlayerHouse*> houses);
	static void SendLocalizedTextMessage(Client* client);

	/* Heroic OP's (/HeroicOp/HeroicOpPackets.cpp) */
	static void SendHeroicOPUpdate(Client* client, HeroicOP* ho);

	//UI updates for trigger count and damage remaining on maintained spells
	static void SendMaintainedExamineUpdate(Client* client, int8 slot_pos, int32 update_value, int8 update_type);
};

