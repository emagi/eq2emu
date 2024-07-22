#include "mob_movement_manager.h"
#include "../Entity.h"
#include "../zoneserver.h"
#include "region_map.h"
#include "map.h"
#include "../../common/timer.h"
#include "pathfinder_interface.h"
#include "position.h"
#include "../../common/Log.h"

#include <vector>
#include <deque>
#include <map>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#endif

extern double frame_time;

class IMovementCommand {
public:
	IMovementCommand() = default;
	virtual ~IMovementCommand() = default;
	virtual bool Process(MobMovementManager* mob_movement_manager, Entity* mob) = 0;
	virtual bool Started() const = 0;
};

class RotateToCommand : public IMovementCommand {
public:
	RotateToCommand(double rotate_to, double dir, MobMovementMode mob_movement_mode)
	{
		m_rotate_to = rotate_to;
		m_rotate_to_dir = dir;
		m_rotate_to_mode = mob_movement_mode;
		m_started = false;
	}

	virtual ~RotateToCommand()
	{

	}

	virtual bool Process(MobMovementManager* mob_movement_manager, Entity* mob)
	{
		auto rotate_to_speed = m_rotate_to_mode == MovementRunning ? 200.0 : 16.0; //todo: get this from mob

		auto from = mob_movement_manager->FixHeading(mob->GetHeading());
		auto to = mob_movement_manager->FixHeading(m_rotate_to);
		auto diff = to - from;

		while (diff < -256.0) {
			diff += 512.0;
		}

		while (diff > 256) {
			diff -= 512.0;
		}

		auto dist = std::abs(diff);

		if (!m_started) {
			m_started = true;
			//mob->SetMoving(true);

			/*if (dist > 15.0f && rotate_to_speed > 0.0 && rotate_to_speed <= 25.0) { //send basic rotation
				mob_movement_manager->SendCommandToClients(
					mob,
					0.0,
					0.0,
					0.0,
					m_rotate_to_dir * rotate_to_speed,
					0,
					ClientRangeClose
				);
			}*/
		}

		auto td = rotate_to_speed * 19.0 * frame_time;

		if (td >= dist) {
			mob->SetHeading(to);
			//mob->SetMoving(false);
			//mob_movement_manager->SendCommandToClients(mob, 0.0, 0.0, 0.0, 0.0, 0, ClientRangeCloseMedium);
			return true;
		}

		from += td * m_rotate_to_dir;
		mob->SetHeading(mob_movement_manager->FixHeading(from));
		return false;
	}

	virtual bool Started() const
	{
		return m_started;
	}

private:
	double          m_rotate_to;
	double          m_rotate_to_dir;
	MobMovementMode m_rotate_to_mode;
	bool            m_started;
};

class MoveToCommand : public IMovementCommand {
public:
	MoveToCommand(float x, float y, float z, MobMovementMode mob_movement_mode)
	{
		m_distance_moved_since_correction = 0.0;
		m_move_to_x = x;
		m_move_to_y = y;
		m_move_to_z = z;
		m_move_to_mode = mob_movement_mode;
		m_last_sent_time = 0.0;
		m_last_sent_speed = 0;
		m_started = false;
		m_total_h_dist = 0.0;
		m_total_v_dist = 0.0;
	}

	virtual ~MoveToCommand()
	{

	}

