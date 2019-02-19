#include <map>

#include "page/diagnostics/dredges_dx.hpp"
#include "configuration.hpp"

#include "module.hpp"
#include "string.hpp"
#include "brushes.hxx"

#include "graphlet/shapelet.hpp"
#include "graphlet/buttonlet.hpp"
#include "graphlet/dashboard/alarmlet.hpp"

#include "iotables/ai_metrics.hpp"
#include "iotables/ai_dredges.hpp"

#include "iotables/di_winches.hpp"
#include "iotables/di_dredges.hpp"
#include "iotables/di_devices.hpp"
#include "iotables/di_pumps.hpp"
#include "iotables/di_hopper_pumps.hpp"

using namespace WarGrey::SCADA;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI;
using namespace Microsoft::Graphics::Canvas::Text;
using namespace Microsoft::Graphics::Canvas::Brushes;

static CanvasSolidColorBrush^ region_background = Colours::make(diagnostics_region_background);
static CanvasSolidColorBrush^ diagnosis_background = Colours::make(diagnostics_alarm_background);

static CanvasSolidColorBrush^ subcolor = Colours::DimGray;
static CanvasSolidColorBrush^ subcolor_highlight = Colours::DodgerBlue;
static CanvasSolidColorBrush^ diagnosis_color = Colours::Silver;
static CanvasSolidColorBrush^ metrics_color = Colours::Yellow;
static CanvasSolidColorBrush^ tips_color = Colours::CadetBlue;

private enum class WGFunction { Block, UpperCheck, LowerCheck, _ }; // NOTE: LowerCheck is actually the Saddle Check

// WARNING: order matters
private enum class WG : unsigned int {
	// Groups
	MiscCondition, WinchCondition, GantryCondition, WinchMetrics,

	// Hydraulical Pumps
	A, B, C, H, F, G,

	// Addition Labels
	Draghead, Intermediate, Trunnion,
	Remote, Scene, Allowed,
	Pull_Push, WindUp_Out,

	// Misc Conditions
	PumpsRunning,
	NoConsoleStop, NoSceneStop,
	NoHeadLifting, NoIntermediateLifting,
	ConsoleAllowed, BackOilPressureOkay,

	// Gantry Conditions
	WinchAtTop, NoPulledIn, NoPushedOut,

	// Winch Conditions
	AllGantriesInOrOut, HopperStopped, NoInflating,
	NoSoftUpper, NoSoftLower, NoUpper, NoSuction, NoSlack, NoSaddle,

	// Winch Metrics
	CurrentPulse, UpperPulse, LowerPulse, SaddlePulse,

	_
};

private class DredgesDx final : public PLCConfirmation {
public:
	DredgesDx(DredgesDiagnostics* master, DX side) : master(master), side(side) {
		this->region_font = make_bold_text_format("Microsoft YaHei", normal_font_size);
		this->diagnosis_font = make_bold_text_format("Microsoft YaHei", small_font_size);

		this->plain_style.number_font = make_bold_text_format("Microsoft YaHei", tiny_font_size);
		this->plain_style.unit_font = this->plain_style.number_font;
		this->plain_style.precision = 1U;

		this->btn_style.font = this->plain_style.unit_font;

		this->color = Colours::make((side == DX::PS) ? default_ps_color : default_sb_color);
		this->inset_ratio = 1.618F;

		this->misc_start = WG::PumpsRunning;
		this->misc_end = WG::BackOilPressureOkay;
		this->gantry_start = WG::WinchAtTop;
		this->gantry_end = WG::NoPushedOut;
		this->winch_start = WG::AllGantriesInOrOut;
		this->winch_end = WG::NoSaddle;
		this->metrics_start = WG::CurrentPulse;
		this->metrics_end = WG::SaddlePulse;
		
		if (side == DX::PS) {
			this->sublabel_start = WG::A;
			this->sublabel_end = WG::C;

			this->winch_limits = &winch_ps_trunnion_limits;
			this->winch_details = &winch_ps_trunnion_details;
		} else {
			this->sublabel_start = WG::H;
			this->sublabel_end = WG::G;

			this->winch_limits = &winch_sb_trunnion_limits;
			this->winch_details = &winch_sb_trunnion_details;
		}
	}

public:
	void pre_read_data(Syslog* logger) override {
		this->master->enter_critical_section();
		this->master->begin_update_sequence();
	}

