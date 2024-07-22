#pragma once

#include "../NPC_AI.h"
#include "Bot.h"

class BotBrain : public Brain {
public:
	BotBrain(Bot* body);
	virtual ~BotBrain();
	void Think();
	bool ProcessSpell(Entity* target, float distance);

	bool ProcessOutOfCombatSpells();

private:
	Bot* Body;

	bool ProcessCombat();
	void SetTarget();
};