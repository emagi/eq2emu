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
#ifndef __EQ2_TRADESKILLS__
#define __EQ2_TRADESKILLS__

#include "../../common/types.h"
#include "../../common/Mutex.h"
#include "../Items/Items.h"
#include <map>
class Player;
class Spawn;
class Recipe;
class Client;

struct TradeskillEvent
{
	char		Name[250];
	int16		Icon;
	int32		Technique;
	int16		SuccessProgress;
	int16		SuccessDurability;
	int16		SuccessHP;
	int16		SuccessPower;
	int32		SuccessSpellID;
	int32		SuccessItemID;
	sint16		FailProgress;
	sint16		FailDurability;
	sint16		FailHP;
	sint16		FailPower;
};

struct Tradeskill
{
	Player*			player;
	Spawn*			table;
	Recipe*			recipe;
	int32			currentProgress;
	int32			currentDurability;
	int32			nextUpdateTime;
	vector<pair<int32, int16>>	usedComponents;
	TradeskillEvent* CurrentEvent;
	bool			eventChecked;
	bool			eventCountered;
};

class TradeskillMgr
{
public:
	TradeskillMgr();
	~TradeskillMgr();

	/// <summary>Determines if an update is needed if so send one and stop crafting if finished</summary>
	void Process();

	/// <summary>Starts the actual crafting process</summary>
	/// <param name='client'>Client that is crafting</param>
	/// <param name='components'>List of items the player is using to craft</param>
	void BeginCrafting(Client* client, vector<pair<int32, int16>> components);

	/// <summary>Stops the crafting process</summary>
	/// <param name='client'>Client that stopped crafting</param>
	/// <param name='lock'>Does the list need a mutex lock? default = true</param>
	void StopCrafting(Client* client, bool lock = true);

	/// <summary>Checks to see if the given client is crafting</summary>
	/// <param name='client'>The client to check</param>
	/// <returns>True if the client is crafting</returns>
	bool IsClientCrafting(Client* client);

	/// <summary>Get the tradeskill struct for the given client</summary>
	/// <param name='client'>The client to get the tradeskill struct for</param>
	/// <returns>Pointer to the clients tradeskill struct, or 0 if they don't have one</returns>
	Tradeskill* GetTradeskill(Client* client);

	/// <summary>Check to see if we countered the tradeskill event</summary>
	/// <param name='client'>The client to check for</param>
	/// <param name='icon'>The icon of the spell we casted</param>
	void CheckTradeskillEvent(Client* client, int16 icon);

	/// <summary>Lock the tradeskill list for reading, should never need to write to the tradeskill list outside of the TradeskillMgr class</summary>
	/// <param name='function'>Function name that called this lock</param>
	/// <param name='line'>Line number this lock was called from</param>
	void ReadLock(const char* function = (const char*)0, int32 line = 0) { m_tradeskills.readlock(function, line); }

	/// <summary>Releases the red lock on the tradeskill list</summary>
	/// <param name='function'>Function name that is releasing the lock</param>
	/// <param name='line'>Line number that is releasing the lock</param>
	void ReleaseReadLock(const char* function = (const char*)0, int32 line = 0) { m_tradeskills.releasereadlock(function, line); }
	
	int32 GetTechniqueSuccessAnim(int16 version, int32 technique);
	int32 GetTechniqueFailureAnim(int16 version, int32 technique);
	int32 GetTechniqueIdleAnim(int16 version, int32 technique);
	int32 GetMissTargetAnim(int16 version);
	int32 GetKillMissTargetAnim(int16 version);
	
	void SetClientIdleVisualState(Client* client, Tradeskill* ts);
private:
	/// <summary>Sends the creation window</summary>
	/// <param name='client'>Client to send the window to</param>
	/// <param name='recipe'>The recipe being crafted</param>
	void SendItemCreationUI(Client* client, Recipe* recipe);
	map<Client*, Tradeskill*>	tradeskillList;
	Mutex m_tradeskills;

	float m_critFail;
	float m_critSuccess;
	float m_fail;
	float m_success;
	float m_eventChance;
};

class MasterTradeskillEventsList
{
public:
	MasterTradeskillEventsList();
	~MasterTradeskillEventsList();

	/// <summary>Adds a tradeskill event to the master list</summery>
	/// <param name='tradeskillEvent'>The event to add</param>
	void AddEvent(TradeskillEvent* tradeskillEvent);

	/// <summary>Gets a list of tradeskill events for the given technique</summery>
	/// <param name='technique'>The skill id of the technique</param>
	/// <returns>Vector of TradeskillEvent* for the given technique</returns>
	vector<TradeskillEvent*>* GetEventByTechnique(int32 technique);
	
	/// <summary>Get the size of the event list</summary>
	/// <returns>int32 containing the size of the list</returns>
	int32 Size();

private:
	Mutex m_eventList;
	map<int32, vector<TradeskillEvent*> > eventList;
};

#endif