	/**
	 * @param mob_movement_manager
	 * @param mob
	 * @return
	 */
	virtual bool Process(MobMovementManager* mob_movement_manager, Entity* mob)
	{
		//Send a movement packet when you start moving		
		double current_time = static_cast<double>(Timer::GetCurrentTime2()) / 1000.0;
		int    current_speed = 0;

		if (m_move_to_mode == MovementRunning) {
			current_speed = ((Spawn*)mob)->GetSpeed();
		}

		if (!m_started) {
			m_started = true;
			//rotate to the point
			//mob->SetMoving(true);
			mob->SetHeading(mob->GetFaceTarget(m_move_to_x, m_move_to_z));

			m_last_sent_speed = current_speed;
			m_last_sent_time = current_time;
			// Z/Y are flipped due to EverQuest 2 using Y as up/down
			m_total_h_dist = DistanceNoZ(glm::vec4(mob->GetX(),mob->GetZ(),mob->GetY(),mob->GetHeading()), glm::vec4(m_move_to_x, m_move_to_z, 0.0f, 0.0f));
			m_total_v_dist = m_move_to_y - mob->GetY();
			//mob_movement_manager->SendCommandToClients(mob, 0.0, 0.0, 0.0, 0.0, current_speed, ClientRangeCloseMedium);
		}

		//When speed changes
		if (current_speed != m_last_sent_speed) {
			//if (RuleB(Map, FixZWhenPathing)) {
			//	mob->FixZ();
			//}

			m_distance_moved_since_correction = 0.0;

			m_last_sent_speed = current_speed;
			m_last_sent_time = current_time;
			//mob_movement_manager->SendCommandToClients(mob, 0.0, 0.0, 0.0, 0.0, current_speed, ClientRangeCloseMedium);
		}

		//If x seconds have passed without sending an update.
		if (current_time - m_last_sent_time >= 5.0) {
			//if (RuleB(Map, FixZWhenPathing)) {
				//mob->FixZ();
			//}

			m_distance_moved_since_correction = 0.0;

			m_last_sent_speed = current_speed;
			m_last_sent_time = current_time;
			//mob_movemesnt_manager->SendCommandToClients(mob, 0.0, 0.0, 0.0, 0.0, current_speed, ClientRangeCloseMedium);
		}

		glm::vec3 p = glm::vec3(mob->GetX(), mob->GetY(), mob->GetZ());
		// our X/Z versus the mobs X/Z
		glm::vec2 tar(m_move_to_x, m_move_to_z);
		glm::vec2 pos(p.x, p.z);
		double    len = glm::distance(pos, tar);
		if (len < .01) {
			return true;
		}

		//mob->SetMoved(true);

		glm::vec2 dir = tar - pos;
		glm::vec2 ndir = glm::normalize(dir);
		double    distance_moved = frame_time * current_speed * 0.4f * 1.45f;

			//mob->SetX(m_move_to_x);
			//mob->SetY(m_move_to_z);
			//mob->SetZ(m_move_to_y);

		mob->ClearRunningLocations();

		if (distance_moved > len) {
			//if (RuleB(Map, FixZWhenPathing)) {
				//mob->FixZ();
			//}
			// we use npos.y because higher up that is the equilvaent Z
			mob->AddRunningLocation(m_move_to_x, m_move_to_y, m_move_to_z, current_speed, distance_moved, true, true, "", true);
			return false;
		}
		else {
			glm::vec2 npos = pos + (ndir * static_cast<float>(distance_moved));

			len -= distance_moved;
			double total_distance_traveled = m_total_h_dist - len;
			double start_y = m_move_to_y - m_total_v_dist;
			double y_at_pos = start_y + (m_total_v_dist * (total_distance_traveled / m_total_h_dist));

			// we use npos.y because higher up that is the equilvaent Z
			mob->AddRunningLocation(m_move_to_x, m_move_to_y, m_move_to_z, current_speed, distance_moved, true, true, "", true);

		//	mob->SetX(npos.x);
		//	mob->SetY(z_at_pos);
		//	mob->SetZ(npos.y);

			//if (RuleB(Map, FixZWhenPathing)) {
			//	m_distance_moved_since_correction += distance_moved;
			//	if (m_distance_moved_since_correction > 10.0f /*RuleR(Map, DistanceCanTravelBeforeAdjustment)*/) {
				//	m_distance_moved_since_correction = 0.0;
					//mob->FixZ();
				//}
		//	}
		}

		return false;
	}

	virtual bool Started() const
	{
		return m_started;
	}

protected:
	double          m_distance_moved_since_correction;
	double          m_move_to_x;
	double          m_move_to_y;
	double          m_move_to_z;
	MobMovementMode m_move_to_mode;
	bool            m_started;

	double m_last_sent_time;
	int    m_last_sent_speed;
	double m_total_h_dist;
	double m_total_v_dist;
};

class SwimToCommand : public MoveToCommand {
public:
	SwimToCommand(float x, float y, float z, MobMovementMode mob_movement_mode) : MoveToCommand(x, y, z, mob_movement_mode)
	{

	}

