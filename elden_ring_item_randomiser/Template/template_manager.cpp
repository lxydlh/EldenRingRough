
#include <fstream>
#include <cassert>
#include <memory>
//#include <regex>
#include <sstream>
#include "template_manager.h"
#include <sstream>
#include <iostream>

template_manager::template_manager()
{
}

void template_manager::init()
{
	template_size::get_instance().init_template_size();

	std::string template_dir = R"(ReplayableEdition/Data/)";
	bin_path = "ReplayableEdition/Data/data.bin";

	//std::string template_dir = R"(F:/elden_ring_item_randomiser-main 0.91/elden_ring_item_randomiser/Template/Data)";

	bin_operate = e_bin_operate_load;

	switch (bin_operate)
	{
	case e_bin_operate_non:
	{
		register_template<DropRuleTemplate>("DropRuleTemplate", template_dir + "DropRuleTemplate.csv");
		register_template<DropTemplate>("DropTemplate", template_dir + "DropTemplate.csv");
		register_template<ReplaceTemplate>("ReplaceTemplate", template_dir + "ReplaceTemplate.csv");
		register_template<ItemPropRandTemplate>("ItemPropRandTemplate", template_dir + "ItemPropRandTemplate.csv");
		register_template<ShopTemplate>("ShopTemplate", template_dir + "ShopTemplate.csv");

		wprintf_s(L"load csv success,\nDropRuleTemplate %d,\nDropTemplate %d,\nReplaceTemplate %d,\nItemPropRandTemplate %d,\nShopTemplate %d,\n\n\n\n\n\n"
			, get_row_count("DropRuleTemplate")
			, get_row_count("DropTemplate")
			, get_row_count("ReplaceTemplate")
			, get_row_count("ItemPropRandTemplate")
			, get_row_count("ShopTemplate"));
	}
		break;
	case e_bin_operate_save:
	{
		register_template<DropRuleTemplate>("DropRuleTemplate", template_dir + "DropRuleTemplate.csv");
		register_template<DropTemplate>("DropTemplate", template_dir + "DropTemplate.csv");
		register_template<ReplaceTemplate>("ReplaceTemplate", template_dir + "ReplaceTemplate.csv");
		register_template<ItemPropRandTemplate>("ItemPropRandTemplate", template_dir + "ItemPropRandTemplate.csv");
		register_template<ShopTemplate>("ShopTemplate", template_dir + "ShopTemplate.csv");

		save_to_bin();

		wprintf_s(L"save bin success,\nDropRuleTemplate %d,\nDropTemplate %d,\nReplaceTemplate %d,\nItemPropRandTemplate %d,\nShopTemplate %d,\n\n\n\n\n\n"
			, get_row_count("DropRuleTemplate")
			, get_row_count("DropTemplate")
			, get_row_count("ReplaceTemplate")
			, get_row_count("ItemPropRandTemplate")
			, get_row_count("ShopTemplate"));

	}
		break;
	case e_bin_operate_load:
	{
		read_from_bin();

		wprintf_s(L"load bin success,\nDropRuleTemplate %d,\nDropTemplate %d,\nReplaceTemplate %d,\nItemPropRandTemplate %d,\nShopTemplate %d,\n\n\n\n\n\n"
			, get_row_count("DropRuleTemplate")
			, get_row_count("DropTemplate")
			, get_row_count("ReplaceTemplate")
			, get_row_count("ItemPropRandTemplate")
			, get_row_count("ShopTemplate"));
	}
		break;
	default:
		break;
	}
}

static std::string get_item_type(std::string& item)
{
	if (item == "INT32" || item == "INT64" || item == "float" || item == "string" || item == "vector<INT32>" || item == "vector<float>" || item == "vector<string>")
	{
		return item;
	}
	else
	{
		return "";
	}
}

