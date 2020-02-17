#pragma once

#include "gps.hpp"

namespace WarGrey::GYDM {
	private enum class BrightnessPort : unsigned short { SCADA, DTPM, _ };

	Platform::String^ brightness_preference_key(WarGrey::GYDM::BrightnessPort bp);
	unsigned short brightness_slang_port(WarGrey::GYDM::BrightnessPort bp);
}
