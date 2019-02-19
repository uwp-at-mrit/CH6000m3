#pragma once

#include "satellite.hpp"
#include "plc.hpp"

namespace WarGrey::SCADA {
	WarGrey::SCADA::ISatellite* make_settings(WarGrey::SCADA::PLCMaster* device);
}