	void on_digital_input(long long timepoint_ms, const uint8* DB4, size_t count4, const uint8* DB205, size_t count205, Syslog* logger) override {
		bool ps = (this->side == DX::PS);
		unsigned int draghead_lift = (ps ? console_ps_draghead_winch_lift_button : console_sb_draghead_winch_lift_button) - 1U;
		unsigned int intermediate_lift = (ps ? console_ps_intermediate_winch_lift_button : console_sb_intermediate_winch_lift_button) - 1U;
		unsigned int emergence = (ps ? console_ps_winch_gantry_stop_button : console_sb_winch_gantry_stop_button) - 1U;
		unsigned int console_allowed = (ps ? ps_winch_gantry_allowed : sb_winch_gantry_allowed) - 1U;
		auto remote_color = (DI_dredges_remote_control(DB4, this->details) ? subcolor_highlight : subcolor);
		auto scene_color = (DI_dredges_scene_control(DB4, this->details) ? subcolor_highlight : subcolor);
		bool pumps_running = true;
		bool scene_stop = false;

		for (WG id = this->sublabel_start; id <= this->sublabel_end; id++) {
			switch (id) {
			case WG::A: pumps_running &= this->check_hydraulic_pump(id, DB4, pump_A_feedback); break;
			case WG::B: pumps_running &= this->check_hydraulic_pump(id, DB4, pump_B_feedback); break;
			case WG::C: pumps_running &= this->check_hydraulic_pump(id, DB4, pump_C_feedback); break;
			case WG::H: pumps_running &= this->check_hydraulic_pump(id, DB4, pump_H_feedback); break;
			case WG::G: pumps_running &= this->check_hydraulic_pump(id, DB4, pump_G_feedback); break;
			case WG::F: pumps_running &= this->check_hydraulic_pump(id, DB4, pump_F_feedback); break;
			}
		}

		for (WG id = WG::Draghead; id <= WG::Trunnion; id++) {
			switch (id) {
			case WG::Draghead: scene_stop |= this->check_scene_stop(id, DB4, (ps ? ps_trunnion_details : sb_trunnion_details)); break;
			case WG::Intermediate: scene_stop |= this->check_scene_stop(id, DB4, (ps ? ps_trunnion_details : sb_trunnion_details)); break;
			case WG::Trunnion: scene_stop |= this->check_scene_stop(id, DB4, (ps ? ps_trunnion_details : sb_trunnion_details)); break;
			}
		}

		this->diagnoses[WG::PumpsRunning]->set_state(pumps_running, AlarmState::Notice, AlarmState::None);
		this->diagnoses[WG::NoConsoleStop]->set_state(DBX(DB4, emergence), AlarmState::None, AlarmState::Notice);
		this->diagnoses[WG::NoSceneStop]->set_state(scene_stop, AlarmState::None, AlarmState::Notice);
		this->diagnoses[WG::NoHeadLifting]->set_state(DBX(DB4, draghead_lift), AlarmState::None, AlarmState::Notice);
		this->diagnoses[WG::NoIntermediateLifting]->set_state(DBX(DB4, intermediate_lift), AlarmState::None, AlarmState::Notice);
		this->diagnoses[WG::ConsoleAllowed]->set_state(DBX(DB205, console_allowed), AlarmState::Notice, AlarmState::None);
		this->diagnoses[WG::BackOilPressureOkay]->set_state(DBX(DB205, backoil_pressure_too_low - 1U), AlarmState::None, AlarmState::Notice);

		DI_boolean_button(this->buttons[WGFunction::Block], DB205, this->winch_details->override);
		DI_boolean_button(this->buttons[WGFunction::UpperCheck], DB205, this->winch_details->upper_check);

		// NOTE: WGFunction::LowerCheck is actually the Saddle Check
		DI_boolean_button(this->buttons[WGFunction::LowerCheck], DB205, this->winch_details->saddle_check);

		{ // check gantry conditions
			unsigned int gantry_limited = this->gantry_limit;
			bool trunnion_upper = DI_winch_upper_limited(DB4, (ps ? &winch_ps_trunnion_limits : &winch_sb_trunnion_limits));
			bool trunnion_soft_upper = DI_winch_soft_upper_limited(DB205, (ps ? &winch_ps_trunnion_details : &winch_sb_trunnion_details));

			this->subgantries[WG::Remote]->set_color(remote_color);
			this->subgantries[WG::Scene]->set_color(scene_color);
			this->subgantries[WG::Allowed]->set_color(DI_dredges_gantry_allowed(DB4, this->details) ? subcolor_highlight : subcolor);

			if (gantry_limited == 0U) {
				gantry_limited = (DI_long_sb_drag(DB205) ? gantry_sb_long_draghead_limited : gantry_sb_short_draghead_limited);
			}

			this->diagnoses[WG::WinchAtTop]->set_state(trunnion_upper || trunnion_soft_upper, AlarmState::Notice, AlarmState::None);
			this->diagnoses[WG::NoPulledIn]->set_state(DI_gantry_pulled_in(DB4, gantry_limited), AlarmState::None, AlarmState::Notice);
			this->diagnoses[WG::NoPushedOut]->set_state(DI_gantry_pushed_out(DB4, gantry_limited), AlarmState::None, AlarmState::Notice);
		}

		{ // check winch conditions
			unsigned int hopper_feedback = (ps ? ps_hopper_pump_feedback : sb_hopper_pump_feedback);
			unsigned int suction_btn_p1 = (ps ? suction_ps_buttons : suction_sb_buttons);
			bool gantries_in = DBX(DB205, (ps ? all_ps_gantries_in : all_sb_gantries_in) - 1U);
			bool gantries_out = DBX(DB205, (ps ? all_ps_gantries_out : all_sb_gantries_out) - 1U);
			bool suction_limited = DI_winch_suction_limited(DB4, this->winch_limits);
			bool stopped = (DI_hopper_type(DB4, hopper_feedback) || DI_underwater_type(DB4, hopper_feedback))
				&& (!DI_hopper_pump_running(DB4, hopper_feedback, true));

			this->subwinches[WG::Remote]->set_color(remote_color);
			this->subwinches[WG::Scene]->set_color(scene_color);
			this->subwinches[WG::Allowed]->set_color(DI_dredges_winch_allowed(DB4, this->details) ? subcolor_highlight : subcolor);

			this->diagnoses[WG::AllGantriesInOrOut]->set_state(gantries_in || gantries_out, AlarmState::Notice, AlarmState::None);
			this->diagnoses[WG::HopperStopped]->set_state((suction_limited && stopped) || (!suction_limited), AlarmState::Notice, AlarmState::None);
			this->diagnoses[WG::NoInflating]->set_state(DI_suction_inflating(DB205, suction_btn_p1), AlarmState::None, AlarmState::Notice);
			this->diagnoses[WG::NoSoftUpper]->set_state(DI_winch_soft_upper_limited(DB205, this->winch_details), AlarmState::None, AlarmState::Notice);
			this->diagnoses[WG::NoSoftLower]->set_state(DI_winch_soft_lower_limited(DB205, this->winch_details), AlarmState::None, AlarmState::Notice);
			this->diagnoses[WG::NoUpper]->set_state(DI_winch_upper_limited(DB4, this->winch_limits), AlarmState::None, AlarmState::Notice);
			this->diagnoses[WG::NoSaddle]->set_state(DI_winch_saddle_limited(DB4, this->winch_limits), AlarmState::None, AlarmState::Notice);
			this->diagnoses[WG::NoSuction]->set_state(suction_limited, AlarmState::None, AlarmState::Notice);
			this->diagnoses[WG::NoSlack]->set_state(DI_winch_slack(DB4, this->winch_limits), AlarmState::None, AlarmState::Notice);
		}
	}

