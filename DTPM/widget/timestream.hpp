#pragma once

#include "timemachine.hpp"
#include "mrit.hpp"

namespace WarGrey::DTPM {
	void initialize_the_timemachine(WarGrey::SCADA::MRMaster* plc, long long time_speed, int frame_rate);
	void launch_the_timemachine();
}
