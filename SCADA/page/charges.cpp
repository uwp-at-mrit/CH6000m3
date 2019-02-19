#include <map>

#include "page/charges.hpp"
#include "page/diagnostics/hopper_pump_dx.hpp"

#include "configuration.hpp"
#include "menu.hpp"

#include "module.hpp"
#include "text.hpp"
#include "paint.hpp"
#include "brushes.hxx"
#include "turtle.hpp"

#include "graphlet/shapelet.hpp"
#include "graphlet/statuslet.hpp"
#include "graphlet/buttonlet.hpp"
#include "graphlet/symbol/pump/hopper_pumplet.hpp"
#include "graphlet/symbol/valve/gate_valvelet.hpp"
#include "graphlet/symbol/valve/tagged_valvelet.hpp"

#include "iotables/ai_pumps.hpp"
#include "iotables/ai_valves.hpp"
#include "iotables/ai_hopper_pumps.hpp"
#include "iotables/ai_dredges.hpp"

#include "iotables/di_pumps.hpp"
#include "iotables/di_hopper_pumps.hpp"
#include "iotables/di_valves.hpp"

#include "iotables/do_valves.hpp"
#include "iotables/do_hopper_pumps.hpp"

#include "decorator/vessel.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::Foundation;
using namespace Windows::Foundation::Numerics;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI;
using namespace Microsoft::Graphics::Canvas::Text;
using namespace Microsoft::Graphics::Canvas::Brushes;
using namespace Microsoft::Graphics::Canvas::Geometry;

private enum class CSFunction { Diagnostics, _ };

// WARNING: order matters
private enum class CS : unsigned int {
	Port, Starboard,
	
	// Valves
	D001, D002, D006, D008, D010, D017, D018, D019, D020, D021, D022, D024, D026,
	D003, D007, D011, D023, D025,
	D004, D005, D009, D012,
	D014, D016,
	D013, D015,

	// Pump Dimensions
	C, F, H,

	// Underwater Pump Motor Metrics
	PSMotor, SBMotor,
	PSU, PSV, PSW, PSMotorDP1, PSMotorDP2, PSDE, PSNDEa, PSNDEr, PSDEoil, PSNDEoil,
	SBU, SBV, SBW, SBMotorDP1, SBMotorDP2, SBDE, SBNDEa, SBNDEr, SBDEoil, SBNDEoil,
	
	// Key Labels
	PSUWPump, SBUWPump, PSHPump, SBHPump, Barge, LMOD,

	// Interconnected Nodes
	I0723, I0923,

	_,
	// anchors used as last jumping points
	d11, d12, d13, d14, d24,
	d0225, d0326, d0406,
	d1720, d1819, d1920, d2122,

	// anchors used for unnamed nodes
	ps, sb, gantry, deck_lx, deck_rx, deck_ty, deck_by, diagnostics,
	d0205, e11, e12, e13, e14, e15, e16, egantry,

	// anchors used for non-interconnected nodes
	n0325, n0405, n0723, n0923
};

static CanvasSolidColorBrush^ water_color = Colours::Green;

static uint16 DO_gate_valve_action(GateValveAction cmd, GateValvelet* valve) {
	uint16 index = 0U;
	auto credit_valve = dynamic_cast<Credit<GateValvelet, CS>*>(valve);

	if (credit_valve != nullptr) {
		index = DO_gate_valve_command(cmd, credit_valve->id);
	}

	return index;
}

/*************************************************************************************************/
private class Vessel final : public PLCConfirmation {
public:
	Vessel(ChargesPage* master) : master(master) {}

public:
	void pre_read_data(Syslog* logger) override {
		this->master->enter_critical_section();
		this->master->begin_update_sequence();

		this->station->clear_subtacks();
	}

	void on_analog_input(long long timepoint_ms, const uint8* DB2, size_t count2, const uint8* DB203, size_t count203, Syslog* logger) override {
		this->pump_pressures[CS::C]->set_value(RealData(DB203, pump_C_pressure), GraphletAnchor::LC);
		this->pump_pressures[CS::F]->set_value(RealData(DB203, pump_F_pressure), GraphletAnchor::LC);
		this->pump_pressures[CS::H]->set_value(RealData(DB203, pump_H_pressure), GraphletAnchor::LC);

		this->progresses[CS::D003]->set_value(RealData(DB203, gate_valve_D03_progress), GraphletAnchor::LB);
		this->progresses[CS::D004]->set_value(RealData(DB203, gate_valve_D04_progress), GraphletAnchor::LT);

		this->powers[CS::PSHPump]->set_value(RealData(DB203, ps_hopper_pump_power), GraphletAnchor::RC);
		this->rpms[CS::PSHPump]->set_value(RealData(DB203, ps_hopper_pump_rpm), GraphletAnchor::LC);
		this->dpressures[CS::PSHPump]->set_value(RealData(DB203, ps_hopper_pump_discharge_pressure), GraphletAnchor::LB);
		this->vpressures[CS::PSHPump]->set_value(RealData(DB203, ps_hopper_pump_vacuum_pressure), GraphletAnchor::RB);

		this->powers[CS::SBHPump]->set_value(RealData(DB203, sb_hopper_pump_power), GraphletAnchor::RC);
		this->rpms[CS::SBHPump]->set_value(RealData(DB203, sb_hopper_pump_rpm), GraphletAnchor::LC);
		this->dpressures[CS::SBHPump]->set_value(RealData(DB203, sb_hopper_pump_discharge_pressure), GraphletAnchor::LT);
		this->vpressures[CS::SBHPump]->set_value(RealData(DB203, sb_hopper_pump_vacuum_pressure), GraphletAnchor::RT);

		this->powers[CS::PSUWPump]->set_value(RealData(DB203, ps_underwater_pump_power), GraphletAnchor::RC);
		this->rpms[CS::PSUWPump]->set_value(RealData(DB203, ps_underwater_pump_rpm), GraphletAnchor::LC);
		this->dpressures[CS::PSUWPump]->set_value(RealData(DB203, ps_underwater_pump_discharge_pressure), GraphletAnchor::LB);
		this->dfpressures[CS::PSUWPump]->set_value(RealData(DB203, ps_draghead_differential_pressure), GraphletAnchor::LB);

		this->powers[CS::SBUWPump]->set_value(RealData(DB203, sb_underwater_pump_power), GraphletAnchor::RC);
		this->rpms[CS::SBUWPump]->set_value(RealData(DB203, sb_underwater_pump_rpm), GraphletAnchor::LC);
		this->dpressures[CS::SBUWPump]->set_value(RealData(DB203, sb_underwater_pump_discharge_pressure), GraphletAnchor::LT);
		this->dfpressures[CS::SBUWPump]->set_value(RealData(DB203, sb_draghead_differential_pressure), GraphletAnchor::LT);

		/*
		this->motors[CS::PSU]->set_value(RealData(DB203, ps_motor_U), GraphletAnchor::LT);
		this->motors[CS::PSV]->set_value(RealData(DB203, ps_motor_V), GraphletAnchor::LT);
		this->motors[CS::PSW]->set_value(RealData(DB203, ps_motor_W), GraphletAnchor::LT);
		this->motors[CS::PSDE]->set_value(RealData(DB203, ps_motor_DE), GraphletAnchor::LT);
		this->motors[CS::PSNDEa]->set_value(RealData(DB203, ps_motor_NDEa), GraphletAnchor::LT);
		this->motors[CS::PSNDEr]->set_value(RealData(DB203, ps_motor_NDEr), GraphletAnchor::LT);
		this->motors[CS::PSDEoil]->set_value(RealData(DB203, ps_motor_DEo), GraphletAnchor::LT);
		this->motors[CS::PSNDEoil]->set_value(RealData(DB203, ps_motor_NDEo), GraphletAnchor::LT);
		this->motors[CS::PSMotorDP1]->set_value(RealData(DB203, ps_motor_dp1), GraphletAnchor::LT);
		this->motors[CS::PSMotorDP2]->set_value(RealData(DB203, ps_motor_dp2), GraphletAnchor::LT);

		this->motors[CS::SBU]->set_value(RealData(DB203, sb_motor_U), GraphletAnchor::LT);
		this->motors[CS::SBV]->set_value(RealData(DB203, sb_motor_V), GraphletAnchor::LT);
		this->motors[CS::SBW]->set_value(RealData(DB203, sb_motor_W), GraphletAnchor::LT);
		this->motors[CS::SBDE]->set_value(RealData(DB203, sb_motor_DE), GraphletAnchor::LT);
		this->motors[CS::SBNDEa]->set_value(RealData(DB203, sb_motor_NDEa), GraphletAnchor::LT);
		this->motors[CS::SBNDEr]->set_value(RealData(DB203, sb_motor_NDEr), GraphletAnchor::LT);
		this->motors[CS::SBDEoil]->set_value(RealData(DB203, sb_motor_DEo), GraphletAnchor::LT);
		this->motors[CS::SBNDEoil]->set_value(RealData(DB203, sb_motor_NDEo), GraphletAnchor::LT);
		this->motors[CS::SBMotorDP1]->set_value(RealData(DB203, sb_motor_dp1), GraphletAnchor::LT);
		this->motors[CS::SBMotorDP2]->set_value(RealData(DB203, sb_motor_dp2), GraphletAnchor::LT);
		*/
	}