	void on_analog_input(long long timepoint_ms, const uint8* DB2, size_t count2, const uint8* DB203, size_t count203, Syslog* logger) override {
		this->metrics[WG::CurrentPulse]->set_text(flstring(DBD(DB2, this->pulses + 16U), 0));
		this->metrics[WG::UpperPulse]->set_text(flstring(DBD(DB2, this->pulses + 0U), 0));
		this->metrics[WG::LowerPulse]->set_text(flstring(DBD(DB2, this->pulses + 4U), 0));
		this->metrics[WG::SaddlePulse]->set_text(flstring(DBD(DB2, this->pulses + 8U), 0));

		this->backoil->set_value(RealData(DB203, master_back_oil_pressure));
	}

	void post_read_data(Syslog* logger) override {
		this->master->end_update_sequence();
		this->master->leave_critical_section();
	}

public:
	void fill_extent(float title_height, float vgapsize, float* width, float* height) {
		unsigned int mc_count = _I(this->misc_end) - _I(this->misc_start) + 1U;
		unsigned int gc_count = _I(this->gantry_end) - _I(this->gantry_start) + 1U;
		unsigned int wc_count = _I(this->winch_end) - _I(this->winch_start) + 1U;
		
		this->diagnosis_height = this->diagnosis_font->FontSize * 2.0F;
		this->region_reserved_height = vgapsize * 4.0F + this->region_font->FontSize;
		this->misc_region_height = (this->diagnosis_height + vgapsize) * float(mc_count) + region_reserved_height;
		this->gantry_region_height = (this->diagnosis_height + vgapsize) * float(gc_count) + region_reserved_height;
		this->winch_region_height = (this->diagnosis_height + vgapsize) * float(wc_count) + region_reserved_height;
		this->metrics_region_height = this->winch_region_height + this->gantry_region_height - this->misc_region_height;
		
		SET_BOX(width, 800.0F);
		SET_BOX(height, this->winch_region_height + this->gantry_region_height + title_height * 3.0F);
	}

