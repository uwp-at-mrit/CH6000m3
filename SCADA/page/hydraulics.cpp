#include <map>

#include "page/hydraulics.hpp"
#include "page/diagnostics/hydraulic_pump_dx.hpp"

#include "configuration.hpp"
#include "menu.hpp"

#include "module.hpp"
#include "brushes.hxx"
#include "turtle.hpp"

#include "graphlet/shapelet.hpp"
#include "graphlet/buttonlet.hpp"
#include "graphlet/statuslet.hpp"

#include "graphlet/symbol/heaterlet.hpp"
#include "graphlet/symbol/pump/hydraulic_pumplet.hpp"
#include "graphlet/symbol/valve/manual_valvelet.hpp"
#include "graphlet/device/tanklet.hpp"
#include "graphlet/dashboard/fueltanklet.hpp"
#include "graphlet/dashboard/thermometerlet.hpp"
#include "graphlet/dashboard/alarmlet.hpp"

#include "iotables/ai_metrics.hpp"
#include "iotables/ai_pumps.hpp"

#include "iotables/di_valves.hpp"
#include "iotables/di_pumps.hpp"
#include "iotables/di_devices.hpp"

#include "iotables/do_pumps.hpp"
#include "iotables/do_devices.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::UI::Xaml::Controls;
using namespace Windows::Foundation::Numerics;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI;
using namespace Microsoft::Graphics::Canvas::Text;
using namespace Microsoft::Graphics::Canvas::Brushes;

private enum class HSFunction { BOPOverride, _ };

private enum class HSMTState { Empty, UltraLow, Low, Normal, High, Full, _ };

static CanvasSolidColorBrush^ oil_color = Colours::Yellow;

// WARNING: order matters
private enum class HS : unsigned int {
	// Pumps
	A, B, G, H,
	C, F, D, E,
	J, I,
	Y, L, M, K,

	// Valves
	SQ1, SQ2, SQy, SQl, SQm, SQi, SQj,
	SQa, SQb, SQg, SQh, SQk2, SQk1,
	SQc, SQf, SQd, SQe,
	
	// Key Labels
	Port, Starboard, Master, Visor, VisorState, Heater, Storage, VisorOil,

	// Pump-driven subsystems
	PSTrunnion, PSGateValves, ShoreDischarge, BowAnchor,
	PSDraghead, PSDoors, SternAnchor, Overflow, Barge, PSCompensator,
	SBTrunnion, SBGateValves, PSIntermediate, ButterflyValves, SBIntermediate,
	DoorsLocking, SBDraghead, SBDoors, WateringValves, SBCompensator,

	// Dimensions
	BackOil,

	// Pump Replacement Indicators
	A2C, B2C, F2C, H2F, G2F, I2J,
	C2A, C2B, C2F, F2H, F2G, J2I,

	// Filter Indicators
	F01, F02, F10,
	
	_,
	
	// anchors used as last jumping points
	a, b, c, d, e, f, g, h, y, l, m, k, k12,

	// anchors used for unnamed corners
	lt, tl, rt, tr, cl, cr, i, j, f02, master, sb
};

static HS A[] = { HS::PSDraghead, HS::PSDoors, HS::SternAnchor, HS::Overflow, HS::Barge, HS::PSCompensator };
static HS B[] = { HS::PSIntermediate };
static HS G[] = { HS::SBIntermediate };
static HS H[] = { HS::SBDraghead, HS::SBDoors, HS::WateringValves, HS::SBCompensator };

static HS C[] = { HS::PSTrunnion, HS::PSGateValves, HS::ShoreDischarge, HS::BowAnchor };
static HS F[] = { HS::SBTrunnion, HS::SBGateValves };
static HS D[] = { HS::ButterflyValves };
static HS E[] = { HS::DoorsLocking };

/*************************************************************************************************/
static HydraulicPumpDiagnostics* satellite; // it will be destroyed by `atexit()`;

static void hydraulics_diagnostics(HydraulicPumplet* pump, PLCMaster* plc) {
	auto credit_pump = dynamic_cast<Credit<HydraulicPumplet, HS>*>(pump);

	if (credit_pump != nullptr) {
		HPDX group = HPDX::Other;
		unsigned int feedback = 0U;
		unsigned int details = 0U;

		if (satellite == nullptr) {
			satellite = new HydraulicPumpDiagnostics(plc);
		}

		switch (credit_pump->id) {
		case HS::A: feedback = pump_A_feedback; group = HPDX::SB; details = pump_A_status; break;
		case HS::B: feedback = pump_B_feedback; group = HPDX::SB; details = pump_B_status; break;
		case HS::G: feedback = pump_G_feedback; group = HPDX::SB; details = pump_G_status; break;
		case HS::H: feedback = pump_H_feedback; group = HPDX::SB; details = pump_H_status; break;

		case HS::C: feedback = pump_C_feedback; group = HPDX::PS; details = pump_C_status; break;
		case HS::F: feedback = pump_F_feedback; group = HPDX::PS; details = pump_F_status; break;
		case HS::D: feedback = pump_D_feedback; group = HPDX::PS; details = pump_D_status; break;
		case HS::E: feedback = pump_E_feedback; group = HPDX::PS; details = pump_E_status; break;

		case HS::I: feedback = pump_I_feedback; group = HPDX::Visor; details = pump_I_status; break;
		case HS::J: feedback = pump_J_feedback; group = HPDX::Visor; details = pump_J_status; break;

		case HS::Y: feedback = pump_Y_feedback; group = HPDX::Other; details = pump_Y_status; break;
		case HS::L: feedback = pump_L_feedback; group = HPDX::Other; details = pump_L_status; break;
		case HS::M: feedback = pump_M_feedback; group = HPDX::Other; details = pump_M_status; break;
		case HS::K: feedback = pump_K_feedback; group = HPDX::Other; details = pump_K_status; break;
		}
		
		// WARNING: `set_pump()`ing before `switch_id()`ing. 
		satellite->set_pump(credit_pump->id.ToString(), group, details);
		satellite->switch_id(feedback);
		satellite->show();
	}
}

