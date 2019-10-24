#pragma once

#include "plc.hpp"

#include "graphlet/symbol/pump/hydraulic_pumplet.hpp"

namespace WarGrey::SCADA {
	private enum class HydraulicsGroup { BothPumps, PSPumps, SBPumps, VisorPumps };
	private enum class HydraulicsGroupAction { Start, Stop, Cancel, _ };
	private enum class HydraulicPumpAction { Start, Stop, Reset, Diagnostics, _ };

	typedef uint16(*hydraulic_pump_action_f)(WarGrey::SCADA::HydraulicPumpAction, WarGrey::SCADA::HydraulicPumplet*);
	typedef void(*hydraulic_pump_diagnostics_f)(WarGrey::SCADA::HydraulicPumplet*, WarGrey::SCADA::PLCMaster*);

	Windows::UI::Xaml::Controls::MenuFlyout^ make_hydraulics_group_menu(WarGrey::SCADA::HydraulicsGroup group, WarGrey::SCADA::PLCMaster* plc);
	Windows::UI::Xaml::Controls::MenuFlyout^ make_hydraulic_pump_menu(WarGrey::SCADA::hydraulic_pump_action_f hpa,
		WarGrey::SCADA::hydraulic_pump_diagnostics_f hpd, WarGrey::SCADA::PLCMaster* plc);

	// DB300, starts from 1
	template<typename E>
	uint16 DO_hydraulic_pump_command(HydraulicPumpAction cmd, E id) {
		uint16 offset = 0U;
		uint16 index = 0U;

		switch (cmd) {
		case HydraulicPumpAction::Start: offset = 0U; break;
		case HydraulicPumpAction::Stop:  offset = 1U; break;
		case HydraulicPumpAction::Reset: offset = 2U; break;
		}

		switch (id) {
		case E::A: index = 9U; break;
		case E::B: index = 12U; break;
		case E::C: index = 15U; break;
		case E::D: index = 18U; break;
		case E::E: index = 21U; break;
		case E::F: index = 24U; break;
		case E::G: index = 27U; break;
		case E::H: index = 30U; break;
		case E::I: index = 33U; break;
		case E::J: index = 36U; break;
		case E::K: index = 39U; break;
		case E::L: index = 42U; break;
		case E::M: index = 45U; break;
		case E::Y: index = 48U; break;
		}

		return index + offset;
	}
}
