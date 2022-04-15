
#include <fstream>

#include "replace_manager.h"
#include "common_tools.h"


void replace_manager::init(uint32_t seed)
{
	if (isInited)
	{
		return;
	}
	isInited = true;
	template_manager::get_instance().init();
	drop_map.clear();
	temp_replace_map_map.clear();
	temp_replace_map_enemy.clear();

	can_rand_ash_ary.clear();
	can_rand_ash_ary.push_back(e_ash_type_1);
	can_rand_ash_ary.push_back(e_ash_type_2);
	can_rand_ash_ary.push_back(e_ash_type_3);

	common_tools::get_instance().set_seed(seed);

	{
		//要按顺序
		init_sub_fill_drop_map();

		//先清空自己，都放到buffer里等待回填
		init_sub_pretreat_send_to_fill_buffer();

		init_sub_pretreat_max_num();
		init_sub_pretreat_special_logic();
		init_sub_pretreat_split_func();

		//所有逻辑执行后，从Buffer里取值回填
		init_sub_pretreat_re_fill_from_buffer();

		init_sub_fill_replace_map();
	}

	wprintf_s(L"init success\n");
}

void replace_manager::init_sub_fill_drop_map()
{
	drop_map.clear();

	template_manager::template_type* temp_template_all = template_manager::get_instance().get_templates("DropTemplate");
	if (temp_template_all == nullptr)
	{
		return;
	}

	for (auto ite = temp_template_all->begin(); ite != temp_template_all->end(); ++ite)
	{
		DropTemplate* temp_template_ptr = (DropTemplate*)(ite->second);
		if (temp_template_ptr == nullptr)
		{
			continue;
		}

		drop_unit* temp_unit = get_drop_unit(temp_template_ptr->GroupName);
		if (temp_unit == nullptr)
		{
			continue;
		}
		temp_unit->add_one_drop(temp_template_ptr);
	}
}

void replace_manager::init_sub_pretreat_special_logic()
{
	drop_unit* temp_unit = get_drop_unit("Ash1");
	if (temp_unit != nullptr)
	{
		temp_unit->do_pretreat_sub_ash1();
	}

	temp_unit = get_drop_unit("Ash2");
	if (temp_unit != nullptr)
	{
		temp_unit->do_pretreat_sub_ash_other(2);
	}

	temp_unit = get_drop_unit("Ash3");
	if (temp_unit != nullptr)
	{
		temp_unit->do_pretreat_sub_ash_other(3);
	}

	temp_unit = get_drop_unit("Magic");
	if (temp_unit != nullptr)
	{
		temp_unit->do_pretreat_sub_magic();
	}

}

void replace_manager::init_sub_pretreat_send_to_fill_buffer()
{
	if (drop_map.size() <= 0)
	{
		return;
	}
	INT32 log_count = 0;
	auto iter = drop_map.begin();
	for (; iter != drop_map.end(); iter++)
	{
		drop_unit* temp_unit = iter->second;
		if (temp_unit == nullptr
			|| temp_unit->get_rule_template_ptr() == nullptr)
		{
			continue;
		}
		std::string temp_method = temp_unit->get_rule_template_ptr()->PretreatMethod;
		if (temp_method != "FillAtLastFunc")
		{
			continue;
		}
		temp_unit->do_send_to_fill_buffer(); 

		log_count++;
	}
#if REPLACE_DEBUG
	wprintf_s(L"Send To Buffer success, process count is %d \n\n\n\n\n\n", log_count);
#endif

}

void replace_manager::init_sub_pretreat_re_fill_from_buffer()
{
	if (drop_map.size() <= 0)
	{
		return;
	}
	INT32 log_count = 0;
	auto iter = drop_map.begin();
	for (; iter != drop_map.end(); iter++)
	{
		drop_unit* temp_unit = iter->second;
		if (temp_unit == nullptr
			|| temp_unit->get_rule_template_ptr() == nullptr)
		{
			continue;
		}
		std::string temp_method = temp_unit->get_rule_template_ptr()->PretreatMethod;
		if (temp_method != "FillAtLastFunc")
		{
			continue;
		}
		temp_unit->do_refill_from_buffer();

		log_count++;
	}
#if REPLACE_DEBUG
	wprintf_s(L"Refill from buffer success, process count is %d \n\n\n\n\n\n", log_count);
#endif
}