static uint16 DO_hydraulics_action(HydraulicPumpAction cmd, HydraulicPumplet* pump) {
	auto credit_pump = dynamic_cast<Credit<HydraulicPumplet, HS>*>(pump);
	uint16 index = 0U;

	if (credit_pump != nullptr) {
		index = DO_hydraulic_pump_command(cmd, credit_pump->id);
	}

	return index;
}

static const HS* select_captions(HS pid, unsigned int* count) {
	HS* captions = nullptr;

	switch (pid) {
	case HS::A: captions = A; SET_BOX(count, sizeof(A) / sizeof(HS)); break;
	case HS::B: captions = B; SET_BOX(count, sizeof(B) / sizeof(HS)); break;
	case HS::G: captions = G; SET_BOX(count, sizeof(G) / sizeof(HS)); break;
	case HS::H: captions = H; SET_BOX(count, sizeof(H) / sizeof(HS)); break;

	case HS::C: captions = C; SET_BOX(count, sizeof(C) / sizeof(HS)); break;
	case HS::F: captions = F; SET_BOX(count, sizeof(F) / sizeof(HS)); break;
	case HS::D: captions = D; SET_BOX(count, sizeof(D) / sizeof(HS)); break;
	case HS::E: captions = E; SET_BOX(count, sizeof(E) / sizeof(HS)); break;

	default: SET_BOX(count, 0);
	}

	return captions;
}

/*************************************************************************************************/
private class Hydraulics final : public PLCConfirmation {
public:
	Hydraulics(HydraulicsPage* master) : master(master) {}

public:
	void pre_read_data(Syslog* logger) override {
		this->master->enter_critical_section();
		this->master->begin_update_sequence();

		this->station->clear_subtacks();
	}

	void on_analog_input(long long timepoint_ms, const uint8* DB2, size_t count2, const uint8* DB203, size_t count203, Syslog* logger) override {
		this->set_temperature(HS::Visor, RealData(DB203, visor_tank_temperature));
		this->set_temperature(HS::Master, RealData(DB203, master_tank_temperature));
		this->set_visor_tank_level(RealData(DB203, visor_tank_level));

		this->pressures[HS::BackOil]->set_value(RealData(DB203, master_back_oil_pressure));
		
		{ // pump pressures
			GraphletAnchor psa = GraphletAnchor::LB;
			GraphletAnchor sba = GraphletAnchor::RB;

			this->pressures[HS::C]->set_value(RealData(DB203, pump_C_pressure), psa);
			this->pressures[HS::F]->set_value(RealData(DB203, pump_F_pressure), psa);
			this->pressures[HS::D]->set_value(RealData(DB203, pump_D_pressure), psa);
			this->pressures[HS::E]->set_value(RealData(DB203, pump_E_pressure), psa);

			this->pressures[HS::A]->set_value(RealData(DB203, pump_A_pressure), sba);
			this->pressures[HS::B]->set_value(RealData(DB203, pump_B_pressure), sba);
			this->pressures[HS::G]->set_value(RealData(DB203, pump_G_pressure), sba);
			this->pressures[HS::H]->set_value(RealData(DB203, pump_H_pressure), sba);

			this->pressures[HS::I]->set_value(RealData(DB203, pump_I_pressure), GraphletAnchor::LB);
			this->pressures[HS::J]->set_value(RealData(DB203, pump_J_pressure), GraphletAnchor::RB);
		}
	}

