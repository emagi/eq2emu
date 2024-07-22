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

//Character Creation Replies, put in globals so name filter can return proper responses
#define UNKNOWNERROR_REPLY				0
#define CREATESUCCESS_REPLY				1
#define NOSERVERSAVAIL_REPLY			2
#define CREATEPENDING_REPLY				3
#define MAXCHARSALLOWED_REPLY			4
#define INVALIDRACE_REPLY				5
#define INVALIDCITY_REPLY				6
#define INVALIDCLASS_REPLY				7
#define INVALIDGENDER_REPLY				8
#define INVALIDFIRST_LVL_REPLY			9
#define BADNAMELENGTH_REPLY				10
#define NAMEINVALID_REPLY				11
#define NAMEFILTER_REPLY				12 // name_filter reply (bad word or blocked words)
#define NAMETAKEN_REPLY					13
#define OVERLOADEDSERVER_REPLY			14
#define UNKNOWNERROR_REPLY2				15
#define INVALIDFEATURES1_REPLY			16
#define INVALIDFEATURES2_REPLY			17
#define INVALIDRACE_APPEARANCE_REPLY	18

#define PLAY_ERROR_PROBLEM					0
#define PLAY_ERROR_ZONE_DOWN				4
#define PLAY_ERROR_CHAR_NOT_LOADED			5
#define PLAY_ERROR_CHAR_NOT_FOUND			6
#define PLAY_ERROR_ACCOUNT_IN_USE			7
#define PLAY_ERROR_SERVER_TIMEOUT			8
#define PLAY_ERROR_SERVER_SHUTDOWN			9
#define PLAY_ERROR_LOADING_ERROR			10
#define PLAY_ERROR_EXCHANGE_SERVER			11
#define PLAY_ERROR_REGION_SERVER			12
#define PLAY_ERROR_CLASS_INVALID			13
#define PLAY_ERROR_TOO_MANY_CHARACTERS		14
#define PLAY_ERROR_EOF_EXP_NOT_FOUND		15
#define PLAY_ERROR_UNKNOWN_RESPONSE			16
#define PLAY_ERROR_UNKNOWN					17
#define PLAY_ERROR_ACCOUNT_BANNED			18
#define PLAY_ERROR_PROHIBITED				19