void replace_manager::init_sub_pretreat_max_num()
{
	if (drop_map.size() <= 0)
	{
		return;
	}

	INT32 log_count = 0;
	auto iter = drop_map.begin();
	for (; iter != drop_map.end(); iter++)
	{
		drop_unit* temp_unit = iter->second;
		if (temp_unit == nullptr
			|| temp_unit->get_rule_template_ptr() == nullptr
			|| temp_unit->get_rule_template_ptr()->RandomParam <= 0)
		{
			continue;
		}

		INT32 temp_old_count = temp_unit->get_count();

		temp_unit->set_max_num(temp_unit->get_rule_template_ptr()->RandomParam);
		temp_unit->set_ignore_weight(true);

		INT32 temp_new_count = temp_unit->get_count();
		log_count++;

#if REPLACE_DEBUG
		std::string temp_name = temp_unit->get_rule_template_ptr()->GroupName;
		std::wstring temp_name_w = std::wstring(temp_name.begin(), temp_name.end());
		wprintf_s(L"Set %s num from %d to %d\n", temp_name_w.c_str(), temp_old_count, temp_new_count);
#endif
	}

#if REPLACE_DEBUG
	wprintf_s(L"Set max count success, process num is %d \n\n\n\n\n\n", log_count);
#endif
}

void replace_manager::init_sub_pretreat_split_func()
{
	if (drop_map.size() <= 0)
	{
		return;
	}
	INT32 log_count = 0;
	auto iter = drop_map.begin();
	for (; iter != drop_map.end(); iter++)
	{
		drop_unit* temp_unit = iter->second;
		if (temp_unit == nullptr
			|| temp_unit->get_rule_template_ptr() == nullptr)
		{
			continue;
		}
		std::string temp_method = temp_unit->get_rule_template_ptr()->PretreatMethod;
		if (temp_method != "SplitFunc")
		{
			continue;
		}

		INT32 max_num = std::atoi(temp_unit->get_rule_template_ptr()->PretreatParam1);
		std::vector<DropTemplate*> temp_erased_vec = temp_unit->set_max_num(max_num);
		drop_unit* temp_recycle_ptr = get_drop_unit(temp_unit->get_rule_template_ptr()->PretreatParam2, false);
		if (temp_recycle_ptr != nullptr)
		{
			for (INT32 j = 0; j < temp_erased_vec.size(); ++j)
			{
				temp_recycle_ptr->add_one_drop(temp_erased_vec[j]);
			}
		}

#if REPLACE_DEBUG
		std::string temp_self_name = temp_unit->get_rule_template_ptr()->GroupName;
		std::wstring temp_self_name_w = std::wstring(temp_self_name.begin(), temp_self_name.end());

		std::string temp_target_name = temp_unit->get_rule_template_ptr()->PretreatParam2;
		std::wstring temp_target_name_w = std::wstring(temp_target_name.begin(), temp_target_name.end());
		wprintf_s(L"Splite group %ws, %d to self, %d to %ws \n\n\n\n\n\n", temp_self_name_w.c_str(), max_num, temp_erased_vec.size(), temp_target_name_w.c_str());
#endif

		log_count++;
	}
#if REPLACE_DEBUG
	wprintf_s(L"Splite Func success, process count is %d \n\n\n\n\n\n", log_count);
#endif
}

void replace_manager::init_sub_fill_replace_map()
{
	temp_replace_map_map.clear();
	temp_replace_map_enemy.clear();
	template_manager::template_type* temp_template_all = template_manager::get_instance().get_templates("ReplaceTemplate");
	if (temp_template_all == nullptr)
	{
		return;
	}

	for (auto ite = temp_template_all->begin(); ite != temp_template_all->end(); ++ite)
	{
		ReplaceTemplate* temp_template_ptr = (ReplaceTemplate*)(ite->second);
		if (temp_template_ptr == nullptr)
		{
			continue;
		}
		replace_data temp_data = convert_to_replace_data(temp_template_ptr);
		std::string temp_type_str = temp_template_ptr->DropType;
		if (temp_type_str == "map")
		{
			temp_replace_map_map.insert({ temp_template_ptr->OriginDropID, temp_data } );
		}
		else if (temp_type_str == "enemy")
		{
			temp_replace_map_enemy.insert({ temp_template_ptr->OriginDropID, temp_data });
		}
	}

	wprintf_s(L"replace map success, map count %d, enemy count %d\n\n\n\n\n\n", temp_replace_map_map.size(), temp_replace_map_enemy.size());
}

replace_data* replace_manager::get_replace_data_map(INT32 inID)
{
	auto iter = temp_replace_map_map.find(inID);
	if (iter == temp_replace_map_map.end())
	{
		return nullptr;
	}

	return &iter->second;
}

replace_data* replace_manager::get_replace_data_enemy(INT32 inID)
{
	auto iter = temp_replace_map_enemy.find(inID);
	if (iter == temp_replace_map_enemy.end())
	{
		return nullptr;
	}

	return &iter->second;
}

