
#include <fstream>

#include "replace_manager.h"
#include "common_tools.h"
#include  <Locale.h>
#include <algorithm>


drop_unit::drop_unit()
{
	clear_data();
}

drop_unit::~drop_unit()
{
	clear_data();
}

void drop_unit::clear_data()
{
	template_ptr_ary.clear();
	max_weight = 0;
	rule_ptr = nullptr;
	is_ignore_weight = false;
	cur_group_name = "";
}

void drop_unit::init(std::string inGroupName)
{
	cur_group_name = inGroupName;
	rule_ptr = template_manager::get_instance().get_drop_rule_template_by_group_name(cur_group_name);
}

void drop_unit::add_one_drop(DropTemplate* inNewTemplatePtr)
{
	if (inNewTemplatePtr == nullptr
		|| inNewTemplatePtr->Weight <= 0)
	{
		return;
	}
	template_ptr_ary.push_back(inNewTemplatePtr);
	max_weight += inNewTemplatePtr->Weight;
}

void drop_unit::remove_one_drop(DropTemplate* inNewTemplatePtr)
{
	std::vector<DropTemplate*>::iterator temp_it = template_ptr_ary.begin();
	for (; temp_it != template_ptr_ary.end(); temp_it++)
	{
		if (*temp_it == inNewTemplatePtr)
		{
			template_ptr_ary.erase(temp_it);
			max_weight -= inNewTemplatePtr->Weight;
			if (max_weight < 0)
			{
				max_weight = 0;
			}
			break;
		}
	}
}

void drop_unit::do_pretreat_sub_ash1()
{
	//三种初级骨灰加一起随机一个出来

	if (rule_ptr == nullptr)
	{
		return;
	}

	drop_unit* temp_1 = replace_manager::get_instance().get_drop_unit("Ash1_1");
	drop_unit* temp_2 = replace_manager::get_instance().get_drop_unit("Ash2_1");
	drop_unit* temp_3 = replace_manager::get_instance().get_drop_unit("Ash3_1");
	if (temp_1 == nullptr
		|| temp_2 == nullptr
		|| temp_2 == nullptr)
	{
		return;
	}

	drop_unit* temp_merged_unit = new drop_unit();

	temp_merged_unit->init("NormalNoBack");
	temp_merged_unit->do_merge(temp_1);
	temp_merged_unit->do_merge(temp_2);
	temp_merged_unit->do_merge(temp_3);

	INT32 trigger_index = -1;
	DropTemplate* temp_ash1 = temp_merged_unit->pop_one(trigger_index);

	if (temp_ash1 != nullptr)
	{
		add_one_drop(temp_ash1);

		std::vector<INT32>& temp_ash_ary = replace_manager::get_instance().can_rand_ash_ary;
		auto iter = temp_ash_ary.begin();
		for (; iter != temp_ash_ary.end(); iter++)
		{
			if (*iter == temp_ash1->ParamEx1)
			{
				temp_ash_ary.erase(iter);
				break;
			}

	}

#if REPLACE_DEBUG
		wprintf_s(L"ash 1 pretreat success id = %d\n", temp_ash1->attribute_id);
#endif
	}
	else
	{
#if REPLACE_DEBUG
		wprintf_s(L"no ash 1 \n");
#endif
	}
}

