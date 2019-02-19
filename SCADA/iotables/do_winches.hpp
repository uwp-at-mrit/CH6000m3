#pragma once

#include "plc.hpp"
#include "graphlet/device/winchlet.hpp"

namespace WarGrey::SCADA {
	private enum class DredgesPosition { psTrunnion, psIntermediate, psDragHead, sbTrunnion, sbIntermediate, sbDragHead, _ };
	private enum class ShipSlot { BowWinch, SternWinch, ShoreWinch, BargeWinch, _ };

	private enum class DredgingWinchAction { WindUp, WindOut, Stop, HighSpeed, Diagnostics, _ };
	private enum class AnchorWinchAction { WindUp, WindOut, Stop, CTension, _ };
	private enum class ShoreWinchAction { WindUp, WindOut, HighSpeed, _ };
	private enum class BargeWinchAction { WindUp, WindOut, Stop, _ };
	
	private enum class BargeCylinderAction { OpenBolt, CloseBolt, _ };
	private enum class ShoreCylinderAction { Hold, Loose, Bolt, Unbolt, _ };
	
	typedef void(*dredges_diagnostics_f)(DredgesPosition, WarGrey::SCADA::PLCMaster*);

	Windows::UI::Xaml::Controls::MenuFlyout^ make_dredging_winch_menu(WarGrey::SCADA::dredges_diagnostics_f wd, WarGrey::SCADA::PLCMaster* plc);
	Windows::UI::Xaml::Controls::MenuFlyout^ make_anchor_winch_menu(WarGrey::SCADA::PLCMaster* plc);
	Windows::UI::Xaml::Controls::MenuFlyout^ make_shore_winch_menu(WarGrey::SCADA::PLCMaster* plc);
	Windows::UI::Xaml::Controls::MenuFlyout^ make_barge_winch_menu(WarGrey::SCADA::PLCMaster* plc);

	Windows::UI::Xaml::Controls::MenuFlyout^ make_shore_cylinder_menu(WarGrey::SCADA::PLCMaster* plc);
	Windows::UI::Xaml::Controls::MenuFlyout^ make_barge_cylinder_menu(WarGrey::SCADA::PLCMaster* plc);

	uint16 DO_winch_override_command(WarGrey::SCADA::DredgesPosition id);
	uint16 DO_winch_upper_check_command(WarGrey::SCADA::DredgesPosition id);
	uint16 DO_winch_saddle_check_command(WarGrey::SCADA::DredgesPosition id);
}