replace_data replace_manager::convert_to_replace_data(ReplaceTemplate* inTemplate)
{
	replace_data return_value;
	if (inTemplate == nullptr)
	{
		return return_value;
	}


	INT32 id_list[8];
	memset(id_list, 0, sizeof(id_list));
	INT32 type_list[8];
	memset(type_list, 0, sizeof(type_list));
	INT32 amount_list[8];
	memset(amount_list, 0, sizeof(amount_list));

	get_item_id(inTemplate->ItemDropName1, id_list[0], type_list[0], amount_list[0]);
	get_item_id(inTemplate->ItemDropName2, id_list[1], type_list[1], amount_list[1]);
	get_item_id(inTemplate->ItemDropName3, id_list[2], type_list[2], amount_list[2]);
	get_item_id(inTemplate->ItemDropName4, id_list[3], type_list[3], amount_list[3]);
	get_item_id(inTemplate->ItemDropName5, id_list[4], type_list[4], amount_list[4]);
	get_item_id(inTemplate->ItemDropName6, id_list[5], type_list[5], amount_list[5]);
	get_item_id(inTemplate->ItemDropName7, id_list[6], type_list[6], amount_list[6]);
	get_item_id(inTemplate->ItemDropName8, id_list[7], type_list[7], amount_list[7]);

	for (INT32 i = 0 ; i < 8; ++i)
	{
		return_value.item_id_array[i] = id_list[i];
		return_value.item_type_array[i] = (MapItemType)type_list[i];
	}

	return_value.item_chance_array[0] = inTemplate->Chance1;
	return_value.item_chance_array[1] = inTemplate->Chance2;
	return_value.item_chance_array[2] = inTemplate->Chance3;
	return_value.item_chance_array[3] = inTemplate->Chance4;
	return_value.item_chance_array[4] = inTemplate->Chance5;
	return_value.item_chance_array[5] = inTemplate->Chance6;
	return_value.item_chance_array[6] = inTemplate->Chance7;
	return_value.item_chance_array[7] = inTemplate->Chance8;

	return_value.item_amount[0] = return_value.item_id_array[0] > 0 ? (inTemplate->Amount1 > 0 ? inTemplate->Amount1 : amount_list[0]) : 0;
	return_value.item_amount[1] = return_value.item_id_array[1] > 0 ? (inTemplate->Amount2 > 0 ? inTemplate->Amount2 : amount_list[1]) : 0;
	return_value.item_amount[2] = return_value.item_id_array[2] > 0 ? (inTemplate->Amount3 > 0 ? inTemplate->Amount3 : amount_list[2]) : 0;
	return_value.item_amount[3] = return_value.item_id_array[3] > 0 ? (inTemplate->Amount4 > 0 ? inTemplate->Amount4 : amount_list[3]) : 0;
	return_value.item_amount[4] = return_value.item_id_array[4] > 0 ? (inTemplate->Amount5 > 0 ? inTemplate->Amount5 : amount_list[4]) : 0;
	return_value.item_amount[5] = return_value.item_id_array[5] > 0 ? (inTemplate->Amount6 > 0 ? inTemplate->Amount6 : amount_list[5]) : 0;
	return_value.item_amount[6] = return_value.item_id_array[6] > 0 ? (inTemplate->Amount7 > 0 ? inTemplate->Amount7 : amount_list[6]) : 0;
	return_value.item_amount[7] = return_value.item_id_array[7] > 0 ? (inTemplate->Amount8 > 0 ? inTemplate->Amount8 : amount_list[7]) : 0;

	return return_value;
}

bool replace_manager::get_item_id(std::string inRandName, INT32& outItemId, INT32& outItemType, INT32& outItemAmount)
{
	outItemId = 0;
	outItemType = 0;
	outItemAmount = 1;
	drop_unit* temp_unit_ptr = get_drop_unit(inRandName);
	if (temp_unit_ptr == nullptr)
	{
		return false;
	}

	INT32 trigger_index = -1;
	DropTemplate* temp_ptr = temp_unit_ptr->pop_one(trigger_index);
	if (temp_ptr == nullptr)
	{
		return false;
	}

	outItemId = std::atoi(temp_ptr->RandomResult);
	if (inRandName == "Ash1"
		|| inRandName == "Ash2"
		|| inRandName == "Ash3")
	{
		if (temp_unit_ptr->get_rule_template_ptr() != nullptr)
		{
			INT32 ash_level = std::atoi(temp_unit_ptr->get_rule_template_ptr()->PretreatParam2);
			if (ash_level > 0
				&& ash_level <= 10)
			{
				outItemId += ash_level;
			}
		}
	}

	outItemType = temp_ptr->ItemType;

	if (temp_ptr->ParamEx1 > 0
		|| temp_ptr->ParamEx2 > 0)
	{
		outItemAmount = get_random(temp_ptr->ParamEx1, temp_ptr->ParamEx2);
	}
	else
	{
		outItemAmount = 1;
	}

	return true;
}

