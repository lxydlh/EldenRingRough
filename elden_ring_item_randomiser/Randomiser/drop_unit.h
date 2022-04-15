#pragma once

#include "../Template/template_manager.h"

enum e_rand_type
{
	e_rand_type_normal			= 0,
	e_rand_type_remove_trigger	= 1,
	e_rand_type_max
};

enum e_weapon_level_rand_type
{
	e_weapon_level_rand_type_non = 0,			//不随机
	e_weapon_level_rand_type_in_template = 1,	//表格填啥是啥
	e_weapon_level_rand_type_rand = 2,			//根据角色信息的最大强化等级修正
	e_weapon_level_rand_type_max
};

enum e_ash_type
{
	e_ash_type_non = 0,
	e_ash_type_1 = 1,
	e_ash_type_2 = 2,
	e_ash_type_3 = 3,
	e_ash_type_max
};

struct replace_data
{
	uint32_t item_id_array[8];
	MapItemType item_type_array[8];
	uint16_t item_chance_array[8];
	uint16_t cumulative_chance[8];
	uint32_t item_completeevent_type[8];
	uint32_t get_item_flag_id;
	uint32_t cumulative_number_flag_id;
	uint8_t cumulative_number_max;
	int8_t undetermined;
	uint8_t item_amount[8];

	replace_data()
	{
		memset(item_id_array, 0, sizeof(item_id_array));
		memset(item_type_array, 0, sizeof(item_type_array));
		memset(item_chance_array, 0, sizeof(item_chance_array));
		memset(cumulative_chance, 0, sizeof(cumulative_chance));
		memset(item_completeevent_type, 0, sizeof(item_completeevent_type));
		get_item_flag_id = 0;
		cumulative_number_flag_id = 0;
		cumulative_number_max = 0;
		undetermined = 0;
		memset(item_amount, 0, sizeof(item_amount));
	}
};

class drop_unit
{
	std::string cur_group_name;
	std::vector<DropTemplate*> template_ptr_ary;
	DropRuleTemplate* rule_ptr;
	INT32 max_weight;
	bool is_ignore_weight = false;
public:
	drop_unit();
	~drop_unit();
	void clear_data();
	void init(std::string inGroupName);
	void add_one_drop(DropTemplate* inNewTemplatePtr);
	void remove_one_drop(DropTemplate* inNewTemplatePtr);
	void do_pretreat_sub_ash1();
	void do_pretreat_sub_ash_other(INT32 inIndex);
	void do_pretreat_sub_magic();
	void do_pretreat_sub_accessory();
	void do_pretreat_sub_boss();
	void do_send_to_fill_buffer();
	void do_refill_from_buffer();

	void do_merge(drop_unit* in_unit);

	std::vector<DropTemplate*> set_max_num(INT32 max_num);
	void set_ignore_weight(bool in_is_ignore) { is_ignore_weight = in_is_ignore; };

	std::vector<DropTemplate*> get_template_ary() { return template_ptr_ary; };
	DropRuleTemplate* get_rule_template_ptr() { return rule_ptr; };
	INT32 get_max_weight() { return max_weight; };
	INT32 get_count() { return template_ptr_ary.size(); };

	DropTemplate* pop_one(INT32& out_index);

};