	virtual bool Process(MobMovementManager* mob_movement_manager, Entity* mob)
	{
		//Send a movement packet when you start moving
		double current_time = static_cast<double>(Timer::GetCurrentTime2()) / 1000.0;
		int    current_speed = 0;

		if (m_move_to_mode == MovementRunning) {
			if (mob->IsFeared()) {
				current_speed = mob->GetBaseSpeed();
			}
			else {
				//runback overrides
				if (mob->GetSpeed() > mob->GetMaxSpeed())
					current_speed = mob->GetSpeed();
				else
					current_speed = mob->GetMaxSpeed();
			}
		}
		else {
			current_speed = mob->GetBaseSpeed();
		}

		if (!m_started) {
			m_started = true;
			//rotate to the point
			//mob->SetMoving(true);
			mob->SetHeading(mob->GetFaceTarget(m_move_to_x, m_move_to_z));

			m_last_sent_speed = current_speed;
			m_last_sent_time = current_time;
			m_total_h_dist = DistanceNoZ(glm::vec4(mob->GetX(),mob->GetZ(),mob->GetY(),mob->GetHeading()), glm::vec4(m_move_to_x, m_move_to_z, 0.0f, 0.0f));
			m_total_v_dist = m_move_to_y - mob->GetY();
			//mob_movement_manager->SendCommandToClients(mob, 0.0, 0.0, 0.0, 0.0, current_speed, ClientRangeCloseMedium);
		}

		//When speed changes
		if (current_speed != m_last_sent_speed) {
			m_last_sent_speed = current_speed;
			m_last_sent_time = current_time;
			//mob_movement_manager->SendCommandToClients(mob, 0.0, 0.0, 0.0, 0.0, current_speed, ClientRangeCloseMedium);
		}

		//If x seconds have passed without sending an update.
		if (current_time - m_last_sent_time >= 1.5) {
			m_last_sent_speed = current_speed;
			m_last_sent_time = current_time;
			//mob_movement_manager->SendCommandToClients(mob, 0.0, 0.0, 0.0, 0.0, current_speed, ClientRangeCloseMedium);
		}

		glm::vec4 p = glm::vec4(mob->GetX(), mob->GetZ(), mob->GetY(), mob->GetHeading());
		glm::vec2 tar(m_move_to_x, m_move_to_y);
		glm::vec2 pos(p.x, p.y);
		double    len = glm::distance(pos, tar);
		if (len == 0) {
			return true;
		}

		//mob->SetMoved(true);

		glm::vec2 dir = tar - pos;
		glm::vec2 ndir = glm::normalize(dir);
		double    distance_moved = frame_time * current_speed * 0.4f * 1.45f;

		mob->SetX(m_move_to_x);
		mob->SetZ(m_move_to_z);
		mob->SetY(m_move_to_y);
		if (distance_moved > len) {
			return true;
		}
		else {
			glm::vec2 npos = pos + (ndir * static_cast<float>(distance_moved));

			len -= distance_moved;
			double total_distance_traveled = m_total_h_dist - len;
			double start_y = m_move_to_y - m_total_v_dist;
			double y_at_pos = start_y + (m_total_v_dist * (total_distance_traveled / m_total_h_dist));

			mob->SetX(npos.x);
			mob->SetZ(npos.y);
			mob->SetY(y_at_pos);
		}

		return false;
	}
};

class TeleportToCommand : public IMovementCommand {
public:
	TeleportToCommand(float x, float y, float z, float heading)
	{
		m_teleport_to_x = x;
		m_teleport_to_y = y;
		m_teleport_to_z = z;
		m_teleport_to_heading = heading;
	}

	virtual ~TeleportToCommand()
	{

	}

	virtual bool Process(MobMovementManager* mob_movement_manager, Entity* mob)
	{
		mob->SetX(m_teleport_to_x);
		mob->SetZ(m_teleport_to_z);
		mob->SetY(m_teleport_to_y);
		mob->SetHeading(mob_movement_manager->FixHeading(m_teleport_to_heading));
		//mob_movement_manager->SendCommandToClients(mob, 0.0, 0.0, 0.0, 0.0, 0, ClientRangeAny);

		return true;
	}

	virtual bool Started() const
	{
		return false;
	}

private:

	double m_teleport_to_x;
	double m_teleport_to_y;
	double m_teleport_to_z;
	double m_teleport_to_heading;
};

class StopMovingCommand : public IMovementCommand {
public:
	StopMovingCommand()
	{
	}

	virtual ~StopMovingCommand()
	{

	}

	virtual bool Process(MobMovementManager* mob_movement_manager, Entity* mob)
	{
		mob->ClearRunningLocations();
		return true;
	}

	virtual bool Started() const
	{
		return false;
	}
};

class EvadeCombatCommand : public IMovementCommand {
public:
	EvadeCombatCommand()
	{
	}

	virtual ~EvadeCombatCommand()
	{

	}

	virtual bool Process(MobMovementManager* mob_movement_manager, Entity* mob)
	{
		return true;
	}

	virtual bool Started() const
	{
		return false;
	}
};

struct MovementStats {
	MovementStats()
	{
		LastResetTime     = static_cast<double>(Timer::GetCurrentTime2()) / 1000.0;
		TotalSent         = 0ULL;
		TotalSentMovement = 0ULL;
		TotalSentPosition = 0ULL;
		TotalSentHeading  = 0ULL;
	}

	double   LastResetTime;
	uint64_t TotalSent;
	uint64_t TotalSentMovement;
	uint64_t TotalSentPosition;
	uint64_t TotalSentHeading;
};