	void on_digital_input(long long timepoint_ms, const uint8* DB4, size_t count4, const uint8* DB205, size_t count205, Syslog* logger) override {
		DI_backoil_pressure_override(this->functions[HSFunction::BOPOverride], DB205, backoil_pressure_override_status);

		{ // tank states
			DI_tank_heater(this->heater, DB4, master_tank_heater_feedback, DB205, tank_heater_status);

			DI_master_tank(this->master_tank, DB4, master_tank_status);
			DI_visor_tank(this->visor_tank, DB4, visor_tank_status);

			switch (this->visor_tank->get_state()) {
			case TankState::Low: {
				this->labels[HS::VisorState]->set_color(Colours::Yellow);
				this->labels[HS::VisorState]->set_text("[" + speak(TankState::Low) + "]");
			}; break;
			case TankState::UltraLow: {
				this->labels[HS::VisorState]->set_color(Colours::Red);
				this->labels[HS::VisorState]->set_text("[" + speak(TankState::UltraLow) + "]");
			}; break;
			default: {
				this->labels[HS::VisorState]->set_text("");
			}
			}
		}
		
		{ // pump states
			DI_hydraulic_pump(this->pumps[HS::A], DB4, pump_A_feedback, DB205, pump_A_status);
			DI_hydraulic_pump(this->pumps[HS::B], DB4, pump_B_feedback, DB205, pump_B_status);
			DI_hydraulic_pump(this->pumps[HS::G], DB4, pump_G_feedback, DB205, pump_G_status);
			DI_hydraulic_pump(this->pumps[HS::H], DB4, pump_H_feedback, DB205, pump_H_status);
			
			DI_hydraulic_pump(this->pumps[HS::C], DB4, pump_C_feedback, DB205, pump_C_status);
			DI_hydraulic_pump(this->pumps[HS::F], DB4, pump_F_feedback, DB205, pump_F_status);
			DI_hydraulic_pump(this->pumps[HS::D], DB4, pump_D_feedback, DB205, pump_D_status);
			DI_hydraulic_pump(this->pumps[HS::E], DB4, pump_E_feedback, DB205, pump_E_status);
			
			DI_hydraulic_pump(this->pumps[HS::Y], DB4, pump_Y_feedback, DB205, pump_Y_status);
			DI_hydraulic_pump(this->pumps[HS::L], DB4, pump_L_feedback, DB205, pump_L_status);
			DI_hydraulic_pump(this->pumps[HS::M], DB4, pump_M_feedback, DB205, pump_M_status);
			DI_hydraulic_pump(this->pumps[HS::K], DB4, pump_K_feedback, DB205, pump_K_status);

			DI_hydraulic_pump(this->pumps[HS::I], DB4, pump_I_feedback, DB205, pump_I_status);
			DI_hydraulic_pump(this->pumps[HS::J], DB4, pump_J_feedback, DB205, pump_J_status);
		}

		{ // valve statuses
			DI_manual_valve(this->valves[HS::SQ1], DB4, manual_valve_SQ1_status);
			DI_manual_valve(this->valves[HS::SQ2], DB4, manual_valve_SQ2_status);

			DI_manual_valve(this->valves[HS::SQk1], DB4, manual_valve_SQk1_status);
			DI_manual_valve(this->valves[HS::SQk2], DB4, manual_valve_SQk2_status);
			DI_manual_valve(this->valves[HS::SQl], DB4, manual_valve_SQl_status);
			DI_manual_valve(this->valves[HS::SQm], DB4, manual_valve_SQm_status);
			DI_manual_valve(this->valves[HS::SQy], DB4, manual_valve_SQy_status);

			DI_manual_valve(this->valves[HS::SQi], DB4, manual_valve_SQi_status);
			DI_manual_valve(this->valves[HS::SQj], DB4, manual_valve_SQj_status);

			DI_manual_valve(this->valves[HS::SQc], DB4, manual_valve_SQc_status);
			DI_manual_valve(this->valves[HS::SQd], DB4, manual_valve_SQd_status);
			DI_manual_valve(this->valves[HS::SQe], DB4, manual_valve_SQe_status);
			DI_manual_valve(this->valves[HS::SQf], DB4, manual_valve_SQf_status);

			DI_manual_valve(this->valves[HS::SQa], DB4, manual_valve_SQa_status);
			DI_manual_valve(this->valves[HS::SQb], DB4, manual_valve_SQb_status);
			DI_manual_valve(this->valves[HS::SQg], DB4, manual_valve_SQc_status);
			DI_manual_valve(this->valves[HS::SQh], DB4, manual_valve_SQd_status);
		}

		{ // filter statuses
			DI_alarm(this->alarms[HS::F01], DB4, filter_01_status);
			DI_alarm(this->alarms[HS::F02], DB4, filter_02_status);
			DI_alarm(this->alarms[HS::F10], DB4, filter_10_status);

			DI_alarm(this->alarms[HS::A2C], DB4, pump_A_replace_C, AlarmState::Notice);
			DI_alarm(this->alarms[HS::B2C], DB4, pump_B_replace_C, AlarmState::Notice);
			DI_alarm(this->alarms[HS::F2C], DB4, pump_F_replace_C, AlarmState::Notice);
			DI_alarm(this->alarms[HS::H2F], DB4, pump_H_replace_F, AlarmState::Notice);
			DI_alarm(this->alarms[HS::G2F], DB4, pump_G_replace_F, AlarmState::Notice);
			DI_alarm(this->alarms[HS::I2J], DB4, pump_I_replace_J, AlarmState::Notice);

			DI_alarm(this->alarms[HS::C2A], DB4, pump_C_replace_A, AlarmState::Notice);
			DI_alarm(this->alarms[HS::C2B], DB4, pump_C_replace_B, AlarmState::Notice);
			DI_alarm(this->alarms[HS::C2F], DB4, pump_C_replace_F, AlarmState::Notice);
			DI_alarm(this->alarms[HS::F2H], DB4, pump_F_replace_H, AlarmState::Notice);
			DI_alarm(this->alarms[HS::F2G], DB4, pump_F_replace_G, AlarmState::Notice);
			DI_alarm(this->alarms[HS::J2I], DB4, pump_J_replace_I, AlarmState::Notice);
		}
	}

