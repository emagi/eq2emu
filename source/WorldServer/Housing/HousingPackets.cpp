#include "../ClientPacketFunctions.h"
#include "../World.h"
#include "../client.h"
#include "../WorldDatabase.h"
#include "../Rules/Rules.h"

extern ConfigReader configReader;
extern World world;
extern WorldDatabase database;
extern RuleManager rule_manager;

void ClientPacketFunctions::SendHousePurchase(Client* client, HouseZone* hz, int32 spawnID) {
	PacketStruct* packet = configReader.getStruct("WS_PlayerHousePurchase", client->GetVersion());
	if (packet) {
		int8 disable_alignment_req = rule_manager.GetGlobalRule(R_Player, DisableHouseAlignmentRequirement)->GetInt8();
		packet->setDataByName("house_name", hz->name.c_str());
		packet->setDataByName("house_id", hz->id);
		packet->setDataByName("spawn_id", spawnID);
		packet->setDataByName("purchase_coins", hz->cost_coin);
		packet->setDataByName("purchase_status", hz->cost_status);
		packet->setDataByName("upkeep_coins", hz->upkeep_coin);
		packet->setDataByName("upkeep_status", hz->upkeep_status);
		packet->setDataByName("vendor_vault_slots", hz->vault_slots);
		string req;
		if (hz->alignment > 0 && !disable_alignment_req) {
			req = "You must be of ";
			if (hz->alignment == 1)
				req.append("Good");
			else
				req.append("Evil");
			req.append(" alignment");
		}
		if (hz->guild_level > 0) {
			if (req.length() > 0) {
				req.append(", and a guild level of ");
				char temp[5];
				sprintf(temp, "%i", hz->guild_level);
				req.append(temp);
				//req.append(std::to_string(static_cast<long long>(hz->guild_level)));
			}
			else {
				req.append("Requires a guild of level ");
				char temp[5];
				sprintf(temp, "%i", hz->guild_level);
				req.append(temp);
				//req.append(std::to_string(static_cast<long long>(hz->guild_level)))
				req.append(" or above");
			}
		}
		if (req.length() > 0) {
			req.append(" in order to purchase a home within the ");
			req.append(hz->name);
			req.append(".");
		}

		packet->setDataByName("additional_reqs", req.c_str());

		bool enable_buy = true;
		if (hz->alignment > 0 && client->GetPlayer()->GetAlignment() != hz->alignment && !disable_alignment_req)
			enable_buy = false;
		if (hz->guild_level > 0 && (!client->GetPlayer()->GetGuild() || (client->GetPlayer()->GetGuild() && client->GetPlayer()->GetGuild()->GetLevel() < hz->guild_level)))
			enable_buy = false;

		packet->setDataByName("enable_buy", enable_buy ? 1 : 0);
		//packet->PrintPacket();
		client->QueuePacket(packet->serialize());
	}

	safe_delete(packet);
}

void ClientPacketFunctions::SendHousingList(Client* client) {
	if(client->GetVersion() <= 561) {
		return; // not supported
	}
	
	std::vector<PlayerHouse*> houses = world.GetAllPlayerHouses(client->GetCharacterID());
	// this packet must be sent first otherwise it blocks out the enter house option after paying upkeep
	PacketStruct* packet = configReader.getStruct("WS_CharacterHousingList", client->GetVersion());
	if(!packet) {
		return;
	}
	packet->setArrayLengthByName("num_houses", houses.size());
	for (int i = 0; i < houses.size(); i++)
	{
		PlayerHouse* ph = (PlayerHouse*)houses[i];
		HouseZone* hz = world.GetHouseZone(ph->house_id);
		string name;
		name = ph->player_name;
		name.append("'s ");
		name.append(hz->name);
		packet->setArrayDataByName("house_id", ph->unique_id, i);
		string zone_name = database.GetZoneName(hz->zone_id);
		if(zone_name.length() > 0)
			packet->setArrayDataByName("zone", zone_name.c_str(), i);
		packet->setArrayDataByName("house_city", hz->name.c_str(), i);
		packet->setArrayDataByName("house_address", "", i); // need this pulled from live
		packet->setArrayDataByName("house_description", name.c_str(), i);
		packet->setArrayDataByName("index", i, i); // they send 2, 4, 6, 8 as the index ID's on the client..

		// this seems to be some kind of timestamp, if we keep updating then in conjunction with upkeep_due
		// in SendBaseHouseWindow/WS_PlayerHouseBaseScreen being a >0 number we can access 'enter house'
		
		int32 upkeep_due = 0;

		if (((sint64)ph->upkeep_due - (sint64)Timer::GetUnixTimeStamp()) > 0)
			upkeep_due = ph->upkeep_due - Timer::GetUnixTimeStamp();
		
		if ( client->GetVersion() >= 63119 )
			packet->setArrayDataByName("unknown2a", 0xFFFFFFFF, i);
		else
			packet->setArrayDataByName("unknown2", 0xFFFFFFFF, i);
	}
	client->QueuePacket(packet->serialize());
	safe_delete(packet);
}