	void load(float width, float height, float title_height, float vgapsize) {
		float region_width = width * 0.5F * 0.90F;
		float diagnosis_width = (region_width - title_height * 1.5F);
		float corner_radius = 8.0F;
		
		this->misc_region = this->master->insert_one(
			new RoundedRectanglet(region_width, this->misc_region_height, corner_radius, region_background));

		this->gantry_region = this->master->insert_one(
			new RoundedRectanglet(region_width, this->gantry_region_height, corner_radius, region_background));

		this->winch_region = this->master->insert_one(
			new RoundedRectanglet(region_width, this->winch_region_height, corner_radius, region_background));

		this->metrics_region = this->master->insert_one(
			new RoundedRectanglet(region_width, this->metrics_region_height, corner_radius, region_background));

		this->load_label(this->labels, WG::MiscCondition, side, this->color, this->region_font);
		this->load_label(this->labels, WG::GantryCondition, WG::Trunnion, this->color, this->region_font);
		this->load_label(this->labels, WG::WinchCondition, WG::Trunnion, this->color, this->region_font);
		this->load_label(this->labels, WG::WinchMetrics, WG::Trunnion, this->color, this->region_font);
		this->load_label(this->labels, WG::Pull_Push, tips_color, this->plain_style.unit_font);
		this->load_label(this->labels, WG::WindUp_Out, tips_color, this->plain_style.unit_font);

		for (WG id = WG::Remote; id <= WG::Allowed; id++) {
			this->load_label(this->subwinches, id, subcolor, this->plain_style.unit_font);
			this->load_label(this->subgantries, id, subcolor, this->plain_style.unit_font);
		}

		{ // load diagnoses
			float icon_size = this->diagnosis_height * 0.618F;
			
			for (WG id = this->misc_start; id <= this->winch_end; id++) {
				this->slots[id] = this->master->insert_one(new Credit<RoundedRectanglet, WG>(
					diagnosis_width, this->diagnosis_height, corner_radius, diagnosis_background), id);

				this->diagnoses[id] = this->master->insert_one(new Credit<Alarmlet, WG>(icon_size), id);

				switch (id) {
				case WG::HopperStopped: case WG::NoInflating: {
					this->load_label(this->labels, id, side, diagnosis_color, this->diagnosis_font);
				}; break;
				default: {
					this->load_label(this->labels, id, diagnosis_color, this->diagnosis_font);
				}
				}
			}

			for (WG id = this->sublabel_start; id <= this->sublabel_end; id++) {
				this->load_label(this->labels, id, subcolor, this->plain_style.unit_font);
			}

			for (WG id = WG::Draghead; id <= WG::Trunnion; id++) {
				this->load_label(this->labels, id, subcolor, this->plain_style.unit_font);
			}

			this->backoil = this->master->insert_one(new Dimensionlet(this->plain_style, "bar"));
		}

		{ // load metrics
			unsigned int mcount = _I(this->metrics_end) - _I(this->metrics_start) + 1U;
			float pulse_slot_height = (this->metrics_region_height - this->region_reserved_height) / float(mcount) - vgapsize;
			float btn_width = diagnosis_width * 0.25F;
			float btn_height = pulse_slot_height * 0.75F;
			
			for (WG id = this->metrics_start; id <= this->metrics_end; id++) {
				this->slots[id] = this->master->insert_one(new Credit<RoundedRectanglet, WG>(
					diagnosis_width, pulse_slot_height, corner_radius, diagnosis_background), id);

				this->load_label(this->labels, id, diagnosis_color, this->diagnosis_font);
				this->load_label(this->metrics, id, metrics_color, this->diagnosis_font);

				this->metrics[id]->set_text("0");
			}

			this->load_buttons(this->buttons, btn_width, btn_height);
		}
	}