	void post_read_data(Syslog* logger) override {
		{ // flow oil
			HS ps_path[] = { HS::lt, HS::tl, HS::cl, HS::Master };
			HS sb_path[] = { HS::rt, HS::tr, HS::cr, HS::Master };
			HS mt_path[] = { HS::f02, HS::master };
			
			this->station->push_subtrack(HS::Master, HS::SQ1, oil_color);
			this->station->push_subtrack(HS::Master, HS::SQ2, oil_color);
			this->station->push_subtrack(HS::Visor, HS::SQi, oil_color);
			this->station->push_subtrack(HS::Visor, HS::SQj, oil_color);
			this->station->push_subtrack(HS::Storage, HS::SQk1, oil_color);

			this->try_flow_oil(HS::SQi, HS::I, HS::i, nullptr, 0, oil_color);
			this->try_flow_oil(HS::SQj, HS::J, HS::j, nullptr, 0, oil_color);

			this->try_flow_oil(HS::SQc, HS::C, HS::c, ps_path, oil_color);
			this->try_flow_oil(HS::SQd, HS::D, HS::d, ps_path, oil_color);
			this->try_flow_oil(HS::SQe, HS::E, HS::e, ps_path, oil_color);
			this->try_flow_oil(HS::SQf, HS::F, HS::f, ps_path, oil_color);

			this->try_flow_oil(HS::SQa, HS::A, HS::a, sb_path, oil_color);
			this->try_flow_oil(HS::SQb, HS::B, HS::b, sb_path, oil_color);
			this->try_flow_oil(HS::SQg, HS::G, HS::g, sb_path, oil_color);
			this->try_flow_oil(HS::SQh, HS::H, HS::h, sb_path, oil_color);

			this->try_flow_oil(HS::SQy, HS::Y, HS::y, mt_path, oil_color);
			this->try_flow_oil(HS::SQl, HS::L, HS::l, mt_path, oil_color);
			this->try_flow_oil(HS::SQm, HS::M, HS::m, mt_path, oil_color);
			this->try_flow_oil(HS::SQk1, HS::K, HS::k, mt_path, oil_color);
			this->try_flow_oil(HS::SQk2, HS::K /* , HS::k, mt_path */, oil_color);

			this->try_flow_oil(HS::SQ2, HS::Port, HS::SQe, oil_color);
			this->try_flow_oil(HS::SQ1, HS::sb, HS::SQh, oil_color);
			this->try_flow_oil(HS::SQ1, HS::SQk2, oil_color);
			this->try_flow_oil(HS::SQ1, HS::SQm, oil_color);
		}
		
		this->master->end_update_sequence();
		this->master->leave_critical_section();
	}

public:
	void construct(float gwidth, float gheight) {
		this->caption_font = make_bold_text_format("Microsoft YaHei", large_font_size);
		this->label_font = make_bold_text_format("Microsoft YaHei", small_font_size);

		this->button_style.font = make_bold_text_format("Consolas", 14.0F);
		this->button_style.corner_radius = 2.0F;
		this->button_style.thickness = 2.0F;

		this->fixnum_style.number_font = make_bold_text_format("Cambria Math", large_metrics_font_size);
		this->fixnum_style.unit_font = make_bold_text_format("Cambria", normal_font_size);
		this->flonum_style = this->fixnum_style;

		this->fixnum_style.precision = 0;
		this->flonum_style.precision = 1;
	}
 
public:
	void load_pump_station(float width, float height, float gwidth, float gheight) {
		Turtle<HS>* pTurtle = new Turtle<HS>(gwidth, gheight, true, HS::Master);

		pTurtle->move_right(2)->move_down(5.5F, HS::SQ1)->move_down()->turn_down_right();
		pTurtle->move_right(13, HS::sb)->turn_right_down()->move_down(17);
		
		pTurtle->jump_right(20, HS::h)->move_left(4, HS::H)->move_left(12, HS::SQh)->move_left(4)->jump_back();
		pTurtle->move_up(3, HS::g)->move_left(4, HS::G)->move_left(12, HS::SQg)->move_left(4)->jump_back();
		pTurtle->move_up(3, HS::b)->move_left(4, HS::B)->move_left(12, HS::SQb)->move_left(4)->jump_back();
		pTurtle->move_up(3, HS::a)->move_left(4, HS::A)->move_left(12, HS::SQa)->move_left(4)->jump_back();
		
		pTurtle->move_up(3, HS::Starboard)->move_up(21, HS::rt)->turn_up_left(HS::tr)->move_left(35, HS::cr);
		pTurtle->turn_left_down()->move_down(4)->jump_up(4);
		pTurtle->turn_up_left(HS::cl)->move_left(35, HS::tl)->turn_left_down(HS::lt)->move_down(21);

		pTurtle->move_down(3, HS::c)->move_right(4, HS::C)->move_right(12, HS::SQc)->move_right(4)->jump_back();
		pTurtle->move_down(3, HS::f)->move_right(4, HS::F)->move_right(12, HS::SQf)->move_right(4)->jump_back();
		pTurtle->move_down(3, HS::d)->move_right(4, HS::D)->move_right(12, HS::SQd)->move_right(4)->jump_back();
		pTurtle->move_down(3, HS::e)->move_right(4, HS::E)->move_right(12, HS::SQe)->move_right(4);

		pTurtle->move_up(12, HS::Port)->move_up(5)->turn_up_right()->move_right(13)->turn_right_up();
		pTurtle->move_up(HS::SQ2)->move_up(5.5F)->move_to(HS::Master);

		pTurtle->jump_back(HS::Master)->jump_right(4, HS::master)->move_up(6.5F)->turn_up_right(HS::f02)->move_right(2);
		pTurtle->move_right(5, HS::y)->move_down(6, HS::Y)->move_down(4, HS::SQy)->move_down(4)->turn_down_left()->jump_back();
		pTurtle->move_right(5, HS::l)->move_down(6, HS::L)->move_down(4, HS::SQl)->move_down(4)->turn_down_left()->jump_back();
		pTurtle->move_right(5, HS::m)->move_down(6, HS::M)->move_down(4, HS::SQm)->move_down(4)->turn_down_left()->jump_back();
		pTurtle->move_right(4, HS::k)->turn_right_down()->move_down(5, HS::K)->move_down(9, HS::k12);
		pTurtle->move_left(3, HS::SQk2)->move_left(8)->jump_back()->move_right(3, HS::SQk1)->move_right(2.5F, HS::Storage);
		
		pTurtle->jump_back(HS::Master)->jump_down(14, HS::Visor);
		pTurtle->move_right(2)->move_down(5, HS::SQi)->move_down(3, HS::I)->move_down(3, HS::i);
		pTurtle->jump_left(4, HS::j)->move_up(3, HS::J)->move_up(3, HS::SQj)->move_up(5)->move_right(2 /* HS::Visor */);
		
		this->station = this->master->insert_one(new Tracklet<HS>(pTurtle, default_pipe_thickness, default_pipe_color));
		
		this->load_label(this->captions, HS::Port, Colours::DarkKhaki, this->caption_font);
		this->load_label(this->captions, HS::Starboard, Colours::DarkKhaki, this->caption_font);
		this->load_label(this->captions, HS::Storage, Colours::Silver);

		this->load_dimension(this->pressures, HS::BackOil, "bar");

		this->load_buttons(this->functions, 48.0F, 24.0F);
	}