void ClientPacketFunctions::SendBaseHouseWindow(Client* client, HouseZone* hz, PlayerHouse* ph, int32 spawnID) {
	// if we don't send this then the enter house option won't be available if upkeep is paid
	if (!hz || !ph)
	{
		client->SimpleMessage(CHANNEL_COLOR_RED, "HouseZone or PlayerHouse missing and cannot send SendBaseHouseWindow");
		return;
	}

	string name;
	name = ph->player_name;
	name.append("'s ");
	name.append(hz->name);

	if (spawnID)
		SendHousingList(client);

	int32 upkeep_due = 0;

	if (((sint64)ph->upkeep_due - (sint64)Timer::GetUnixTimeStamp()) > 0)
		upkeep_due = ph->upkeep_due - Timer::GetUnixTimeStamp();
		
	// need this to enable the "enter house" button
	PacketStruct* packet = nullptr;


	if(client->GetVersion() > 561 && client->GetCurrentZone()->GetInstanceType() != PERSONAL_HOUSE_INSTANCE
			&& client->GetCurrentZone()->GetInstanceType() != GUILD_HOUSE_INSTANCE) {
		packet = configReader.getStruct("WS_UpdateHouseAccessDataMsg", client->GetVersion());
		
		if(!packet) {
			return; // we need this for these clients or enter house will not work properly
		}
		if (packet) {
			packet->setDataByName("house_id", 0xFFFFFFFFFFFFFFFF);
			packet->setDataByName("success",  (upkeep_due > 0) ? 0xFFFFFFFF : 0);
			packet->setDataByName("unknown2", 0xFFFFFFFF);
			packet->setDataByName("unknown3", 0xFFFFFFFF);
		}
		client->QueuePacket(packet->serialize());
	}
	safe_delete(packet);
	
	packet = configReader.getStruct("WS_PlayerHouseBaseScreen", client->GetVersion());
	if (packet) {
		packet->setDataByName("house_id", ph->unique_id);
		packet->setDataByName("spawn_id", spawnID);
		packet->setDataByName("character_id", client->GetPlayer()->GetCharacterID());
		packet->setDataByName("house_name", name.c_str());
		packet->setDataByName("zone_name", hz->name.c_str());
		packet->setDataByName("upkeep_cost_coins", hz->upkeep_coin);
		packet->setDataByName("upkeep_cost_status", hz->upkeep_status);

		packet->setDataByName("upkeep_due", upkeep_due);

		packet->setDataByName("escrow_balance_coins", ph->escrow_coins);
		packet->setDataByName("escrow_balance_status", ph->escrow_status);
		// temp - set priv level to owner for now
		packet->setDataByName("privlage_level", 4);
		// temp - set house type to personal house for now
		packet->setDataByName("house_type", 0);
			
		if(client->GetCurrentZone()->GetInstanceType() == PERSONAL_HOUSE_INSTANCE
			|| client->GetCurrentZone()->GetInstanceType() == GUILD_HOUSE_INSTANCE) {
				packet->setDataByName("inside_house", 1);
				packet->setDataByName("public_access_level", 1);
		}
		packet->setDataByName("num_access", 0);
		packet->setDataByName("num_history", 0);

		// allows deposits/history to be seen -- at this point seems plausible supposed to be 'inside_house'..?
		packet->setDataByName("unknown3", (ph->deposits.size() || ph->history.size()) ? 1 : 0);

		packet->setArrayLengthByName("num_deposit", ph->deposits.size());
		list<Deposit>::iterator itr;
		int d = 0;
		for (itr = ph->deposits.begin(); itr != ph->deposits.end(); itr++)
		{
			packet->setArrayDataByName("deposit_name", itr->name.c_str(), d);
			packet->setArrayDataByName("deposit_total_coin", itr->amount, d);
			packet->setArrayDataByName("deposit_time_stamp", itr->timestamp, d);
			packet->setArrayDataByName("deposit_last_coin", itr->last_amount, d);
			packet->setArrayDataByName("deposit_total_status", itr->status, d);
			packet->setArrayDataByName("deposit_last_status", itr->last_status, d);
			d++;
		}


		packet->setArrayLengthByName("num_history", ph->history.size());
		list<HouseHistory>::iterator hitr;
		d = 0;
		for (hitr = ph->history.begin(); hitr != ph->history.end(); hitr++)
		{
			packet->setArrayDataByName("history_name", hitr->name.c_str(), d);
			packet->setArrayDataByName("history_coins", hitr->amount, d);
			packet->setArrayDataByName("history_status", hitr->status, d);
			packet->setArrayDataByName("history_time_stamp", hitr->timestamp, d);
			packet->setArrayDataByName("history_reason", hitr->reason.c_str(), d);
			packet->setArrayDataByName("history_add_flag", hitr->pos_flag, d);
			d++;
		}
		
		EQ2Packet* pack = packet->serialize();
		//DumpPacket(pack);
		client->QueuePacket(pack);
	}
	safe_delete(packet);
}

