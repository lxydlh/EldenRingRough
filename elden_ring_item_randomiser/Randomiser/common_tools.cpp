

#include "common_tools.h"


void common_tools::set_seed(int32_t inNewSeed)
{
	cur_seed = inNewSeed;
	srand(inNewSeed);
}
int32_t common_tools::get_random(int32_t in_min, int32_t in_max)
{
	uint32_t return_value = 0;
	uint32_t temp_value = rand() << 16 | rand();
	if (in_max < 0)
	{
		return_value = temp_value % (in_max - 1 - in_min) + in_min;
	}
	else
	{
		return_value = temp_value % (in_max + 1 - in_min) + in_min;
	}

	return return_value;
}