	void load_tanks(float width, float height, float gwidth, float gheight) {
		float thickness = default_pipe_thickness * 2.0F;
		float alarm_size = gwidth * 1.2F;

		this->master_tank = this->make_tank(HSMTState::Empty, gwidth * 20.0F, gheight * 8.0F, thickness);
		this->visor_tank = this->master->insert_one(new Tanklet(drag_visor_tank_range, gwidth * 18.0F, gheight * 7.0F, 8U, thickness));
		
		this->heater = this->master->insert_one(new Heaterlet(gwidth * 1.618F));
		this->load_label(this->labels, HS::Heater, Colours::Salmon);

		this->load_thermometer(this->thermometers, this->temperatures, HS::Master, gwidth * 2.5F, gheight * 4.5F);
		this->load_thermometer(this->thermometers, this->temperatures, HS::Visor, gwidth * 2.5F, gheight * 4.5F);
		this->load_dimension(this->levels, HS::VisorOil, "centimeter");
		this->load_label(this->labels, "", HS::VisorState, Colours::Silver, this->label_font);

		this->storage_tank = this->master->insert_one(new FuelTanklet(gwidth * 2.5F, 0.0F, thickness, Colours::WhiteSmoke));

		this->load_alarms(HS::A2C, HS::I2J, alarm_size, this->alarms, this->alabels);
		this->load_alarms(HS::C2A, HS::J2I, alarm_size, this->alarms, this->alabels);
		this->load_alarms(HS::F01, HS::F10, alarm_size, this->alarms, this->alabels);
	}

	void load_devices(float width, float height, float gwidth, float gheight) {
		float radius = resolve_gridsize(gwidth, gheight);
		float pradius = radius * 1.2F;

		{ // load pumps
			this->load_devices(this->pumps, this->labels, this->captions, HS::A, HS::H, pradius, 0.000, Colours::Salmon);
			this->load_devices(this->pumps, this->labels, this->captions, HS::C, HS::E, pradius, 180.0, Colours::Salmon);
			this->load_devices(this->pumps, this->labels, this->captions, HS::Y, HS::K, pradius, -90.0, Colours::Salmon);
			this->load_devices(this->pumps, this->labels, this->captions, HS::J, HS::I, pradius, 90.00, Colours::Salmon);

			this->load_dimensions(this->pressures, HS::A, HS::I, "bar");
		}

		{ // load valves
			this->load_devices(this->valves, this->labels, HS::SQ1, HS::SQj, radius, 90.000);
			this->load_devices(this->valves, this->labels, HS::SQa, HS::SQk1, radius, 0.000);
			this->load_devices(this->valves, this->labels, HS::SQc, HS::SQe, radius,  0.000);
		}
	}

public:
	void reflow_pump_station(float width, float height, float gwidth, float gheight) {
		float cx = width * 0.5F;
		float cy = height * 0.5F;
		float sq1_y;

		this->master->move_to(this->station, cx, cy, GraphletAnchor::CC);
		this->station->map_graphlet_at_anchor(this->storage_tank, HS::Storage, GraphletAnchor::LC);
		this->station->fill_anchor_location(HS::SQ1, nullptr, &sq1_y, true);
		this->station->map_graphlet_at_anchor(this->master_tank, HS::Master, GraphletAnchor::CC);
		this->station->map_graphlet_at_anchor(this->visor_tank, HS::Visor, GraphletAnchor::CC);
		this->master->move_to(this->thermometers[HS::Master], this->master_tank, 0.25F, 0.5F, GraphletAnchor::CC);
		this->master->move_to(this->thermometers[HS::Visor], this->visor_tank, 0.25F, 0.5F, GraphletAnchor::CC);
		
		this->station->map_credit_graphlet(this->captions[HS::Port], GraphletAnchor::CB, -gwidth * 10.0F);
		this->station->map_credit_graphlet(this->captions[HS::Starboard], GraphletAnchor::CB, -gwidth * 10.0F);
		this->master->move_to(this->captions[HS::Storage], this->storage_tank, GraphletAnchor::CB, GraphletAnchor::CT);

		this->master->move_to(this->pressures[HS::BackOil], this->station, GraphletAnchor::CT, GraphletAnchor::CB);
		this->master->move_to(this->functions[HSFunction::BOPOverride],
			this->pressures[HS::BackOil], GraphletAnchor::RB, GraphletAnchor::LB, gwidth);
		
		{ // reflow heater
			float hspace, vspace;

			this->heater->fill_margin(0.0F, 0.0F, &vspace, &hspace);
			this->master->move_to(this->heater, this->master_tank, GraphletAnchor::CB,
				GraphletAnchor::CB, 0.0F, vspace - hspace);

			this->master->move_to(this->labels[HS::Heater], this->master_tank, GraphletAnchor::CB, GraphletAnchor::CT);
		}
	}
	