	void reflow(float width, float height, float title_height, float vgapsize) {
		{ // reflow layout
			float gapsize = (height - title_height - this->winch_region_height - this->gantry_region_height) / 3.0F;

			this->master->move_to(this->misc_region, width * 0.25F, title_height + gapsize, GraphletAnchor::CT);
			this->master->move_to(this->metrics_region, this->misc_region, GraphletAnchor::CB, GraphletAnchor::CT, 0.0F, gapsize);
			this->master->move_to(this->winch_region, width * 0.75F, title_height + gapsize, GraphletAnchor::CT);
			this->master->move_to(this->gantry_region, this->winch_region, GraphletAnchor::CB, GraphletAnchor::CT, 0.0F, gapsize);
			
			this->master->move_to(this->labels[WG::MiscCondition], this->misc_region, GraphletAnchor::CT, GraphletAnchor::CT, 0.0F, vgapsize);
			this->master->move_to(this->labels[WG::GantryCondition], this->gantry_region, GraphletAnchor::CT, GraphletAnchor::CT, 0.0F, vgapsize);
			this->master->move_to(this->labels[WG::WinchCondition], this->winch_region, GraphletAnchor::CT, GraphletAnchor::CT, 0.0F, vgapsize);
			this->master->move_to(this->labels[WG::WinchMetrics], this->metrics_region, GraphletAnchor::CT, GraphletAnchor::CT, 0.0F, vgapsize);
		}

		this->reflow(this->slots, WG::MiscCondition, this->misc_start, this->misc_end, GraphletAnchor::CB, GraphletAnchor::CT, 0.0F, vgapsize);
		this->reflow(this->slots, WG::GantryCondition, this->gantry_start, this->gantry_end, GraphletAnchor::CB, GraphletAnchor::CT, 0.0F, vgapsize);
		this->reflow(this->slots, WG::WinchCondition, this->winch_start, this->winch_end, GraphletAnchor::CB, GraphletAnchor::CT, 0.0F, vgapsize);
		this->reflow(this->slots, WG::WinchMetrics, this->metrics_start, this->metrics_end, GraphletAnchor::CB, GraphletAnchor::CT, 0.0F, vgapsize);
		
		{ // reflow diagnostics
			float inset = vgapsize * this->inset_ratio;
			float step = vgapsize;
			float icon_width;

			this->diagnoses[this->misc_start]->fill_extent(0.0F, 0.0F, &icon_width, nullptr);
			step += icon_width;

			this->master->move_to(this->labels[WG::WindUp_Out], this->slots[this->winch_start], 0.0F, this->labels[WG::WinchCondition], 1.0F, GraphletAnchor::LB, inset);
			this->master->move_to(this->subwinches[WG::Allowed], this->slots[this->winch_start], 1.0F, this->labels[WG::WinchCondition], 1.0F, GraphletAnchor::RB, -inset);
			this->master->move_to(this->subwinches[WG::Scene], this->subwinches[WG::Allowed], GraphletAnchor::LB, GraphletAnchor::RB);
			this->master->move_to(this->subwinches[WG::Remote], this->subwinches[WG::Scene], GraphletAnchor::LB, GraphletAnchor::RB, -vgapsize);

			this->master->move_to(this->labels[WG::Pull_Push], this->slots[this->gantry_start], 0.0F, this->labels[WG::GantryCondition], 1.0F, GraphletAnchor::LB, inset);
			this->master->move_to(this->subgantries[WG::Allowed], this->slots[this->gantry_start], 1.0F, this->labels[WG::GantryCondition], 1.0F, GraphletAnchor::RB, -inset);
			this->master->move_to(this->subgantries[WG::Scene], this->subgantries[WG::Allowed], GraphletAnchor::LB, GraphletAnchor::RB);
			this->master->move_to(this->subgantries[WG::Remote], this->subgantries[WG::Scene], GraphletAnchor::LB, GraphletAnchor::RB, -vgapsize);

			for (WG id = this->misc_start; id <= this->misc_end; id++) {
				this->master->move_to(this->diagnoses[id], this->slots[id], GraphletAnchor::LC, GraphletAnchor::LC, step * 0.0F + inset);
				this->master->move_to(this->labels[id], this->slots[id], GraphletAnchor::LC, GraphletAnchor::LC, step * 1.0F + inset + vgapsize);

				switch (id) {
				case WG::PumpsRunning: {
					this->reflow(this->labels, id, this->sublabel_start, this->sublabel_end, GraphletAnchor::RB, GraphletAnchor::LB, vgapsize, 0.0F);
				}; break;
				case WG::NoSceneStop: {
					this->reflow(this->labels, id, WG::Draghead, WG::Trunnion, GraphletAnchor::RB, GraphletAnchor::LB, vgapsize, 0.0F);
				}; break;
				case WG::BackOilPressureOkay: {
					this->master->move_to(this->backoil, this->labels[WG::BackOilPressureOkay], GraphletAnchor::RB, GraphletAnchor::LB, vgapsize);
				}; break;
				}
			}

			for (WG id = this->gantry_start; id <= this->winch_end; id++) {
				IGraphlet* target = ((id < this->winch_start) ? this->labels[WG::Pull_Push] : this->labels[WG::WindUp_Out]);
				float fx = 0.5F;

				switch (id) {
				case WG::NoSuction: case WG::HopperStopped: case WG::NoInflating: case WG::NoSlack: {
					DredgesPosition pos = this->master->get_id();

					if ((pos != DredgesPosition::psTrunnion) && (pos != DredgesPosition::sbTrunnion)) {
						this->master->move_to(this->diagnoses[id], 0.0F, 0.0F);
						this->master->move_to(this->labels[id], 0.0F, 0.0F);

						continue;
					}
				}; break;
				}

				switch (id) {
				case WG::NoUpper: case WG::NoSoftUpper: case WG::NoPulledIn: fx = 0.25F; break;
				case WG::NoSuction: case WG::NoSlack: case WG::NoSoftLower: case WG::NoSaddle: case WG::NoPushedOut: fx = 0.75F; break;
				}

				this->master->move_to(this->diagnoses[id], target, fx, this->slots[id], 0.5F, GraphletAnchor::CC);
				this->master->move_to(this->labels[id], target, 1.0F, this->slots[id], 0.5F, GraphletAnchor::LC, inset);
			}

			for (WG id = this->metrics_start; id <= this->metrics_end; id++) {
				Buttonlet* btn = nullptr;

				this->master->move_to(this->labels[id], this->slots[id], GraphletAnchor::LC, GraphletAnchor::LC, inset);
				this->master->move_to(this->metrics[id], this->labels[id], GraphletAnchor::RC, GraphletAnchor::LC, inset);

				switch (id) {
				case WG::CurrentPulse: btn = this->buttons[WGFunction::Block]; break;
				case WG::UpperPulse: btn = this->buttons[WGFunction::UpperCheck]; break;
				case WG::LowerPulse: btn = this->buttons[WGFunction::LowerCheck]; break;
				}

				if (btn != nullptr) {
					this->master->move_to(btn, this->slots[id], GraphletAnchor::RC, GraphletAnchor::RC, -inset);
				}
			}
		}
	}

public:
	bool available() override {
		return (this->master->surface_ready() && this->master->shown());
	}