	void on_digital_input(long long timepoint_ms, const uint8* DB4, size_t count4, const uint8* DB205, size_t count205, Syslog* logger) override {
		DI_hopper_pumps(this->hoppers[CS::PSHPump], this->hoppers[CS::PSUWPump], DB4, ps_hopper_pump_feedback, DB205, ps_hopper_pump_details, ps_underwater_pump_details);
		DI_hopper_pumps(this->hoppers[CS::SBHPump], this->hoppers[CS::SBUWPump], DB4, sb_hopper_pump_feedback, DB205, sb_hopper_pump_details, sb_underwater_pump_details);

		DI_hydraulic_pump_dimension(this->pump_pressures[CS::C], DB4, pump_C_feedback);
		DI_hydraulic_pump_dimension(this->pump_pressures[CS::F], DB4, pump_F_feedback);
		DI_hydraulic_pump_dimension(this->pump_pressures[CS::H], DB4, pump_H_feedback);

		this->set_valves_status(CS::D001, DB4, gate_valve_D01_feedback, motor_valve_D01_feedback, DB205, gate_valve_D01_status, motor_valve_D01_status);
		this->set_valves_status(CS::D002, DB4, gate_valve_D02_feedback, motor_valve_D02_feedback, DB205, gate_valve_D02_status, motor_valve_D02_status);
		this->set_valves_status(CS::D005, DB4, gate_valve_D05_feedback, motor_valve_D05_feedback, DB205, gate_valve_D05_status, motor_valve_D05_status);
		this->set_valves_status(CS::D006, DB4, gate_valve_D06_feedback, motor_valve_D06_feedback, DB205, gate_valve_D06_status, motor_valve_D06_status);
		this->set_valves_status(CS::D007, DB4, gate_valve_D07_feedback, motor_valve_D07_feedback, DB205, gate_valve_D07_status, motor_valve_D07_status);
		this->set_valves_status(CS::D008, DB4, gate_valve_D08_feedback, motor_valve_D08_feedback, DB205, gate_valve_D08_status, motor_valve_D08_status);
		this->set_valves_status(CS::D009, DB4, gate_valve_D09_feedback, motor_valve_D09_feedback, DB205, gate_valve_D09_status, motor_valve_D09_status);
		this->set_valves_status(CS::D010, DB4, gate_valve_D10_feedback, motor_valve_D10_feedback, DB205, gate_valve_D10_status, motor_valve_D10_status);
		this->set_valves_status(CS::D011, DB4, gate_valve_D11_feedback, motor_valve_D11_feedback, DB205, gate_valve_D11_status, motor_valve_D11_status);
		this->set_valves_status(CS::D012, DB4, gate_valve_D12_feedback, motor_valve_D12_feedback, DB205, gate_valve_D12_status, motor_valve_D12_status);
		this->set_valves_status(CS::D013, DB4, gate_valve_D13_feedback, motor_valve_D13_feedback, DB205, gate_valve_D13_status, motor_valve_D13_status);
		this->set_valves_status(CS::D014, DB4, gate_valve_D14_feedback, motor_valve_D14_feedback, DB205, gate_valve_D14_status, motor_valve_D14_status);
		this->set_valves_status(CS::D015, DB4, gate_valve_D15_feedback, motor_valve_D15_feedback, DB205, gate_valve_D15_status, motor_valve_D15_status);
		this->set_valves_status(CS::D016, DB4, gate_valve_D16_feedback, motor_valve_D16_feedback, DB205, gate_valve_D16_status, motor_valve_D16_status);
		this->set_valves_status(CS::D017, DB4, gate_valve_D17_feedback, motor_valve_D17_feedback, DB205, gate_valve_D17_status, motor_valve_D17_status);
		this->set_valves_status(CS::D018, DB4, gate_valve_D18_feedback, motor_valve_D18_feedback, DB205, gate_valve_D18_status, motor_valve_D18_status);
		this->set_valves_status(CS::D019, DB4, gate_valve_D19_feedback, motor_valve_D19_feedback, DB205, gate_valve_D19_status, motor_valve_D19_status);
		this->set_valves_status(CS::D020, DB4, gate_valve_D20_feedback, motor_valve_D20_feedback, DB205, gate_valve_D20_status, motor_valve_D20_status);
		this->set_valves_status(CS::D021, DB4, gate_valve_D21_feedback, motor_valve_D21_feedback, DB205, gate_valve_D21_status, motor_valve_D21_status);
		this->set_valves_status(CS::D022, DB4, gate_valve_D22_feedback, motor_valve_D22_feedback, DB205, gate_valve_D22_status, motor_valve_D22_status);
		this->set_valves_status(CS::D023, DB4, gate_valve_D23_feedback, motor_valve_D23_feedback, DB205, gate_valve_D23_status, motor_valve_D23_status);
		this->set_valves_status(CS::D024, DB4, gate_valve_D24_feedback, motor_valve_D24_feedback, DB205, gate_valve_D24_status, motor_valve_D24_status);
		this->set_valves_status(CS::D025, DB4, gate_valve_D25_feedback, motor_valve_D25_feedback, DB205, gate_valve_D25_status, motor_valve_D25_status);
		this->set_valves_status(CS::D026, DB4, gate_valve_D26_feedback, motor_valve_D26_feedback, DB205, gate_valve_D26_status, motor_valve_D26_status);
	
		DI_gate_valve(this->gvalves[CS::D003], DB205, gate_valve_D03_feedback, DB205, gate_valve_D03_status);
		DI_motor_valve(this->mvalves[CS::D003], DB4, motor_valve_D03_feedback, DB205, motor_valve_D03_status);

		DI_gate_valve(this->gvalves[CS::D004], DB205, gate_valve_D04_feedback, DB205, gate_valve_D04_status);
		DI_motor_valve(this->mvalves[CS::D004], DB4, motor_valve_D04_feedback, DB205, motor_valve_D04_status);
	}