	void reflow_devices(float width, float height, float gwidth, float gheight) {
		GraphletAnchor lbl_a, cpt_a, bar_a;
		float lbl_dx, lbl_dy, cpt_dx, cpt_dy, bar_dx, bar_dy, margin;
		float pradius = this->pumps[HS::A]->get_radiusX();
		float vradius = this->valves[HS::SQ1]->get_radiusY();
		float text_hspace = default_pipe_thickness * 2.0F;
		float x0 = 0.0F;
		float y0 = 0.0F;

		for (auto it = this->pumps.begin(); it != this->pumps.end(); it++) {
			switch (it->second->id) {
			case HS::F: case HS::C: case HS::D: case HS::E: {
				lbl_dx = x0 - pradius; lbl_dy = y0; lbl_a = GraphletAnchor::RT;
				cpt_dx = x0 + pradius; cpt_dy = y0; cpt_a = GraphletAnchor::LT;
				bar_dx = x0 + pradius; bar_dy = y0; bar_a = GraphletAnchor::LB;
			} break;
			case HS::A: case HS::B: case HS::G: case HS::H: {
				lbl_dx = x0 + pradius; lbl_dy = y0; lbl_a = GraphletAnchor::LT;
				cpt_dx = x0 - pradius; cpt_dy = y0; cpt_a = GraphletAnchor::RT;
				bar_dx = x0 - pradius; bar_dy = y0; bar_a = GraphletAnchor::RB;
			} break;
			case HS::Y: case HS::L: case HS::M: case HS::K: {
				lbl_dx = x0 - pradius; lbl_dy = y0; lbl_a = GraphletAnchor::RB;
				cpt_dx = x0 + text_hspace; cpt_dy = y0 - pradius; cpt_a = GraphletAnchor::LB;
				bar_dx = x0; bar_dy = y0; bar_a = GraphletAnchor::CC; // these devices have no metrics
			} break;
			default: {
				cpt_dx = x0; cpt_dy = y0 + gheight * 3.0F; cpt_a = GraphletAnchor::CT;
			
				if (it->second->id == HS::I) {
					lbl_dx = x0 + pradius; lbl_dy = y0; lbl_a = GraphletAnchor::LT;
					bar_dx = x0 + text_hspace; bar_dy = y0 + pradius; bar_a = GraphletAnchor::LT;
				} else { // HS::J
					lbl_dx = x0 - pradius; lbl_dy = y0; lbl_a = GraphletAnchor::RT;
					bar_dx = x0 - text_hspace; bar_dy = y0 + pradius; bar_a = GraphletAnchor::RT;
				}
			}
			}

			this->station->map_credit_graphlet(it->second, GraphletAnchor::CC, x0, y0);
			this->station->map_credit_graphlet(this->labels[it->first], lbl_a, lbl_dx, lbl_dy);

			if (this->captions.find(it->first) != this->captions.end()) {
				this->station->map_credit_graphlet(this->captions[it->first], cpt_a, cpt_dx, cpt_dy);
			} else {
				unsigned int count = 0U;
				const HS* cpts = select_captions(it->first, &count);
				IGraphlet* target = this->captions[cpts[0]];
				GraphletAnchor tgt_a = ((cpt_a == GraphletAnchor::LT) ? GraphletAnchor::RT : GraphletAnchor::LT);
				float hgapsize = ((cpt_dx > 0.0F) ? text_hspace : -text_hspace);

				this->station->map_graphlet_at_anchor(target, it->first, cpt_a, cpt_dx, cpt_dy);
				for (unsigned int idx = 1; idx < count; idx++) {
					this->master->move_to(this->captions[cpts[idx]], target, tgt_a, cpt_a, hgapsize);
					target = this->captions[cpts[idx]];
				}
			}

			if (this->pressures.find(it->first) != this->pressures.end()) {
				this->station->map_credit_graphlet(this->pressures[it->first], bar_a, bar_dx, bar_dy);
			}
		}

		for (auto it = this->valves.begin(); it != this->valves.end(); it++) {
			if (it->second->get_direction_degrees() == 90.0) {
				switch (it->first) {
				case HS::SQ2: case HS::SQj: {
					it->second->fill_margin(x0, y0, nullptr, nullptr, nullptr, &margin);
					lbl_dx = x0 - vradius + margin; lbl_dy = y0; lbl_a = GraphletAnchor::RC;
				}; break;
				case HS::SQ1: case HS::SQi: default: {
					lbl_dx = x0 + vradius; lbl_dy = y0; lbl_a = GraphletAnchor::LC;
				}; break;
				}
			} else {
				it->second->fill_margin(x0, y0, &margin, nullptr, nullptr, nullptr);
				lbl_dx = x0; lbl_dy = y0 - vradius + margin; lbl_a = GraphletAnchor::CB;
			}

			this->station->map_credit_graphlet(it->second, GraphletAnchor::CC, x0, y0);
			this->station->map_credit_graphlet(this->labels[it->first], lbl_a, lbl_dx, lbl_dy);
		}
	}

	void reflow_metrics(float width, float height, float gwidth, float gheight) {
		this->master->move_to(this->temperatures[HS::Master], this->thermometers[HS::Master], 1.0F, 0.75F, GraphletAnchor::LC, gwidth);
		this->master->move_to(this->temperatures[HS::Visor], this->thermometers[HS::Visor], 1.0F, 0.75F, GraphletAnchor::LB, gwidth);
		this->master->move_to(this->levels[HS::VisorOil], this->thermometers[HS::Visor], 1.0F, 0.75F, GraphletAnchor::LT, gwidth);
		
		{ // reflow alarms
			float lblgap = gheight * 0.25F;
			float vgap = lblgap * 0.5F;
			
			this->master->move_to(this->alarms[HS::F01], this->thermometers[HS::Master], 1.0F, 0.25F, GraphletAnchor::LB, gwidth, -vgap);
			this->master->move_to(this->alarms[HS::F02], this->thermometers[HS::Master], 1.0F, 0.25F, GraphletAnchor::LT, gwidth, +vgap);
			this->master->move_to(this->alarms[HS::F10], this->thermometers[HS::Visor], 1.0F, 0.25F, GraphletAnchor::LC, gwidth);

			this->master->move_to(this->alarms[HS::A2C], this->pumps[HS::C], 0.5F, this->master_tank, 0.0F, GraphletAnchor::LT);
			this->master->move_to(this->alarms[HS::C2A], this->valves[HS::SQc], 0.0F, this->master_tank, 0.0F, GraphletAnchor::RT, -gwidth * 2.0F);

			for (HS id = HS::A2C; id < HS::J2I; id++) {
				if (id != HS::I2J) {
					this->master->move_to(this->alarms[_E(HS, _I(id) + 1U)],
						this->alarms[id], GraphletAnchor::CB, GraphletAnchor::CT,
						0.0F, vgap * 3.0F);
				}
			}

			for (auto it = this->alabels.begin(); it != this->alabels.end(); it++) {
				this->master->move_to(it->second, this->alarms[it->first],
					GraphletAnchor::RC, GraphletAnchor::LC, lblgap);
			}
		}
	}

public:
	bool pumps_selected(HS id0, HS idn, int tolerance) {
		bool okay = false;
		int ok = 0;

		for (HS id = id0; id <= idn; id++) {
			if (this->master->is_selected(this->pumps[id])) {
				ok += 1;

				if (ok >= tolerance) {
					okay = true;
					break;
				}
			}
		}

		return okay;
	}

private:
	template<class G, typename E>
	void load_devices(std::map<E, G*>& gs, E id0, E idn, float radius, double degrees) {
		for (E id = id0; id <= idn; id++) {
			gs[id] = this->master->insert_one(new G(radius, degrees), id);
		}
	}