	void switch_position(DredgesPosition pos) {
		bool ps = (this->side == DX::PS);
		
		switch (pos) {
		case DredgesPosition::psTrunnion: case DredgesPosition::sbTrunnion: {
			this->gantry_limit = (ps ? gantry_ps_trunnion_limited : gantry_sb_trunnion_limited);
			this->winch_limits = (ps ? &winch_ps_trunnion_limits : &winch_sb_trunnion_limits);
			this->winch_details = (ps ? &winch_ps_trunnion_details : &winch_sb_trunnion_details);
			this->details = (ps ? ps_draghead_details : sb_draghead_details);
			this->pulses = (ps ? winch_ps_trunnion_pulses : winch_sb_trunnion_pulses);
			this->reset_captions(WG::Trunnion);
		}; break;
		case DredgesPosition::psIntermediate: case DredgesPosition::sbIntermediate: {
			this->gantry_limit = (ps ? gantry_ps_intermediate_limited : gantry_sb_intermediate_limited);
			this->winch_limits = (ps ? &winch_ps_intermediate_limits : &winch_sb_intermediate_limits);
			this->winch_details = (ps ? &winch_ps_intermediate_details : &winch_sb_intermediate_details);
			this->details = (ps ? ps_intermediate_details : sb_intermediate_details);
			this->pulses = (ps ? winch_ps_intermediate_pulses : winch_sb_intermediate_pulses);
			this->reset_captions(WG::Intermediate);
		}; break;
		case DredgesPosition::psDragHead: case DredgesPosition::sbDragHead: {
			this->gantry_limit = (ps ? gantry_ps_draghead_limited : 0U);
			this->winch_limits = (ps ? &winch_ps_draghead_limits : &winch_sb_draghead_limits);
			this->winch_details = (ps ? &winch_ps_draghead_details : &winch_sb_draghead_details);
			this->details = (ps ? ps_trunnion_details : sb_trunnion_details);
			this->pulses = (ps ? winch_ps_draghead_pulses : winch_sb_draghead_pulses);
			this->reset_captions(WG::Draghead);
		}; break;
		}
	}

