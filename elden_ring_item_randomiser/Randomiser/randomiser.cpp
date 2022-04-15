#include "../item_randomiser_main.h"
#include "../Template/template_base.h"
#include "../Template/template_manager.h"

extern ERItemRandomiser* main_mod;

bool ERRandomiser::Shuffle() {


	using namespace std::chrono_literals;

	// Get the two runes we're going to keep static
	static_rune_01 = static_runes.at(main_mod->GetSeededRandomUint(0, 1, randomkey_seed));
	static_rune_02 = static_runes.at(main_mod->GetSeededRandomUint(2, 3, randomkey_seed));

	uint64_t solo_param_repository_ld = *(uint64_t*)(solo_param_repository);
	while (!solo_param_repository_ld) {
		solo_param_repository_ld = *(uint64_t*)(solo_param_repository);
		std::this_thread::sleep_for(1s);
	};

	// As a standard, make sure this param is loaded already
	uint64_t itemlotparam_map = *(uint64_t*)(solo_param_repository_ld + 0x670);
	while (!itemlotparam_map) {
		itemlotparam_map = *(uint64_t*)(solo_param_repository_ld + 0x670);
		std::this_thread::sleep_for(1s);
	};

	// Just randomise all of the selected params
	for (size_t q = 0; q < main_mod->param_container_names.size(); q++)
	{
		std::string template_name(main_mod->param_container_names.at(q).begin(), main_mod->param_container_names.at(q).end());
		if (template_name != "EquipParamGem"
			&& template_name != "ItemLotParam_enemy"
			&& template_name != "ItemLotParam_map"
			&& template_name != "ShopLineupParam")
		{
			continue;
		}

		auto random_preference = param_randomisation_instruction_map.find(main_mod->param_container_names.at(q));
		if (random_preference != param_randomisation_instruction_map.end())
		{
			random_preference->second(this, solo_param_repository_ld, main_mod->param_container_names.at(q), enemyitem_seed, randomisetype_shuffle & randomisetype_shuffle, randomisetype_shuffle & randomisetype_scramble);
		}
	};

	return true;
};

bool ERRandomiser::ShuffleItemLotParam_map(uint64_t solo_param_repository, std::wstring param_name, uint32_t seed, bool shuffle, bool scramble) 
{
	replace_manager::get_instance().init(enemyitem_seed);

	// Shuffle ItemLotParam_map entries carefully

	uint64_t itemlotparam_map = *(uint64_t*)(solo_param_repository + 0x670);
	itemlotparam_map = *(uint64_t*)(itemlotparam_map + 0x80);
	itemlotparam_map = *(uint64_t*)(itemlotparam_map + 0x80);

	uint32_t param_entries = *(uint32_t*)(itemlotparam_map - 0x0C);

	//拆包写死，否则崩溃
	//uint32_t param_entries = 4062;
	std::vector< ItemLotParam_map*> mapitem_list;
	std::vector< ItemLotParam_map*> mapitem_list_copy;

	uint32_t start_offset = (*(uint32_t*)(itemlotparam_map - 0x10) + 15) & -16;
	uint64_t itemlotparam_map_idrepository = itemlotparam_map + start_offset;

	INT32 temp_count = 0;
	for (uint32_t i = 1; i < param_entries; i++)
	{
		uint32_t mapitem_id = *(uint32_t*)(itemlotparam_map_idrepository + (i * 8));
		if (mapitem_id <= 0)
		{
			continue;
		}

		uint32_t entry = *(uint32_t*)(itemlotparam_map_idrepository + (i * 8) + 4);
		if ((int)entry < 0)
		{
			continue;
		};

		uint32_t container_offset = (entry + 3) * 3;

//#if ITEM_DEBUG
//		//std::wstring tempABC = std::wstring(segment.begin(), segment.end());
//		wprintf_s(L"Map, MapID is %d\n", tempPtr->MapId);
//#endif

		ItemLotParam_map* param_container = reinterpret_cast<ItemLotParam_map*>(itemlotparam_map + *(uint64_t*)(itemlotparam_map + (container_offset * 8)));
		if (param_container == nullptr)
		{
			continue;
		}
		replace_data* temp_replace_data = replace_manager::get_instance().get_replace_data_map(mapitem_id);
		if (temp_replace_data == nullptr)
		{
			continue;
		}

		for (int32 j = 0; j < 8; ++j)
		{
			//ID
			param_container->item_id_array[j] = temp_replace_data->item_id_array[j];

			//类型
			param_container->item_type_array[j] = temp_replace_data->item_type_array[j];

			//几率
			param_container->item_chance_array[j] = temp_replace_data->item_chance_array[j];

			//数量
			param_container->item_amount[j] = temp_replace_data->item_amount[j];

#if ITEM_DEBUG
			wprintf_s(L"Map Origin Drop id %d replaced, index %d, item id %d, item type %d, chance %d, amount %d\n",
				mapitem_id,
				j,
				param_container->item_id_array[j],
				param_container->item_type_array[j],
				param_container->item_chance_array[j],
				param_container->item_amount[j]);
#endif
		}

		temp_count++;
	};

#if ITEM_DEBUG
	wprintf_s(L"Map Replace Success,Count %d\n", temp_count);
#endif
	return true;
};