	template<class G, typename E>
	void load_devices(std::map<E, G*>& gs, std::map<E, Credit<Labellet, E>*>& ls, E id0, E idn, float radius, double degrees) {
		this->load_devices(gs, id0, idn, radius, degrees);

		for (E id = id0; id <= idn; id++) {
			this->load_label(ls, id.ToString(), id, Colours::Silver);
		}
	}

	template<class G, typename E>
	void load_devices(std::map<E, G*>& gs, std::map<E, Credit<Labellet, E>*>& ls, std::map<E, Credit<Labellet, E>*>& cs
		, E id0, E idn, float radius, double degrees, CanvasSolidColorBrush^ color = Colours::Silver) {
		unsigned int count = 0U;

		this->load_devices(gs, id0, idn, radius, degrees);

		for (E id = id0; id <= idn; id++) {
			const HS* captions = select_captions(id, &count);

			this->load_label(ls, id.ToString(), id, color);

			if (count == 0) {
				this->load_label(cs, id, color);
			} else {
				for (unsigned int idx = 0; idx < count; idx++) {
					this->load_label(cs, captions[idx], color);
				}
			}
		}
	}

	template<typename E>
	void load_dimension(std::map<E, Credit<Dimensionlet, E>*>& ds, E id, Platform::String^ unit) {
		ds[id] = this->master->insert_one(new Credit<Dimensionlet, E>(this->flonum_style, unit, _speak(id)), id);
	}

	template<typename E>
	void load_dimensions(std::map<E, Credit<Dimensionlet, E>*>& ds, E id0, E idn, Platform::String^ unit) {
		for (E id = id0; id <= idn; id++) {
			ds[id] = this->master->insert_one(new Credit<Dimensionlet, E>(this->fixnum_style, unit), id);
		}
	}

	template<class A, typename E>
	void load_alarms(E id0, E idn, float size, std::map<E, Credit<A, E>*>& bs, std::map<E, Credit<Labellet, E>*>& ls) {
		for (E id = id0; id <= idn; id++) {
			this->load_label(ls, id, Colours::Silver);
			bs[id] = this->master->insert_one(new Credit<A, E>(size), id);
		}
	}

	template<class B, typename CMD>
	void load_buttons(std::map<CMD, Credit<B, CMD>*>& bs, float width = 128.0F, float height = 32.0F) {
		for (CMD cmd = _E(CMD, 0); cmd < CMD::_; cmd++) {
			bs[cmd] = this->master->insert_one(new Credit<B, CMD>(cmd.ToString(), width, height), cmd);
			bs[cmd]->set_style(this->button_style);
		}
	}

	template<class T, typename E>
	void load_thermometer(std::map<E, Credit<T, E>*>& ts, std::map<E, Credit<Dimensionlet, E>*>& ds, E id, float width, float height) {
		ts[id] = this->master->insert_one(new Credit<T, E>(100.0, width, height, 2.5F), id);
		this->load_dimension(ds, id, "celsius");
	}

	template<typename E>
	void load_label(std::map<E, Credit<Labellet, E>*>& ls, Platform::String^ caption, E id
		, CanvasSolidColorBrush^ color, CanvasTextFormat^ font = nullptr) {
		ls[id] = this->master->insert_one(new Credit<Labellet, E>(caption, ((font == nullptr) ? this->label_font : font), color), id);
	}

	template<typename E>
	void load_label(std::map<E, Credit<Labellet, E>*>& ls, E id, CanvasSolidColorBrush^ color, CanvasTextFormat^ font = nullptr) {
		this->load_label(ls, _speak(id), id, color, font);
	}

	template<typename E>
	StateTanklet<E>* make_tank(E id, float width, float height, float thickness) {
		StateTanklet<E>* tank = new StateTanklet<E>(id, width, height, thickness);
		StateTankStyle ulow, low, normal;

		normal.mark_weight = 0.50F;
		low.mark_weight    = 0.20F;
		ulow.mark_weight   = 0.10F;
		
		tank->set_style(E::Normal,   normal);
		tank->set_style(E::Low,      low);
		tank->set_style(E::UltraLow, ulow);

		// WARNING: set styles before inserting 

		return this->master->insert_one(tank);
	}

private:
	void set_temperature(HS id, float t) {
		this->thermometers[id]->set_value(t);
		this->temperatures[id]->set_value(t, GraphletAnchor::LB);
	}

	void set_visor_tank_level(float t) {
		this->visor_tank->set_value(t);
		this->levels[HS::VisorOil]->set_value(t, GraphletAnchor::LB);
		this->master->move_to(this->labels[HS::VisorState], this->levels[HS::VisorOil], GraphletAnchor::RC, GraphletAnchor::LC);
	}

private:
	void try_flow_oil(HS vid, HS pid, CanvasSolidColorBrush^ color) {
		switch (this->valves[vid]->get_state()) {
		case ManualValveState::Open: {
			this->station->push_subtrack(vid, pid, color);
		}
		}
	}

	void try_flow_oil(HS vid, HS mid, HS eid, CanvasSolidColorBrush^ color) {
		switch (this->valves[vid]->get_state()) {
		case ManualValveState::Open: {
			this->station->push_subtrack(vid, mid, color);
			this->station->push_subtrack(mid, eid, color);
		}
		}
	}

	void try_flow_oil(HS vid, HS pid, HS _id, HS* path, unsigned int count, CanvasSolidColorBrush^ color) {
		this->try_flow_oil(vid, pid, color);

		switch (this->pumps[pid]->get_state()) {
		case HydraulicPumpState::Running: case HydraulicPumpState::StopReady: {
			this->station->push_subtrack(pid, _id, oil_color);

			if (path != nullptr) {
				this->station->push_subtrack(_id, path[0], oil_color);
				this->station->push_subtrack(path, count, color);
			}
		}
		}
	}