void drop_unit::do_pretreat_sub_ash_other(INT32 inIndex)
{
	//replace_manager::get_instance().can_rand_ash_ary中剩下的骨灰加一起随机一个
	//并且删除随出的骨灰类型，下次不再随出此次类型

	drop_unit* temp_merged_unit = new drop_unit();
	temp_merged_unit->init("NormalNoBack");

	for (int32 i = 0; i < replace_manager::get_instance().can_rand_ash_ary.size(); ++i)
	{
		INT32 temp_remain_type = replace_manager::get_instance().can_rand_ash_ary[i];
		if (temp_remain_type == e_ash_type_1)
		{
			temp_merged_unit->do_merge(replace_manager::get_instance().get_drop_unit("Ash1_1"));
			temp_merged_unit->do_merge(replace_manager::get_instance().get_drop_unit("Ash1_2"));
			temp_merged_unit->do_merge(replace_manager::get_instance().get_drop_unit("Ash1_3"));
		}

		if (temp_remain_type == e_ash_type_2)
		{
			temp_merged_unit->do_merge(replace_manager::get_instance().get_drop_unit("Ash2_1"));
			temp_merged_unit->do_merge(replace_manager::get_instance().get_drop_unit("Ash2_2"));
			temp_merged_unit->do_merge(replace_manager::get_instance().get_drop_unit("Ash2_3"));
		}

		if (temp_remain_type == e_ash_type_3)
		{
			temp_merged_unit->do_merge(replace_manager::get_instance().get_drop_unit("Ash3_1"));
			temp_merged_unit->do_merge(replace_manager::get_instance().get_drop_unit("Ash3_2"));
			temp_merged_unit->do_merge(replace_manager::get_instance().get_drop_unit("Ash3_3"));
		}
	}

	INT32 trigger_index = -1;
	DropTemplate* temp_ash1 = temp_merged_unit->pop_one(trigger_index);

	if (temp_ash1 != nullptr)
	{
		add_one_drop(temp_ash1);

		std::vector<INT32>& temp_ash_ary = replace_manager::get_instance().can_rand_ash_ary;
		auto iter = temp_ash_ary.begin();
		for (; iter != temp_ash_ary.end(); iter++)
		{
			if (*iter == temp_ash1->ParamEx1)
			{
				temp_ash_ary.erase(iter);
				break;
			}

		}

#if REPLACE_DEBUG
		wprintf_s(L"ash %d pretreat success id = %d\n", inIndex, temp_ash1->attribute_id);
#endif
	}
	else
	{
#if REPLACE_DEBUG
		wprintf_s(L"no ash %d \n", inIndex);
#endif
	}

	delete temp_merged_unit;
}

