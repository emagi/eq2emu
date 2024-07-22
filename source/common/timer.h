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
#ifndef TIMER_H
#define TIMER_H

#include "types.h"
#include <chrono>

// Disgrace: for windows compile
#ifdef WIN32
	#include <WinSock2.h>
	#include <windows.h>
	int gettimeofday (timeval *tp, ...);
#endif

class Timer
{
public:
	Timer();
	Timer(int32 timer_time, bool iUseAcurateTiming = false);
	Timer(int32 start, int32 timer, bool iUseAcurateTiming);
	~Timer() { }

	bool Check(bool iReset = true);
	void Enable();
	void Disable();
	void Start(int32 set_timer_time=0, bool ChangeResetTimer = true);
	void SetTimer(int32 set_timer_time=0);
	int32 GetRemainingTime();
	int32 GetElapsedTime();
	inline const int32& GetTimerTime()		{ return timer_time; }
	inline const int32& GetSetAtTrigger()	{ return set_at_trigger; }
	void Trigger();
	void SetAtTrigger(int32 set_at_trigger, bool iEnableIfDisabled = false);

	inline bool Enabled() { return enabled; }
	inline int32 GetStartTime() { return(start_time); }
	inline int32 GetDuration() { return(timer_time); }

	static const int32& SetCurrentTime();
	static const int32& GetCurrentTime2();
	static int32 GetUnixTimeStamp();

private:
	int32	start_time;
	int32	timer_time;
	bool	enabled;
	int32	set_at_trigger;

	// Tells the timer to be more acurate about happening every X ms.
	// Instead of Check() setting the start_time = now,
	// it it sets it to start_time += timer_time
	bool	pUseAcurateTiming;

//	static int32 current_time;
//	static int32 last_time;
};

struct BenchTimer
{
	typedef std::chrono::high_resolution_clock clock;

	BenchTimer() : start_time(clock::now()) {}
	void reset() { start_time = clock::now(); }
	// this is seconds
	double elapsed() { return std::chrono::duration<double>(clock::now() - start_time).count(); }
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
};

#endif
