#pragma once
 
#include "template_base.h"
 
void ShopTemplateSize();
struct ShopTemplate : public template_base
{
	char equip_name[500];
	INT32 value;
	INT32 material_id;
	INT32 set_eventFlag;
	INT32 conditional_event_flag;
	INT32 quantity_available;
	INT32 equip_type;
	INT32 currency_type;
	INT32 quantity_produced;
	float price_rate;
	INT32 Unk4;
	INT32 shop_display_id;
	INT32 spell_instruction;
	INT32 Unk6;
	INT32 Unk7;
};