	void post_read_data(Syslog* logger) override {
		{ // flow PS water
			CS c0910[] = { CS::I0923, CS::D010 };

			this->try_flow_water(CS::D004, CS::Port, water_color);
			this->try_flow_water(CS::D006, CS::D004, CS::D009, water_color);
			this->try_flow_water(CS::D009, c0910, water_color);
			this->try_flow_water(CS::D017, CS::D010, water_color);

			if (this->valve_open(CS::D005)) {
				CS c0517[] = { CS::D004, CS::D005, CS::d0205, CS::PSHPump, CS::D017 };

				this->station->push_subtrack(c0517, water_color);
				this->nintercs[CS::n0405]->set_color(water_color);
			} else {
				this->nintercs[CS::n0405]->set_color(default_pipe_color);
			}

			this->try_flow_water(CS::D010, CS::D016, water_color);
			this->try_flow_water(CS::D012, CS::e12, water_color);
			this->try_flow_water(CS::D014, CS::e14, water_color);
			this->try_flow_water(CS::D016, CS::e16, water_color);
		}

		{ // flow SB water
			CS c0708[] = { CS::I0723, CS::D008 };
			
			this->try_flow_water(CS::D003, CS::Starboard, water_color);
			this->try_flow_water(CS::D026, CS::D003, CS::D007, water_color);
			this->try_flow_water(CS::D007, c0708, water_color);
			this->try_flow_water(CS::D018, CS::D008, water_color);

			if (this->valve_open(CS::D025)) {
				CS c0318[] = { CS::D003, CS::D025, CS::d0225, CS::SBHPump, CS::D018 };

				this->station->push_subtrack(c0318, water_color);
				this->nintercs[CS::n0325]->set_color(water_color);
			} else {
				this->nintercs[CS::n0325]->set_color(default_pipe_color);
			}

			this->try_flow_water(CS::D008, CS::D015, water_color);
			this->try_flow_water(CS::D011, CS::e11, water_color);
			this->try_flow_water(CS::D013, CS::e13, water_color);
			this->try_flow_water(CS::D015, CS::e15, water_color);
		}

		if (this->valve_open(CS::D023)) {
			CS d0810[] = { CS::D008, CS::I0723, CS::I0923, CS::D010 };

			this->station->push_subtrack(d0810, water_color);
			this->nintercs[CS::n0723]->set_color(water_color);
			this->nintercs[CS::n0923]->set_color(water_color);
		} else {
			this->nintercs[CS::n0723]->set_color(default_pipe_color);
			this->nintercs[CS::n0923]->set_color(default_pipe_color);
		}

		if (this->valve_open(CS::D024)) {
			this->station->push_subtrack(CS::d24, CS::egantry, water_color);
			this->gantry_pipe->set_color(water_color);
		} else {
			this->gantry_pipe->set_color(default_pipe_color);
		}

		this->master->end_update_sequence();
		this->master->leave_critical_section();
	}

public:
	void construct(float gwidth, float gheight) {
		this->caption_font = make_bold_text_format("Microsoft YaHei", normal_font_size);
		this->label_font = make_bold_text_format("Microsoft YaHei", small_font_size);
		this->special_font = make_text_format(tiny_font_size);

		this->motor_style = make_highlight_dimension_style(small_font_size, 6U, 4U, 0);
		this->pump_style = make_highlight_dimension_style(large_metrics_font_size, 6U, 0, Colours::Background);
		this->highlight_style = make_highlight_dimension_style(large_metrics_font_size, 6U, 0, Colours::Green);

		this->relationship_style = make_dash_stroke(CanvasDashStyle::DashDot);
		this->relationship_color = Colours::DarkGray;

		this->hopper_style.number_font = make_bold_text_format("Cambria Math", large_metrics_font_size);
		this->hopper_style.unit_font = make_bold_text_format("Cambria", normal_font_size);
	}
 
public:
	void load(float width, float height, float gwidth, float gheight) {
		float radius = resolve_gridsize(gwidth, gheight);
		Turtle<CS>* pTurtle = new Turtle<CS>(gwidth, gheight, false);

		pTurtle->move_left(CS::deck_rx)->move_left(2, CS::D021)->move_left(2, CS::d2122);
		pTurtle->move_down(5)->move_right(2, CS::D022)->move_right(3)->jump_back();
		pTurtle->move_left(2, CS::d1920)->move_left(2, CS::D020)->move_left(7, CS::d1720);

		pTurtle->move_left(3, CS::D017)->move_left(11, CS::n0405)->move_left(4, CS::D010)->move_left(6, CS::d12);
		pTurtle->move_down(2, CS::D012)->move_down(3, CS::e12)->jump_down(CS::LMOD)->jump_back(CS::d12);
		pTurtle->move_left(4, CS::d14)->move_left_down(2, CS::D014)->move_left_down(1.5F, CS::e14)->jump_back(CS::d14);
		pTurtle->move_left(5, CS::d24)->move_left(4)->move_left_down(2, CS::D016)->move_left_down(1.5F, CS::e16)->jump_back(CS::d24);
		pTurtle->jump_up(2.5F, CS::gantry)->turn_up_left()->move_left(3, CS::D024)->move_left(3)->turn_left_up();
		pTurtle->move_up(0.5F, CS::Barge)->move_left(CS::egantry)->jump_back(CS::Barge)->move_right()->jump_back(CS::d1720);
		
		pTurtle->move_down(3.5F, CS::PSHPump)->move_left(6, CS::n0923);
		pTurtle->move_left(8, CS::d0205)->move_up(1.5F, CS::D005)->move_up(1.5F)->jump_up();
		pTurtle->move_up(3, CS::d0406)->move_right(4, CS::D006)->move_right(4)->move_down(0.5F, CS::deck_ty)->move_down(CS::D009);
		pTurtle->move_down(2, CS::I0923)->move_down(3)->jump_down()->move_down(2, CS::D023)->jump_back(CS::d0406);

		pTurtle->move_up(1.5F, CS::D004)->move_up(2, CS::ps)->move_up(2, CS::diagnostics)->turn_up_left();
		pTurtle->move_left(8, CS::PSUWPump)->move_left(10)->move_left(CS::Port);

		pTurtle->jump_back(CS::D023)->move_down(2)->jump_down()->move_down(3, CS::I0723)->move_down(2, CS::D007);
		pTurtle->move_down(CS::deck_by)->move_down(0.5F)->move_left(4, CS::D026)->move_left(4, CS::d0326);
		pTurtle->move_up(3)->jump_up()->move_up(1.5F, CS::D025)->move_up(1.5F, CS::d0225);
		pTurtle->move_right(8, CS::n0723)->move_right(6, CS::SBHPump)->move_down(3.5F, CS::d1819)->jump_back(CS::d0225);
		pTurtle->move_up(2.5F)->move_left(2, CS::D002)->move_left(28, CS::D001)->move_left(2)->jump_back(CS::d1819);

		pTurtle->move_left(3, CS::D018)->move_left(11, CS::n0325)->move_left(4, CS::D008);
		pTurtle->move_left(6, CS::d11)->move_up(2, CS::D011)->move_up(3, CS::e11)->jump_back(CS::d11);
		pTurtle->move_left(4, CS::d13)->move_left_up(2, CS::D013)->move_left_up(1.5F, CS::e13)->jump_back(CS::d13);
		pTurtle->move_left(9)->move_left_up(2, CS::D015)->move_left_up(1.5F, CS::e15);

		pTurtle->jump_back(CS::d0326)->move_down(1.5F, CS::D003)->move_down(2, CS::sb)->move_down(2, CS::C)->turn_down_left();
		pTurtle->move_left(8, CS::SBUWPump)->move_left(10)->move_left(CS::Starboard);

		pTurtle->jump_back(CS::d1819)->move_right(5, CS::deck_lx)->move_right(2, CS::D019)->move_right(2)->move_to(CS::d1920);
		
		this->station = this->master->insert_one(new Tracklet<CS>(pTurtle, default_pipe_thickness, default_pipe_color));
		this->load_buttons(this->functions);

		{ // load gantry pipe segement
			CanvasStrokeStyle^ gantry_style = make_dash_stroke(CanvasDashStyle::Dash);
			float gantry_y, vessel_y;

			this->station->fill_anchor_location(CS::gantry, nullptr, &gantry_y);
			this->station->fill_anchor_location(CS::D008, nullptr, &vessel_y);

			this->gantry_pipe = this->master->insert_one(
				new Linelet(0.0F, 0.0F, 0.0F, vessel_y - gantry_y,
					default_pipe_thickness, default_pipe_color, gantry_style));
		}
		
		{ // load valves
			this->load_valve(this->gvalves, this->vlabels, this->captions, CS::D001, radius, 0.0);
			this->load_valves(this->gvalves, this->mvalves, this->vlabels, this->captions, CS::D002, CS::D026, radius, 00.0);
			this->load_valves(this->gvalves, this->mvalves, this->vlabels, this->captions, CS::D003, CS::D025, radius, 90.0);
			this->load_valves(this->gvalves, this->mvalves, this->vlabels, this->captions, CS::D004, CS::D012, radius, -90.0);
			this->load_valves(this->gvalves, this->mvalves, this->vlabels, this->captions, CS::D014, CS::D016, radius, -45.0);
			this->load_valves(this->gvalves, this->mvalves, this->vlabels, this->captions, CS::D013, CS::D015, radius, 45.0);
		}

		{ // load special nodes
			float nic_radius = gheight * 0.25F;

			this->load_pump(this->hoppers, this->captions, CS::PSHPump, -radius, +2.0F, 0.0, gwidth);
			this->load_pump(this->hoppers, this->captions, CS::SBHPump, -radius, -2.0F, 0.0, gwidth);
			this->load_pump(this->hoppers, this->captions, CS::PSUWPump, -radius, -2.0F, -90.0, gwidth);
			this->load_pump(this->hoppers, this->captions, CS::SBUWPump, -radius, +2.0F, +90.0, gwidth);

			this->LMOD = this->master->insert_one(new Arclet(0.0, 360.0, gheight, gheight, 1.0F, Colours::Green));

			this->tips[CS::Port] = this->master->insert_one(
				new Segmentlet(-90.0, 90.0, gwidth * 2.0F, gheight,
					default_ps_color, default_pipe_thickness));

			this->tips[CS::Starboard] = this->master->insert_one(
				new Segmentlet(-90.0, 90.0, gwidth * 2.0F, gheight,
					default_sb_color, default_pipe_thickness));

			for (CS id = CS::I0723; id <= CS::I0923; id++) {
				this->intercs[id] = this->master->insert_one(
					new Circlelet(default_pipe_thickness * 2.0F, Colours::Green));
			}

			for (CS id = CS::n0325; id <= CS::n0923; id++) {
				this->nintercs[id] = this->master->insert_one(
					new Omegalet(-90.0, nic_radius, default_pipe_thickness, default_pipe_color));
			}
		}

		{ // load labels and dimensions
			this->load_percentage(this->progresses, CS::D003);
			this->load_percentage(this->progresses, CS::D004);
			this->load_dimensions(this->pump_pressures, CS::C, CS::H, "bar");

			this->load_label(this->captions, CS::Barge, Colours::Yellow, this->caption_font);
			this->load_label(this->captions, CS::LMOD, Colours::Cyan, this->special_font);
		}
	}

public:
	void reflow(float width, float height, float gwidth, float gheight, float vinset) {
		GraphletAnchor anchor;
		float dx, dy, margin, label_height, ox, oy;
		float gridsize = std::fminf(gwidth, gheight);
		float x0 = 0.0F;
		float y0 = 0.0F;

		this->master->move_to(this->station, width * 0.5F, height * 0.5F, GraphletAnchor::CC);
		this->station->map_graphlet_at_anchor(this->gantry_pipe, CS::gantry, GraphletAnchor::CT);

		this->station->map_credit_graphlet(this->captions[CS::Barge], GraphletAnchor::CB);
		this->station->map_credit_graphlet(this->captions[CS::LMOD], GraphletAnchor::CB);

		this->station->map_graphlet_at_anchor(this->tips[CS::Port], CS::Port, GraphletAnchor::RC);
		this->station->map_graphlet_at_anchor(this->tips[CS::Starboard], CS::Starboard, GraphletAnchor::RC);
		this->station->map_graphlet_at_anchor(this->LMOD, CS::LMOD, GraphletAnchor::CC);

		for (auto it = this->intercs.begin(); it != this->intercs.end(); it++) {
			this->station->map_graphlet_at_anchor(it->second, it->first, GraphletAnchor::CC);
		}

		for (auto it = this->nintercs.begin(); it != this->nintercs.end(); it++) {
			/** NOTE
			 * Lines are brush-based shape, they do not have stroke, `Shapelet` does not know how width they are,
			 * thus, we have to do aligning on our own.
			 */
			this->station->map_graphlet_at_anchor(it->second, it->first, GraphletAnchor::LC, -default_pipe_thickness * 0.5F);
		}

		for (auto it = this->hoppers.begin(); it != this->hoppers.end(); it++) {
			it->second->fill_pump_origin(&ox, &oy);

			if (ox == 0.0F) {
				this->station->map_credit_graphlet(it->second, GraphletAnchor::CC, 0.0F, -oy);
			} else {
				this->station->map_credit_graphlet(it->second, GraphletAnchor::CC, -ox, 0.0F);
			}

			ox = max(std::fabsf(ox), std::fabsf(oy));
			switch (it->first) {
			case CS::PSHPump: {
				this->master->move_to(this->captions[it->first], it->second, GraphletAnchor::RC, GraphletAnchor::LC, ox);
				this->master->move_to(this->powers[it->first], it->second, GraphletAnchor::LB, GraphletAnchor::RB, -ox);
				this->master->move_to(this->rpms[it->first], it->second, GraphletAnchor::RB, GraphletAnchor::LB, ox);
				this->master->move_to(this->dpressures[it->first], it->second, GraphletAnchor::CT, GraphletAnchor::LB);
				this->master->move_to(this->vpressures[it->first], it->second, GraphletAnchor::LC, GraphletAnchor::RB, -ox);
			}; break;
			case CS::PSUWPump: {
				this->master->move_to(this->captions[it->first], it->second, GraphletAnchor::LC, GraphletAnchor::RB, -ox);
				this->master->move_to(this->powers[it->first], it->second, GraphletAnchor::LC, GraphletAnchor::RT, -ox, ox);
				this->master->move_to(this->rpms[it->first], it->second, GraphletAnchor::RC, GraphletAnchor::LT, ox, ox);
				this->master->move_to(this->dpressures[it->first], it->second, GraphletAnchor::RC, GraphletAnchor::LB, ox);
				this->master->move_to(this->dfpressures[it->first], this->tips[CS::Port], GraphletAnchor::RC, GraphletAnchor::LB, 0.0F, -ox);
			}; break;
			case CS::SBHPump: {
				this->master->move_to(this->captions[it->first], it->second, GraphletAnchor::RC, GraphletAnchor::LC, ox);
				this->master->move_to(this->powers[it->first], it->second, GraphletAnchor::LT, GraphletAnchor::RT, -ox);
				this->master->move_to(this->rpms[it->first], it->second, GraphletAnchor::RT, GraphletAnchor::LT, ox);
				this->master->move_to(this->dpressures[it->first], it->second, GraphletAnchor::CB, GraphletAnchor::LT);
				this->master->move_to(this->vpressures[it->first], it->second, GraphletAnchor::LC, GraphletAnchor::RT, -ox);
			}; break;
			case CS::SBUWPump: {
				this->master->move_to(this->captions[it->first], it->second, GraphletAnchor::LC, GraphletAnchor::RT, -ox);
				this->master->move_to(this->powers[it->first], it->second, GraphletAnchor::LC, GraphletAnchor::RB, -ox, -ox);
				this->master->move_to(this->rpms[it->first], it->second, GraphletAnchor::RC, GraphletAnchor::LB, ox, -ox);
				this->master->move_to(this->dpressures[it->first], it->second, GraphletAnchor::RC, GraphletAnchor::LT, ox);
				this->master->move_to(this->dfpressures[it->first], this->tips[CS::Starboard], GraphletAnchor::RC, GraphletAnchor::LT, 0.0F, ox);
			}; break;
			}
		}

		this->vlabels[CS::D001]->fill_extent(0.0F, 0.0F, nullptr, &label_height);
		
		for (auto it = this->gvalves.begin(); it != this->gvalves.end(); it++) {
			switch (it->first) {
			case CS::D014: case CS::D016: {
				dx = x0 + gwidth; dy = y0; anchor = GraphletAnchor::LB;
			}; break;
			case CS::D013: case CS::D015: {
				dx = x0 + gwidth; dy = y0; anchor = GraphletAnchor::LB;
			}; break;
			case CS::D006: case CS::D010: case CS::D020: case CS::D021: case CS::D022: case CS::D024: {
				it->second->fill_margin(x0, y0, nullptr, nullptr, &margin, nullptr);
				dx = x0; dy = y0 + gridsize - margin; anchor = GraphletAnchor::CT;
			}; break;
			case CS::D017: {
				dx = x0 + gwidth; dy = y0 - label_height; anchor = GraphletAnchor::LB;
			}; break;
			case CS::D018: {
				dx = x0 + gwidth; dy = y0; anchor = GraphletAnchor::LT;
			}; break;
			case CS::D001: case CS::D002: case CS::D008: case CS::D019: case CS::D026: {
				it->second->fill_margin(x0, y0, &margin, nullptr, nullptr, nullptr);
				dx = x0; dy = y0 - gridsize - label_height + margin; anchor = GraphletAnchor::CB;
			}; break;
			default: {
				it->second->fill_margin(x0, y0, nullptr, &margin, nullptr, nullptr);
				dx = x0 + gridsize - margin; dy = y0; anchor = GraphletAnchor::LB;
			}
			}

			this->station->map_credit_graphlet(it->second, GraphletAnchor::CC, x0, y0);
			this->station->map_credit_graphlet(this->captions[it->first], anchor, dx, dy);
			this->master->move_to(this->vlabels[it->first], this->captions[it->first], GraphletAnchor::CB, GraphletAnchor::CT);
		}

		{ // reflow motor-driven valves
			float polar45 = gridsize * std::sqrtf(2.0F) * 0.618F;

			for (auto it = this->mvalves.begin(); it != this->mvalves.end(); it++) {
				switch (it->first) {
				case CS::D014: case CS::D016: {
					dx = x0 - polar45; dy = y0 - polar45; anchor = GraphletAnchor::CC;
				}; break;
				case CS::D013: case CS::D015: {
					dx = x0 - polar45; dy = y0 + polar45; anchor = GraphletAnchor::CC;
				}; break;
				case CS::D003: case CS::D004: case CS::D005: case CS::D007: case CS::D009:
				case CS::D011: case CS::D012: case CS::D023: case CS::D025: {
					this->gvalves[CS::D003]->fill_margin(x0, y0, nullptr, nullptr, nullptr, &margin);
					dx = x0 - gridsize + margin; dy = y0; anchor = GraphletAnchor::RC;
				}; break;
				case CS::D002: case CS::D008: case CS::D017: case CS::D019: case CS::D026: {
					dx = x0; dy = y0 + gridsize; anchor = GraphletAnchor::CC;
				}; break;
				default: {
					dx = x0; dy = y0 - gridsize; anchor = GraphletAnchor::CC;
				}
				}

				it->second->fill_valve_origin(&ox, &oy);
				this->station->map_credit_graphlet(it->second, anchor, dx - ox, dy - oy);
			}
		}

		{ // reflow dimensions
			float offset = default_pipe_thickness * 2.0F;

			this->master->move_to(this->progresses[CS::D003], this->gvalves[CS::D003], GraphletAnchor::CB, GraphletAnchor::LT, offset, -offset);
			this->master->move_to(this->progresses[CS::D004], this->gvalves[CS::D004], GraphletAnchor::CT, GraphletAnchor::LB, offset);

			this->station->map_credit_graphlet(this->pump_pressures[CS::C], GraphletAnchor::LT, gwidth * 3.0F);
			this->master->move_to(this->pump_pressures[CS::F], this->pump_pressures[CS::C], GraphletAnchor::RC, GraphletAnchor::LC, gwidth);
			this->master->move_to(this->pump_pressures[CS::H], this->pump_pressures[CS::F], GraphletAnchor::RC, GraphletAnchor::LC, gwidth);

			this->master->move_to(this->tips[CS::PSMotor], vinset * 0.618F, vinset * 1.618F, GraphletAnchor::LT);
			this->master->move_to(this->captions[CS::PSMotor], this->tips[CS::PSMotor], GraphletAnchor::CC, GraphletAnchor::CC);
			this->master->move_to(this->motors[CS::PSU], this->tips[CS::PSMotor], GraphletAnchor::LB, GraphletAnchor::LT, 0.0F, 1.0F);
			this->master->move_to(this->motors[CS::PSDE], this->tips[CS::PSMotor], GraphletAnchor::RB, GraphletAnchor::RT, 0.0F, 1.0F);

			this->master->move_to(this->tips[CS::SBMotor], vinset * 0.618F, height - vinset * 1.618F, GraphletAnchor::LB);
			this->master->move_to(this->captions[CS::SBMotor], this->tips[CS::SBMotor], GraphletAnchor::CC, GraphletAnchor::CC);
			this->master->move_to(this->motors[CS::SBMotorDP2], this->tips[CS::SBMotor], GraphletAnchor::LT, GraphletAnchor::LB, 0.0F, -1.0F);
			this->master->move_to(this->motors[CS::SBNDEoil], this->tips[CS::SBMotor], GraphletAnchor::RT, GraphletAnchor::RB, 0.0F, -1.0F);

			for (CS id = CS::PSU; id < CS::PSNDEoil; id++) {
				if (id != CS::PSMotorDP2) {
					this->master->move_to(this->motors[_E(CS, _I(id) + 1U)],
						this->motors[id], GraphletAnchor::LB, GraphletAnchor::LT,
						0.0F, 1.0F);
				}
			}

			for (CS id = CS::SBNDEoil; id > CS::SBU; id--) {
				if (id != CS::SBDE) {
					this->master->move_to(this->motors[_E(CS, _I(id) - 1U)],
						this->motors[id], GraphletAnchor::LT, GraphletAnchor::LB,
						0.0F, -1.0F);
				}
			}
		}

		this->station->map_graphlet_at_anchor(this->functions[CSFunction::Diagnostics], CS::diagnostics, GraphletAnchor::LB, gwidth * 3.0F);
	}

public:
	bool hopper_selected(CS pid) {
		return this->master->is_selected(this->hoppers[pid]);
	}