	void on_tap(Credit<Buttonlet, WGFunction>* button, PLCMaster* plc) {
		DredgesPosition pos = this->master->get_id();

		switch (button->id) {
		case WGFunction::Block: plc->send_command(DO_winch_override_command(pos)); break;
		case WGFunction::UpperCheck: plc->send_command(DO_winch_upper_check_command(pos)); break;

		// NOTE:  WGFunction::LowerCheck is actually the Saddle Check.
		case WGFunction::LowerCheck: plc->send_command(DO_winch_saddle_check_command(pos)); break;
		}
	}

private:
	template<typename E>
	void load_label(std::map<E, Credit<Labellet, E>*>& ls, E id, CanvasSolidColorBrush^ color, CanvasTextFormat^ font = nullptr) {
		ls[id] = this->master->insert_one(new Credit<Labellet, E>(_speak(id), font, color), id);
	}

	template<typename E, typename P>
	void load_label(std::map<E, Credit<Labellet, E>*>& ls, E id, P prefix, CanvasSolidColorBrush^ color, CanvasTextFormat^ font = nullptr) {
		ls[id] = this->master->insert_one(new Credit<Labellet, E>(_speak(prefix.ToString() + id.ToString()), font, color), id);
	}

	template<class B, typename CMD>
	void load_buttons(std::map<CMD, Credit<B, CMD>*>& bs, float width = 128.0F, float height = 32.0F) {
		for (CMD cmd = _E(CMD, 0); cmd < CMD::_; cmd++) {
			bs[cmd] = this->master->insert_one(new Credit<B, CMD>(cmd.ToString(), width, height), cmd);
			bs[cmd]->set_style(this->btn_style);
		}
	}

private:
	template<class G, typename E>
	void reflow(std::map<E, Credit<G, E>*>& gs, E label, E start, E end, GraphletAnchor ta, GraphletAnchor a, float xoff, float yoff) {
		IGraphlet* target = this->labels[label];

		for (WG id = start; id <= end; id++) {
			this->master->move_to(gs[id], target, ta, a, xoff, yoff);
			target = gs[id];
		}
	}

private:
	bool check_hydraulic_pump(WG id, const uint8* db4, unsigned int feedback) {
		bool okay = DI_hydraulic_pump_running(db4, feedback);

		this->labels[id]->set_color(okay ? subcolor_highlight : subcolor);

		return okay;
	}

	bool check_scene_stop(WG id, const uint8* db4, unsigned int details) {
		bool okay = DI_dredges_emergence_stop(db4, details);

		this->labels[id]->set_color(okay ? subcolor : subcolor_highlight);

		return okay;
	}