drop_unit* replace_manager::get_drop_unit(std::string inGroupName, bool inAutoCreate)
{
	auto iter = drop_map.find(inGroupName);
	if (iter == drop_map.end())
	{
		if (!inAutoCreate)
		{
			return nullptr;
		}
		drop_unit* new_ptr = new drop_unit();
		new_ptr->init(inGroupName);
		drop_map.insert({ inGroupName ,new_ptr });
	}
	return drop_map[inGroupName];
}

INT32 replace_manager::get_weapon_rand_prop(const ItemInfo* in_item_info, INT32 in_weapon_level_max, bool in_is_special_weapon)
{
	//算出变质+升级后的新物品ID

	INT32 return_value = 0;
	if (in_item_info == nullptr)
	{
		return return_value;
	}

	INT32 temp_origin_item_id = get_origin_item_id(in_item_info->item_id);

	INT32 temp_rand_special_prop = weapon_rand_sub_special_prop(temp_origin_item_id);
	INT32 temp_rand_level = weapon_rand_sub_level(temp_origin_item_id, in_weapon_level_max, in_is_special_weapon);

#if REPLACE_DEBUG
	wprintf_s(L"item prop rand success, origin id %d, special prop %d, temp_rand_level %d\n", temp_origin_item_id, temp_rand_special_prop, temp_rand_level);
#endif

	return temp_origin_item_id + temp_rand_special_prop + temp_rand_level;
}

INT32 replace_manager::weapon_rand_sub_special_prop(INT32 in_origin_item_id)
{
	ItemPropRandTemplate* temp_rand_ptr = template_manager::get_instance().get_template<ItemPropRandTemplate>("ItemPropRandTemplate", in_origin_item_id);
	if (temp_rand_ptr == nullptr)
	{
		return 0;
	}

	drop_unit* temp_drop_unit_ptr = get_drop_unit(temp_rand_ptr->SpecialPropGroupName);
	if (temp_drop_unit_ptr == nullptr)
	{
		return 0;
	}

	INT32 trigger_index = -1;
	DropTemplate* temp_drop_ptr = temp_drop_unit_ptr->pop_one(trigger_index);
	if (temp_drop_ptr == nullptr)
	{
		return 0;
	}

	INT32 special_prop = std::atoi(temp_drop_ptr->RandomResult);
	return special_prop * 100;
}

INT32 replace_manager::weapon_rand_sub_level(INT32 in_origin_item_id, INT32 in_weapon_level_max, bool in_is_special_weapon)
{
	ItemPropRandTemplate* temp_rand_ptr = template_manager::get_instance().get_template<ItemPropRandTemplate>("ItemPropRandTemplate", in_origin_item_id);
	if (temp_rand_ptr == nullptr)
	{
		return 0;
	}

	switch (temp_rand_ptr->LevelRandType)
	{
	case e_weapon_level_rand_type_in_template:
	{
		INT32 temp_rand_value = 0;
		temp_rand_value = get_random(temp_rand_ptr->LevelRandValueMin, temp_rand_ptr->LevelRandValueMax);
		return temp_rand_value;
	}
	break;
	case e_weapon_level_rand_type_rand:
	{
		int32 temp_total_max_level = 25;
		if (in_is_special_weapon)
		{
			temp_total_max_level = 10;
			std::array<uint8_t, 27> temp_convert = { 0, 0, 1, 1, 1, 2, 2, 3, 3, 3, 3, 4, 4, 5, 5, 5, 6, 6, 7, 7, 7, 8, 8, 9, 9, 9, 10 };
			if (in_weapon_level_max > 0
				&& in_weapon_level_max < temp_convert.size())
			{
				in_weapon_level_max = temp_convert[in_weapon_level_max];
			}
			else
			{
				in_weapon_level_max = 10;
			}
		}

		int32 temp_real_min = in_weapon_level_max + temp_rand_ptr->LevelRandValueMin;
		int32 temp_real_max = in_weapon_level_max + temp_rand_ptr->LevelRandValueMax;

		if (temp_real_min < 0)
		{
			temp_real_min = 0;
		}
		if (temp_real_min > temp_total_max_level)
		{
			temp_real_min = temp_total_max_level;
		}
		if (temp_real_max < 0)
		{
			temp_real_max = 0;
		}
		if (temp_real_max > temp_total_max_level)
		{
			temp_real_max = temp_total_max_level;
		}

		INT32 temp_rand_value = 0;
		temp_rand_value = get_random(temp_real_min, temp_real_max);

#if REPLACE_DEBUG
		wprintf_s(L"item level rand success, origin id %d, weapon_level_max is %d, min %d, max %d\n", in_origin_item_id, in_weapon_level_max, temp_real_min, temp_real_max);
#endif

		return temp_rand_value;
	}
	break;
	default:
	{
		return 0;
	}
	break;
	}
	return 0;
}