	bool valves_selected(CS vids[], unsigned int count, int tolerance) {
		bool okay = false;
		int ok = 0;

		for (unsigned int idx = 0; idx < count; idx++) {
			if (this->master->is_selected(this->gvalves[vids[idx]])) {
				ok += 1;

				if (ok >= tolerance) {
					okay = true;
					break;
				}
			}
		}

		return okay;
	}

	template<unsigned int N>
	bool valves_selected(CS(&vids)[N], int tolerance) {
		return this->valves_selected(vids, N, tolerance);
	}

public:
	void draw_relationships(CanvasDrawingSession^ ds, float Width, float Height) {
		float ox, oy, gx, gy, mx, my;

		for (auto it = this->mvalves.begin(); it != this->mvalves.end(); it++) {
			this->master->fill_graphlet_location(it->second, &mx, &my, GraphletAnchor::CC);
			this->master->fill_graphlet_location(this->gvalves[it->first], &gx, &gy, GraphletAnchor::CC);
			it->second->fill_valve_origin(&ox, &oy);

			ds->DrawLine(mx + ox, my + oy, gx, gy, this->relationship_color, 1.0F, this->relationship_style);
		}
	}

private:
	template<class G, typename E>
	void load_valve(std::map<E, G*>& gs, std::map<E, Credit<Labellet, E>*>& ls, std::map<E, Credit<Labellet, E>*>& cs
		, E id, float radius, double degrees) {
		this->load_label(ls, "(" + id.ToString() + ")", id, Colours::Silver, this->label_font);
		this->load_label(cs, id, Colours::Silver, this->label_font);

		gs[id] = this->master->insert_one(new G(radius, degrees), id);
	}
	
