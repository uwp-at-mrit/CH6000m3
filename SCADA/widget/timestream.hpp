#pragma once

#include "timemachine.hpp"
#include "plc.hpp"

namespace WarGrey::SCADA {
	void initialize_the_timemachine(WarGrey::SCADA::PLCMaster* plc, long long time_speed, int frame_rate);
	void launch_the_timemachine();
}