void ClientPacketFunctions::SendHouseVisitWindow(Client* client, vector<PlayerHouse*> houses) {
	PacketStruct* packet = configReader.getStruct("WS_DisplayVisitScreen", client->GetVersion());
	if (packet) {
		vector<PlayerHouse*>::iterator itr;
		packet->setArrayLengthByName("num_houses", houses.size());
		int16 i = 0;
		for (itr = houses.begin(); itr != houses.end(); itr++) {
			PlayerHouse* ph = *itr;
			if (ph) {
				HouseZone* hz = world.GetHouseZone(ph->house_id);
				if (hz) {
					packet->setArrayDataByName("house_id", ph->unique_id, i);
					packet->setArrayDataByName("house_owner", ph->player_name.c_str(), i);
					packet->setArrayDataByName("house_location", hz->name.c_str(), i);
					packet->setArrayDataByName("house_zone", client->GetCurrentZone()->GetZoneName(), i);

					if ( string(client->GetPlayer()->GetName()).compare(ph->player_name) == 0 )
						packet->setArrayDataByName("access_level", 4, i);
					else
						packet->setArrayDataByName("access_level", 1, i);
					packet->setArrayDataByName("visit_flag", 0, i); // 0 = allowed to visit, 1 = owner hasn't paid upkeep
					i++;
				}
			}
		}
		client->QueuePacket(packet->serialize());
	}
	safe_delete(packet);
}	

/*
<Struct Name="WS_DisplayVisitScreen" ClientVersion="1193" OpcodeName="OP_DisplayInnVisitScreenMsg">
<Data ElementName="num_houses" Type="int16" Size="1" />
<Data ElementName="visithouse_array" Type="Array" ArraySizeVariable="num_houses">
  <Data ElementName="house_id" Type="int64" />
  <Data ElementName="house_owner" Type="EQ2_16Bit_String" />
  <Data ElementName="house_location" Type="EQ2_16Bit_string" />
  <Data ElementName="house_zone" Type="EQ2_16Bit_String" />
  <Data ElementName="access_level" Type="int8" Size="1" />
  <Data ElementName="unknown3" Type="int8" Size="3" />
  <Data ElementName="visit_flag" Type="int8" Size="1" />
</Data>
<Data ElementName="unknown4" Type="int32" Size="1" />
<Data ElementName="unknown5" Type="int8" Size="1" />
</Struct>
*/