	template<class G, class M, typename E>
	void load_valves(std::map<E, G*>& gs, std::map<E, M*>& ms, std::map<E, Credit<Labellet, E>*>& ls
		, std::map<E, Credit<Labellet, E>*>& cs, E id0, E idn, float radius, double degrees) {
		float mradius = radius * 0.8F;

		for (E id = id0; id <= idn; id++) {
			double mdegrees = 0.0;

			switch (id) {
			case CS::D014: case CS::D016: mdegrees = 45.0; break;
			case CS::D013: case CS::D015: mdegrees = -45.0; break;
			case CS::D002: case CS::D008: case CS::D009: case CS::D017: case CS::D019: case CS::D026: mdegrees = -180.0; break;
			}

			// moter-driven valves' second, catching events first 
			this->load_valve(gs, ls, cs, id, radius, degrees);
			ms[id] = this->master->insert_one(new M(mradius, mdegrees, false), id);
		}
	}

	template<class B, typename CMD>
	void load_buttons(std::map<CMD, Credit<B, CMD>*>& bs, float width = 128.0F, float height = 32.0F) {
		for (CMD cmd = _E(CMD, 0); cmd < CMD::_; cmd++) {
			bs[cmd] = this->master->insert_one(new Credit<B, CMD>(cmd.ToString(), width, height), cmd);
		}
	}