void drop_unit::do_pretreat_sub_magic()
{
	//随机一系魔法+祷告，放入大列表A
	//从EX魔法、EX祷告中随机指定个数，放入大列表A
	//从大列表中随机指定个数，为本drop_unit的掉落
	//其他的扔给其他（BOSS）组

	if (rule_ptr == nullptr)
	{
		return;
	}

	drop_unit* temp_magic_system = replace_manager::get_instance().get_drop_unit("MagicSystem");
	drop_unit* temp_pray_system = replace_manager::get_instance().get_drop_unit("PraySystem");
	if (temp_magic_system == nullptr
		|| temp_pray_system == nullptr)
	{
		return;
	}

	//先随出魔法、祷告的系

	INT32 magic_trigger_index = -1;
	INT32 pray_trigger_index = -1;
	DropTemplate* temp_magic_system_template = temp_magic_system->pop_one(magic_trigger_index);
	DropTemplate* temp_pray_system_template = temp_pray_system->pop_one(pray_trigger_index);
	if (temp_magic_system_template == nullptr
		|| temp_pray_system_template == nullptr)
	{
		return;
	}

	std::string magic_system_name = temp_magic_system_template->RandomResult;
	std::string pray_system_name = temp_pray_system_template->RandomResult;

	//根据系中的配置找出具体的魔法、祷告列表
	drop_unit* temp_real_magic_List = replace_manager::get_instance().get_drop_unit(magic_system_name);
	drop_unit* temp_real_pray_List = replace_manager::get_instance().get_drop_unit(pray_system_name);
	if (temp_real_magic_List == nullptr
		|| temp_real_pray_List == nullptr)
	{
		return;
	}

	{
		//根据系中的配置找出对应的低级魔法、祷告列表，存储在真正的低级Group中
		drop_unit* temp_low_magic_List = replace_manager::get_instance().get_drop_unit(magic_system_name + "_Low");
		drop_unit* temp_low_pray_List = replace_manager::get_instance().get_drop_unit(pray_system_name + "_Low");
		drop_unit* temp_real_low_magic_List = replace_manager::get_instance().get_drop_unit("MagicLow");
		drop_unit* temp_real_low_pray_List = replace_manager::get_instance().get_drop_unit("PrayLow");
		if (temp_low_magic_List == nullptr
			|| temp_low_pray_List == nullptr
			|| temp_real_low_magic_List == nullptr
			|| temp_real_low_pray_List == nullptr)
		{
			return;
		}
		temp_real_low_magic_List->do_merge(temp_low_magic_List);
		temp_real_low_pray_List->do_merge(temp_low_pray_List);
	}


	drop_unit* temp_merged_ptr = new drop_unit();
	temp_merged_ptr->init("NormalNoBack");

	//魔法、祷告列表合体
	temp_merged_ptr->do_merge(temp_real_magic_List);
	temp_merged_ptr->do_merge(temp_real_pray_List);

	{
		//额外魔法、祷告合体
		drop_unit* temp_magic_ex = replace_manager::get_instance().get_drop_unit("MagicEX");
		if (temp_magic_ex != nullptr)
		{
			DropRuleTemplate* temp_rule_ptr = temp_magic_ex->get_rule_template_ptr();
			if (temp_rule_ptr != nullptr)
			{
				int32 ex_num = std::atoi(temp_rule_ptr->PretreatParam1);
				for (int32 i = 0; i < ex_num; ++i)
				{
					INT32 trigger_index = -1;
					DropTemplate* new_ex_ptr = temp_magic_ex->pop_one(trigger_index);
					if (new_ex_ptr != nullptr)
					{
						temp_merged_ptr->add_one_drop(new_ex_ptr);
					}
				}
			}
		}
		drop_unit* temp_pray_ex = replace_manager::get_instance().get_drop_unit("PrayEX");
		if (temp_pray_ex != nullptr)
		{
			DropRuleTemplate* temp_rule_ptr = temp_pray_ex->get_rule_template_ptr();
			if (temp_rule_ptr != nullptr)
			{
				int32 ex_num = std::atoi(temp_rule_ptr->PretreatParam1);
				for (int32 i = 0; i < ex_num; ++i)
				{
					INT32 trigger_index = -1;
					DropTemplate* new_ex_ptr = temp_pray_ex->pop_one(trigger_index);
					if (new_ex_ptr != nullptr)
					{
						temp_merged_ptr->add_one_drop(new_ex_ptr);
					}
				}
			}
		}

	}

	//大列表填充完毕，选择指定数量魔法存给自己
	INT32 temp_max_num = std::atoi(rule_ptr->PretreatParam1);
	for (INT32 i = 0; i < temp_max_num; ++i)
	{
		INT32 trigger_index = -1;
		DropTemplate* temp_one = temp_merged_ptr->pop_one(trigger_index);
		if (temp_one != nullptr)
		{
			add_one_drop(temp_one);
		}
	}

#if REPLACE_DEBUG
	for (int32 i = 0; i < template_ptr_ary.size(); ++i)
	{
		if (template_ptr_ary[i] != nullptr)
		{
			wprintf_s(L"This Magic Send to Pool, index %d Magic drop ID %d\n", i, template_ptr_ary[i]->attribute_id);
		}
	}
#endif

	//剩下的丢给其他
	drop_unit* temp_other_drop = replace_manager::get_instance().get_drop_unit(rule_ptr->PretreatParam2);
	if (temp_other_drop != nullptr)
	{
#if REPLACE_DEBUG
		std::vector<DropTemplate*> temp_log_ary = temp_merged_ptr->get_template_ary();
		for (int32 i = 0; i < temp_log_ary.size(); ++i)
		{
			if (temp_log_ary[i] != nullptr)
			{
				wprintf_s(L"This Magic Send to Boss, index %d, Magic drop ID %d\n", i, temp_log_ary[i]->attribute_id);
			}
		}
#endif
		temp_other_drop->do_merge(temp_merged_ptr);

		{
			//魔法系武器丢给BOSS
			DropTemplate* magic_weapon_ptr = template_manager::get_instance().get_template<DropTemplate>("DropTemplate", temp_magic_system_template->ParamEx1);
			if (magic_weapon_ptr != nullptr)
			{
				temp_other_drop->add_one_drop(magic_weapon_ptr);

				wprintf_s(L"This Magic Weapon Send to Boss, drop ID %d\n", magic_weapon_ptr->attribute_id);
			}
			DropTemplate* pray_weapon_ptr = template_manager::get_instance().get_template<DropTemplate>("DropTemplate", temp_pray_system_template->ParamEx1);
			if (pray_weapon_ptr != nullptr)
			{
				temp_other_drop->add_one_drop(pray_weapon_ptr);

				wprintf_s(L"This Pray Weapon Send to Boss, drop ID %d\n\n", pray_weapon_ptr->attribute_id);
			}
		}

#if REPLACE_DEBUG
		wprintf_s(L"magic pretreat success, MagicCount %d, CurOtherItemCount %d\n\n\n\n\n\n", template_ptr_ary.size(), temp_other_drop->get_template_ary().size());
#endif
	}
}

