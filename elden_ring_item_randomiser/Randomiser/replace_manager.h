#pragma once

#include "../Template/template_manager.h"
#include "../RandomiserProperties/randomiser_properties.h"
#include "drop_unit.h"

#define REPLACE_DEBUG 0

class replace_manager
{
	bool isInited = false;

	std::map<uint32_t, replace_data> temp_replace_map_map;
	std::map<uint32_t, replace_data> temp_replace_map_enemy;

	std::map<std::string, drop_unit*> drop_map;

public:
	static replace_manager& get_instance()
	{
		static replace_manager instance;
		return instance;
	}

	std::vector<INT32> can_rand_ash_ary;
public:
	void init(uint32_t seed);
	void init_sub_fill_drop_map();
	void init_sub_pretreat_special_logic();
	void init_sub_pretreat_send_to_fill_buffer();
	void init_sub_pretreat_re_fill_from_buffer();
	void init_sub_pretreat_max_num();
	void init_sub_pretreat_split_func();

	void init_sub_fill_replace_map();

	replace_data* get_replace_data_map(INT32 inID);
	replace_data* get_replace_data_enemy(INT32 inID);

	replace_data convert_to_replace_data(ReplaceTemplate* inTemplate);

	bool get_item_id(std::string inRandName, INT32& outItemId, INT32& outItemType, INT32& outItemAmount);
	drop_unit* get_drop_unit(std::string inGroupName, bool inAutoCreate = true);
	INT32 get_weapon_rand_prop(const ItemInfo* in_item_info, INT32 in_weapon_level_max, bool in_is_special_weapon);
	INT32 weapon_rand_sub_special_prop(INT32 in_origin_item_id);
	INT32 weapon_rand_sub_level(INT32 in_origin_item_id, INT32 in_weapon_level_max, bool in_is_special_weapon);
	INT32 get_weapon_slot(const EquipParamWeapon* in_weapon_info);
	INT32 get_weapon_rand_ash(const ItemInfo* in_item_info);
	INT32 get_weapon_rand_quantity(const ItemInfo* in_item_info);
	INT32 get_origin_item_id(INT32 in_item_id);

	INT32 get_random(INT32 in_min, INT32 in_max);
};


