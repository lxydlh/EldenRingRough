#pragma once
 
#include "template_base.h"
 
void ReplaceTemplateSize();
struct ReplaceTemplate : public template_base
{
	INT32 OriginDropID;
	char DropType[500];

	char ItemDropName1[500];
	char ItemDropName2[500];
	char ItemDropName3[500];
	char ItemDropName4[500];
	char ItemDropName5[500];
	char ItemDropName6[500];
	char ItemDropName7[500];
	char ItemDropName8[500];
	INT32 Amount1;
	INT32 Amount2;
	INT32 Amount3;
	INT32 Amount4;
	INT32 Amount5;
	INT32 Amount6;
	INT32 Amount7;
	INT32 Amount8;
	INT32 Chance1;
	INT32 Chance2;
	INT32 Chance3;
	INT32 Chance4;
	INT32 Chance5;
	INT32 Chance6;
	INT32 Chance7;
	INT32 Chance8;
};
