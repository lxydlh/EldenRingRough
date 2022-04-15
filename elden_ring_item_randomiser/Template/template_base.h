
#pragma once
#include <unordered_map>
#include <intsafe.h>
#include <map>

#pragma pack(push, 1)
	struct template_base
	{
		INT32   attribute_id;						// ID
        template_base& operator= (const template_base & temp)
        {
            attribute_id            = temp.attribute_id;            
            return *this;
        }
	};
#pragma pack(pop)
	class template_size
	{
	public:
		void init_template_size();
	private:
 		void init_template_base()
 		{
			template_size_map.clear();
 			template_size_map["template_base"].push_back("int32");
 			template_size_map["template_base"].push_back("template_base&");
 			template_size_map["template_base"].push_back("attribute_id");
 			template_size_map["template_base"].push_back("return");
 			template_size_map["template_base"].push_back("}");
 		}
	public:
		static template_size& get_instance()
		{
			static template_size s_template_size;
			return s_template_size;
		}
	public:
		std::unordered_map<std::string, std::vector<std::string>> template_size_map;
	};
