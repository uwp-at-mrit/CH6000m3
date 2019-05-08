#pragma once

#include "ch6000m3/plc.hpp"

namespace WarGrey::SCADA {
	void initialize_the_alarm(WarGrey::SCADA::PLCMaster* plc);
	void display_the_alarm();
	void update_the_shown_alarm(long long count, long long interval, long long uptime);
}
