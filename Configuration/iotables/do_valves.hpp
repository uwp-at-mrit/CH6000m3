#pragma once

#include "plc.hpp"

#include "graphlet/symbol/valve/gate_valvelet.hpp"

namespace WarGrey::SCADA {
	private enum class GateValveAction {
		Open, Close, VirtualOpen, VirtualClose,
		StopGateValves, CloseGateValves,
		MOpen, MClose, MVirtualOpen, MVirtualClose, MHeat,
		_
	};

	private enum class ButterflyValveAction {
		Open, Close, VirtualOpen, VirtualClose,
		StopButterflyValves, CloseButterflyValves,
		_ 
	};

	typedef uint16(*gate_valve_action_f)(WarGrey::SCADA::GateValveAction, WarGrey::SCADA::GateValvelet*);
	typedef uint16(*butterfly_valve_action_f)(WarGrey::SCADA::ButterflyValveAction, WarGrey::SCADA::GateValvelet*);

	Windows::UI::Xaml::Controls::MenuFlyout^ make_gate_valve_menu(WarGrey::SCADA::gate_valve_action_f gva, WarGrey::SCADA::PLCMaster* plc);
	Windows::UI::Xaml::Controls::MenuFlyout^ make_butterfly_valve_menu(WarGrey::SCADA::butterfly_valve_action_f bfva, WarGrey::SCADA::PLCMaster* plc);

	// DB300, starts from 1
	static unsigned int close_all_gate_valves = 319U;
	static unsigned int stop_all_gate_valves = 805U;

	static unsigned int close_all_butterfly_valves = 303U;
	static unsigned int stop_all_butterfly_valves = 806U;

	template<typename E>
	uint16 DO_gate_valve_command(WarGrey::SCADA::GateValveAction cmd, E id) {
		int16 offset = -1;
		uint16 gindex = 0U;
		uint16 mindex = 0U;
		uint16 heat = 0U;
		uint16 condition = 0U;
		bool motor = false;

		switch (cmd) {
		case GateValveAction::Open:            offset = 0U; break;
		case GateValveAction::Close:           offset = 1U; break;
		case GateValveAction::VirtualOpen:     offset = 2U; break;
		case GateValveAction::VirtualClose:    offset = 3U; break;
		case GateValveAction::CloseGateValves: condition = close_all_gate_valves; break;
		case GateValveAction::StopGateValves:  condition = stop_all_gate_valves; break;
		case GateValveAction::MOpen:           offset = 0U; motor = true; break;
		case GateValveAction::MClose:          offset = 1U; motor = true; break;
		case GateValveAction::MVirtualOpen:    offset = 2U; motor = true; break;
		case GateValveAction::MVirtualClose:   offset = 3U; motor = true; break;
		case GateValveAction::MHeat:           motor = true; break;
		}

		switch (id) {
		case E::D001: gindex = 73U;  mindex = 1017U; heat = 1121U; break;
		case E::D002: gindex = 85U;  mindex = 1029U; heat = 1124U; break;
		case E::D003: gindex = 81U;  mindex = 1025U; heat = 1123U; break;
		case E::D004: gindex = 77U;  mindex = 1021U; heat = 1122U; break;
		case E::D005: gindex = 89U;  mindex = 1033U; heat = 1125U; break;
		case E::D006: gindex = 93U;  mindex = 1037U; heat = 1126U; break;
		case E::D007: gindex = 97U;  mindex = 1041U; heat = 1127U; break;
		case E::D008: gindex = 101U; mindex = 1045U; heat = 1128U; break;
		case E::D009: gindex = 105U; mindex = 1049U; heat = 1129U; break;
		case E::D010: gindex = 109U; mindex = 1053U; heat = 1130U; break;
		case E::D011: gindex = 113U; mindex = 1057U; heat = 1131U; break;
		case E::D012: gindex = 117U; mindex = 1061U; heat = 1132U; break;
		case E::D013: gindex = 121U; mindex = 1065U; heat = 1133U; break;
		case E::D014: gindex = 125U; mindex = 1069U; heat = 1134U; break;
		case E::D015: gindex = 129U; mindex = 1073U; heat = 1135U; break;
		case E::D016: gindex = 133U; mindex = 1077U; heat = 1136U; break;
		case E::D017: gindex = 137U; mindex = 1081U; heat = 1137U; break;
		case E::D018: gindex = 141U; mindex = 1085U; heat = 1138U; break;
		case E::D019: gindex = 145U; mindex = 1089U; heat = 1139U; break;
		case E::D020: gindex = 149U; mindex = 1093U; heat = 1140U; break;
		case E::D021: gindex = 153U; mindex = 1097U; heat = 1141U; break;
		case E::D022: gindex = 157U; mindex = 1101U; heat = 1142U; break;
		case E::D023: gindex = 161U; mindex = 1105U; heat = 1143U; break;
		case E::D024: gindex = 165U; mindex = 1109U; heat = 1144U; break;
		case E::D025: gindex = 169U; mindex = 1113U; heat = 1145U; break;
		case E::D026: gindex = 173U; mindex = 1117U; heat = 1146U; break;
		}

		return (motor
			? ((offset >= 0) ? (mindex + offset) : heat)
			: ((condition > 0) ? condition : (gindex + offset)));
	}

	template<typename E>
	uint16 DO_butterfly_valve_command(ButterflyValveAction cmd, E id) {
		uint16 offset = 0U;
		uint16 index = 0U;
		uint16 condition = 0U;

		switch (cmd) {
		case ButterflyValveAction::Open:                 offset = 0U; break;
		case ButterflyValveAction::Close:                offset = 1U; break;
		case ButterflyValveAction::VirtualOpen:          offset = 2U; break;
		case ButterflyValveAction::VirtualClose:         offset = 3U; break;
		case ButterflyValveAction::CloseButterflyValves: condition = close_all_butterfly_valves; break;
		case ButterflyValveAction::StopButterflyValves:  condition = stop_all_butterfly_valves; break;
		}

		switch (id) {
		case E::HBV01: index = 185U; break;
		case E::HBV02: index = 189U; break;
		case E::HBV03: index = 193; break;
		case E::HBV04: index = 197U; break;
		case E::HBV05: index = 201U; break;
		case E::HBV06: index = 205U; break;
		case E::HBV07: index = 209U; break;
		case E::HBV08: index = 213U; break;
		case E::HBV09: index = 217U; break;
		case E::HBV10: index = 221U; break;
		case E::HBV11: index = 225U; break;
		case E::HBV12: index = 229U; break;
		case E::HBV13: index = 233U; break;
		case E::HBV14: index = 237U; break;
		case E::HBV15: index = 241U; break;
		case E::HBV16: index = 245U; break;
		case E::HBV17: index = 249U; break;
		case E::HBV18: index = 253U; break;
		}

		return (condition > 0) ? condition : (index + offset);
	}
}
