
#include "DropTemplate.h"

void DropTemplateSize()
{
	template_size::get_instance().template_size_map["DropTemplate"].push_back("INT32");

	template_size::get_instance().template_size_map["DropTemplate"].push_back("string");
	template_size::get_instance().template_size_map["DropTemplate"].push_back("string");
	template_size::get_instance().template_size_map["DropTemplate"].push_back("INT32");
	template_size::get_instance().template_size_map["DropTemplate"].push_back("INT32");
	template_size::get_instance().template_size_map["DropTemplate"].push_back("INT32");
	template_size::get_instance().template_size_map["DropTemplate"].push_back("INT32");
}
