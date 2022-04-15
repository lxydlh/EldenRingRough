#pragma once
 
#include "template_base.h"
 
void DropTemplateSize();
struct DropTemplate : public template_base
{
	char GroupName[500];
	char RandomResult[500];
	INT32 ItemType;
	INT32 Weight;
	INT32 ParamEx1;
	INT32 ParamEx2;
};