	template<class G, typename E>
	void load_pump(std::map<E, G*>& gs, std::map<E, Credit<Labellet, E>*>& ls, E id, float rx, float fy, double degrees, float gapsize) {
		E title = E::_;
		E start = E::_;
		E end = E::_;

		this->load_label(ls, id, Colours::Salmon, this->caption_font);

		gs[id] = this->master->insert_one(new G(rx, std::fabsf(rx) * fy, degrees), id);
		
		this->load_dimension(this->powers, id, "kwatt", 0);
		this->load_dimension(this->rpms, id, "rpm", 0);
		this->load_dimension(this->dpressures, id, "bar", 1);

		switch (id) {
		case E::PSHPump: case E::SBHPump: {
			this->load_dimension(this->vpressures, id, "bar", 1);
		}; break;
		case E::PSUWPump: {
			this->load_dimension(this->dfpressures, id, "bar", 1);
			
			title = E::PSMotor;
			start = E::PSU;
			end = E::PSNDEoil;
		}; break;
		case E::SBUWPump: {
			this->load_dimension(this->dfpressures, id, "bar", 1);
			
			title = E::SBMotor;
			start = E::SBU;
			end = E::SBNDEoil;
		}; break;
		}

		/*
		if (title != E::_) {
			Platform::String^ unit = nullptr;
			float mwidth, mheight;
			
			for (E mid = start; mid <= end; mid++) {
				switch (mid) {
				case E::PSMotorDP1: case E::PSMotorDP2: unit = "bar"; this->motor_style.precision = 2; break;
				case E::SBMotorDP1: case E::SBMotorDP2: unit = "bar"; this->motor_style.precision = 2; break;
				default: unit = "celsius"; this->motor_style.precision = 0; break;
				}

				this->motors[mid] = this->master->insert_one(new Credit<Dimensionlet, E>(this->motor_style, unit, _speak(mid)), mid);
			}

			this->motors[start]->fill_extent(0.0F, 0.0F, &mwidth, &mheight);
			
			this->tips[title] = this->master->insert_one(
				new Rectanglet(mwidth * 2.0F + gapsize, mheight,
					Colours::RoyalBlue, Colours::DodgerBlue));

			this->captions[title] = this->master->insert_one(
				new Credit<Labellet, E>(_speak(title), this->motor_style.label_font, this->motor_style.label_color),
				title);
		}
		*/
	}

