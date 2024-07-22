#include "BotBrain.h"
#include "../Combat.h"
#include "../Spells.h"
#include "../../common/Log.h"
#include "../Rules/Rules.h"

extern RuleManager rule_manager;

BotBrain::BotBrain(Bot* body) : Brain(body) {
	Body = body;
}

BotBrain::~BotBrain() {

}

void BotBrain::Think() {
	// No ownder do nothing, probably despawn as owner should never be empty for bots
	if (!m_body->GetOwner())
		return;

	// Not in a group yet then do nothing
	if (!m_body->GetGroupMemberInfo())
		return;

	if (!Body->Alive())
		return;

	if (Body->IsMezzedOrStunned())
		return;

	// If combat was processed we can return out
	if (ProcessCombat())
		return;

	// Combat failed to process so do out of combat tasks like follow the player
	if (ProcessOutOfCombatSpells())
		return;

	// put htis here so bots don't try to follow the owner while in combat
	if (Body->EngagedInCombat())
		return;

	// Set target to owner
	Spawn* target = GetBody()->GetFollowTarget();

	if(target)
	{
		// Get distance from the owner
		float distance = GetBody()->GetDistance(target);

		// If out of melee range then move closer
		if (distance > rule_manager.GetGlobalRule(R_Combat, MaxCombatRange)->GetFloat())
			MoveCloser(target);
	}
}

bool BotBrain::ProcessCombat() {
	SetTarget();

	if (Body->GetTarget() && Body->EngagedInCombat()) {
		if (Body->GetTarget() && Body->GetTarget()->IsEntity() && Body->AttackAllowed((Entity*)Body->GetTarget())) {
			Entity* target = (Entity*)Body->GetTarget();
			float distance = Body->GetDistance(target);

			if (!ProcessSpell(target, distance)) {
				if (Body->ShouldMelee())
					ProcessMelee(target, distance);
			}

			NPC* pet = (NPC*)Body->GetPet();
			if (pet) {
				if (pet->Brain()->GetHate(target) == 0)
					pet->AddHate(target, 1);
			}
		}

		return true;
	}

	return false;
}

void BotBrain::SetTarget() {
	// The target issued from /bot attack
	if (Body->GetCombatTarget() && Body->GetCombatTarget()->Alive()) {
		Body->SetTarget(Body->GetCombatTarget());
		Body->InCombat(true);
		return;
	}

	// Assist
	Entity* owner = Body->GetOwner();
	if (owner && owner->EngagedInCombat()) {
		if (owner->GetTarget() && owner->GetTarget()->IsEntity() && owner->GetTarget()->Alive() && owner->AttackAllowed((Entity*)owner->GetTarget())) {
			Body->SetTarget(owner->GetTarget());
			Body->InCombat(true);
			// Add some hate to keep the bot attacking if
			// the player toggles combat off
			if (GetHate((Entity*)Body->GetTarget()) == 0)
				AddHate((Entity*)Body->GetTarget(), 1);
			return;
		}
	}

	// Most hated
	Entity* hated = GetMostHated();
	if (hated && hated->Alive()) {
		if (hated == Body->GetOwner()) {
			ClearHate(hated);
		}
		else {
			Body->SetTarget(hated);
			Body->InCombat(true);
			return;
		}
	}

	// None of the above true so clear target and turn combat off
	Body->SetTarget(0);
	Body->InCombat(false);
}

bool BotBrain::ProcessSpell(Entity* target, float distance) {
	if (Body->IsStifled() || Body->IsFeared())
		return false;
	
	if (Body->IsCasting())
		return false;

	if (!HasRecovered())
		return false;

	Spell* spell = Body->SelectSpellToCast(distance);
	if (spell) {
		// Target can change (heals for example) so recalculate distance and if out of range move closer
		float distance = Body->GetDistance(Body->GetTarget());
		if (distance > spell->GetSpellData()->range) {
			if (Body->GetTarget()->IsEntity())
				MoveCloser((Spawn*)Body->GetTarget());
		}
		else {
			// stop movement if spell can't be cast while moving
			if (!spell->GetSpellData()->cast_while_moving)
				Body->CalculateRunningLocation(true);

			Body->GetZone()->ProcessSpell(spell, Body, Body->GetTarget());
			m_spellRecovery = (int32)(Timer::GetCurrentTime2() + (spell->GetSpellData()->cast_time * 10) + (spell->GetSpellData()->recovery * 10) + 2000);
			// recast time
			int32 time = Timer::GetCurrentTime2() + (spell->CalculateRecastTimer(Body));
			Body->SetRecast(spell, time);

			string str = "I am casting ";
			str += spell->GetName();
			Body->MessageGroup(str);
		}
		return true;
	}

	return false;
}

bool BotBrain::ProcessOutOfCombatSpells() {
	if (Body->IsStifled() || Body->IsFeared())
		return false;

	if (Body->IsCasting())
		return false;

	if (!HasRecovered())
		return false;

	Spell* spell = Body->GetHealSpell();
	if (!spell)
		spell = Body->GetRezSpell();
	if (!spell)
		spell = Body->GetNextBuffSpell();

	if (spell) {
		// stop movement if spell can't be cast while moving
		if (!spell->GetSpellData()->cast_while_moving)
			Body->CalculateRunningLocation(true);

		// See if we are in range of target, if not move closer
		float distance = Body->GetDistance(Body->GetTarget());
		if (distance > spell->GetSpellData()->range) {
			if (Body->GetTarget()->IsEntity())
				MoveCloser((Spawn*)Body->GetTarget());
		}
		else {
			Body->GetZone()->ProcessSpell(spell, Body, Body->GetTarget());
			m_spellRecovery = (int32)(Timer::GetCurrentTime2() + (spell->GetSpellData()->cast_time * 10) + (spell->GetSpellData()->recovery * 10) + 2000);
			// recast time
			int32 time = Timer::GetCurrentTime2() + (spell->CalculateRecastTimer(Body));
			Body->SetRecast(spell, time);

			string str = "I am casting ";
			str += spell->GetName();
			Body->MessageGroup(str);
		}
		return true;
	}

	return false;
}