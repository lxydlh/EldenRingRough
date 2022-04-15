#pragma once
 
#include "template_base.h"
 
void DropRuleTemplateSize();
struct DropRuleTemplate : public template_base
{
	char GroupName[500];
	INT32 RandomType;	
	INT32 RandomParam;
	char PretreatMethod[500];
	char PretreatParam1[500];
	char PretreatParam2[500];
};
