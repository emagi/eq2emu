# File: `HousingPackets.cpp`

## Classes

_None detected_

## Functions

- `void ClientPacketFunctions::SendHousePurchase(Client* client, HouseZone* hz, int32 spawnID) {`
- `void ClientPacketFunctions::SendHousingList(Client* client) {`
- `void ClientPacketFunctions::SendBaseHouseWindow(Client* client, HouseZone* hz, PlayerHouse* ph, int32 spawnID) {`
- `void ClientPacketFunctions::SendHouseVisitWindow(Client* client, vector<PlayerHouse*> houses) {`
- `void ClientPacketFunctions::SendLocalizedTextMessage(Client* client)`

## Notable Comments

- //req.append(std::to_string(static_cast<long long>(hz->guild_level)));
- //req.append(std::to_string(static_cast<long long>(hz->guild_level)))
- //packet->PrintPacket();
- // this packet must be sent first otherwise it blocks out the enter house option after paying upkeep
- // this seems to be some kind of timestamp, if we keep updating then in conjunction with upkeep_due
- // in SendBaseHouseWindow/WS_PlayerHouseBaseScreen being a >0 number we can access 'enter house'
- // if we don't send this then the enter house option won't be available if upkeep is paid
- // need this to enable the "enter house" button
- // temp - set priv level to owner for now
- // temp - set house type to personal house for now
