#pragma once

#include "plc.hpp"

namespace WarGrey::SCADA {
	void initialize_the_alarm(WarGrey::SCADA::PLCMaster* plc);
	void display_the_alarm();
	void update_the_shown_alarm(long long count, long long interval, long long uptime);
}
