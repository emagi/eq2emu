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
#ifndef _EQ2COMMON_STRUCTS_
#define _EQ2COMMON_STRUCTS_

#define SPAWN_PACKET_SIZE					895
#define EQUIPMENT_L_WEAPON_INDEX			0 //chars left hand weapon
#define EQUIPMENT_R_WEAPON_INDEX			1 //chars right hand weapon
#define EQUIPMENT_HELMET					2

#pragma pack(1)
struct KeyGen_Struct{
	int32 size;
};
struct KeyGen_End_Struct{
	int32	exponent_len;
	int8	exponent;
};
struct LoginByNumRequest_Struct{
	int32	account_id;
	int32	access_code;
	int16	version;
	int32	unknown2[5];
};
struct LS_LoginResponse{
	int8	reply_code; // 0 granted, 1 denied
	int16	unknown01;
	int8	unknown02;
	sint32	unknown03; // -1 denied, 0 granted
	sint32	unknown04; 
	sint32	unknown05;
	sint32	unknown06;
	int8	unknown07;
	int8	unknown08;
	int8	unknown09;
	int8	unknown10;
	sint32	unknown11;
	int32	accountid;
	int16	unknown12;
};
#pragma pack()
enum EQ2_EquipmentSlot {
	slot_primary=0,
	slot_secondary=1,
	slot_head=2,
	slot_chest=3,
	slot_shoulders=4,
	slot_forearms=5,
	slot_hands=6,
	slot_legs=7,
	slot_feet=8,
	slot_left_ring=9,
	slot_right_ring=10,
	slot_ears=11,
	slot_neck=12,
	slot_left_wrist=13,
	slot_right_wrist=14,
	slot_ranged=15,
	slot_ammo=16,
	slot_waist=17,
	slot_activate1=18,
	slot_activate2=19,
	slot_textures=20,
	slot_hair=21,
	slot_beard=22,
	slot_naked_chest=23,
	slot_naked_legs=24
};
struct EQ2_EquipmentItem{
	int16		type;
	EQ2_Color	color;
	EQ2_Color	highlight;
};
struct EQ2_Equipment{
	int16		equip_id[25];
	EQ2_Color	color[25];
	EQ2_Color	highlight[25];
};
#pragma pack(1)
struct CharFeatures{	
	int16				hair_type;
	int16				hair_face_type;
	int16				wing_type;
	int16				chest_type;
	int16				legs_type;
	sint8				eye_type[3];
	sint8				ear_type[3];
	sint8				eye_brow_type[3];
	sint8				cheek_type[3];
	sint8				lip_type[3];
	sint8				chin_type[3];
	sint8				nose_type[3];
	sint8				body_size;
	sint8				body_age;
	sint8				soga_eye_type[3];
	sint8				soga_ear_type[3];
	sint8				soga_eye_brow_type[3];
	sint8				soga_cheek_type[3];
	int16				soga_chest_type;
	int16				soga_legs_type;
	sint8				soga_lip_type[3];
	sint8				soga_chin_type[3];
	sint8				soga_nose_type[3];
	sint8				soga_body_size;
	sint8				soga_body_age;
	int16				soga_hair_type;
	int16				soga_hair_face_type;
	int16				combat_voice;
	int16				emote_voice;
	int16				mount_model_type;

	EQ2_Color			mount_saddle_color;
	EQ2_Color			mount_color;
	EQ2_Color			skin_color;
	EQ2_Color			eye_color;
	EQ2_Color			hair_type_color;
	EQ2_Color			hair_type_highlight_color;	
	EQ2_Color			hair_face_color;
	EQ2_Color			hair_face_highlight_color;	
	EQ2_Color			hair_highlight_color;
	EQ2_Color			wing_color1;
	EQ2_Color			wing_color2;	
	EQ2_Color			shirt_color;	
	EQ2_Color			pants_color;	
	EQ2_Color			hair_color1;
	EQ2_Color			hair_color2;	
	EQ2_Color			soga_skin_color;
	EQ2_Color			soga_eye_color;	
	EQ2_Color			soga_hair_color1;
	EQ2_Color			soga_hair_color2;	
	EQ2_Color			soga_hair_type_color;
	EQ2_Color			soga_hair_type_highlight_color;	
	EQ2_Color			soga_hair_face_color;
	EQ2_Color			soga_hair_face_highlight_color;
	EQ2_Color			soga_hair_highlight_color;
	
	EQ2_Color			model_color;
	EQ2_Color			soga_model_color;
};
struct PositionData{
	int32			grid_id;
	int32			bad_grid_id;
	sint8			Speed1;
	sint8			Speed2;
	sint16			Dir1;
	sint16			Dir2;
	sint16          Pitch1;
	sint16          Pitch2;
	sint16          Roll;
	float			X;
	float			Y;
	float			Z;
	float			X2;
	float			Y2;
	float			Z2;
	float			X3;
	float			Y3;
	float			Z3;
	float			SpawnOrigX;
	float			SpawnOrigY;
	float			SpawnOrigZ;
	float			SpawnOrigHeading;
	float           SpawnOrigPitch;
	float           SpawnOrigRoll;
	float			SpeedX;
	float			SpeedY;
	float			SpeedZ;
	float			SideSpeed;
	float			VertSpeed;
	float			ClientHeading1;
	float			ClientHeading2;
	float			ClientPitch;
	int16			collision_radius;
	int16			state;
};
struct AppearanceData {
	PositionData	pos;
	int16			model_type;
	int16			soga_model_type;
	int16			activity_status;
	int16			visual_state;
	int16			action_state;
	int16			mood_state;
	int16           emote_state;
	int8			attackable;
	int8			icon;
	int8            hide_hood;
	int8			show_level;