static std::vector<std::string> get_struct_type(std::string& template_name)
{
	std::vector<std::string> result_types;
	auto iter = template_size::get_instance().template_size_map.find(template_name);
	if (iter == template_size::get_instance().template_size_map.end())
	{
		return result_types;
	}

	for (auto& item : iter->second)
	{
		std::string type_string = get_item_type(item);
		if (type_string == "")
		{
			auto base_struct_types = get_struct_type(item);
			result_types.reserve(result_types.size() + base_struct_types.size());
			result_types.insert(result_types.end(), base_struct_types.begin(), base_struct_types.end());
		}
		else
		{
			result_types.push_back(type_string);
		}
	}
	return result_types;
}

template <class T>
void template_manager::register_template(std::string&& template_name, std::string&& file_name)
{
	common::csv_parser parser;
	parser.load(file_name);
	INT32 row_num = parser.get_row_count();

	if (row_num <= 0)
	{
		return;
	}
	template_type& template_item = template_map[template_name];
	std::vector<std::string> result_types = get_struct_type(template_name);
	common::csv_row* row_content_0 = parser.get_row(0);
	if (nullptr == row_content_0)
	{
		return;
	}
	INT32 data_size = sizeof(T);
	INT32 column_count = row_content_0->get_colomn_num();

	if (result_types.size() != column_count)
	{
		return;
	}
	re_struct_in_memory<T>(template_item, template_name, row_content_0, result_types, column_count, data_size);
	for (INT32 i = 1; i < row_num; ++i)
	{
		common::csv_row* row_content = parser.get_row(i);
		if (row_content)
		{
			re_struct_in_memory<T>(template_item, template_name, row_content, result_types, column_count, data_size);
		}
	}
}

void template_manager::add_template_by_mem(std::string& template_name, INT8* in_mem_content)
{
	if (in_mem_content == nullptr)
	{
		return;
	}

	int32 temp_row_size = get_template_row_size_new(template_name);
	template_type& template_item = template_map[template_name];
	INT32 temp_key = 0;
	memcpy(&temp_key, in_mem_content, sizeof(INT32));

	//INT32 template_line_key = std::atoi(row_content->get(0).c_str());

	template_type::iterator ite_find = template_item.find(temp_key);
	if (ite_find == template_item.end())
	{
		template_item[temp_key] = new INT8[temp_row_size];
		memset(template_item[temp_key], 0, temp_row_size);
	}
	memcpy(template_item[temp_key], in_mem_content, temp_row_size);
}

void template_manager::remove_template(std::string&& template_name)
{
	auto iter = template_map.find(template_name);
	if (iter == template_map.end())
	{
		return;
	}
	for (template_type::iterator it = iter->second.begin(); it != iter->second.end(); ++it)
	{
		delete[] it->second;
	}
	iter->second.clear();
}
template <class T>
static void  parse_string(const std::string& str, std::vector<T>* result)
{
	if (result == nullptr)
	{
		return;
	}
	auto word_begin = str.begin();
	std::string sub_string;
	bool is_set_begin = false;
	for (auto iter = str.begin(); iter != str.end(); ++iter)
	{
		if (*iter == '(')
			continue;

		if (*iter != ',' && *iter != ')')
		{
			if (is_set_begin)
				continue;

			word_begin = iter;
			is_set_begin = true;
		}
		else
		{
			if (!is_set_begin)
				break;
			sub_string = std::string(word_begin, iter);
			T TValue;
			std::stringstream ss;
			ss << sub_string;
			ss >> TValue;
			result->push_back(TValue);
			is_set_begin = false;

			if (*iter == ')')
				break;
		}
	}
}