void drop_unit::do_pretreat_sub_accessory()
{
	//选择指定个饰品存给自己，其他的扔给其他组

	if (rule_ptr == nullptr)
	{
		return;
	}

	drop_unit* temp_group = replace_manager::get_instance().get_drop_unit("AccessoryPool");
	if (temp_group == nullptr)
	{
		return;
	}

	INT32 temp_max_num = std::atoi(rule_ptr->PretreatParam1);
	for (INT32 i = 0; i < temp_max_num; ++i)
	{
		INT32 trigger_index = -1;
		DropTemplate* temp_one = temp_group->pop_one(trigger_index);
		if (temp_one != nullptr)
		{

#if REPLACE_DEBUG
			wprintf_s(L"This Accessory Send to Pool, index %d Accessory drop ID %d\n", i, temp_one->attribute_id);
#endif
			add_one_drop(temp_one);
		}
	}

	//剩下的丢给其他
	drop_unit* other_drop = replace_manager::get_instance().get_drop_unit(rule_ptr->PretreatParam2);
	if (other_drop != nullptr)
	{
#if REPLACE_DEBUG
		std::vector<DropTemplate*> temp_log_ary = temp_group->get_template_ary();
		for (int32 i = 0; i < temp_log_ary.size(); ++i)
		{
			if (temp_log_ary[i] != nullptr)
			{
				wprintf_s(L"This Accessory Send to Other, index %d, Accessory drop ID %d\n", i, temp_log_ary[i]->attribute_id);
			}
		}
#endif

		other_drop->do_merge(temp_group);

#if REPLACE_DEBUG
		wprintf_s(L"Accessory pretreat success, AccessoryCount %d, CurOtherItemCount %d\n\n\n\n\n\n", template_ptr_ary.size(), other_drop->get_template_ary().size());
#endif

	}
}

void drop_unit::do_pretreat_sub_boss()
{
	//BOSS由两部分组成
	//其他地方扔过来的
	//扔过来的还不够，要用其他东西填满的

	if (rule_ptr == nullptr)
	{
		return;
	}

	drop_unit* fill_group = replace_manager::get_instance().get_drop_unit(rule_ptr->PretreatParam2);
	if (fill_group == nullptr)
	{
		return;
	}

	INT32 temp_old_count = template_ptr_ary.size();
	INT32 temp_max_num = std::atoi(rule_ptr->PretreatParam1);
	for (INT32 i = template_ptr_ary.size(); i < temp_max_num; ++i)
	{
		INT32 trigger_index = -1;
		DropTemplate* temp_ptr = fill_group->pop_one(trigger_index);
		if (temp_ptr == nullptr)
		{
			break;
		}
		add_one_drop(temp_ptr);

#if REPLACE_DEBUG
		wprintf_s(L"Boss filling, Index %d, ID %d\n", i, temp_ptr->attribute_id);
#endif
	}

#if REPLACE_DEBUG
	wprintf_s(L"Boss filling Success, Old Count %d, Cur Count %d\n\n\n\n\n\n", temp_old_count, template_ptr_ary.size());
#endif
}

void drop_unit::do_send_to_fill_buffer()
{
	if (rule_ptr == nullptr)
	{
		return;
	}

	drop_unit* fill_group = replace_manager::get_instance().get_drop_unit(rule_ptr->PretreatParam2);
	if (fill_group == nullptr)
	{
		return;
	}

	for (INT32 i = 0; i < template_ptr_ary.size(); ++i)
	{
		fill_group->add_one_drop(template_ptr_ary[i]);
	}

	template_ptr_ary.clear();
	max_weight = 0;

#if REPLACE_DEBUG
	std::string self_name = rule_ptr->GroupName;
	std::wstring self_name_w = std::wstring(self_name.begin(), self_name.end());

	std::string target_name = rule_ptr->PretreatParam2;
	std::wstring target_name_w = std::wstring(target_name.begin(), target_name.end());

	wprintf_s(L"Do %ws Send To Buff %ws,Cur Buffer Count %d\n", self_name_w.c_str(), target_name_w.c_str(), fill_group->get_count());
#endif
}