bool ERRandomiser::ShuffleItemLotParam_enemy(uint64_t solo_param_repository, std::wstring param_name, uint32_t seed, bool shuffle, bool scramble)
{
	replace_manager::get_instance().init(enemyitem_seed);
	// Get offset by name
	uint64_t found_param_container = 0;
	for (int i = 0; i < 185; i++) {
		int param_offset = i * 0x48;
		if (*(int*)(solo_param_repository + param_offset + 0x80) > 0) {
			uint64_t param_container = *(uint64_t*)(solo_param_repository + param_offset + 0x88);
			wchar_t* container_name = (wchar_t*)(param_container + 0x18);
			if (*(uint32_t*)(param_container + 0x28) >= 8) {
				container_name = (wchar_t*)(*(uint64_t*)container_name);
			};
			if (wcsncmp(param_name.c_str(), container_name, param_name.size()) == 0) {
				found_param_container = param_container;
				break;
			};
		};
	};

	if (!found_param_container) {
		return false;
	};

#if ITEM_DEBUG
	wprintf_s(L"Randomising %ws\n", param_name.c_str());
#endif

	// Shuffle param container
	found_param_container = *(uint64_t*)(found_param_container + 0x80);
	found_param_container = *(uint64_t*)(found_param_container + 0x80);

	// We need at least 2 entries in order to shuffle them around. Some like "NetworkParam" only have 1
	uint32_t param_entries = *(uint32_t*)(found_param_container - 0x0C);
	if (param_entries < 2) 
	{
		return true;
	};

	uint32_t start_offset = (*(uint32_t*)(found_param_container - 0x10) + 15) & -16;
	uint64_t itemlotparam_map_idrepository = found_param_container + start_offset;
	for (uint32_t i = 1; i < param_entries; i++) 
	{
		uint32_t mapitem_id = *(uint32_t*)(itemlotparam_map_idrepository + (i * 8));
		uint32_t entry = *(uint32_t*)(itemlotparam_map_idrepository + (i * 8) + 4);
		if ((int)entry < 0) {
			continue;
		};

		uint32_t container_offset = (entry + 3) * 3;

		ItemLotParam_enemy* param_container = reinterpret_cast<ItemLotParam_enemy*>(found_param_container + *(uint64_t*)(found_param_container + (container_offset * 8)));
		if (param_container == nullptr)
		{
			continue;
		}
		replace_data* temp_replace_data = replace_manager::get_instance().get_replace_data_enemy(mapitem_id);
		if (temp_replace_data == nullptr)
		{
			continue;
		}

		for (int32 j = 0; j < 8; ++j)
		{
			//ID
			param_container->item_id_array[j] = temp_replace_data->item_id_array[j];

			//类型
			param_container->item_type_array[j] = temp_replace_data->item_type_array[j];

			//几率
			param_container->item_chance_array[j] = temp_replace_data->item_chance_array[j];

			//数量
			param_container->item_amount[j] = temp_replace_data->item_amount[j];

#if ITEM_DEBUG
			wprintf_s(L"Enemy Origin Drop id %d replaced, index %d, item id %d, item type %d, chance %d, amount %d\n",
				mapitem_id,
				j,
				param_container->item_id_array[j],
				param_container->item_type_array[j],
				param_container->item_chance_array[j],
				param_container->item_amount[j]);
#endif
		}
	};

	return true;
}

