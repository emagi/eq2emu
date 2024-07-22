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
#include "../common/debug.h"
// Disgrace: for windows compile
#ifndef WIN32
	#include <sys/time.h>
#else
	#include <sys/timeb.h>
#endif

#include <iostream>
using namespace std;

#include "timer.h"

int32 started_unix_timestamp = 0;
int32 current_time = 0;
int32 last_time = 0;

Timer::Timer(){
	timer_time = 30000; //default to 30 seconds
	start_time = current_time;
	set_at_trigger = timer_time;
	pUseAcurateTiming = false;
	enabled = false;
}
Timer::Timer(int32 in_timer_time, bool iUseAcurateTiming) {
	timer_time = in_timer_time;
	start_time = current_time;
	set_at_trigger = timer_time;
	pUseAcurateTiming = iUseAcurateTiming;
	if (timer_time == 0) {
		enabled = false;
	}
	else {
		enabled = true;
	}
}

Timer::Timer(int32 start, int32 timer, bool iUseAcurateTiming = false) {
	timer_time = timer;
	start_time = start;
	set_at_trigger = timer_time;
	pUseAcurateTiming = iUseAcurateTiming;
	if (timer_time == 0) {
		enabled = false;
	}
	else {
		enabled = true;
	}
}

/* Reimplemented for MSVC - Bounce */
#ifdef WIN32
int gettimeofday (timeval *tp, ...)
{
	timeb tb;

	ftime (&tb);

	tp->tv_sec  = tb.time;
	tp->tv_usec = tb.millitm * 1000;

	return 0;
}
#endif

/* This function checks if the timer triggered */
bool Timer::Check(bool iReset)
{
    if (enabled && current_time-start_time > timer_time) {
		if (iReset) {
			if (pUseAcurateTiming)
				start_time += timer_time;
			else
				start_time = current_time; // Reset timer
			timer_time = set_at_trigger;
		}
		return true;
    }
	
    return false;
}

/* This function disables the timer */
void Timer::Disable() {
	enabled = false;
}

void Timer::Enable() {
	enabled = true;
}

/* This function set the timer and restart it */
void Timer::Start(int32 set_timer_time, bool ChangeResetTimer) {	
    start_time = current_time;
	enabled = true;
    if (set_timer_time != 0)
    {	
		timer_time = set_timer_time;
		if (ChangeResetTimer)
			set_at_trigger = set_timer_time;
    }
}

/* This timer updates the timer without restarting it */
void Timer::SetTimer(int32 set_timer_time) {
    /* If we were disabled before => restart the timer */
    if (!enabled) {
		start_time = current_time;
		enabled = true;
    }
    if (set_timer_time != 0) {
		timer_time = set_timer_time;
		set_at_trigger = set_timer_time;
    }
}

int32 Timer::GetElapsedTime(){
	if (enabled) {
	    return current_time - start_time;
    }
	else {
		return 0xFFFFFFFF;
	}
}

int32 Timer::GetRemainingTime() {
    if (enabled) {
	    if (current_time-start_time > timer_time)
			return 0;
		else
			return (start_time + timer_time) - current_time;
    }
	else {
		return 0xFFFFFFFF;
	}
}

void Timer::SetAtTrigger(int32 in_set_at_trigger, bool iEnableIfDisabled) {
	set_at_trigger = in_set_at_trigger;
	if (!Enabled() && iEnableIfDisabled) {
		Enable();
	}
}

void Timer::Trigger()
{
	enabled = true;

	timer_time = set_at_trigger;
	start_time = current_time-timer_time-1;
}

const int32& Timer::GetCurrentTime2()
{	
    return current_time;
}

const int32& Timer::SetCurrentTime()
{
    struct timeval read_time;	
    int32 this_time;

    gettimeofday(&read_time,0);
	if(started_unix_timestamp == 0)
		started_unix_timestamp = read_time.tv_sec;

    this_time = (read_time.tv_sec - started_unix_timestamp) * 1000 + read_time.tv_usec / 1000;

    if (last_time == 0)
    {
		current_time = 0;
    }
    else
    {
		current_time += this_time - last_time;
    }
    
	last_time = this_time;

//	cerr << "Current time:" << current_time << endl;
	return current_time;
}

int32 Timer::GetUnixTimeStamp(){
	struct timeval read_time;	
    gettimeofday(&read_time,0);
	return read_time.tv_sec;
}