void drop_unit::do_refill_from_buffer()
{
	if (rule_ptr == nullptr)
	{
		return;
	}

	drop_unit* buffer_unit_ptr = replace_manager::get_instance().get_drop_unit(rule_ptr->PretreatParam2, false);
	if (buffer_unit_ptr == nullptr)
	{
		return;
	}

	INT32 temp_old_count = template_ptr_ary.size();
	INT32 temp_max_num = std::atoi(rule_ptr->PretreatParam1);
	for (INT32 i = template_ptr_ary.size(); i < temp_max_num; ++i)
	{
		INT32 trigger_index = -1;
		DropTemplate* temp_ptr = buffer_unit_ptr->pop_one(trigger_index);
		if (temp_ptr == nullptr)
		{
			break;
		}
		add_one_drop(temp_ptr);

#if REPLACE_DEBUG
		wprintf_s(L"Refilling, Index %d, ID %d\n", i, temp_ptr->attribute_id);
#endif
	}

#if REPLACE_DEBUG
	std::string self_name = rule_ptr->GroupName;
	std::wstring self_name_w = std::wstring(self_name.begin(), self_name.end());
	wprintf_s(L"%ws Refilling Success, Old Count %d, Cur Count %d\n\n\n\n\n\n", self_name_w.c_str(), temp_old_count, template_ptr_ary.size());
#endif
}

void drop_unit::do_merge(drop_unit* in_unit)
{
	if (in_unit == nullptr)
	{
		return;
	}

	std::vector<DropTemplate*> temp_vec = in_unit->get_template_ary();
	for (INT32 i = 0; i < temp_vec.size(); ++i)
	{
		add_one_drop(temp_vec[i]);
	}
}

std::vector<DropTemplate*> drop_unit::set_max_num(INT32 max_num)
{
	std::vector<DropTemplate*> erased_vec;
	if (template_ptr_ary.size() < max_num)
	{
		return erased_vec;
	}

	erased_vec = template_ptr_ary;

	std::vector<DropTemplate*> temp_vec;
	for (INT32 i = 0 ; i < max_num; ++i)
	{
		INT32 trigger_index = -1;
		DropTemplate* temp_template_ptr = pop_one(trigger_index);
		if (temp_template_ptr != nullptr)
		{
			temp_vec.push_back(temp_template_ptr);
		}
		if (trigger_index >= 0	
			&& trigger_index < erased_vec.size())
		{
			erased_vec.erase(erased_vec.begin() + trigger_index);
		}
	}

	template_ptr_ary.clear();
	max_weight = 0;

	for (INT32 i = 0 ; i < temp_vec.size(); ++i)
	{
		add_one_drop(temp_vec[i]);
	}

	return erased_vec;
}

DropTemplate* drop_unit::pop_one(INT32& out_index)
{
	out_index = -1;
	if (max_weight <= 0
		|| template_ptr_ary.size() <= 0)
	{
		return nullptr;
	}

	DropTemplate* temp_trigger_ptr = nullptr;
	std::vector<DropTemplate*>::iterator temp_it = template_ptr_ary.begin();

	if (is_ignore_weight)
	{
		INT32 temp_rand_value = common_tools::get_instance().get_random(0, template_ptr_ary.size() - 1);
		if (temp_rand_value < 0
			|| temp_rand_value >= template_ptr_ary.size())
		{
			return nullptr;
		}
		temp_trigger_ptr = template_ptr_ary[temp_rand_value];
		temp_it += temp_rand_value;
		out_index = temp_rand_value;
	}
	else
	{
		INT32 temp_cur_check_value = 0;
		INT32 temp_rand_value = common_tools::get_instance().get_random(0, max_weight);

		for (; temp_it != template_ptr_ary.end(); temp_it++)
		{
			DropTemplate* temp_ptr = *temp_it;
			if (temp_ptr == nullptr
				|| temp_ptr->Weight <= 0)
			{
				continue;
			}

			temp_cur_check_value += temp_ptr->Weight;
			if (temp_rand_value <= temp_cur_check_value)
			{
				temp_trigger_ptr = temp_ptr;
				break;
			}

		}

		out_index = temp_it - template_ptr_ary.begin();
	}

	if (temp_trigger_ptr == nullptr
		|| temp_it == template_ptr_ary.end())
	{
		return nullptr;
	}

	if (rule_ptr != nullptr
		&& rule_ptr->RandomType == e_rand_type_remove_trigger)
	{
		template_ptr_ary.erase(temp_it);
		max_weight -= temp_trigger_ptr->Weight;
		if (max_weight < 0)
		{
			max_weight = 0;
		}
	}
	return temp_trigger_ptr;
}