	template<typename E>
	void load_percentage(std::map<E, Credit<Percentagelet, E>*>& ps, E id) {
		ps[id] = this->master->insert_one(new Credit<Percentagelet, E>(this->plain_style), id);
	}

	template<typename E>
	void load_dimension(std::map<E, Credit<Dimensionlet, E>*>& ds, E id, Platform::String^ unit, int precision) {
		this->hopper_style.precision = precision;
		ds[id] = this->master->insert_one(new Credit<Dimensionlet, E>(this->hopper_style, unit), id);
	}

	template<typename E>
	void load_dimensions(std::map<E, Credit<Dimensionlet, E>*>& ds, E id0, E idn, Platform::String^ unit) {
		for (E id = id0; id <= idn; id++) {
			ds[id] = this->master->insert_one(new Credit<Dimensionlet, E>(unit, id.ToString()), id);

			ds[id]->set_style(DimensionState::Default, this->pump_style);
			ds[id]->set_style(DimensionState::Highlight, this->highlight_style);
		}
	}

	template<typename E>
	void load_label(std::map<E, Credit<Labellet, E>*>& ls, Platform::String^ caption, E id
		, CanvasSolidColorBrush^ color, CanvasTextFormat^ font = nullptr) {
		ls[id] = this->master->insert_one(new Credit<Labellet, E>(caption, font, color), id);
	}

	template<typename E>
	void load_label(std::map<E, Credit<Labellet, E>*>& ls, E id, CanvasSolidColorBrush^ color, CanvasTextFormat^ font = nullptr) {
		this->load_label(ls, _speak(id), id, color, font);
	}

private:
	void set_valves_status(CS id
		, const uint8* db4, unsigned int gidx4_p1, unsigned int midx4_p1
		, const uint8* db205, unsigned int gidx205_p1, unsigned int midx205_p1) {
		DI_gate_valve(this->gvalves[id], db4, gidx4_p1, db205, gidx205_p1);

		if (this->mvalves.find(id) != this->mvalves.end()) {
			DI_motor_valve(this->mvalves[id], db4, midx4_p1, db205, midx205_p1);
		}
	}

private:
	bool valve_open(CS vid) {
		return (this->gvalves[vid]->get_state() == GateValveState::Open);
	}

	void try_flow_water(CS vid, CS eid1, CS eid2, CanvasSolidColorBrush^ color) {
		if (this->valve_open(vid)) {
			this->station->push_subtrack(vid, eid1, color);

			if (eid2 != CS::_) {
				this->station->push_subtrack(vid, eid2, color);
			}
		}
	}

	void try_flow_water(CS vid, CS* path, unsigned int count, CanvasSolidColorBrush^ color) {
		if (this->valve_open(vid)) {
			this->station->push_subtrack(vid, path[0], color);
			this->station->push_subtrack(path, count, color);
		}
	}

	template<unsigned int N>
	void try_flow_water(CS vid, CS(&path)[N], CanvasSolidColorBrush^ color) {
		this->try_flow_water(vid, path, N, color);
	}

	void try_flow_water(CS vid, CS eid, CanvasSolidColorBrush^ color) {
		this->try_flow_water(vid, eid, CS::_, color);
	}

// never deletes these graphlets mannually
private:
	Tracklet<CS>* station;
	Linelet* gantry_pipe;
	std::map<CS, Credit<Labellet, CS>*> captions;
	std::map<CS, Credit<HopperPumplet, CS>*> hoppers;
	std::map<CS, Credit<GateValvelet, CS>*> gvalves;
	std::map<CS, Credit<MotorValvelet, CS>*> mvalves;
	std::map<CS, Credit<Labellet, CS>*> vlabels;
	std::map<CS, Credit<Percentagelet, CS>*> progresses;
	std::map<CS, Credit<Dimensionlet, CS>*> pump_pressures;
	std::map<CS, Credit<Dimensionlet, CS>*> dpressures;
	std::map<CS, Credit<Dimensionlet, CS>*> vpressures;
	std::map<CS, Credit<Dimensionlet, CS>*> dfpressures;
	std::map<CS, Credit<Dimensionlet, CS>*> motors;
	std::map<CS, Credit<Dimensionlet, CS>*> powers;
	std::map<CS, Credit<Dimensionlet, CS>*> rpms;
	std::map<CS, Omegalet*> nintercs;
	std::map<CS, Circlelet*> intercs;
	std::map<CS, Shapelet*> tips;
	Arclet* LMOD;

private:
	std::map<CSFunction, Credit<Buttonlet, CSFunction>*> functions;
	
private:
	CanvasTextFormat^ caption_font;
	CanvasTextFormat^ label_font;
	CanvasTextFormat^ special_font;
	ICanvasBrush^ relationship_color;
	CanvasStrokeStyle^ relationship_style;
	DimensionStyle pump_style;
	DimensionStyle highlight_style;
	DimensionStyle plain_style;
	DimensionStyle hopper_style;
	DimensionStyle motor_style;

private:
	ChargesPage* master;
};

private class VesselDecorator : public TVesselDecorator<Vessel, CS> {
public:
	VesselDecorator(Vessel* master) : TVesselDecorator<Vessel, CS>(master) {}

public:
	void draw_non_important_lines(Tracklet<CS>* station, CanvasDrawingSession^ ds, float x, float y, CanvasStrokeStyle^ style) override {
		float d0525_x, d05_y, d25_y;

		station->fill_anchor_location(CS::D005, &d0525_x, &d05_y, false);
		station->fill_anchor_location(CS::D025, nullptr, &d25_y, false);

		ds->DrawLine(x + d0525_x, y + d05_y, x + d0525_x, y + d25_y,
			Colours::DimGray, default_pipe_thickness, style);
	}
};