struct NavigateTo {
	NavigateTo()
	{
		navigate_to_x       = 0.0;
		navigate_to_y       = 0.0;
		navigate_to_z       = 0.0;
		navigate_to_heading = 0.0;
		last_set_time       = 0.0;
	}

	double navigate_to_x;
	double navigate_to_y;
	double navigate_to_z;
	double navigate_to_heading;
	double last_set_time;
};

struct MobMovementEntry {
	std::deque<std::unique_ptr<IMovementCommand>> Commands;
	NavigateTo                                    NavTo;
};

void AdjustRoute(std::list<IPathfinder::IPathNode> &nodes, Entity *who)
{
	if (who->GetZone() == nullptr || !who->GetMap() /*|| !zone->HasWaterMap()*/) {
		return;
	}

	auto offset = who->GetYOffset();

	for (auto &node : nodes) {
		//if (!zone->watermap->InLiquid(node.pos)) {
			auto best_z = who->FindBestZ(node.pos, nullptr);
			if (best_z != BEST_Z_INVALID) {
				node.pos.z = best_z + offset;
			}
		//} // todo: floating logic?
	}
}

struct MobMovementManager::Implementation {
	std::map<Entity *, MobMovementEntry> Entries;
	std::vector<Client *>             Clients;
	MovementStats                     Stats;
};

MobMovementManager::MobMovementManager()
{
	MobListMutex.SetName("MobMovementManager");
	_impl.reset(new Implementation());
}

MobMovementManager::~MobMovementManager()
{
}

void MobMovementManager::Process()
{
	MobListMutex.readlock();
	for (auto &iter : _impl->Entries) {
		auto &ent      = iter.second;
		auto &commands = ent.Commands;

		if (commands.size() < 1)
			continue;

		iter.first->MCommandMutex.writelock();
		while (true != commands.empty()) {
			auto &cmd = commands.front();
			auto r    = cmd->Process(this, iter.first);

			if (true != r) {
				break;
			}

			commands.pop_front();
		}
		iter.first->MCommandMutex.releasewritelock();
	}
	MobListMutex.releasereadlock();
}

/**
 * @param mob
 */
void MobMovementManager::AddMob(Entity *mob)
{
	MobListMutex.writelock();
	_impl->Entries.insert(std::make_pair(mob, MobMovementEntry()));
	MobListMutex.releasewritelock();
}

/**
 * @param mob
 */
void MobMovementManager::RemoveMob(Entity *mob)
{
	MobListMutex.writelock();
	auto iter = _impl->Entries.find(mob);
	if(iter != _impl->Entries.end())
		_impl->Entries.erase(iter);
	MobListMutex.releasewritelock();
}

/**
 * @param client
 */
void MobMovementManager::AddClient(Client *client)
{
	_impl->Clients.push_back(client);
}

/**
 * @param client
 */
void MobMovementManager::RemoveClient(Client *client)
{
	auto iter = _impl->Clients.begin();
	while (iter != _impl->Clients.end()) {
		if (client == *iter) {
			_impl->Clients.erase(iter);
			return;
		}

		++iter;
	}
}

/**
 * @param who
 * @param to
 * @param mob_movement_mode
 */
void MobMovementManager::RotateTo(Entity *who, float to, MobMovementMode mob_movement_mode)
{
	MobListMutex.readlock();
	auto iter = _impl->Entries.find(who);

	if (iter == _impl->Entries.end())
	{
		MobListMutex.releasereadlock();
		return; // does not exist in navigation
	}

	auto &ent = (*iter);

	if (true != ent.second.Commands.empty()) {
		MobListMutex.releasereadlock();
		return;
	}

	PushRotateTo(ent.second, who, to, mob_movement_mode);
	MobListMutex.releasereadlock();
}

/**
 * @param who
 * @param x
 * @param y
 * @param z
 * @param heading
 */
void MobMovementManager::Teleport(Entity *who, float x, float y, float z, float heading)
{
	MobListMutex.readlock();
	auto iter = _impl->Entries.find(who);

	if (iter == _impl->Entries.end())
	{
		MobListMutex.releasereadlock();
		return; // does not exist in navigation
	}

	auto &ent = (*iter);

	ent.second.Commands.clear();

	PushTeleportTo(ent.second, x, y, z, heading);
	MobListMutex.releasereadlock();
}

/**
 * @param who
 * @param x
 * @param y
 * @param z
 * @param mode
 */
