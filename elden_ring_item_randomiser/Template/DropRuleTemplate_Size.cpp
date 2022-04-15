
#include "DropRuleTemplate.h"

void DropRuleTemplateSize()
{
	template_size::get_instance().template_size_map["DropRuleTemplate"].push_back("INT32");

	template_size::get_instance().template_size_map["DropRuleTemplate"].push_back("string");

	template_size::get_instance().template_size_map["DropRuleTemplate"].push_back("INT32");
	template_size::get_instance().template_size_map["DropRuleTemplate"].push_back("INT32");

	template_size::get_instance().template_size_map["DropRuleTemplate"].push_back("string");
	template_size::get_instance().template_size_map["DropRuleTemplate"].push_back("string");
	template_size::get_instance().template_size_map["DropRuleTemplate"].push_back("string");
}
