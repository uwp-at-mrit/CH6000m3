#pragma once

namespace WarGrey::SCADA {
	private enum class GantryWinchTrunnionSettings {
		_Gantry, GantryFlow, PushFlow, PullFlow, GantryPressure,
		_Winch, WinchFlow, PushUpFlow, PushOutFlow, PullUpFlow, PullOutFlow, WinchPressure, RemoteFlow, RemotePressure,
		_
	};

	private enum class GantryWinchIntermediateSettings {
		_Gantry, GantryFlow, PushFlow, PullFlow, GantryPressure,
		_Winch, WinchFlow, PushUpFlow, PushOutFlow, PullUpFlow, PullOutFlow, DragOutFlow, DragUpFlow, SPWCUpFlow, SPWCOutFlow, WinchPressure,
		_
	};

	private enum class GantryWinchDragHeadSettings {
		_Gantry, GantryFlow, PushFlow, PullFlow, GantryPressure,
		_Winch, WinchFlow, PushUpFlow, PushOutFlow, PullUpFlow, PullOutFlow,
		DragOutFlow, DragUpFlow, SPWCFastUpFlow, SPWCFastOutFlow, SPWCSlowUpFlow, SPWCSlowOutFlow, WinchPressure,
		_
	};

	private enum class DragPipesSettings { ForearmDegrees, BackarmDegrees, ForeBackDegrees, CableLength, _ };
	private enum class DoorsSettings { _UpperDoors, UpperFlow, UpperPressure, _BottomDoors, BottomFlow, OpenPressure, ClosePressure, _ };

	private enum class ShoreDischargeSettings {
		_Winch, NormalFlow, FastFlow, WinchPressure,
		_Holdhoop, HoldhoopFlow, HoldhoopPressure,
		_Bolt, BoltFlow, BoltPressure,
		_
	};

	private enum class OtherSettings {
		_Barge, BargeFlow, BargePressure,
		_Overflow, OverflowFlow, OverflowPressure,
		_ALMO, ChargeDensity, DischargeDensity,
		_
	};

	unsigned int AO_gantry_winch_trunnion_settings(GantryWinchTrunnionSettings setting, bool ps);
	unsigned int AO_gantry_winch_intermediate_settings(GantryWinchIntermediateSettings setting, bool ps);
	unsigned int AO_gantry_winch_draghead_settings(GantryWinchDragHeadSettings setting, bool ps);
	unsigned int AO_drag_pipes_settings(DragPipesSettings setting, bool ps);
	unsigned int AO_doors_settings(DoorsSettings setting, bool ps);
	unsigned int AO_shore_discharge_settings(ShoreDischargeSettings setting, bool placeholder_for_useless_args);

	unsigned int AO_other_settings(OtherSettings setting, bool placeholder_for_useless_args);
}
