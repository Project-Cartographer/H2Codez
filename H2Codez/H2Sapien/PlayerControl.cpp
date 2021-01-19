#include "PlayerControl.h"
#include "util/Patches.h"
#include "stdafx.h"
#include "Common/TagInterface.h"


#pragma pack(push, 1)
//stores target data etc
struct s_aim_assist_targeting_result
{
	datum target_object;
	datum model_target;
	datum target_player;
	char gap_C[12];
	signed __int16 auto_aim_flags;
	char gap_1A[2];
	float primary_auto_aim_level;
	float secondary_auto_aim_level;

	void clear()
	{
		primary_auto_aim_level = 0;
		secondary_auto_aim_level = 0;
		target_object = 0xFFFFFFFF;
		model_target = 0xFFFFFFFF;
		target_player = 0xFFFFFFFF;
		auto_aim_flags = 0;
	}
};
CHECK_STRUCT_SIZE(s_aim_assist_targeting_result, 0x24);

struct s_player_actions
{
	int control_flag0;
	int control_flag1;
	real_euler_angles2d facing;
	float throttle_x;
	float throttle_y;
	float trigger;
	float secondary_trigger;
	DWORD action_flags;
	WORD weapon_set_identifier;
	BYTE primary_weapon_index;
	BYTE secondary_weapon_index;
	WORD grenade_index;
	WORD zoom_level;
	int interaction_type;
	int interaction_object;
	int melee_target_unit;
	s_aim_assist_targeting_result target_info;
	int unk;
};
CHECK_STRUCT_SIZE(s_player_actions, 0x60);


struct s_player_control
{
	datum slave_object;
	int control_flag;
	int field_8;
	int field_C;
	int field_10;
	int field_14;
	s_player_actions actions;
	char gap_78[4];
	DWORD action_context;
	char gap_80[31];
	char field_9F;
	char gap_A0[12];
	int field_AC;
};
CHECK_STRUCT_SIZE(s_player_control, 0xB0);


struct s_player_control_globals_data
{
	int field_0;
	int field_4;
	int field_8;
	int field_C;
	int field_10;
	int field_14;
	s_player_control local_players[4];
};
CHECK_STRUCT_SIZE(s_player_control_globals_data, 0x2D8);

struct s_unit_control_data
{
	string_id animation_state;
	unsigned __int16 aiming_speed;
	unsigned __int16 weapon_set_identifier;
	unsigned __int8 field_8;
	unsigned __int8 field_9;
	WORD grenade_index;
	unsigned __int16 zoom_level;
	char gap_E[2];
	DWORD control_flag0;
	DWORD control_flag1;
	real_vector3d throttle;
	float trigger;
	float secondary_trigger;
	real_vector3d desired_facing;
	real_vector3d desired_aiming;
	real_vector3d desired_looking;
	DWORD field_50;
	DWORD field_54;
	DWORD field_58;
	s_aim_assist_targeting_result target_info;

	void clear()
	{
		memset(this, 0, sizeof(s_unit_control_data));
		animation_state = 0x6000086;
		aiming_speed = 0;
		weapon_set_identifier = 0xFFFF;
		field_8 = 0xFF;
		field_9 = 0xFF;
		grenade_index = 0xFFFF;
		zoom_level = 0xFFFF;

		real_vector3d* global_forward3d;
		global_forward3d = (real_vector3d*)(*(DWORD*)(0x992A98));

		desired_facing = *global_forward3d;
		desired_aiming = *global_forward3d;
		desired_looking = *global_forward3d;

		target_info.clear();
	}
};
CHECK_STRUCT_SIZE(s_unit_control_data, 0x80);

#pragma pack(pop)



static bool input_enabled = false;
s_player_control_globals_data* player_controls;

void __cdecl player_submit_control(unsigned __int16 datum_index, s_player_actions *action)
{


	typedef void(__cdecl *unit_control)(WORD object_idx, s_unit_control_data* data);
	auto unit_control_impl = reinterpret_cast<unit_control>(0x6568D0);

	typedef void(__cdecl *player_control_get_facing_direction)(BYTE UserIndex, real_vector3d* out_facing_3d);
	auto player_control_get_facing_direction_impl = reinterpret_cast<player_control_get_facing_direction>(0x56CFC0);

	s_unit_control_data control;
	real_vector3d aiming_vector;

	player_control_get_facing_direction_impl(0, &aiming_vector);
	control.clear();

	control.control_flag1 = action->control_flag1;
	control.control_flag0 = player_controls->local_players[0].control_flag;
	control.desired_aiming = aiming_vector;
	control.desired_looking = aiming_vector;

	//player_build_facing_vector_from_throttle is responsible for proving facing value..for now aiming vector works well
	control.desired_facing = aiming_vector; //FIX ME	

	control.throttle.i = action->throttle_x;
	control.throttle.j = action->throttle_y;
	control.throttle.k = 0.0f;

	control.zoom_level = action->zoom_level;
	control.trigger = action->trigger;
	control.secondary_trigger = action->secondary_trigger;
	control.grenade_index = action->grenade_index;
	control.weapon_set_identifier = action->weapon_set_identifier;

	control.animation_state = 0x6000086; //stringid_COMBAT
	control.aiming_speed = 0;

	memcpy(&control.target_info, &action->target_info, sizeof(s_aim_assist_targeting_result));
	//flag test offset 0x138
	unit_control_impl(datum_index, &control);

}



void player_update()
{

	typedef bool(__cdecl *player_input_enabled)();
	auto player_input_enabled_impl = reinterpret_cast<player_input_enabled>(0x4F2300);


	if (!player_input_enabled_impl())return;

	player_controls = (s_player_control_globals_data*)(*(DWORD*)(0xB1FB78));
	auto player_control_entry = player_controls->local_players[0];//hardcoded for local player 0 

	if (player_control_entry.slave_object.is_valid())
		player_submit_control(player_control_entry.slave_object.index, &player_control_entry.actions);


}

//use this for updating any thing that requires to be executed per tick
//basically for nullstubs or etc
//should be placed at a more globally accesible area ig
void game_tick_update()
{
	//need to do all the update stuffs here

	player_update();
}

void unit_playtest_patches()
{

	//NopFill(0x6569CD, 5);
	//replace the null sub with ours
	PatchCall(0x501057, (void*)game_tick_update);

}