	void reset_captions(WG prefix) {
		for (WG id = WG::WinchCondition; id <= WG::WinchMetrics; id++) {
			this->labels[id]->set_text(_speak(prefix.ToString() + id.ToString()));
		}
	}

private: // never delete these graphlets mannually
	std::map<WG, Credit<Labellet, WG>*> labels;
	std::map<WG, Credit<Alarmlet, WG>*> diagnoses;
	std::map<WG, Credit<RoundedRectanglet, WG>*> slots;
	std::map<WG, Credit<Labellet, WG>*> subwinches;
	std::map<WG, Credit<Labellet, WG>*> subgantries;
	std::map<WG, Credit<Labellet, WG>*> metrics;
	std::map<WGFunction, Credit<Buttonlet, WGFunction>*> buttons;
	Dimensionlet* backoil;
	RoundedRectanglet* misc_region;
	RoundedRectanglet* gantry_region;
	RoundedRectanglet* winch_region;
	RoundedRectanglet* metrics_region;
	
private:
	CanvasSolidColorBrush^ color;
	CanvasTextFormat^ region_font;
	CanvasTextFormat^ diagnosis_font;
	DimensionStyle plain_style;
	ButtonStyle btn_style;

private:
	float region_reserved_height;
	float diagnosis_height;
	float misc_region_height;
	float gantry_region_height;
	float winch_region_height;
	float metrics_region_height;
	float inset_ratio;

private:
	WG misc_start;
	WG misc_end;
	WG gantry_start;
	WG gantry_end;
	WG winch_start;
	WG winch_end;
	WG metrics_start;
	WG metrics_end;
	WG sublabel_start;
	WG sublabel_end;

private:
	DredgesDiagnostics* master;
	unsigned int pulses;
	unsigned int details;
	unsigned int gantry_limit;
	WinchLimits* winch_limits;
	WinchDetails* winch_details;
	DX side;
};

DredgesDiagnostics::DredgesDiagnostics(DX side, PLCMaster* plc)
	: ICreditSatellite(plc->get_logger(), side.ToString() + "_" + __MODULE__), device(plc) {
	DredgesDx* dashboard = new DredgesDx(this, side);
	
	this->dashboard = dashboard;
	
	this->device->push_confirmation_receiver(dashboard);
}

DredgesDiagnostics::~DredgesDiagnostics() {
	if (this->dashboard != nullptr) {
		delete this->dashboard;
	}
}

void DredgesDiagnostics::fill_extent(float* width, float* height) {
	auto dashboard = dynamic_cast<DredgesDx*>(this->dashboard);
	float db_width = 400.0F;
	float db_height = 600.0F;
	
	this->title_height = large_font_size * 2.0F;
	this->vgapsize = this->title_height * 0.16F;

	if (dashboard != nullptr) {
		dashboard->fill_extent(this->title_height, this->vgapsize, &db_width, &db_height);
	}

	SET_BOX(width, db_width);
	SET_BOX(height, db_height);
}

void DredgesDiagnostics::load(CanvasCreateResourcesReason reason, float width, float height) {
	auto dashboard = dynamic_cast<DredgesDx*>(this->dashboard);
	
	if (dashboard != nullptr) {
		auto caption_font = make_bold_text_format("Microsoft YaHei", large_font_size);
		
		dashboard->load(width, height, this->title_height, this->vgapsize);
		
		this->titlebar = this->insert_one(new Rectanglet(width, this->title_height, Colours::make(diagnostics_caption_background)));
		this->title = this->insert_one(new Labellet(this->display_name(), caption_font, diagnostics_caption_foreground));
	}
}

void DredgesDiagnostics::reflow(float width, float height) {
	auto dashboard = dynamic_cast<DredgesDx*>(this->dashboard);

	if (dashboard != nullptr) {
		dashboard->reflow(width, height, this->title_height, this->vgapsize);
		this->move_to(this->title, this->titlebar, GraphletAnchor::CC, GraphletAnchor::CC);
	}
}

void DredgesDiagnostics::on_id_changed(DredgesPosition pos) {
	auto dashboard = dynamic_cast<DredgesDx*>(this->dashboard);

	if (dashboard != nullptr) {
		dashboard->switch_position(pos);
	}
}

bool DredgesDiagnostics::can_select(IGraphlet* g) {
	return button_enabled(g);
}

void DredgesDiagnostics::on_tap_selected(IGraphlet* g, float local_x, float local_y) {
	auto dashboard = dynamic_cast<DredgesDx*>(this->dashboard);
	auto btn = dynamic_cast<Credit<Buttonlet, WGFunction>*>(g);

	if ((dashboard != nullptr) && (btn != nullptr)) {
		dashboard->on_tap(btn, this->device);
	}
}
