#pragma once
 
#include "template_base.h"
 
void ItemPropRandTemplateSize();
struct ItemPropRandTemplate : public template_base
{
	char SpecialPropGroupName[500];
	char AshOfWarGroupName[500];
	INT32 ItemAmountMin;
	INT32 ItemAmountMax;
	INT32 LevelRandType;
	INT32 LevelRandValueMin;
	INT32 LevelRandValueMax;
};
