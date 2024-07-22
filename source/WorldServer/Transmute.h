#ifndef TRANSMUTE_H
#define TRANSMUTE_H

#include "../common/types.h"
#include <vector>

class Client;
class Player;
class Item;
class DatabaseResult;

class Transmute {
public:
	static int32 CreateItemRequest(Client* client, Player* player);
	static void HandleItemResponse(Client* client, Player* player, int32 req_id, int32 item_id);
	static bool ItemIsTransmutable(Item* item);
	static void SendConfirmRequest(Client* client, int32 req_id, Item* item);
	static void HandleConfirmResponse(Client* client, Player* player, int32 item_id);
	static void CompleteTransmutation(Client* client, Player* player);
	static void ProcessDBResult(DatabaseResult& res);

private:
	struct TransmutingTier {
		int32 min_level;
		int32 max_level;
		int32 fragment_id;
		int32 powder_id;
		int32 infusion_id;
		int32 mana_id;
	};

	static std::vector<TransmutingTier>& GetTransmutingTiers();
};

#endif