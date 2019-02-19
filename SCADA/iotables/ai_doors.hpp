#pragma once

#include "graphlet/symbol/door/hopper_doorlet.hpp"

namespace WarGrey::SCADA {
	// DB203, real data
	static unsigned int bottom_door_PS1_progress = 48U;
	static unsigned int bottom_door_PS2_progress = 49U;
	static unsigned int bottom_door_PS3_progress = 50U;
	static unsigned int bottom_door_PS4_progress = 72U;
	static unsigned int bottom_door_PS5_progress = 73U;
	static unsigned int bottom_door_PS6_progress = 74U;
	static unsigned int bottom_door_PS7_progress = 75U;

	static unsigned int bottom_door_SB1_progress = 64U;
	static unsigned int bottom_door_SB2_progress = 65U;
	static unsigned int bottom_door_SB3_progress = 66U;
	static unsigned int bottom_door_SB4_progress = 88U;
	static unsigned int bottom_door_SB5_progress = 89U;
	static unsigned int bottom_door_SB6_progress = 90U;
	static unsigned int bottom_door_SB7_progress = 91U;

	static unsigned int upper_door_PS1_progress = 52U;
	static unsigned int upper_door_PS2_progress = 53U;
	static unsigned int upper_door_PS3_progress = 54U;
	static unsigned int upper_door_PS4_progress = 76U;
	static unsigned int upper_door_PS5_progress = 77U;
	static unsigned int upper_door_PS6_progress = 78U;
	static unsigned int upper_door_PS7_progress = 79U;

	static unsigned int upper_door_SB1_progress = 68U;
	static unsigned int upper_door_SB2_progress = 69U;
	static unsigned int upper_door_SB3_progress = 70U;
	static unsigned int upper_door_SB4_progress = 92U;
	static unsigned int upper_door_SB5_progress = 93U;
	static unsigned int upper_door_SB6_progress = 94U;
	static unsigned int upper_door_SB7_progress = 95U;

	static float bottom_door_open_threshold = 98.0F;
	static float upper_door_open_threshold = 98.5F;
	static float upper_door_closed_threshold = 2.0F;

	void AI_hopper_door(WarGrey::SCADA::IHopperDoorlet* target,
		float progress, float open_threshold, float closed_threshold);
}
