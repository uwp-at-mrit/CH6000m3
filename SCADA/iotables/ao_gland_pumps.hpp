#pragma once

namespace WarGrey::SCADA {
	// DB20, DBD
	template<typename E>
	uint16 AO_gland_pump_setting(E id) {
		int16 index = -1;

		switch (id) {
		case E::PSHPa: index = 226U; break;
		case E::PSHPb: index = 230U; break;
		case E::SBHPa: index = 234U; break;
		case E::SBHPb: index = 238U; break;
		}

		return index;
	}
}
