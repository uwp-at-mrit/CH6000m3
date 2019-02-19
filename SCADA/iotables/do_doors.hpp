#pragma once

#include "plc.hpp"

namespace WarGrey::SCADA {
	private enum class Door { SB1, SB2, SB3, SB4, SB5, SB6, SB7, PS1, PS2, PS3, PS4, PS5, PS6, PS7 };
	private enum class BottomDoorsGroup { HDoor12, HDoor35, HDoor67, HDoor17 };

	private enum class BottomDoorCommand { AutoLock, Locked, OpenDoorCheck, CloseDoorCheck };

	private enum class DoorsGroupAction { Open, Stop, Close, _ };
	private enum class DoorAction { Open, Stop, Close, Disable, _ };
	private enum class UpperDoorAction { Open, Stop, Close, Disable, Pair, _ };

	Windows::UI::Xaml::Controls::MenuFlyout^ make_bottom_door_menu(WarGrey::SCADA::PLCMaster* plc);
	Windows::UI::Xaml::Controls::MenuFlyout^ make_bottom_doors_group_menu(WarGrey::SCADA::BottomDoorsGroup group, WarGrey::SCADA::PLCMaster* plc);
	Windows::UI::Xaml::Controls::MenuFlyout^ make_upper_door_menu(WarGrey::SCADA::PLCMaster* plc);

	uint16 DO_bottom_doors_special_command(WarGrey::SCADA::BottomDoorCommand cmd);
}
