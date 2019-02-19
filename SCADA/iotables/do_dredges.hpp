#pragma once

#include "plc.hpp"
#include "graphlet/device/gantrylet.hpp"

#include "iotables/do_winches.hpp"

namespace WarGrey::SCADA {
	private enum class DredgesGroup { PSGantries, SBGantries, _ };
	
	private enum class GantryAction { PushOut, PullIn, Stop, Diagnostics, _ };
	private enum class WaveCompensatorAction { Charge, Discharge, Stop, _ };
	private enum class DragVisorAction { Up, Down, Stop, _ };

	private enum class DragUseCommand { LongDrag, _ };
	private enum class SuctionCommand { Inflate, Deflate, _ };
	private enum class DragVisorCommand { CResistance, _ };
	private enum class GantryCommand { VirtualUp, VirtualOut, _ };
	private enum class LMODCommand { PSALMO, SBALMO, _ };

	Windows::UI::Xaml::Controls::MenuFlyout^ make_gantry_menu(WarGrey::SCADA::dredges_diagnostics_f gd, WarGrey::SCADA::PLCMaster* plc);
	Windows::UI::Xaml::Controls::MenuFlyout^ make_wave_compensator_menu(WarGrey::SCADA::PLCMaster* plc);
	Windows::UI::Xaml::Controls::MenuFlyout^ make_drag_visor_menu(WarGrey::SCADA::PLCMaster* plc);
	Windows::UI::Xaml::Controls::MenuFlyout^ make_gantry_group_menu(WarGrey::SCADA::DredgesGroup group, WarGrey::SCADA::PLCMaster* plc);

	// DB300, starts from 1
	static unsigned int drag_ps_visor_constant_resistance_command = 659U;
	static unsigned int drag_sb_visor_constant_resistance_command = 660U;

	uint16 DO_suction_command(WarGrey::SCADA::SuctionCommand cmd, bool ps);
	uint16 DO_LMOD_command(WarGrey::SCADA::LMODCommand cmd);
	uint16 DO_gantry_virtual_action_command(WarGrey::SCADA::DredgesPosition gid, WarGrey::SCADA::GantryCommand cmd);
	uint16 DO_use_long_sb_drag_command(bool cmd);
}
