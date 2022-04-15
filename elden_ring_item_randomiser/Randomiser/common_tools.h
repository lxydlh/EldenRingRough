#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <iostream>


class common_tools
{
	int32_t cur_seed;
public:
	static common_tools& get_instance()
	{
		static common_tools instance;
		return instance;
	}

public:
	void set_seed(int32_t inNewSeed);
	int32_t get_random(int32_t in_min, int32_t in_max);
};


