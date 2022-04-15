
#include "template_base.h"
#include "DropTemplate.h"
#include "DropRuleTemplate.h"
#include "ReplaceTemplate.h"
#include "ItemPropRandTemplate.h"
#include "ShopTemplate.h"

void template_size::init_template_size()
{
	init_template_base();

	DropTemplateSize();
	DropRuleTemplateSize();
	ReplaceTemplateSize();
	ItemPropRandTemplateSize();
	ShopTemplateSize();
}