/*************************************************************************************************/
ChargesPage::ChargesPage(PLCMaster* plc) : Planet(__MODULE__), device(plc) {
	Vessel* dashboard = new Vessel(this);

	this->dashboard = dashboard;
	
	if (this->device != nullptr) {
		this->diagnostics = new HopperPumpDiagnostics(plc);

		this->gate_valve_op = make_gate_valve_menu(DO_gate_valve_action, plc);
		this->ghopper_op = make_charge_condition_menu(GroupChargeAction::BothHopper, plc);
		this->gunderwater_op = make_charge_condition_menu(GroupChargeAction::BothUnderWater, plc);
		this->ghbarge_op = make_charge_condition_menu(GroupChargeAction::HPBarge, plc);
		this->guwbarge_op = make_charge_condition_menu(GroupChargeAction::UWPBarge, plc);
		this->ghps_op = make_charge_condition_menu(GroupChargeAction::PSHopper, plc);
		this->guwps_op = make_charge_condition_menu(GroupChargeAction::PSUnderWater, plc);
		this->ghsb_op = make_charge_condition_menu(GroupChargeAction::SBHopper, plc);
		this->guwsb_op = make_charge_condition_menu(GroupChargeAction::SBUnderWater, plc);
		this->ps_hopper_op = make_ps_hopper_pump_charge_menu(plc);
		this->sb_hopper_op = make_sb_hopper_pump_charge_menu(plc);
		this->ps_underwater_op = make_ps_underwater_pump_charge_menu(plc);
		this->sb_underwater_op = make_sb_underwater_pump_charge_menu(plc);

		this->device->push_confirmation_receiver(dashboard);
	}

	{ // load decorators
		this->grid = new GridDecorator();

#ifdef _DEBUG
		this->push_decorator(this->grid);
#else
		this->grid->set_active_planet(this);
#endif

		this->push_decorator(new VesselDecorator(dashboard));
	}
}

ChargesPage::~ChargesPage() {
	if (this->dashboard != nullptr) {
		delete this->dashboard;
	}

#ifndef _DEBUG
	delete this->grid;
#endif
}

void ChargesPage::load(CanvasCreateResourcesReason reason, float width, float height) {
	auto dashboard = dynamic_cast<Vessel*>(this->dashboard);
	
	if (dashboard != nullptr) {
		float vinset = statusbar_height();
		float gwidth = width / 64.0F;
		float gheight = height / 36.0F;

		this->grid->set_grid_width(gwidth);
		this->grid->set_grid_height(gheight, vinset);
		
		dashboard->construct(gwidth, gheight);
		dashboard->load(width, height, gwidth, gheight);
	}
}

void ChargesPage::reflow(float width, float height) {
	auto dashboard = dynamic_cast<Vessel*>(this->dashboard);
	
	if (dashboard != nullptr) {
		float vinset = statusbar_height();
		float gwidth = this->grid->get_grid_width();
		float gheight = this->grid->get_grid_height();

		dashboard->reflow(width, height, gwidth, gheight, vinset);
	}
}

void ChargesPage::on_timestream(long long timepoint_ms, size_t addr0, size_t addrn, uint8* data, size_t size, Syslog* logger) {
	auto dashboard = dynamic_cast<Vessel*>(this->dashboard);

	if (dashboard != nullptr) {
		dashboard->on_all_signals(timepoint_ms, addr0, addrn, data, size, logger);
	}
}

bool ChargesPage::can_select(IGraphlet* g) {
	return ((this->device != nullptr)
		&& (dynamic_cast<Credit<Buttonlet, CSFunction>*>(g)
			|| (dynamic_cast<GateValvelet*>(g) != nullptr)
			|| (dynamic_cast<HopperPumplet*>(g) != nullptr)));
}

bool ChargesPage::can_select_multiple() {
	return (this->device != nullptr);
}

void ChargesPage::on_tap_selected(IGraphlet* g, float local_x, float local_y) {
	auto gvalve = dynamic_cast<GateValvelet*>(g);
	auto hpump = dynamic_cast<Credit<HopperPumplet, CS>*>(g);
	auto diagnose = dynamic_cast<Credit<Buttonlet, CSFunction>*>(g);
	
	if (gvalve != nullptr) {
		menu_popup(this->gate_valve_op, g, local_x, local_y);
	} else if (diagnose != nullptr) {
		this->diagnostics->show();
	} else if (hpump != nullptr) {
		switch (hpump->id) {
		case CS::PSHPump: menu_popup(this->ps_hopper_op, g, local_x, local_y); break;
		case CS::SBHPump: menu_popup(this->sb_hopper_op, g, local_x, local_y); break;
		case CS::PSUWPump: menu_popup(this->ps_underwater_op, g, local_x, local_y); break;
		case CS::SBUWPump: menu_popup(this->sb_underwater_op, g, local_x, local_y); break;
		}
	}
}

void ChargesPage::on_gesture(std::list<float2>& anchors, float x, float y) {
	auto dashboard = dynamic_cast<Vessel*>(this->dashboard);

	if (dashboard != nullptr) {
		bool ps_underwater = dashboard->hopper_selected(CS::PSUWPump);
		bool sb_underwater = dashboard->hopper_selected(CS::SBUWPump);
		bool ps_hopper = dashboard->hopper_selected(CS::PSHPump);
		bool sb_hopper = dashboard->hopper_selected(CS::SBHPump);
		CS barges[] = { CS::D024 };
		CS uwps[] = { CS::D004, CS::D006, CS::D009 };
		CS hps[] = { CS::D004, CS::D005, CS::D017 };
		CS uwsb[] = { CS::D003, CS::D026, CS::D007 };
		CS hsb[] = { CS::D003, CS::D025, CS::D018 };

		if (ps_underwater && sb_underwater) {
			group_menu_popup(this->gunderwater_op, this, x, y);
		} else if (ps_hopper && sb_hopper) {
			group_menu_popup(this->ghopper_op, this, x, y);
		} else if (sb_underwater) {
			if (dashboard->valves_selected(barges, 1)) {
				group_menu_popup(this->guwbarge_op, this, x, y);
			} else if (dashboard->valves_selected(uwsb, 3)) {
				group_menu_popup(this->guwsb_op, this, x, y);
			}
		} else if (sb_hopper) {
			if (dashboard->valves_selected(barges, 1)) {
				group_menu_popup(this->ghbarge_op, this, x, y);
			} else if (dashboard->valves_selected(uwsb, 1)) {
				group_menu_popup(this->ghsb_op, this, x, y);
			}
		} else if (ps_underwater) {
			if (dashboard->valves_selected(uwps, 3)) {
				group_menu_popup(this->guwps_op, this, x, y);
			}
		} else if (ps_hopper) {
			if (dashboard->valves_selected(hps, 3)) {
				group_menu_popup(this->ghps_op, this, x, y);
			}
		}
	}
}