	int8			locked_no_loot;
	int8			quest_flag;
	int8			heroic_flag;
	int8			show_command_icon;
	int8			display_hand_icon;
	int8			player_flag;
	int8			targetable;
	int8			display_name;
	char			sub_title[255]; //Guild
	int32			display_hp;//0 = 100 percent
	int32			power_left; //bar not shown if >=100
	int8			adventure_class;
	int8			tradeskill_class;
	int8			level;
	int8			tradeskill_level;
	int8			min_level;
	int8			max_level;
	int8			difficulty;	
	int16			visible; // 02 = normal, 15 = shadow
	char			name[128]; //size around here somewhere
	char			last_name[64];
	char			prefix_title[128];
	char			suffix_title[128];
	int8			race;
	int8			gender;	
	int32			randomize;
	int8			lua_race_id;
};
struct Player_Update{
/*0000*/	int32	activity;
/*0004*/	float	unknown2; // 1
/*0008*/	float	direction1;	
/*0012*/	float	unknown3[8];
/*0044*/	float	speed;
/*0048*/	float	side_speed;
/*0052*/	float	vert_speed;
/*0056*/	float	orig_x;
/*0060*/	float	orig_y;
/*0064*/	float	orig_z;
/*0068*/	float	orig_x2;
/*0072*/	float	orig_y2;
/*0076*/	float	orig_z2;
/*0080*/	float	unknown5[3];
/*0092*/	int32	unknown6;
/*0096*/	float	unknown7[3];
/*0108*/	int32	unknown8;
/*0112*/	int32	grid_location;
/*0116*/	float	x;
/*0120*/	float	y;
/*0124*/	float	z;
/*0128*/	float	direction2;
/*0132*/	float	pitch;
/*0136*/	float	unknown10;
/*0140*/	float	speed_x;
/*0144*/	float	speed_y;
/*0148*/	float	speed_z;
};
struct Player_Update283 {
	/*0000*/	int32	activity;
	/*0004*/	int32	movement_mode; // 1
	/*0008*/	float	direction1;
	/*0012*/	float	desiredpitch;
	/*0016*/	float	desired_heading_speed;
	/*0020*/	float	desired_pitch_speed;
	/*0024*/	float	collision_radius;
	/*0028*/	float	collision_scale;
	/*0032*/	float	temp_scale;
	/*0036*/	float	speed_modifier;
	/*0040*/	float	swim_speed_modifier;
	/*0044*/	float	speed;
	/*0048*/	float	side_speed;
	/*0052*/	float	vert_speed;
	/*0056*/	float	orig_x;
	/*0060*/	float	orig_y;
	/*0064*/	float	orig_z;
	/*0068*/	float	orig_x2;
	/*0072*/	float	orig_y2;
	/*0076*/	float	orig_z2;
	/*0080*/	int32	face_actor_id;
	/*0084*/	int32	face_actor_range;
	/*0088*/	int32	grid_location;
	/*0092*/	float	x;
	/*0096*/	float	y;
	/*0100*/	float	z;
	/*0104*/	float	direction2;
	/*0108*/	float	pitch;
	/*0112*/	float	roll;
	/*0116*/	float	speed_x;
	/*0120*/	float	speed_y;
	/*0124*/	float	speed_z;
};//0128
struct Player_Update1096{
/*0000*/	int32	activity;
/*0004*/	float	unknown2; // 1
/*0008*/	float	direction1;	
/*0012*/	float	unknown3[8];
/*0044*/	float	unk_speed;
/*0048*/	float	speed;
/*0052*/	float	side_speed;
/*0056*/	float	vert_speed;
/*0060*/	float	orig_x;
/*0064*/	float	orig_y;
/*0068*/	float	orig_z;
/*0072*/	float	orig_x2;
/*0076*/	float	orig_y2;
/*0080*/	float	orig_z2;
/*0092*/	float	unknown5[3];
/*0096*/	int32	unknown6;
/*0108*/	float	unknown7[3];
/*0112*/	int32	unknown8;
/*0116*/	int32	grid_location;
/*0120*/	float	x;
/*0124*/	float	y;
/*0128*/	float	z;
/*0132*/	float	direction2;
/*0136*/	float	pitch;
/*0140*/	float	unknown10;
/*0144*/	float	speed_x;
/*0148*/	float	speed_y;
/*0152*/	float	speed_z;
};

struct Player_Update1144{
/*0000*/    int32    activity;
/*0004*/    float    unknown2; // 1
/*0008*/    float    direction1;    
/*0012*/    float    unknown3[12];
/*0044*/    float    unk_speed;
/*0048*/    float    speed;
/*0052*/    float    side_speed;
/*0056*/    float    vert_speed;
/*0060*/    float    orig_x;
/*0064*/    float    orig_y;
/*0068*/    float    orig_z;
/*0072*/    float    orig_x2;
/*0076*/    float    orig_y2;
/*0080*/    float    orig_z2;
/*0092*/    float    unknown5[3];
/*0096*/    int32    unknown6;
/*0108*/    float    unknown7[3];
/*0112*/    int32    unknown8;
/*0116*/    int32    grid_location;
/*0120*/    float    x;
/*0124*/    float    y;
/*0128*/    float    z;
/*0132*/    float    direction2;
/*0136*/    float    pitch;
/*0140*/    float    unknown10;
/*0144*/    float    speed_x;
/*0148*/    float    speed_y;
/*0152*/    float    speed_z;
};
#pragma pack()
#endif