void MobMovementManager::NavigateTo(Entity *who, float x, float y, float z, MobMovementMode mode, bool overrideDistance)
{
	glm::vec3 targPos(x, z, y);
	glm::vec3 origPos(who->GetX(), who->GetZ(), who->GetY());

	if (IsPositionEqualWithinCertainZ(targPos, origPos, 6.0f)) {
		return;
	}
	MobListMutex.readlock();
	auto iter = _impl->Entries.find(who);

	if (iter == _impl->Entries.end())
	{
		MobListMutex.releasereadlock();
		return; // does not exist in navigation
	}

	auto &ent = (*iter);
	auto &nav = ent.second.NavTo;

	double current_time = static_cast<double>(Timer::GetCurrentTime2()) / 1000.0;
	if ((current_time - nav.last_set_time) > 0.5) {
		//Can potentially recalc

		auto within        = IsPositionWithinSimpleCylinder(
			glm::vec3(who->GetX(), who->GetZ(), who->GetY()),
			glm::vec3(nav.navigate_to_x, nav.navigate_to_z, nav.navigate_to_y),
			1.5f,
			6.0f
		);

		who->MCommandMutex.writelock();

		if (within && ent.second.Commands.size() > 0 && nav.last_set_time != 0)
		{
			//who->ClearRunningLocations();
			//StopNavigation((Entity*)who);
			who->MCommandMutex.releasewritelock();
			MobListMutex.releasereadlock();
			return;
		}
		else if (!within && ent.second.Commands.size() > 0 && nav.last_set_time != 0)
		{
			who->MCommandMutex.releasewritelock();
			MobListMutex.releasereadlock();
			return;
		}

		LogWrite(MAP__DEBUG, 0, "Map", "%s %f %f %f: within: %i, commands: %i, lastnav: %f %f %f", who->GetName(), 
			who->GetX(),who->GetY(),who->GetZ(),within,
			ent.second.Commands.size(), nav.navigate_to_x, nav.navigate_to_y, nav.navigate_to_z);
		//auto heading_match = IsHeadingEqual(0.0, nav.navigate_to_heading);

		//if (/*false == within ||*/ false == heading_match || ent.second.Commands.size() == 0) {
			ent.second.Commands.clear();

			//Path is no longer valid, calculate a new path
			UpdatePath(who, x, y, z, mode);
			nav.navigate_to_x       = x;
			nav.navigate_to_y       = y;
			nav.navigate_to_z       = z;
			nav.navigate_to_heading = 0.0;
			nav.last_set_time       = current_time;

			who->MCommandMutex.releasewritelock();
		//}
	}
	MobListMutex.releasereadlock();
}

/**
 * @param who
 */
void MobMovementManager::StopNavigation(Entity *who)
{
	MobListMutex.readlock();
	auto iter = _impl->Entries.find(who);

	if (iter == _impl->Entries.end())
	{
		MobListMutex.releasereadlock();
		return; // does not exist in navigation
	}

	auto &ent = (*iter);
	auto &nav = ent.second.NavTo;

	nav.navigate_to_x       = 0.0;
	nav.navigate_to_y       = 0.0;
	nav.navigate_to_z       = 0.0;
	nav.navigate_to_heading = 0.0;
	nav.last_set_time = 0.0;

	who->MCommandMutex.writelock();
	if (true == ent.second.Commands.empty()) {
		PushStopMoving(ent.second);
		who->MCommandMutex.releasewritelock();
		MobListMutex.releasereadlock();
		return;
	}

	if (!who->IsRunning()) {
		ent.second.Commands.clear();
		who->MCommandMutex.releasewritelock();
		MobListMutex.releasereadlock();
		return;
	}

	ent.second.Commands.clear();
	PushStopMoving(ent.second);

	who->MCommandMutex.releasewritelock();
	MobListMutex.releasereadlock();
}

void MobMovementManager::DisruptNavigation(Entity* who)
{
	MobListMutex.readlock();
	auto iter = _impl->Entries.find(who);

	if (iter == _impl->Entries.end())
	{
		MobListMutex.releasereadlock();
		return; // does not exist in navigation
	}

	auto& ent = (*iter);
	auto& nav = ent.second.NavTo;

	nav.navigate_to_x = 0.0;
	nav.navigate_to_y = 0.0;
	nav.navigate_to_z = 0.0;
	nav.navigate_to_heading = 0.0;
	nav.last_set_time = 0.0;

	if (!who->IsRunning()) {
		who->MCommandMutex.writelock();
		ent.second.Commands.clear();
		who->MCommandMutex.releasewritelock();
	}
	MobListMutex.releasereadlock();
}

/**
 * @param in
 * @return
 */
float MobMovementManager::FixHeading(float in)
{
	auto h = in;
	while (h > 512.0) {
		h -= 512.0;
	}

	while (h < 0.0) {
		h += 512.0;
	}

	return h;
}