bool ERRandomiser::ShuffleShopLineupParam(uint64_t solo_param_repository, std::wstring param_name, uint32_t seed, bool shuffle, bool scramble) 
{
	uint64_t shoplineupparam = *(uint64_t*)(solo_param_repository + 0x820);
	shoplineupparam = *(uint64_t*)(shoplineupparam + 0x80);
	shoplineupparam = *(uint64_t*)(shoplineupparam + 0x80);

	uint32_t param_entries = *(uint32_t*)(shoplineupparam - 0x0C);

	uint32_t start_offset = (*(uint32_t*)(shoplineupparam - 0x10) + 15) & -16;
	uint64_t shoplineupparam_idrepository = shoplineupparam + start_offset;


	int32 temp_counter = 0;
	for (uint32_t i = 1; i < param_entries; i++) 
	{
		uint32_t shopitem_id = *(uint32_t*)(shoplineupparam_idrepository + (i * 8));
		uint32_t entry = *(uint32_t*)(shoplineupparam_idrepository + (i * 8) + 4);
		if ((int)entry < 0) 
		{
			continue;
		};

		uint32_t container_offset = (entry + 3) * 3;
		NewShopStruct* param_container = reinterpret_cast<NewShopStruct*>(shoplineupparam + *(uint64_t*)(shoplineupparam + (container_offset * 8)));
		if (param_container == nullptr)
		{
			continue;
		}

		ShopTemplate* temp_shop_ptr = template_manager::get_instance().get_template<ShopTemplate>("ShopTemplate", shopitem_id);
		if (temp_shop_ptr == nullptr)
		{
			continue;
		}
		std::string temp_equip_name = temp_shop_ptr->equip_name;
		INT32 old_item_id = param_container->equip_id;
		INT32 new_item_id = std::atoi(temp_shop_ptr->equip_name);

		if (temp_equip_name == "0"
			|| temp_equip_name.length() == 0)
		{
			new_item_id = 0;
			param_container->equip_id = new_item_id;
			param_container->value = temp_shop_ptr->value;
			param_container->quantity_available = temp_shop_ptr->quantity_available;
		}
		else if (new_item_id > 0)
		{
			param_container->equip_id = new_item_id;
			param_container->value = temp_shop_ptr->value;
			param_container->quantity_available = temp_shop_ptr->quantity_available;
		}
		else
		{
			drop_unit* temp_drop_unit = replace_manager::get_instance().get_drop_unit(temp_shop_ptr->equip_name);
			if (temp_drop_unit == nullptr)
			{
				continue;
			}
			INT32 temp_index = 0;
			DropTemplate* temp_drop_ptr = temp_drop_unit->pop_one(temp_index);
			if (temp_drop_ptr == nullptr)
			{
				continue;
			}

			new_item_id = std::atoi(temp_drop_ptr->RandomResult);
			int32 temp_quantity = temp_drop_ptr->ParamEx1;
			int32 temp_value = temp_drop_ptr->ParamEx2;
			param_container->equip_id = new_item_id;
			param_container->quantity_available = temp_quantity > 0 ? temp_quantity : temp_shop_ptr->quantity_available;
			param_container->value = temp_value > 0 ? temp_value : temp_shop_ptr->value;
		}

		param_container->material_id = temp_shop_ptr->material_id;
		param_container->set_eventFlag = temp_shop_ptr->set_eventFlag;
		param_container->conditional_event_flag = temp_shop_ptr->conditional_event_flag;

		param_container->equip_type = temp_shop_ptr->equip_type;

		param_container->currency_type = temp_shop_ptr->currency_type;
		param_container->quantity_produced = temp_shop_ptr->quantity_produced;
		param_container->price_rate = temp_shop_ptr->price_rate;
		param_container->Unk4 = temp_shop_ptr->Unk4;
		param_container->shop_display_id = temp_shop_ptr->shop_display_id;
		param_container->spell_instruction = temp_shop_ptr->spell_instruction;
		param_container->Unk6 = temp_shop_ptr->Unk6;
		param_container->Unk7 = temp_shop_ptr->Unk7;


		temp_counter++;

#if ITEM_DEBUG
		wprintf_s(L"Shop item be replaced, old item is %d, new item is %d\n", old_item_id, new_item_id);
#endif
	}

#if ITEM_DEBUG
	wprintf_s(L"Shop replace success, count %d\n", temp_counter);
#endif

	return true;
};

