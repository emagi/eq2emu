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

#ifndef __HEROICOP_H__
#define __HEROICOP_H__

#include <map>
#include <vector>

#include "../../common/types.h"

using namespace std;

struct HeroicOPStarter {
	int32 id;
	int8 start_class;
	int16 starter_icon;
	int16 abilities[6];
};

struct HeroicOPWheel {
	int8 order;
	int16 shift_icon;
	float chance;
	int16 abilities[6];
	int32 spell_id;
};

class HeroicOP {
public:
	HeroicOP();
	~HeroicOP();

	/// <summary>Sets the complete flag for this Heroic OP</summary>
	/// <param name='val'>The value to set the complete flag to, 1 = failed 2 = finished</param>
	void SetComplete(int8 val) { m_complete = val; }

	/// <summary>Sets the current stage of the starter chain or the wheel chain is at</summary>
	/// <param name='val'>The stage to set this Heroic OP to</param>
	void SetStage(int8 val) { m_currentStage = val; }

	/// <summary>Sets the wheel for this Heroic OP</summary>
	/// <param name='val'>The wheel we are setting the Heroic OP to</param>
	void SetWheel(HeroicOPWheel* val);
	
	/// <summary>Sets the start time for the wheel</summary>
	/// <param name='val'>Value to set the start time to</param>
	void SetStartTime(int32 val) { m_startTime = val; }

	/// <summary>Sets the total time to complete the wheel</summary>
	/// <param name='val'>Value to set the total time to</param>
	void SetTotalTime(float val) { m_totalTime = val; }

	/// <summary>Sets the target of this HO</summary>
	/// <param name='val'>The ID of the spawn</param>
	void SetTarget(int32 val);

	/// <summary>Gets the complete flag for this Heroic OP</summary>
	/// <returns>0 = not complete, 1 = complete, 2+= failed</returns>
	int8 GetComplete() { return m_complete; }

	/// <summary>Gets the wheel for this heroic op</summary>
	HeroicOPWheel* GetWheel() { return m_wheel; }

	/// <summary>Gets a pointer to the list of starter chains</summary>
	vector<HeroicOPStarter*>* GetStarterChains() { return &starters; }

	/// <summary>Gets the current stage the HO is on</summary>
	int8 GetStage() { return m_currentStage; }

	/// <summary>Gets the start time for the wheel</summary>
	int32 GetStartTime() { return m_startTime; }

	/// <summary>Gets the total time players have to complete the wheel</summary>
	float GetTotalTime() { return m_totalTime; }

	/// <summary>Gets the ID of this HO's target</summary>
	int32 GetTarget() { return m_target; }

	/// <summary></summary>
	bool HasShifted() { return m_shifted; }

	/// <summary>Checks to see if the given icon will advance the Heroic OP</summary>
	/// <param name='icon'>The icon that is trying to advance the Heroic OP</param>
	/// <returns>True if the icon advanced the HO</returns>
	bool UpdateHeroicOP(int16 icon);

	/// <summary>Reset the stage to 0</summary>
	void ResetStage() { m_currentStage = 0; }

	/// <summary>Adds a starter chain to the Heroic OP</summary>
	/// <param name='starter'>The starter chain to add</param>
	void AddStarterChain(HeroicOPStarter* starter);

	/// <summary>Attempts to shift the wheel</summary>
	bool ShiftWheel();

	int8 countered[6];

private:
	int8 m_complete;
	int8 m_currentStage;
	int32 m_startTime;
	float m_totalTime;
	int32 m_target;
	bool m_shifted;
	HeroicOPWheel* m_wheel;
	vector<HeroicOPStarter*> starters;
};

class MasterHeroicOPList {
public:
	MasterHeroicOPList();
	~MasterHeroicOPList();

	/// <summary>Adds the starter chain to the list</summary>
	/// <param name='start_class'>Class id for the starter chain</param>
	/// <param name='starter'>Starter chain to add</param>
	void AddStarter(int8 start_class, HeroicOPStarter* starter);

	/// <summary>Add the wheel chain to the list</summary>
	/// <param name='starter_id'>Id of the starter this wheel belongs to</param>
	/// <param name='wheel'>Wheel to add</param>
	void AddWheel(int32 starter_id, HeroicOPWheel* wheel);

	/// <summary>Creates a new HO</summary>
	/// <param name='class_id'>Class ID starting the HO</param>
	HeroicOP* GetHeroicOP(int8 class_id);

	/// <summary>Gets a random wheel from the given starter</summary>
	/// <param name='starter'>The starter to determine what wheels to choose from</param>
	HeroicOPWheel* GetWheel(HeroicOPStarter* starter);

private:
	// map<class, map<starter, vector<wheel> > >
	map<int8, map<HeroicOPStarter*, vector<HeroicOPWheel*> > > m_hoList;
};

#endif