#pragma once

#include "satellite.hpp"
#include "ch6000m3/plc.hpp"

namespace WarGrey::SCADA {
	WarGrey::SCADA::ISatellite* make_settings(WarGrey::SCADA::PLCMaster* device);
}