void ClientPacketFunctions::SendLocalizedTextMessage(Client* client)
{
	/***
	-- OP_ReloadLocalizedTxtMsg --
5/26/2020 19:08:41
69.174.200.100 -> 192.168.1.1
0000:	01 FF 63 01 62 00 00 00 1C 00 49 72 6F 6E 74 6F ..c.b.....Ironto
0010:	65 73 20 45 61 73 74 20 4C 61 72 67 65 20 49 6E es East Large In
0020:	6E 20 52 6F 6F 6D 07 01 00 00 00 1C 00 49 72 6F n Room.......Iro
0030:	6E 74 6F 65 73 20 45 61 73 74 20 4C 61 72 67 65 ntoes East Large
0040:	20 49 6E 6E 20 52 6F 6F 6D 07 02 00 00 00 1C 00  Inn Room.......
0050:	49 72 6F 6E 74 6F 65 73 20 45 61 73 74 20 4C 61 Irontoes East La
0060:	72 67 65 20 49 6E 6E 20 52 6F 6F 6D 07 03 00 00 rge Inn Room....
0070:	00 1C 00 49 72 6F 6E 74 6F 65 73 20 45 61 73 74 ...Irontoes East
0080:	20 4C 61 72 67 65 20 49 6E 6E 20 52 6F 6F 6D 07  Large Inn Room.
0090:	04 00 00 00 1C 00 49 72 6F 6E 74 6F 65 73 20 45 ......Irontoes E
00A0:	61 73 74 20 4C 61 72 67 65 20 49 6E 6E 20 52 6F ast Large Inn Ro
00B0:	6F 6D 07 05 00 00 00 1C 00 49 72 6F 6E 74 6F 65 om.......Irontoe
00C0:	73 20 45 61 73 74 20 4C 61 72 67 65 20 49 6E 6E s East Large Inn
00D0:	20 52 6F 6F 6D 07 06 00 00 00 1C 00 49 72 6F 6E  Room.......Iron
00E0:	74 6F 65 73 20 45 61 73 74 20 4C 61 72 67 65 20 toes East Large 
00F0:	49 6E 6E 20 52 6F 6F 6D 07 07 00 00 00 19 00 51 Inn Room.......Q
0100:	65 79 6E 6F 73 20 47 75 69 6C 64 20 48 61 6C 6C eynos Guild Hall
0110:	2C 20 54 69 65 72 20 31 07 08 00 00 00 16 00 4C , Tier 1.......L
0120:	69 6F 6E 27 73 20 4D 61 6E 65 20 53 75 69 74 65 ion's Mane Suite
0130:	20 52 6F 6F 6D 07 09 00 00 00 16 00 4C 69 6F 6E  Room.......Lion
0140:	27 73 20 4D 61 6E 65 20 53 75 69 74 65 20 52 6F 's Mane Suite Ro
0150:	6F 6D 07 0A 00 00 00 16 00 4C 69 6F 6E 27 73 20 om.......Lion's 
0160:	4D 61 6E 65 20 53 75 69 74 65 20 52 6F 6F 6D 07 Mane Suite Room.
0170:	0B 00 00 00 16 00 4C 69 6F 6E 27 73 20 4D 61 6E ......Lion's Man
0180:	65 20 53 75 69 74 65 20 52 6F 6F 6D 07 0C 00 00 e Suite Room....
0190:	00 0E 00 32 20 4C 75 63 69 65 20 53 74 72 65 65 ...2 Lucie Stree
01A0:	74 07 0D 00 00 00 0F 00 31 37 20 54 72 61 6E 71 t.......17 Tranq
01B0:	75 69 6C 20 57 61 79 07 0E 00 00 00 0E 00 38 20 uil Way.......8 
01C0:	4C 75 63 69 65 20 53 74 72 65 65 74 07 0F 00 00 Lucie Street....
01D0:	00 0F 00 31 32 20 4C 75 63 69 65 20 53 74 72 65 ...12 Lucie Stre
01E0:	65 74 07 10 00 00 00 0F 00 31 38 20 4C 75 63 69 et.......18 Luci
01F0:	65 20 53 74 72 65 65 74 07 11 00 00 00 0F 00 32 e Street.......2
0200:	30 20 4C 75 63 69 65 20 53 74 72 65 65 74 07 12 0 Lucie Street..
0210:	00 00 00 0E 00 33 20 54 72 61 6E 71 75 69 6C 20 .....3 Tranquil 
0220:	57 61 79 07 13 00 00 00 0E 00 37 20 54 72 61 6E Way.......7 Tran
0230:	71 75 69 6C 20 57 61 79 07 14 00 00 00 0F 00 31 quil Way.......1
0240:	33 20 54 72 61 6E 71 75 69 6C 20 57 61 79 07 15 3 Tranquil Way..
0250:	00 00 00 0F 00 31 35 20 54 72 61 6E 71 75 69 6C .....15 Tranquil
0260:	20 57 61 79 07 16 00 00 00 19 00 51 65 79 6E 6F  Way.......Qeyno
0270:	73 20 47 75 69 6C 64 20 48 61 6C 6C 2C 20 54 69 s Guild Hall, Ti
0280:	65 72 20 32 07 17 00 00 00 0F 00 38 20 45 72 6F er 2.......8 Ero
0290:	6C 6C 69 73 69 20 4C 61 6E 65 07 18 00 00 00 0F llisi Lane......
02A0:	00 35 20 45 72 6F 6C 6C 69 73 69 20 4C 61 6E 65 .5 Erollisi Lane
02B0:	07 19 00 00 00 0E 00 35 20 4B 61 72 61 6E 61 20 .......5 Karana 
02C0:	43 6F 75 72 74 07 1A 00 00 00 0D 00 32 20 42 61 Court.......2 Ba
02D0:	79 6C 65 20 43 6F 75 72 74 07 1B 00 00 00 0D 00 yle Court.......
02E0:	34 20 42 61 79 6C 65 20 43 6F 75 72 74 07 1C 00 4 Bayle Court...
02F0:	00 00 16 00 4C 69 6F 6E 27 73 20 4D 61 6E 65 20 ....Lion's Mane 
0300:	53 75 69 74 65 20 52 6F 6F 6D 07 1D 00 00 00 16 Suite Room......
0310:	00 4C 69 6F 6E 27 73 20 4D 61 6E 65 20 53 75 69 .Lion's Mane Sui
0320:	74 65 20 52 6F 6F 6D 07 1E 00 00 00 16 00 4C 69 te Room.......Li
0330:	6F 6E 27 73 20 4D 61 6E 65 20 53 75 69 74 65 20 on's Mane Suite 
0340:	52 6F 6F 6D 07 1F 00 00 00 16 00 4C 69 6F 6E 27 Room.......Lion'
0350:	73 20 4D 61 6E 65 20 53 75 69 74 65 20 52 6F 6F s Mane Suite Roo
0360:	6D 07 20 00 00 00 0E 00 35 20 4C 75 63 69 65 20 m. .....5 Lucie 
0370:	53 74 72 65 65 74 07 21 00 00 00 0F 00 32 30 20 Street.!.....20 
0380:	4B 61 72 61 6E 61 20 43 6F 75 72 74 07 22 00 00 Karana Court."..
0390:	00 0E 00 39 20 4C 75 63 69 65 20 53 74 72 65 65 ...9 Lucie Stree
03A0:	74 07 23 00 00 00 0F 00 31 35 20 4C 75 63 69 65 t.#.....15 Lucie
03B0:	20 53 74 72 65 65 74 07 24 00 00 00 0F 00 31 37  Street.$.....17
03C0:	20 4C 75 63 69 65 20 53 74 72 65 65 74 07 25 00  Lucie Street.%.
03D0:	00 00 0F 00 32 31 20 4C 75 63 69 65 20 53 74 72 ....21 Lucie Str
03E0:	65 65 74 07 26 00 00 00 0E 00 36 20 4B 61 72 61 eet.&.....6 Kara
03F0:	6E 61 20 43 6F 75 72 74 07 27 00 00 00 0F 00 31 na Court.'.....1
0400:	32 20 4B 61 72 61 6E 61 20 43 6F 75 72 74 07 28 2 Karana Court.(
0410:	00 00 00 0F 00 31 34 20 4B 61 72 61 6E 61 20 43 .....14 Karana C
0420:	6F 75 72 74 07 29 00 00 00 0F 00 31 38 20 4B 61 ourt.).....18 Ka
0430:	72 61 6E 61 20 43 6F 75 72 74 07 2A 00 00 00 1E rana Court.*....
0440:	00 43 6F 6E 63 6F 72 64 69 75 6D 20 54 6F 77 65 .Concordium Towe
0450:	72 20 4D 61 67 69 63 61 6C 20 4D 61 6E 6F 72 07 r Magical Manor.
0460:	2B 00 00 00 15 00 41 72 63 61 6E 65 20 41 63 61 +.....Arcane Aca
0470:	64 65 6D 79 20 50 6F 72 74 61 6C 07 2C 00 00 00 demy Portal.,...
0480:	13 00 43 6F 75 72 74 20 6F 66 20 74 68 65 20 4D ..Court of the M
0490:	61 73 74 65 72 07 2D 00 00 00 13 00 43 69 74 79 aster.-.....City
04A0:	20 6F 66 20 4D 69 73 74 20 45 73 74 61 74 65 07  of Mist Estate.
04B0:	2E 00 00 00 10 00 44 61 72 6B 6C 69 67 68 74 20 ......Darklight 
04C0:	50 61 6C 61 63 65 07 2F 00 00 00 11 00 44 65 65 Palace./.....Dee
04D0:	70 77 61 74 65 72 20 52 65 74 72 65 61 74 07 30 pwater Retreat.0
04E0:	00 00 00 24 00 44 68 61 6C 67 61 72 20 50 72 65 ...$.Dhalgar Pre
04F0:	63 69 70 69 63 65 20 6F 66 20 74 68 65 20 44 65 cipice of the De
0500:	65 70 20 50 6F 72 74 61 6C 07 31 00 00 00 12 00 ep Portal.1.....
0510:	44 69 6D 65 6E 73 69 6F 6E 61 6C 20 50 6F 63 6B Dimensional Pock
0520:	65 74 07 32 00 00 00 0B 00 44 6F 6A 6F 20 50 6F et.2.....Dojo Po
0530:	72 74 61 6C 07 33 00 00 00 21 00 45 6C 61 62 6F rtal.3...!.Elabo
0540:	72 61 74 65 20 45 73 74 61 74 65 20 6F 66 20 55 rate Estate of U
0550:	6E 72 65 73 74 20 50 6F 72 74 61 6C 07 34 00 00 nrest Portal.4..
0560:	00 11 00 45 74 68 65 72 6E 65 72 65 20 45 6E 63 ...Ethernere Enc
0570:	6C 61 76 65 07 35 00 00 00 10 00 45 76 65 72 66 lave.5.....Everf
0580:	72 6F 73 74 20 50 6F 72 74 61 6C 07 36 00 00 00 rost Portal.6...
0590:	16 00 46 65 61 72 66 75 6C 20 52 65 74 72 65 61 ..Fearful Retrea
05A0:	74 20 50 6F 72 74 61 6C 07 37 00 00 00 0F 00 46 t Portal.7.....F
05B0:	65 6C 77 69 74 68 65 20 50 6F 72 74 61 6C 07 38 elwithe Portal.8
05C0:	00 00 00 10 00 46 72 65 65 62 6C 6F 6F 64 20 50 .....Freeblood P
05D0:	6F 72 74 61 6C 07 39 00 00 00 0C 00 46 72 69 67 ortal.9.....Frig
05E0:	68 74 20 4D 61 6E 6F 72 07 3A 00 00 00 11 00 47 ht Manor.:.....G
05F0:	61 6C 6C 65 6F 6E 20 6F 66 20 44 72 65 61 6D 73 alleon of Dreams
0600:	07 3B 00 00 00 14 00 48 61 6C 6C 20 6F 66 20 74 .;.....Hall of t
0610:	68 65 20 43 68 61 6D 70 69 6F 6E 07 3C 00 00 00 he Champion.<...
0620:	10 00 48 75 61 20 4D 65 69 6E 20 52 65 74 72 65 ..Hua Mein Retre
0630:	61 74 07 3D 00 00 00 1C 00 49 73 6C 65 20 6F 66 at.=.....Isle of
0640:	20 52 65 66 75 67 65 20 50 72 65 73 74 69 67 65  Refuge Prestige
0650:	20 48 6F 6D 65 07 3E 00 00 00 0F 00 4B 65 72 61  Home.>.....Kera
0660:	66 79 72 6D 27 73 20 4C 61 69 72 07 3F 00 00 00 fyrm's Lair.?...
0670:	0E 00 4B 72 6F 6D 7A 65 6B 20 50 6F 72 74 61 6C ..Kromzek Portal
0680:	07 40 00 00 00 10 00 4C 61 76 61 73 74 6F 72 6D .@.....Lavastorm
0690:	20 50 6F 72 74 61 6C 07 41 00 00 00 0E 00 4C 69  Portal.A.....Li
06A0:	62 72 61 72 79 20 50 6F 72 74 61 6C 07 42 00 00 brary Portal.B..
06B0:	00 0B 00 4D 61 72 61 20 45 73 74 61 74 65 07 43 ...Mara Estate.C
06C0:	00 00 00 21 00 4D 61 6A 27 44 75 6C 20 41 73 74 ...!.Maj'Dul Ast
06D0:	72 6F 6E 6F 6D 65 72 27 73 20 54 6F 77 65 72 20 ronomer's Tower 
06E0:	50 6F 72 74 61 6C 07 44 00 00 00 14 00 4D 61 6A Portal.D.....Maj
06F0:	27 44 75 6C 20 53 75 69 74 65 20 50 6F 72 74 61 'Dul Suite Porta
0700:	6C 07 45 00 00 00 17 00 4D 69 73 74 6D 6F 6F 72 l.E.....Mistmoor
0710:	65 20 43 72 61 67 73 20 45 73 74 61 74 65 73 07 e Crags Estates.
0720:	46 00 00 00 0D 00 4F 61 6B 6D 79 73 74 20 47 6C F.....Oakmyst Gl
0730:	61 64 65 07 47 00 00 00 12 00 4F 70 65 72 61 20 ade.G.....Opera 
0740:	48 6F 75 73 65 20 50 6F 72 74 61 6C 07 48 00 00 House Portal.H..
0750:	00 16 00 50 65 72 73 6F 6E 61 6C 20 47 72 6F 74 ...Personal Grot
0760:	74 6F 20 50 6F 72 74 61 6C 07 49 00 00 00 17 00 to Portal.I.....
0770:	52 75 6D 20 52 75 6E 6E 65 72 73 20 43 6F 76 65 Rum Runners Cove
0780:	20 50 6F 72 74 61 6C 07 4A 00 00 00 12 00 50 6C  Portal.J.....Pl
0790:	61 6E 65 74 61 72 69 75 6D 20 50 6F 72 74 61 6C anetarium Portal
07A0:	07 4B 00 00 00 14 00 52 65 73 65 61 72 63 68 65 .K.....Researche
07B0:	72 27 73 20 53 61 6E 63 74 75 6D 07 4C 00 00 00 r's Sanctum.L...
07C0:	1E 00 52 65 73 69 64 65 6E 63 65 20 6F 66 20 74 ..Residence of t
07D0:	68 65 20 42 6C 61 64 65 73 20 50 6F 72 74 61 6C he Blades Portal
07E0:	07 4D 00 00 00 16 00 53 61 6E 63 74 75 73 20 53 .M.....Sanctus S
07F0:	65 72 75 20 50 72 6F 6D 65 6E 61 64 65 07 4E 00 eru Promenade.N.
0800:	00 00 22 00 53 61 6E 74 61 20 47 6C 75 67 27 73 ..".Santa Glug's
0810:	20 43 68 65 65 72 66 75 6C 20 48 6F 6C 69 64 61  Cheerful Holida
0820:	79 20 48 6F 6D 65 07 4F 00 00 00 17 00 53 65 63 y Home.O.....Sec
0830:	6C 75 64 65 64 20 53 61 6E 63 74 75 6D 20 50 6F luded Sanctum Po
0840:	72 74 61 6C 07 50 00 00 00 18 00 53 6B 79 62 6C rtal.P.....Skybl
0850:	61 64 65 20 53 6B 69 66 66 20 4C 61 75 6E 63 68 ade Skiff Launch
0860:	70 61 64 07 51 00 00 00 0E 00 53 6E 6F 77 79 20 pad.Q.....Snowy 
0870:	44 77 65 6C 6C 69 6E 67 07 52 00 00 00 1D 00 53 Dwelling.R.....S
0880:	70 72 6F 63 6B 65 74 27 73 20 49 6E 74 65 72 6C procket's Interl
0890:	6F 63 6B 69 6E 67 20 50 6C 61 6E 65 07 53 00 00 ocking Plane.S..
08A0:	00 17 00 53 74 6F 72 6D 20 54 6F 77 65 72 20 49 ...Storm Tower I
08B0:	73 6C 65 20 50 6F 72 74 61 6C 07 54 00 00 00 21 sle Portal.T...!
08C0:	00 52 65 6C 69 63 20 54 69 6E 6B 65 72 20 50 72 .Relic Tinker Pr
08D0:	65 73 74 69 67 65 20 48 6F 6D 65 20 50 6F 72 74 estige Home Port
08E0:	61 6C 07 55 00 00 00 10 00 54 65 6E 65 62 72 6F al.U.....Tenebro
08F0:	75 73 20 50 6F 72 74 61 6C 07 56 00 00 00 10 00 us Portal.V.....
0900:	54 68 65 20 42 61 75 62 62 6C 65 73 68 69 72 65 The Baubbleshire
0910:	07 57 00 00 00 0F 00 54 69 6E 6B 65 72 65 72 27 .W.....Tinkerer'
0920:	73 20 49 73 6C 65 07 58 00 00 00 12 00 54 6F 77 s Isle.X.....Tow
0930:	65 72 20 6F 66 20 4B 6E 6F 77 6C 65 64 67 65 07 er of Knowledge.
0940:	59 00 00 00 15 00 55 6E 63 61 6E 6E 79 20 45 73 Y.....Uncanny Es
0950:	74 61 74 65 20 50 6F 72 74 61 6C 07 5A 00 00 00 tate Portal.Z...
0960:	1E 00 56 61 63 61 6E 74 20 45 73 74 61 74 65 20 ..Vacant Estate 
0970:	6F 66 20 55 6E 72 65 73 74 20 50 6F 72 74 61 6C of Unrest Portal
0980:	07 5B 00 00 00 18 00 56 61 6C 65 20 6F 66 20 48 .[.....Vale of H
0990:	61 6C 66 70 69 6E 74 20 44 65 6C 69 67 68 74 07 alfpint Delight.
09A0:	5C 00 00 00 26 00 4C 69 6F 6E 27 73 20 4D 61 6E \...&.Lion's Man
09B0:	65 20 56 65 73 74 69 67 65 20 52 6F 6F 6D 20 2D e Vestige Room -
09C0:	20 4E 65 74 74 6C 65 76 69 6C 6C 65 07 5D 00 00  Nettleville.]..
09D0:	00 2C 00 4C 69 6F 6E 27 73 20 4D 61 6E 65 20 56 .,.Lion's Mane V
09E0:	65 73 74 69 67 65 20 52 6F 6F 6D 20 2D 20 53 74 estige Room - St
09F0:	61 72 63 72 65 73 74 20 43 6F 6D 6D 75 6E 65 07 arcrest Commune.
0A00:	5E 00 00 00 29 00 4C 69 6F 6E 27 73 20 4D 61 6E ^...).Lion's Man
0A10:	65 20 56 65 73 74 69 67 65 20 52 6F 6F 6D 20 2D e Vestige Room -
0A20:	20 47 72 61 79 73 74 6F 6E 65 20 59 61 72 64 07  Graystone Yard.
0A30:	5F 00 00 00 2C 00 4C 69 6F 6E 27 73 20 4D 61 6E _...,.Lion's Man
0A40:	65 20 56 65 73 74 69 67 65 20 52 6F 6F 6D 20 2D e Vestige Room -
0A50:	20 43 61 73 74 6C 65 76 69 65 77 20 48 61 6D 6C  Castleview Haml
0A60:	65 74 07 60 00 00 00 2A 00 4C 69 6F 6E 27 73 20 et.`...*.Lion's 
0A70:	4D 61 6E 65 20 56 65 73 74 69 67 65 20 52 6F 6F Mane Vestige Roo
0A80:	6D 20 2D 20 54 68 65 20 57 69 6C 6C 6F 77 20 57 m - The Willow W
0A90:	6F 6F 64 07 61 00 00 00 2B 00 4C 69 6F 6E 27 73 ood.a...+.Lion's
0AA0:	20 4D 61 6E 65 20 56 65 73 74 69 67 65 20 52 6F  Mane Vestige Ro
0AB0:	6F 6D 20 2D 20 54 68 65 20 42 61 75 62 62 6C 65 om - The Baubble
0AC0:	73 68 69 72 65 07 62 00 00 00 FF FF FF FF       shire.b.......
*/
}