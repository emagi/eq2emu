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

#include "../ClientPacketFunctions.h"
#include "../../common/Log.h"
#include "HeroicOp.h"
#include "../Spells.h"

extern ConfigReader configReader;
extern MasterSpellList master_spell_list;

void ClientPacketFunctions::SendHeroicOPUpdate(Client* client, HeroicOP* ho) {
	if (!client) {
		LogWrite(PACKET__ERROR, 0, "Packets", "SendHeroicOPUpdate() called without a valid client");
		return;
	}

	if (!ho) {
		LogWrite(PACKET__ERROR, 0, "Packets", "SendHeroicOPUpdate() called without a valid HO");
		return;
	}

	PacketStruct* packet = configReader.getStruct("WS_HeroicOpportunity", client->GetVersion());
	Spell* spell = 0;
	if (packet) {
		packet->setDataByName("id", client->GetPlayer()->GetIDWithPlayerSpawn(client->GetPlayer()));
		if (ho->GetWheel()) {
			spell = master_spell_list.GetSpell(ho->GetWheel()->spell_id, 1);
			if (!spell) {
				LogWrite(SPELL__ERROR, 0, "HO", "Unable to get the spell (%u)", ho->GetWheel()->spell_id);
				return;
			}

			packet->setDataByName("name", spell->GetName());
			packet->setDataByName("description", spell->GetDescription());
			packet->setDataByName("order", ho->GetWheel()->order);
			packet->setDataByName("time_total", ho->GetTotalTime());
			packet->setDataByName("time_left", max(0.0f, (float)(((ho->GetStartTime() + (ho->GetTotalTime() * 1000)) - Timer::GetCurrentTime2()) / 1000)));
			// This is not displayed in the wheel so set it to 0xFFFF
			packet->setDataByName("starter_icon", 0xFFFF);

			if (ho->HasShifted())
				packet->setDataByName("shift_icon", 0xFFFF);
			else
				packet->setDataByName("shift_icon", ho->GetWheel()->shift_icon);

			// If completed set special values
			if (ho->GetComplete() > 0) {
				packet->setDataByName("wheel_type", 2);
				packet->setDataByName("unknown", ho->GetComplete());
			}

			char temp[20];
			char ability[20];

			// Set the icons for the whee;
			for (int8 i = 1; i < 7; i++) {
				strcpy(ability, "icon");
				itoa(i, temp, 10);
				strcat(ability, temp);
				packet->setDataByName(ability, ho->GetWheel()->abilities[i-1]);
			}

			// Flag the icons that are completed
			for (int8 i = 1; i < 7; i++) {
				strcpy(ability, "countered");
				itoa(i, temp, 10);
				strcat(ability, temp);
				packet->setDataByName(ability, ho->countered[i-1]);
			}

		}
		else {
			if (ho->GetComplete() > 0) {
				// This will make the ui element vanish
				packet->setDataByName("wheel_type", 5);
				packet->setDataByName("unknown", 8);
			}
			else {
				packet->setDataByName("wheel_type", 4);
			}

			packet->setDataByName("icon1", 0xFFFF);
			packet->setDataByName("icon2", 0xFFFF);
			packet->setDataByName("icon3", 0xFFFF);
			packet->setDataByName("icon4", 0xFFFF);
			packet->setDataByName("icon5", 0xFFFF);
			packet->setDataByName("icon6", 0xFFFF);
			packet->setDataByName("shift_icon", 0xFFFF);
			
			int8 index = 1;
			char temp[20];
			char ability[20];
			vector<HeroicOPStarter*>::iterator itr;
			for (itr = ho->GetStarterChains()->begin(); itr != ho->GetStarterChains()->end(); itr++, index++) {
				if (index > 6 )
					break;

				strcpy(ability, "icon");
				itoa(index, temp, 10);
				strcat(ability, temp);

				packet->setDataByName(ability, (*itr)->abilities[ho->GetStage()]);

				// Only set this once
				if (index == 1)
					packet->setDataByName("starter_icon", (*itr)->starter_icon);
			}
		}
		client->QueuePacket(packet->serialize());
	}
	safe_delete(packet);
}


/*
<Struct Name="WS_HeroicOpportunity" ClientVersion="1" OpcodeName="OP_UpdateOpportunityMsg">
<Data ElementName="name" Type="EQ2_16Bit_String" />
<Data ElementName="description" Type="EQ2_16Bit_String" />
<Data ElementName="id" Type="int32" />
<Data ElementName="wheel_type" Type="int8" />
<Data ElementName="unknown" Type="int8" />
<Data ElementName="order" Type="int8" />
<Data ElementName="shift_icon" Type="int16" />
<Data ElementName="starter_icon" Type="int16" />
<Data ElementName="time_total" Type="float" />
<Data ElementName="time_left" Type="float" />
<Data ElementName="icon1" Type="int16" />
<Data ElementName="icon2" Type="int16" />
<Data ElementName="icon3" Type="int16" />
<Data ElementName="icon4" Type="int16" />
<Data ElementName="icon5" Type="int16" />
<Data ElementName="icon6" Type="int16" />
<Data ElementName="countered1" Type="int16" />
<Data ElementName="countered2" Type="int16" />
<Data ElementName="countered3" Type="int16" />
<Data ElementName="countered4" Type="int16" />
<Data ElementName="countered5" Type="int16" />
<Data ElementName="countered6" Type="int16" />
</Struct>
*/