void MobMovementManager::ClearStats()
{
	_impl->Stats.LastResetTime     = static_cast<double>(Timer::GetCurrentTime2()) / 1000.0;
	_impl->Stats.TotalSent         = 0;
	_impl->Stats.TotalSentHeading  = 0;
	_impl->Stats.TotalSentMovement = 0;
	_impl->Stats.TotalSentPosition = 0;
}


/**
 * @param who
 * @param x
 * @param y
 * @param z
 * @param mob_movement_mode
 */
void MobMovementManager::UpdatePath(Entity *who, float x, float y, float z, MobMovementMode mob_movement_mode)
{
	if (!who->GetMap() /*|| !zone->HasWaterMap()*/) {
		MobListMutex.readlock();
		auto iter = _impl->Entries.find(who);

		if (iter == _impl->Entries.end())
		{
			MobListMutex.releasereadlock();
			return; // does not exist in navigation
		}

		auto &ent = (*iter);

		PushMoveTo(ent.second, x, y, z, mob_movement_mode);
		PushStopMoving(ent.second);
		MobListMutex.releasereadlock();
		return;
	}
	/*
	if (who-?()) {
		UpdatePathBoat(who, x, y, z, mob_movement_mode);
	}
	else if (who->IsUnderwaterOnly()) {
		UpdatePathUnderwater(who, x, y, z, mob_movement_mode);
	}*/
	//else {
		UpdatePathGround(who, x, y, z, mob_movement_mode);
	//}
}

/**
 * @param who
 * @param x
 * @param y
 * @param z
 * @param mode
 */
void MobMovementManager::UpdatePathGround(Entity *who, float x, float y, float z, MobMovementMode mode)
{
	PathfinderOptions opts;
	opts.smooth_path = true;
	opts.step_size = 100.0f;//RuleR(Pathing, NavmeshStepSize);
	opts.offset      = who->GetYOffset()+1.0f;
	opts.flags       = PathingNotDisabled ^ PathingZoneLine;

	//This is probably pointless since the nav mesh tool currently sets zonelines to disabled anyway
	auto partial = false;
	auto stuck   = false;
	auto route   = who->GetZone()->pathing->FindPath(
		glm::vec3(who->GetX(), who->GetZ(), who->GetY()),
		glm::vec3(x, z, y),
		partial,
		stuck,
		opts
	);

	MobListMutex.readlock();
	auto eiter = _impl->Entries.find(who);

	if (eiter == _impl->Entries.end())
	{
		MobListMutex.releasereadlock();
		return; // does not exist in navigation
	}

	auto &ent  = (*eiter);

	if (route.size() == 0) {
		HandleStuckBehavior(who, x, y, z, mode);
		MobListMutex.releasereadlock();
		return;
	}

	AdjustRoute(route, who);

	//avoid doing any processing if the mob is stuck to allow normal stuck code to work.
	if (!stuck) {

		//there are times when the routes returned are no differen than where the mob is currently standing. What basically happens
		//is a mob will get 'stuck' in such a way that it should be moving but the 'moving' place is the exact same spot it is at.
		//this is a problem and creates an area of ground that if a mob gets to, will stay there forever. If socal this creates a
		//"Ball of Death" (tm). This code tries to prevent this by simply warping the mob to the requested x/y. Better to have a warp than
		//have stuck mobs.

		auto routeNode   = route.begin();
		bool noValidPath = true;
		while (routeNode != route.end() && noValidPath == true) {
			auto &currentNode = (*routeNode);

			if (routeNode == route.end()) {
				continue;
			}

			if (!(currentNode.pos.x == who->GetX() && currentNode.pos.y == who->GetZ())) {
				//if one of the nodes to move to, is not our current node, pass it.
				noValidPath = false;
				break;
			}
			//move to the next node
			routeNode++;

		}

		if (noValidPath) {
			//we are 'stuck' in a path, lets just get out of this by 'teleporting' to the next position.
			PushTeleportTo(
				ent.second,
				x,
				y,
				z,
				CalculateHeadingAngleBetweenPositions(who->GetX(), who->GetZ(), x, z)
			);

			MobListMutex.releasereadlock();
			return;
		}
	}

	auto iter = route.begin();

	glm::vec3 previous_pos(who->GetX(), who->GetZ(), who->GetY());

	bool first_node = true;
	while (iter != route.end()) {
		auto &current_node = (*iter);

		iter++;

		if (iter == route.end()) {
			continue;
		}

		previous_pos = current_node.pos;
		auto &next_node = (*iter);

		if (first_node) {

			if (mode == MovementWalking) {
				auto h = who->GetFaceTarget(next_node.pos.x, next_node.pos.y);
				PushRotateTo(ent.second, who, h, mode);
			}

			first_node = false;
		}

		//move to / teleport to node + 1
		if (next_node.teleport && next_node.pos.x != 0.0f && next_node.pos.y != 0.0f) {
			float calcedHeading =
				CalculateHeadingAngleBetweenPositions(
					current_node.pos.x,
					current_node.pos.y,
					next_node.pos.x,
					next_node.pos.y
				);
			PushTeleportTo(
				ent.second,
				next_node.pos.x,
				next_node.pos.z,
				next_node.pos.y,
				calcedHeading
			);
		}
		else {
/*			if (who->GetZone()->watermap->InLiquid(previous_pos)) {
				PushSwimTo(ent.second, next_node.pos.x, next_node.pos.y, next_node.pos.z, mode);
			}
			else {*/
				PushMoveTo(ent.second, next_node.pos.x, next_node.pos.z, next_node.pos.y, mode);
//			}
		}
	}

	if (stuck) {
		HandleStuckBehavior(who, x, y, z, mode);
	}
	else {
		PushStopMoving(ent.second);
	}
	MobListMutex.releasereadlock();
}