bool ERRandomiser::ShuffleAshParam(uint64_t solo_param_repository, std::wstring param_name, uint32_t seed, bool shuffle, bool scramble)
{
	uint64_t found_param_container = 0;
	for (int i = 0; i < 185; i++) {
		int param_offset = i * 0x48;
		if (*(int*)(solo_param_repository + param_offset + 0x80) > 0) {
			uint64_t param_container = *(uint64_t*)(solo_param_repository + param_offset + 0x88);
			wchar_t* container_name = (wchar_t*)(param_container + 0x18);
			if (*(uint32_t*)(param_container + 0x28) >= 8) {
				container_name = (wchar_t*)(*(uint64_t*)container_name);
			};
			if (wcsncmp(param_name.c_str(), container_name, param_name.size()) == 0) {
				found_param_container = param_container;
				break;
			};
		};
	};

	if (!found_param_container) 
	{
		return false;
	};

	// Shuffle param container
	found_param_container = *(uint64_t*)(found_param_container + 0x80);
	found_param_container = *(uint64_t*)(found_param_container + 0x80);

	// We need at least 2 entries in order to shuffle them around. Some like "NetworkParam" only have 1
	uint32_t param_entries = *(uint32_t*)(found_param_container - 0x0C);
	if (param_entries < 2)
	{
		return true;
	};

	int32 temp_counter = 0;
	uint32_t start_offset = (*(uint32_t*)(found_param_container - 0x10) + 15) & -16;
	uint64_t itemlotparam_map_idrepository = found_param_container + start_offset;
	for (uint32_t i = 1; i < param_entries; i++)
	{
		uint32_t mapitem_id = *(uint32_t*)(itemlotparam_map_idrepository + (i * 8));
		uint32_t entry = *(uint32_t*)(itemlotparam_map_idrepository + (i * 8) + 4);
		if ((int)entry < 0) {
			continue;
		};

		uint32_t container_offset = (entry + 3) * 3;

		AshStruct* param_container = reinterpret_cast<AshStruct*>(found_param_container + *(uint64_t*)(found_param_container + (container_offset * 8)));
		if (param_container == nullptr)
		{
			continue;
		}

		for (int32 i = 0; i < 5; ++i)
		{
			param_container->available_ary[i] = 0;
		}

		temp_counter++;
	}
#if ITEM_DEBUG
	wprintf_s(L"ash replace success, count %d\n", temp_counter);
#endif

	return true;
}

bool ERRandomiser::ShouldRandomiseMapItem(ItemLotParam_map* param_container) {

	bool should_randomise = true;

	// Don't include empty item containers in the randomisation
	if (!param_container->item_id_array[0]) {
		return false;
	};

	for (int i = 0; i < sizeof(ItemLotParam_map::item_id_array) / sizeof(uint32_t); i++) {
		uint32_t item_id = param_container->item_id_array[i];
		if (item_id) {

			switch (param_container->item_type_array[i]) {

				// Don't randomise certain keys and crafting materials
			case(mapitemtype_goods): {

				ParamContainer goods_param_container = {};
				equipparamgoods_function(&goods_param_container, item_id);
				if (!goods_param_container.param_entry) {
					// Not a valid entry
					return false;
				};

				if (item_id < 100) {
					// Cut / broken / ?GoodsName?
					return false;
				};

				if (item_id == 106) {
					// You can't open the first door without this
					return false;
				};

				if ((item_id >= 15000) && (item_id <= 53658)) {
					// Invalid
					return false;
				};

				// Don't estus flask upgrade materials if selected not to do so
				if (!randomise_estusupgrade) {
					if ((item_id == 10010) || (item_id == 10020)) {
						return false;
					};
				};

				// Don't randomise keys if selected not to do so
				if (!random_keys) {
					for (size_t q = 0; q < excluded_items.size(); q++) {
						if (excluded_items.at(q) == item_id) {
							return false;
						};
					};

					if ((item_id >= 8600) && (item_id <= 8618)) {
						// Maps
						return false;
					};

					if ((item_id == static_rune_01) || (item_id == static_rune_02)) {
						// The two runes we need to keep static to preserve progression
						return false;
					};
				};

				break;
			};
			case(mapitemtype_weapon): {

				// Invalid
				if (item_id == 100000) {
					return false;
				};

				break;
			};
			case(mapitemtype_armour): {

				// Invalid
				if ((item_id >= 1000) && (item_id <= 10300)) {
					return false;
				};

				break;
			};
			case(mapitemtype_accessory):
			case(mapitemtype_gem):
			default: break;
			};
		};
	};
	return should_randomise;
};