	template<unsigned int N>
	void try_flow_oil(HS vid, HS pid, HS _id, HS (&path)[N], CanvasSolidColorBrush^ color) {
		this->try_flow_oil(vid, pid, _id, path, N, color);
	}

private: // never deletes these graphlets mannually
	Tracklet<HS>* station;
	Heaterlet* heater;
	Tanklet* visor_tank;
	FuelTanklet* storage_tank;
	StateTanklet<HSMTState>* master_tank;
	std::map<HSFunction, Credit<Buttonlet, HSFunction>*> functions;
	std::map<HS, Credit<Thermometerlet, HS>*> thermometers;
	std::map<HS, Credit<Dimensionlet, HS>*> temperatures;
	std::map<HS, Credit<Dimensionlet, HS>*> levels;
	std::map<HS, Credit<Labellet, HS>*> captions;
	std::map<HS, Credit<Labellet, HS>*> labels;
	std::map<HS, Credit<HydraulicPumplet, HS>*> pumps;
	std::map<HS, Credit<ManualValvelet, HS>*> valves;
	std::map<HS, Credit<Dimensionlet, HS>*> pressures;
	std::map<HS, Credit<Alarmlet, HS>*> alarms;
	std::map<HS, Credit<Labellet, HS>*> alabels;
	
private:
	CanvasTextFormat^ caption_font;
	CanvasTextFormat^ label_font;
	DimensionStyle fixnum_style;
	DimensionStyle flonum_style;
	ButtonStyle button_style;

private:
	HydraulicsPage* master;
};

HydraulicsPage::HydraulicsPage(PLCMaster* plc) : Planet(__MODULE__), device(plc) {
	Hydraulics* dashboard = new Hydraulics(this);

	this->dashboard = dashboard;
	
	if (this->device != nullptr) {
		this->gbs_op = make_hydraulics_group_menu(HydraulicsGroup::BothPumps, plc);
		this->gps_op = make_hydraulics_group_menu(HydraulicsGroup::PSPumps, plc);
		this->gsb_op = make_hydraulics_group_menu(HydraulicsGroup::SBPumps, plc);
		this->gvisor_op = make_hydraulics_group_menu(HydraulicsGroup::VisorPumps, plc);
		this->pump_op = make_hydraulic_pump_menu(DO_hydraulics_action, hydraulics_diagnostics, plc);
		this->heater_op = make_tank_heater_menu(plc);

		this->device->push_confirmation_receiver(dashboard);
	}

	{ // load decorators
		this->grid = new GridDecorator();

#ifdef _DEBUG
		this->push_decorator(this->grid);
#else
		this->grid->set_active_planet(this);
#endif
	}
}

HydraulicsPage::~HydraulicsPage() {
	if (this->dashboard != nullptr) {
		delete this->dashboard;
	}

#ifndef _DEBUG
	delete this->grid;
#endif
}

void HydraulicsPage::load(CanvasCreateResourcesReason reason, float width, float height) {
	auto dashboard = dynamic_cast<Hydraulics*>(this->dashboard);
	
	if (dashboard != nullptr) {
		float gwidth = width / 76.0F;
		float gheight = height / 38.0F;

		this->grid->set_grid_width(gwidth);
		this->grid->set_grid_height(gheight);
		
		dashboard->construct(gwidth, gheight);

		dashboard->load_pump_station(width, height, gwidth, gheight);
		dashboard->load_tanks(width, height, gwidth, gheight);
		dashboard->load_devices(width, height, gwidth, gheight);
	}
}

void HydraulicsPage::reflow(float width, float height) {
	auto dashboard = dynamic_cast<Hydraulics*>(this->dashboard);
	
	if (dashboard != nullptr) {
		float gwidth = this->grid->get_grid_width();
		float gheight = this->grid->get_grid_height();

		dashboard->reflow_pump_station(width, height, gwidth, gheight);
		dashboard->reflow_devices(width, height, gwidth, gheight);
		dashboard->reflow_metrics(width, height, gwidth, gheight);
	}
}

void HydraulicsPage::on_timestream(long long timepoint_ms, size_t addr0, size_t addrn, uint8* data, size_t size, Syslog* logger) {
	auto dashboard = dynamic_cast<Hydraulics*>(this->dashboard);

	if (dashboard != nullptr) {
		dashboard->on_all_signals(timepoint_ms, addr0, addrn, data, size, logger);
	}
}

bool HydraulicsPage::can_select(IGraphlet* g) {
	return ((this->device != nullptr)
		&& ((dynamic_cast<HydraulicPumplet*>(g) != nullptr)
			|| (dynamic_cast<Heaterlet*>(g) != nullptr)
			|| (dynamic_cast<Buttonlet*>(g) != nullptr)));
}

bool HydraulicsPage::can_select_multiple() {
	return (this->device != nullptr);
}

void HydraulicsPage::on_tap_selected(IGraphlet* g, float local_x, float local_y) {
	auto pump = dynamic_cast<HydraulicPumplet*>(g);
	auto heater = dynamic_cast<Heaterlet*>(g);
	auto override = dynamic_cast<Buttonlet*>(g);

	if (pump != nullptr) {
		menu_popup(this->pump_op, g, local_x, local_y);
	} else if (heater != nullptr) {
		menu_popup(this->heater_op, g, local_x, local_y);
	} else if (override != nullptr) {
		this->device->send_command(backoil_pressure_override_command);
	}
}

void HydraulicsPage::on_gesture(std::list<float2>& anchors, float x, float y) {
	auto dashboard = dynamic_cast<Hydraulics*>(this->dashboard);

	if (dashboard != nullptr) {
		if (dashboard->pumps_selected(HS::C, HS::E, 1) && dashboard->pumps_selected(HS::A, HS::H, 1)) {
			group_menu_popup(this->gbs_op, this, x, y);
		} else if (dashboard->pumps_selected(HS::C, HS::E, 2)) {
			group_menu_popup(this->gps_op, this, x, y);
		} else if (dashboard->pumps_selected(HS::A, HS::H, 2)) {
			group_menu_popup(this->gsb_op, this, x, y);
		} else if (dashboard->pumps_selected(HS::J, HS::I, 2)) {
			group_menu_popup(this->gvisor_op, this, x, y);
		}
	}
}