/**
 * @param who
 * @param x
 * @param y
 * @param z
 * @param movement_mode
 */
void MobMovementManager::UpdatePathUnderwater(Entity *who, float x, float y, float z, MobMovementMode movement_mode)
{
	MobListMutex.readlock();
	auto eiter = _impl->Entries.find(who);

	if (eiter == _impl->Entries.end())
	{
		MobListMutex.releasereadlock();
		return; // does not exist in navigation
	}

	auto &ent  = (*eiter);
	if (/*zone->watermap->InLiquid(who->GetPosition()) && zone->watermap->InLiquid(glm::vec3(x, y, z)) &&*/
		who->CheckLoS(glm::vec3(who->GetX(),who->GetZ(),who->GetY()), glm::vec3(x, z, y))) {
		PushSwimTo(ent.second, x, y, z, movement_mode);
		PushStopMoving(ent.second);
		MobListMutex.releasereadlock();
		return;
	}

	PathfinderOptions opts;
	opts.smooth_path = true;
	opts.step_size = 100.0f;// RuleR(Pathing, NavmeshStepSize);
	opts.offset      = who->GetYOffset();
	opts.flags       = PathingNotDisabled ^ PathingZoneLine;

	auto partial = false;
	auto stuck   = false;
	auto route   = who->GetZone()->pathing->FindPath(
		glm::vec3(who->GetX(), who->GetY(), who->GetZ()),
		glm::vec3(x, y, z),
		partial,
		stuck,
		opts
	);

	if (route.size() == 0) {
		HandleStuckBehavior(who, x, z, y, movement_mode);
		MobListMutex.releasereadlock();
		return;
	}

	AdjustRoute(route, who);

	auto      iter       = route.begin();
	glm::vec3 previous_pos(who->GetX(), who->GetY(), who->GetZ());
	bool      first_node = true;

	while (iter != route.end()) {
		auto &current_node = (*iter);

/*		if (!zone->watermap->InLiquid(current_node.pos)) {
			stuck = true;

			while (iter != route.end()) {
				iter = route.erase(iter);
			}

			break;
		}
		else {*/
			iter++;
//		}
	}

	if (route.size() == 0) {
		HandleStuckBehavior(who, x, y, z, movement_mode);
		MobListMutex.releasereadlock();
		return;
	}

	iter = route.begin();

	while (iter != route.end()) {
		auto &current_node = (*iter);

		iter++;

		if (iter == route.end()) {
			continue;
		}

		previous_pos = current_node.pos;
		auto &next_node = (*iter);

		if (first_node) {

			if (movement_mode == MovementWalking) {
				auto h = who->GetFaceTarget(next_node.pos.x, next_node.pos.y);
				PushRotateTo(ent.second, who, h, movement_mode);
			}

			first_node = false;
		}

		//move to / teleport to node + 1
		if (next_node.teleport && next_node.pos.x != 0.0f && next_node.pos.y != 0.0f) {
			float calcHeading = CalculateHeadingAngleBetweenPositions(
				current_node.pos.x,
				current_node.pos.y,
				next_node.pos.x,
				next_node.pos.y
			);

			PushTeleportTo(
				ent.second, next_node.pos.x, next_node.pos.z, next_node.pos.y, calcHeading);
		}
		else {
			PushSwimTo(ent.second, next_node.pos.x, next_node.pos.z, next_node.pos.y, movement_mode);
		}
	}

	if (stuck) {
		HandleStuckBehavior(who, x, y, z, movement_mode);
	}
	else {
		PushStopMoving(ent.second);
	}
	MobListMutex.releasereadlock();
}