bool ERRandomiser::RandomiseGenericParamContainer(uint64_t solo_param_repository, std::wstring param_name, uint32_t seed, bool shuffle, bool scramble)
{
	return true;

	replace_manager::get_instance().init(enemyitem_seed);


	// Get offset by name
	uint64_t found_param_container = 0;
	for (int i = 0; i < 185; i++) {
		int param_offset = i * 0x48;
		if (*(int*)(solo_param_repository + param_offset + 0x80) > 0) {
			uint64_t param_container = *(uint64_t*)(solo_param_repository + param_offset + 0x88);
			wchar_t* container_name = (wchar_t*)(param_container + 0x18);
			if (*(uint32_t*)(param_container + 0x28) >= 8) {
				container_name = (wchar_t*)(*(uint64_t*)container_name);
			};
			if (wcsncmp(param_name.c_str(), container_name, param_name.size()) == 0) {
				found_param_container = param_container;
				break;
			};
		};
	};

	if (!found_param_container) {
		return false;
	};

#if ITEM_DEBUG
	wprintf_s(L"Randomising %ws\n", param_name.c_str());
#endif

	// Shuffle param container
	found_param_container = *(uint64_t*)(found_param_container + 0x80);
	found_param_container = *(uint64_t*)(found_param_container + 0x80);

	// We need at least 2 entries in order to shuffle them around. Some like "NetworkParam" only have 1
	uint32_t param_entries = *(uint32_t*)(found_param_container - 0x0C);
	if (param_entries < 2) {
		return true;
	};

	std::function<bool(ERRandomiserProperties*, uint64_t, uint32_t, uint32_t, uint32_t)> randomiser_property_proc = randomiser_properties.GetGenerateSpecialProperties_Static(param_name);
	uint32_t start_offset = (*(uint32_t*)(found_param_container - 0x10) + 15) & -16;
	uint64_t idrepository = found_param_container + start_offset;

	std::vector<uint32_t> offset_list;
	std::vector<uint32_t> entry_list;
	for (uint32_t i = 1; i < param_entries; i++) {

		uint32_t entry = *(uint32_t*)(idrepository + (i * 8) + 4);
		if ((int)entry < 0) {
			continue;
		};

		// Call any specified special property functions. These are functions specific to a param which can be used to make special changes
		// for example not randomising a critical entry which might make the game unplayable
		if (randomiser_property_proc) {
			if (!std::invoke(randomiser_property_proc, &randomiser_properties, found_param_container, *(uint32_t*)(idrepository + (i * 8)), entry, shuffle + (scramble * 2))) {
				continue;
			};
		};

		offset_list.push_back(i);
		entry_list.push_back(entry);
	};

	std::shuffle(entry_list.begin(), entry_list.end(), std::default_random_engine(seed));

	uint8_t param_container_entry_type = *(uint8_t*)(found_param_container + 0x2D);
	size_t param_entry_size = 0;
	if (scramble) {
		switch (param_container_entry_type & 127) {
		case(2): {
			param_entry_size = *(uint64_t*)(found_param_container + 0x38) - *(uint64_t*)(found_param_container + 0x34);
			break;
		};
		case(3): {
			param_entry_size = *(uint64_t*)(found_param_container + 0x48) - *(uint64_t*)(found_param_container + 0x44);
			break;
		};
		case(5): {
			if (param_container_entry_type < 128) {
				param_entry_size = *(uint64_t*)(found_param_container + 0x48) - *(uint64_t*)(found_param_container + 0x44);
				break;
			};
			// Otherwise, run case 4
		};
		case(4): {
			if (*(uint8_t*)(found_param_container + 0x2E) & 2) {
				param_entry_size = *(uint64_t*)(found_param_container + 0x60) - *(uint64_t*)(found_param_container + 0x48);
			}
			else {
				param_entry_size = *(uint64_t*)(found_param_container + 0x48) - *(uint64_t*)(found_param_container + 0x44);
			};
			break;
		};
		default: break;
		};
	};

	size_t param_container_entry_size = entry_list.size();
	std::vector<uint32_t*> param_container_vector;
	for (size_t q = 0; q < param_container_entry_size; q++) {

		uint32_t entry = entry_list.at(q);
		if (shuffle) {
			*(uint32_t*)(idrepository + (offset_list.at(q) * 8) + 4) = entry;
		};

		// Stop here if the param isn't to be scrambled
		if (param_entry_size < 4) {
			continue;
		};

		uint32_t* generic_param_container = (uint32_t*)randomiser_properties.GetParamEntry(found_param_container, entry);
		param_container_vector.push_back(generic_param_container);
	};

	// The generic randomiser doesn't know the structure of each param, so just scramble it in 4 byte increments
	param_entry_size /= 4;
	size_t vector_size = param_container_vector.size();
	for (uint32_t i = 0; i < param_entry_size; i++) {
		std::vector<uint32_t> offset_xx;
		for (size_t q = 0; q < vector_size; q++) {
			offset_xx.push_back(param_container_vector.at(q)[i]);
		};

		std::shuffle(offset_xx.begin(), offset_xx.end(), std::default_random_engine(seed + i));

		for (size_t q = 0; q < vector_size; q++) {
			param_container_vector.at(q)[i] = offset_xx.at(q);
		};

		offset_xx.clear();
	};

	return true;
};

ERRandomiserProperties* ERRandomiser::GetSpecialPropertyClass() {
	return &randomiser_properties;
};
