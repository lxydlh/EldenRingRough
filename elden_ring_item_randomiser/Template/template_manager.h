#pragma once

#include "csv_parser.hpp"
#include "template_base.h"
#include "ReplaceTemplate.h"
#include "DropTemplate.h"
#include "DropRuleTemplate.h"
#include "ItemPropRandTemplate.h"
#include "ShopTemplate.h"


enum e_bin_operate
{
	e_bin_operate_non = 0,
	e_bin_operate_save,
	e_bin_operate_load,
};

class template_manager
{

	e_bin_operate bin_operate;
	std::string bin_path;

public:
	static template_manager& get_instance()
	{
		static template_manager instance;
		return instance;
	}
public:
	void init();
	typedef std::map<INT32, INT8*> template_type;

public:
	template<class T>
	T* get_template(std::string&& template_name, INT32 id)
	{
		if (id <= 0)
		{
			return nullptr;
		}
		auto iter = template_map.find(template_name);
		if (iter == template_map.end())
		{
			return nullptr;
		}

		auto item_iter = iter->second.find(id);
		if (item_iter == iter->second.end())
		{
			return nullptr;
		}
		return (T*)(item_iter->second);
	}

	template_type* get_templates(std::string&& template_name)
	{
		auto iter = template_map.find(template_name);
		if (iter == template_map.end())
		{
			return nullptr;
		}
		return &iter->second;
	}

	size_t get_template_row_size(std::string&& template_name)
	{
		auto iter = template_map.find(template_name);
		if (iter == template_map.end())
		{
			return 0;
		}
		return iter->second.size();
	}
	INT32 get_row_count(std::string&& template_name)
	{
		auto iter = template_map.find(template_name);
		if (iter == template_map.end())
		{
			return -1;
		}

		return iter->second.size();
	}


	void										remove_template(std::string&& template_name);

private:
	template_manager();
	template_manager(const template_manager&);
	void										operator=(const template_manager&);
	template<class T>
	void										register_template(std::string&& template_name, std::string&& file_name);
	void										add_template_by_mem(std::string& template_name, INT8* in_mem_content);
	template <class T>
	void										re_struct_in_memory(template_type& template_file, std::string& template_name, common::csv_row* row_content, std::vector<std::string> result_types, int32 column_count, int32 data_size);
	std::map<std::string, template_type>		template_map;
	std::vector<INT32>							m_empty_int_array;
	std::vector<float>							m_empty_float_array;
	std::vector<std::pair<INT32, INT8*>>		vec_grade_qiyuan_templates;
	std::vector<std::pair<INT32, INT8*>>		vec_flair_qiyuan_templates;

public:
	std::string return_empty_string;//用于返回空字符串的引用
	std::vector<std::string> return_string_list;//用于返回空vector的引用

public:

	void save_to_bin();
	int32 get_template_row_size_new(std::string in_name);
	void read_from_bin();

	DropRuleTemplate* get_drop_rule_template_by_group_name(std::string in_group_name);
};


