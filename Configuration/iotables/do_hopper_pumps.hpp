#pragma once

#include "plc.hpp"

#include "graphlet/symbol/pump/hydraulic_pumplet.hpp"

namespace WarGrey::SCADA {
	private enum class GearboxLubricator { Master, Spare, _ };

	private enum class PSHopperPumpChargeAction { Prepare, Start, Stop, Reset, PSHopper, BothHopper, _ };
	private enum class SBHopperPumpChargeAction { Prepare, Start, Stop, Reset, SBHopper, BothHopper, HPBarge, _ };
	private enum class PSUnderWaterPumpChargeAction { Prepare, Start, Stop, Reset, PSUnderWater, BothUnderWater, _ };
	private enum class SBUnderWaterPumpChargeAction { Prepare, Start, Stop, Reset, SBUnderWater, BothUnderWater, UWPBarge, _ };
	private enum class PSHopperPumpDischargeAction { Prepare, Start, Stop, Reset, PSShoring, PSRainbowing, BothShoring, BothRainbowing, _ };
	private enum class SBHopperPumpDischargeAction { Prepare, Start, Stop, Reset, SBShoring, SBRainbowing, BothShoring, BothRainbowing, _ };

	private enum class HopperGroup { ChargeCondition, DischargeCondition, _ };
	private enum class GroupChargeAction { PSHopper, SBHopper, HPBarge, BothHopper, PSUnderWater, SBUnderWater, UWPBarge, BothUnderWater, _ };
	private enum class GroupDischargeAction { BothShoring, BothRainbowing, _ };

	private enum class GlandPumpAction { Start, Stop, Reset, Auto, Diagnostics, _ };
	private enum class LubricationUnitAction { Start, Stop, _ };
	private enum class GearboxLubricatorAction { Start, Stop, Cancel, Auto, _ };

	typedef uint16(*gland_pump_action_f)(WarGrey::SCADA::GlandPumpAction, WarGrey::SCADA::HydraulicPumplet*);
	typedef void(*gland_pump_diagnostics_f)(WarGrey::SCADA::HydraulicPumplet*, WarGrey::SCADA::PLCMaster*);

	Windows::UI::Xaml::Controls::MenuFlyout^ make_gland_pump_menu(WarGrey::SCADA::gland_pump_action_f gpc,
		WarGrey::SCADA::gland_pump_diagnostics_f gpd, WarGrey::SCADA::PLCMaster* plc);

	Windows::UI::Xaml::Controls::MenuFlyout^ make_lubrication_unit_menu(WarGrey::SCADA::PLCMaster* plc);
	Windows::UI::Xaml::Controls::MenuFlyout^ make_gearbox_lubricator_menu(WarGrey::SCADA::PLCMaster* plc);

	Windows::UI::Xaml::Controls::MenuFlyout^ make_charge_condition_menu(WarGrey::SCADA::GroupChargeAction action, WarGrey::SCADA::PLCMaster* plc);
	Windows::UI::Xaml::Controls::MenuFlyout^ make_ps_hopper_pump_charge_menu(WarGrey::SCADA::PLCMaster* plc);
	Windows::UI::Xaml::Controls::MenuFlyout^ make_sb_hopper_pump_charge_menu(WarGrey::SCADA::PLCMaster* plc);
	Windows::UI::Xaml::Controls::MenuFlyout^ make_ps_underwater_pump_charge_menu(WarGrey::SCADA::PLCMaster* plc);
	Windows::UI::Xaml::Controls::MenuFlyout^ make_sb_underwater_pump_charge_menu(WarGrey::SCADA::PLCMaster* plc);

	Windows::UI::Xaml::Controls::MenuFlyout^ make_discharge_condition_menu(WarGrey::SCADA::PLCMaster* plc);
	Windows::UI::Xaml::Controls::MenuFlyout^ make_ps_hopper_pump_discharge_menu(WarGrey::SCADA::PLCMaster* plc);
	Windows::UI::Xaml::Controls::MenuFlyout^ make_sb_hopper_pump_discharge_menu(WarGrey::SCADA::PLCMaster* plc);

	uint16 DO_gate_flushing_pump_command(WarGrey::SCADA::GlandPumpAction cmd, bool ps);
	uint16 DO_gland_pump_command(WarGrey::SCADA::GlandPumpAction cmd, bool ps, bool hopper, bool master);

	unsigned int DO_hopper_pump_reset_command(bool ps, bool hopper);
}