static void  parse_string_ex(const std::string& str, std::vector<std::string>* result)
{
	if (result == nullptr)
	{
		return;
	}
	auto word_begin = str.begin();
	std::string sub_string;
	bool is_set_begin = false;
	for (auto iter = str.begin(); iter != str.end(); ++iter)
	{
		if (*iter == '(')
			continue;

		if (*iter != ',' && *iter != ')')
		{
			if (is_set_begin)
				continue;

			word_begin = iter;
			is_set_begin = true;
		}
		else
		{
			if (!is_set_begin)
				break;
			sub_string = std::string(word_begin, iter);
			INT32 size_sub_string = sub_string.size();
			result->push_back(sub_string.substr(1, size_sub_string - 2));
			is_set_begin = false;

			if (*iter == ')')
				break;
		}
	}
}
template <class T>
void template_manager::re_struct_in_memory(template_type& template_file, std::string& template_name, common::csv_row* row_content, std::vector<std::string> result_types, INT32 column_count, INT32 data_size)
{
	if (nullptr == row_content || row_content->get_colomn_num() != column_count)
	{
		return;
	}
	static INT8 memory[20000];
	memset(memory, 0, sizeof(memory));
	INT32 offset = 0;
	INT32 id = 0;
	for (INT32 i = 0; i < column_count; ++i)
	{
		const std::string& str = row_content->get(i);
		if (result_types[i] == "INT32")
		{
			INT32 ivalue = 0;
			//if (str != "")
			//{
			ivalue = std::atoi(str.c_str());
			//}
			memcpy(memory + offset, &ivalue, sizeof(INT32));
			offset += sizeof(INT32);
		}
		else if (result_types[i] == "INT64")
		{
			INT64 ivalue = 0;
			//if (str != "")
			//{
			ivalue = atol(str.c_str());
			//}
			memcpy(memory + offset, &ivalue, sizeof(INT64));
			offset += sizeof(INT64);
		}
		else if (result_types[i] == "float")
		{
			float f = 0.f;
			//if (str != "")
			//{
			f = std::atof(str.c_str());
			//}
			memcpy(memory + offset, &f, sizeof(float));
			offset += sizeof(float);
		}
		else if (result_types[i] == "string")
		{
			//std::string* str_item = new(memory + offset) std::string;
			//*str_item = str;
			//offset += sizeof(std::string);

			memcpy(memory + offset, str.c_str(), 500);
			offset += 500;
		}
		else if (result_types[i] == "vector<INT32>")
		{
			std::vector<INT32>* vect_item = new(memory + offset) std::vector<INT32>;
			parse_string<INT32>(str, vect_item);
			offset += sizeof(std::vector<INT32>);
		}
		else if (result_types[i] == "vector<float>")
		{
			std::vector<float>* vect_item = new(memory + offset) std::vector<float>;
			parse_string<float>(str, vect_item);
			offset += sizeof(std::vector<float>);
		}
		else if (result_types[i] == "vector<string>")
		{
			std::vector<std::string>* vect_item = new(memory + offset) std::vector<std::string>;
			parse_string_ex(str, vect_item);
			offset += sizeof(std::vector<std::string>);
		}
	}
	INT32 template_line_key = std::atoi(row_content->get(0).c_str());
	template_type::iterator ite_find = template_file.find(template_line_key);
	if (ite_find == template_file.end())
	{
		template_file[template_line_key] = new INT8[data_size + data_size / 2];
		memset(template_file[template_line_key], 0, data_size + data_size / 2);
	}
	memcpy(template_file[template_line_key], memory, data_size);
}

