#pragma once

#include "plc.hpp"

namespace WarGrey::SCADA {
	private enum class PSWaterPumpAction {
		Prepare, Start, Stop, Reset,
		PS_PS, PS_SB, PS_2,
		S2_PS, S2_SB, S2_2, P2_2, I2_2,
		PS_H, S2_H, P2_H,
		_
	};

	private enum class SBWaterPumpAction {
		Prepare, Start, Stop, Reset,
		SB_PS, SB_SB, SB_2,
		S2_PS, S2_SB, S2_2, P2_2, I2_2,
		SB_H, S2_H, P2_H,
		_
	};

	private enum class WaterPumpConditionAction {
		PS_PS, PS_SB, PS_2,
		SB_PS, SB_SB, SB_2,
		S2_PS, S2_SB, S2_2, P2_2, I2_2,
		PS_H, SB_H, S2_H, P2_H,
		_
	};

	private enum class FlushingCommand { LeftShift, RightShift, _ };

	Windows::UI::Xaml::Controls::MenuFlyout^ make_ps_water_pump_menu(WarGrey::SCADA::PLCMaster* plc);
	Windows::UI::Xaml::Controls::MenuFlyout^ make_sb_water_pump_menu(WarGrey::SCADA::PLCMaster* plc);
	Windows::UI::Xaml::Controls::MenuFlyout^ make_water_pump_condition_menu(
		WarGrey::SCADA::WaterPumpConditionAction action, WarGrey::SCADA::PLCMaster* plc);

	unsigned int DO_water_pump_reset_command(bool ps);

	// DB300, starts from 1
	static unsigned int left_shifting_command  = 559U;
	static unsigned int right_shifting_command = 560U;
}