/**
 * @param who
 * @param x
 * @param y
 * @param z
 * @param mode
 */
void MobMovementManager::UpdatePathBoat(Entity *who, float x, float y, float z, MobMovementMode mode)
{
	MobListMutex.readlock();
	auto eiter = _impl->Entries.find(who);

	if (eiter == _impl->Entries.end())
	{
		MobListMutex.releasereadlock();
		return; // does not exist in navigation
	}

	auto &ent  = (*eiter);

	PushSwimTo(ent.second, x, y, z, mode);
	PushStopMoving(ent.second);
	MobListMutex.releasereadlock();
}

/**
 * @param ent
 * @param x
 * @param y
 * @param z
 * @param heading
 */
void MobMovementManager::PushTeleportTo(MobMovementEntry &ent, float x, float y, float z, float heading)
{
	ent.Commands.push_back(std::unique_ptr<IMovementCommand>(new TeleportToCommand(x, y, z, heading)));
}

/**
 * @param ent
 * @param x
 * @param y
 * @param z
 * @param mob_movement_mode
 */
void MobMovementManager::PushMoveTo(MobMovementEntry &ent, float x, float y, float z, MobMovementMode mob_movement_mode)
{
	ent.Commands.push_back(std::unique_ptr<IMovementCommand>(new MoveToCommand(x, y, z, mob_movement_mode)));
}

/**
 * @param ent
 * @param x
 * @param y
 * @param z
 * @param mob_movement_mode
 */
void MobMovementManager::PushSwimTo(MobMovementEntry &ent, float x, float y, float z, MobMovementMode mob_movement_mode)
{
	ent.Commands.push_back(std::unique_ptr<IMovementCommand>(new SwimToCommand(x, y, z, mob_movement_mode)));
}

/**
 * @param ent
 * @param who
 * @param to
 * @param mob_movement_mode
 */
void MobMovementManager::PushRotateTo(MobMovementEntry &ent, Entity *who, float to, MobMovementMode mob_movement_mode)
{
	auto from = FixHeading(who->GetHeading());
	to = FixHeading(to);

	float diff = to - from;

	if (std::abs(diff) < 0.001f) {
		return;
	}

	while (diff < -256.0) {
		diff += 512.0;
	}

	while (diff > 256) {
		diff -= 512.0;
	}

	ent.Commands.push_back(std::unique_ptr<IMovementCommand>(new RotateToCommand(to, diff > 0 ? 1.0 : -1.0, mob_movement_mode)));
}

/**
 * @param mob_movement_entry
 */
void MobMovementManager::PushStopMoving(MobMovementEntry &mob_movement_entry)
{
	mob_movement_entry.Commands.push_back(std::unique_ptr<IMovementCommand>(new StopMovingCommand()));
}

/**
 * @param mob_movement_entry
 */
void MobMovementManager::PushEvadeCombat(MobMovementEntry &mob_movement_entry)
{
	mob_movement_entry.Commands.push_back(std::unique_ptr<IMovementCommand>(new EvadeCombatCommand()));
}

/**
 * @param who
 * @param x
 * @param y
 * @param z
 * @param mob_movement_mode
 */
void MobMovementManager::HandleStuckBehavior(Entity *who, float x, float y, float z, MobMovementMode mob_movement_mode)
{
	//LogDebug("Handle stuck behavior for {0} at ({1}, {2}, {3}) with movement_mode {4}", who->GetName(), x, y, z, mob_movement_mode);

	MobListMutex.readlock();
	auto sb = RunToTarget;//who->GetStuckBehavior();
	MobStuckBehavior behavior = RunToTarget;

	if (sb >= 0 && sb < MaxStuckBehavior) {
		behavior = (MobStuckBehavior) sb;
	}

	auto eiter = _impl->Entries.find(who);

	if (eiter == _impl->Entries.end())
	{
		MobListMutex.releasereadlock();
		return; // does not exist in navigation
	}

	auto &ent = (*eiter);

	switch (sb) {
		case RunToTarget:
			PushMoveTo(ent.second, x, y, z, mob_movement_mode);
			PushStopMoving(ent.second);
			break;
		case WarpToTarget:
			PushTeleportTo(ent.second, x, y, z, 0.0f);
			PushStopMoving(ent.second);
			break;
		case TakeNoAction:
			PushStopMoving(ent.second);
			break;
		case EvadeCombat:
			PushEvadeCombat(ent.second);
			break;
	}

	MobListMutex.releasereadlock();
}