void template_manager::save_to_bin()
{
	std::ofstream f;
	f.open(bin_path.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
	int32 temp_count = template_map.size();

	char temp_template_size[10];
	memset(temp_template_size, 0, sizeof(temp_template_size));
	sprintf_s(temp_template_size, "%d", template_map.size());
	f.write(temp_template_size, 10);

	//f.write(temp_template_size, 10);
	//f.close();
	//return;

	auto template_it = template_map.begin();
	for (; template_it != template_map.end(); template_it++)
	{
		char temp_template_name[50];
		memset(temp_template_name, 0, sizeof(temp_template_name));
		sprintf_s(temp_template_name, "%s", template_it->first.c_str());
		f.write(temp_template_name, 50);

		char temp_template_row_size[10] = { 0 };
		memset(temp_template_row_size, 0, sizeof(temp_template_row_size));
		sprintf_s(temp_template_row_size, "%d", template_it->second.size());
		f.write(temp_template_row_size, 10);

		auto data_it = template_it->second.begin();
		for (; data_it != template_it->second.end(); data_it++)
		{

			std::string temp_content_s = (char*)data_it->second;

			f.write((char*)data_it->second, get_template_row_size_new(template_it->first));
		}
	}

	f.close();
}

int32 template_manager::get_template_row_size_new(std::string in_name)
{
	if (in_name == "DropRuleTemplate")
	{
		return sizeof(DropRuleTemplate);
	}
	else if (in_name == "DropTemplate")
	{
		return sizeof(DropTemplate);
	}
	else if (in_name == "ItemPropRandTemplate")
	{
		return sizeof(ItemPropRandTemplate);
	}
	else if (in_name == "ReplaceTemplate")
	{
		return sizeof(ReplaceTemplate);
	}
	else if (in_name == "ShopTemplate")
	{
		return sizeof(ShopTemplate);
	}

	return 0;
}

void template_manager::read_from_bin()
{
	std::ifstream f;
	f.open(bin_path.c_str(), std::ios::in | std::ios::binary);

	if (!f.is_open())
	{
		return;
	}

	int32 temp_offset = 0;

	char template_count_s[10] = {0};
	f.read(template_count_s, 10);
	int32 template_count_i = std::atoi(template_count_s);

	for (int32 i = 0; i < template_count_i; ++i)
	{
		char template_name[50] = { 0 };
		f.read(template_name, 50);
		std::string name_s = template_name;
		
		char template_row_count_s[10] = { 0 };
		f.read(template_row_count_s, 10);
		int32 template_row_count_i = std::atoi(template_row_count_s);

		for (int32 j = 0; j < template_row_count_i; ++j)
		{
			int32 temp_row_size = get_template_row_size_new(template_name);
			static INT8 temp_row[20000];
			memset(temp_row, 0, sizeof(temp_row));
			f.read((char*)temp_row, temp_row_size);

			add_template_by_mem(name_s, temp_row);

			//if (name_s == "DropRuleTemplate")
			//{
			//	DropRuleTemplate* temp_abc = (DropRuleTemplate*)temp_row;
			//	int32 abc = 200;
			//}
			//else if (name_s == "DropTemplate")
			//{
			//	DropTemplate* temp_abc = (DropTemplate*)temp_row;
			//	int32 abc = 200;
			//}
			//else if (name_s == "ItemPropRandTemplate")
			//{
			//	ItemPropRandTemplate* temp_abc = (ItemPropRandTemplate*)temp_row;
			//	int32 abc = 200;
			//}
			//else if (name_s == "ReplaceTemplate")
			//{
			//	ReplaceTemplate* temp_abc = (ReplaceTemplate*)temp_row;
			//	int32 abc = 200;
			//}
			//else if (name_s == "ShopTemplate")
			//{
			//	ShopTemplate* temp_abc = (ShopTemplate*)temp_row;
			//	int32 abc = 200;
			//}

			//delete temp_row;
		}
	}

	f.close();
}

DropRuleTemplate* template_manager::get_drop_rule_template_by_group_name(std::string in_group_name)
{
	template_manager::template_type* temp_table = template_manager::get_instance().get_templates("DropRuleTemplate");
	if (nullptr == temp_table)
	{
		return nullptr;
	}

	for (auto ite = temp_table->begin(); ite != temp_table->end(); ++ite)
	{
		DropRuleTemplate* temp_template_ptr = (DropRuleTemplate*)(ite->second);
		if (temp_template_ptr != nullptr
			&& temp_template_ptr->GroupName == in_group_name)
		{
			return temp_template_ptr;
		}
	}

	return nullptr;
}

