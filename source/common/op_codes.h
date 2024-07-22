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
#ifndef _OP_CODES_H

#define _OP_CODES_H

static const char OP_SessionRequest	=	0x01;
static const char OP_SessionResponse	=	0x02;
static const char OP_Combined		=	0x03;
static const char OP_SessionDisconnect	=	0x05;
static const char OP_KeepAlive		=	0x06;
static const char OP_ServerKeyRequest	=	0x07;
static const char OP_SessionStatResponse=	0x08;
static const char OP_Packet		=	0x09;
static const char OP_Fragment		=	0x0d;
static const char OP_OutOfOrderAck	=	0x11;
static const char OP_Ack		=	0x15;
static const char OP_AppCombined	=	0x19;
static const char OP_OutOfSession	=	0x1d;

#if defined(LOGIN) || defined(CHAT)
	#define APP_OPCODE_SIZE	1
#else
	#define APP_OPCODE_SIZE	2
#endif

#endif