INT32 replace_manager::get_weapon_slot(const EquipParamWeapon* in_weapon_info)
{
	if (in_weapon_info == nullptr)
	{
		return -1;
	}

	uint32_t weapon_slot_equip = __UINT32_MAX__;
	switch (in_weapon_info->weapon_type)
	{
	case(weapon_type_lightbow):
	case(weapon_type_bow):
	case(weapon_type_greatbow):
	case(weapon_type_crossbow):
	case(weapon_type_ballista):
	case(weapon_type_glintstone_staff):
	case(weapon_type_sacred_seal):
	{
		weapon_slot_equip = 1;
		break;
	};
	case(weapon_type_small_shield):
	case(weapon_type_med_shield):
	case(weapon_type_great_shield):
	{
		weapon_slot_equip = 0;
		break;
	};
	case(weapon_type_arrow):
	case(weapon_type_greatarrow):
	{
		weapon_slot_equip = 6;
		break;
	};
	case(weapon_type_bolt):
	case(weapon_type_greatbolt):
	{
		weapon_slot_equip = 7;
		break;
	};

	default:
	{
		weapon_slot_equip = 1;
		break;
	};
	}
	return weapon_slot_equip;
}

INT32 replace_manager::get_weapon_rand_ash(const ItemInfo* in_item_info)
{
	if (in_item_info == nullptr)
	{
		return 0;
	}

	INT32 temp_origin_item_id = get_origin_item_id(in_item_info->item_id);
	ItemPropRandTemplate* temp_rand_ptr = template_manager::get_instance().get_template<ItemPropRandTemplate>("ItemPropRandTemplate", temp_origin_item_id);
	if (temp_rand_ptr == nullptr)
	{
		return in_item_info->item_ashes_of_war;
	}

	drop_unit* temp_unit_ptr = get_drop_unit(temp_rand_ptr->AshOfWarGroupName);
	if (temp_unit_ptr == nullptr)
	{
		return in_item_info->item_ashes_of_war;
	}

	INT32 trigger_index = -1;
	DropTemplate* temp_template_ptr = temp_unit_ptr->pop_one(trigger_index);
	if (temp_template_ptr == nullptr)
	{
		return in_item_info->item_ashes_of_war;
	}

	INT32 temp_ash_id = std::atoi(temp_template_ptr->RandomResult);
#if REPLACE_DEBUG
	wprintf_s(L"item ash rand success %d\n", temp_ash_id);
#endif
	return 2147483648 | temp_ash_id;
}

INT32 replace_manager::get_weapon_rand_quantity(const ItemInfo* in_item_info)
{
	if (in_item_info == nullptr)
	{
		return 0;
	}

	INT32 temp_origin_item_id = get_origin_item_id(in_item_info->item_id);
	ItemPropRandTemplate* temp_rand_ptr = template_manager::get_instance().get_template<ItemPropRandTemplate>("ItemPropRandTemplate", temp_origin_item_id);
	if (temp_rand_ptr == nullptr)
	{
		return in_item_info->item_quantity;
	}

	if (temp_rand_ptr->ItemAmountMin == 0
		&& temp_rand_ptr->ItemAmountMin == 0)
	{
		return in_item_info->item_quantity;
	}

	INT32 temp_rand_num = 0;
	temp_rand_num = get_random(temp_rand_ptr->ItemAmountMin, temp_rand_ptr->ItemAmountMax);
	return temp_rand_num;
}

INT32 replace_manager::get_origin_item_id(INT32 in_item_id)
{
	return in_item_id - in_item_id % 10000;
}


INT32 replace_manager::get_random(INT32 in_min, INT32 in_max)
{
	if (in_min > in_max)
	{
		INT32 real_min = in_max;

		in_max = in_min;
		in_min = real_min;
	}
	if (in_max < 0)
	{
		return std::rand() % (in_max - 1 - in_min) + in_min;
	}
	else
	{
		return std::rand() % (in_max + 1 - in_min) + in_min;
	}
}