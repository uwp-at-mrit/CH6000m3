#include <map>

#include "page/diagnostics/hydraulic_pump_dx.hpp"
#include "configuration.hpp"

#include "module.hpp"
#include "string.hpp"
#include "brushes.hxx"

#include "graphlet/shapelet.hpp"
#include "graphlet/dashboard/alarmlet.hpp"

#include "iotables/di_pumps.hpp"
#include "iotables/di_valves.hpp"
#include "iotables/di_devices.hpp"
#include "iotables/di_dredges.hpp"
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
static CanvasSolidColorBrush^ caption_color = Colours::Salmon;
static CanvasSolidColorBrush^ diagnosis_color = Colours::Silver;

// WARNING: order matters
private enum class P : unsigned int {
	// Groups
	MiscCondition, PumpCondition,

	// Addition Labels
	SQk1Open, SQk2Open,

	// Misc Conditions
	NoConsolePSStop, NoConsoleSBStop, NoSailingStop, NoPSWinchGantryStop, NoSBWinchGantryStop,
	NoMasterTankFS01, NoMasterTankFS02, NoMasterTankLS1, NoMasterTankLS2,
	NoVisorTankFS, NoVisorTankLS1, NoVisorTankLS2,
	SQ1Open, SQ2Open, CoolantRunning, Flushing,

	// Pump Conditions
	Ready, NoRunning, NoBroken,

	SQaOpen, SQbOpen, SQgOpen, SQhOpen,
	SQcOpen, SQfOpen, SQdOpen, SQeOpen,
	SQiOpen, SQjOpen,
	SQyOpen, SQlOpen, SQmOpen, SQkOpen,
	
	NoC2A, NoC2B, NoF2G, NoF2H,
	NoA2C, NoB2C, NoF2C, NoC2F, NoH2F, NoG2F,
	NoJ2I, NoI2J,

	_
};

static P other[] = { P::NoMasterTankFS02, P::NoSailingStop, P::NoMasterTankLS1, P::NoMasterTankLS2 };
static P visor[] = { P::NoSailingStop, P::NoVisorTankFS, P::NoVisorTankLS1, P::NoVisorTankLS2 };
static P ps[] = { P::NoConsolePSStop, P::NoSailingStop, P::NoPSWinchGantryStop, P::NoMasterTankFS01,
	P::NoMasterTankLS1, P::NoMasterTankLS2, P::SQ2Open, P::CoolantRunning, P::Flushing };
static P sb[] = { P::NoConsoleSBStop, P::NoSailingStop, P::NoSBWinchGantryStop, P::NoMasterTankFS01,
	P::NoMasterTankLS1, P::NoMasterTankLS2, P::SQ1Open, P::CoolantRunning, P::Flushing };

static P A[] = { P::Ready, P::NoRunning, P::NoBroken, P::SQaOpen, P::NoC2A };
static P B[] = { P::Ready, P::NoRunning, P::NoBroken, P::SQbOpen, P::NoC2B };
static P G[] = { P::Ready, P::NoRunning, P::NoBroken, P::SQgOpen, P::NoF2G };
static P H[] = { P::Ready, P::NoRunning, P::NoBroken, P::SQhOpen, P::NoF2H };

static P C[] = { P::Ready, P::NoRunning, P::NoBroken, P::SQcOpen, P::NoA2C, P::NoB2C, P::NoF2C };
static P F[] = { P::Ready, P::NoRunning, P::NoBroken, P::SQfOpen, P::NoC2F, P::NoH2F, P::NoG2F };
static P D[] = { P::Ready, P::NoRunning, P::NoBroken, P::SQdOpen };
static P E[] = { P::Ready, P::NoRunning, P::NoBroken, P::SQeOpen };

static P I[] = { P::Ready, P::NoRunning, P::NoBroken, P::SQyOpen, P::NoJ2I };
static P J[] = { P::Ready, P::NoRunning, P::NoBroken, P::SQjOpen, P::NoI2J };

static P Y[] = { P::Ready, P::NoRunning, P::NoBroken, P::SQyOpen };
static P L[] = { P::Ready, P::NoRunning, P::NoBroken, P::SQlOpen };
static P M[] = { P::Ready, P::NoRunning, P::NoBroken, P::SQmOpen };
static P K[] = { P::Ready, P::NoRunning, P::NoBroken, P::SQkOpen };

static const P* select_group_conditions(HPDX gid, unsigned int* count) {
	const P* group = nullptr;

	switch (gid) {
	case HPDX::PS: group = ps; SET_BOX(count, sizeof(ps) / sizeof(P)); break;
	case HPDX::SB: group = sb; SET_BOX(count, sizeof(sb) / sizeof(P)); break;
	case HPDX::Visor: group = visor; SET_BOX(count, sizeof(visor) / sizeof(P)); break;
	case HPDX::Other: group = other; SET_BOX(count, sizeof(other) / sizeof(P)); break;
	}

	return group;
}

static const P* select_pump_conditions(unsigned int id, unsigned int* count) {
	const P* pump = nullptr;

	switch (id) {
	case pump_A_feedback: pump = A; SET_BOX(count, sizeof(A) / sizeof(P)); break;
	case pump_B_feedback: pump = B; SET_BOX(count, sizeof(B) / sizeof(P)); break;
	case pump_G_feedback: pump = G; SET_BOX(count, sizeof(G) / sizeof(P)); break;
	case pump_H_feedback: pump = H; SET_BOX(count, sizeof(H) / sizeof(P)); break;
	case pump_C_feedback: pump = C; SET_BOX(count, sizeof(C) / sizeof(P)); break;
	case pump_F_feedback: pump = F; SET_BOX(count, sizeof(F) / sizeof(P)); break;
	case pump_D_feedback: pump = D; SET_BOX(count, sizeof(D) / sizeof(P)); break;
	case pump_E_feedback: pump = E; SET_BOX(count, sizeof(E) / sizeof(P)); break;
	case pump_Y_feedback: pump = Y; SET_BOX(count, sizeof(Y) / sizeof(P)); break;
	case pump_L_feedback: pump = L; SET_BOX(count, sizeof(L) / sizeof(P)); break;
	case pump_M_feedback: pump = M; SET_BOX(count, sizeof(M) / sizeof(P)); break;
	case pump_K_feedback: pump = K; SET_BOX(count, sizeof(K) / sizeof(P)); break;
	case pump_I_feedback: pump = I; SET_BOX(count, sizeof(I) / sizeof(P)); break;
	case pump_J_feedback: pump = J; SET_BOX(count, sizeof(J) / sizeof(P)); break;
	}

	return pump;
}

/*************************************************************************************************/
private class PumpDx final : public PLCConfirmation {
public:
	PumpDx(HydraulicPumpDiagnostics* master) : master(master) {
		this->region_font = make_bold_text_format("Microsoft YaHei", normal_font_size);
		this->diagnosis_font = make_bold_text_format("Microsoft YaHei", small_font_size);
		this->subfont = make_bold_text_format("Microsoft YaHei", tiny_font_size);
	}

public:
	void pre_read_data(Syslog* logger) override {
		this->master->enter_critical_section();
		this->master->begin_update_sequence();
	}

	void on_digital_input(long long timepoint_ms, const uint8* DB4, size_t count4, const uint8* DB205, size_t count205, Syslog* logger) override {
		this->diagnoses[P::NoConsolePSStop]->set_state(DBX(DB4, console_ps_hydraulics_stop_button - 1U), AlarmState::None, AlarmState::Notice);
		this->diagnoses[P::NoConsoleSBStop]->set_state(DBX(DB4, console_sb_hydraulics_stop_button - 1U), AlarmState::None, AlarmState::Notice);
		this->diagnoses[P::NoSailingStop]->set_state(DBX(DB4, sailing_hydraulics_stop_button - 1U), AlarmState::None, AlarmState::Notice);
		this->diagnoses[P::NoPSWinchGantryStop]->set_state(DBX(DB4, console_ps_winch_gantry_stop_button - 1U), AlarmState::None, AlarmState::Notice);
		this->diagnoses[P::NoSBWinchGantryStop]->set_state(DBX(DB4, console_sb_winch_gantry_stop_button - 1U), AlarmState::None, AlarmState::Notice);
		this->diagnoses[P::NoMasterTankFS01]->set_state(DBX(DB4, filter_01_status - 1U), AlarmState::None, AlarmState::Notice);
		this->diagnoses[P::NoMasterTankFS02]->set_state(DBX(DB4, filter_02_status - 1U), AlarmState::None, AlarmState::Notice);
		this->diagnoses[P::NoVisorTankFS]->set_state(DBX(DB4, filter_10_status - 1U), AlarmState::None, AlarmState::Notice);
		this->diagnoses[P::NoMasterTankLS1]->set_state(DI_tank_level_low(DB4, master_tank_status), AlarmState::None, AlarmState::Notice);
		this->diagnoses[P::NoMasterTankLS2]->set_state(DI_tank_level_too_low(DB4, master_tank_status), AlarmState::None, AlarmState::Notice);
		this->diagnoses[P::NoVisorTankLS1]->set_state(DI_tank_level_low(DB4, visor_tank_status), AlarmState::None, AlarmState::Notice);
		this->diagnoses[P::NoVisorTankLS2]->set_state(DI_tank_level_too_low(DB4, visor_tank_status), AlarmState::None, AlarmState::Notice);
		this->diagnoses[P::SQ1Open]->set_state(DI_manual_valve_open(DB4, manual_valve_SQ1_status), AlarmState::Notice, AlarmState::None);
		this->diagnoses[P::SQ2Open]->set_state(DI_manual_valve_open(DB4, manual_valve_SQ2_status), AlarmState::Notice, AlarmState::None);
		this->diagnoses[P::CoolantRunning]->set_state(DI_hydraulic_pump_running(DB4, pump_K_feedback), AlarmState::Notice, AlarmState::None);
		this->diagnoses[P::Flushing]->set_state(DI_hydraulic_pump_running(DB4, pump_L_feedback), AlarmState::Notice, AlarmState::None);

		{ // check pumps
			unsigned int feedback = this->master->get_id();

			this->diagnoses[P::Ready]->set_state(DI_hydraulic_pump_ready(DB205, this->details), AlarmState::Notice, AlarmState::None);
			this->diagnoses[P::NoRunning]->set_state(DI_hydraulic_pump_running(DB4, feedback), AlarmState::None, AlarmState::Notice);
			this->diagnoses[P::NoBroken]->set_state(DI_hydraulic_pump_broken(DB4, feedback), AlarmState::None, AlarmState::Notice);
			
			this->diagnoses[P::NoA2C]->set_state(DBX(DB4, pump_A_replace_C - 1U), AlarmState::None, AlarmState::Notice);
			this->diagnoses[P::NoC2A]->set_state(DBX(DB4, pump_C_replace_A - 1U), AlarmState::None, AlarmState::Notice);
			this->diagnoses[P::NoB2C]->set_state(DBX(DB4, pump_B_replace_C - 1U), AlarmState::None, AlarmState::Notice);
			this->diagnoses[P::NoC2B]->set_state(DBX(DB4, pump_C_replace_B - 1U), AlarmState::None, AlarmState::Notice);
			this->diagnoses[P::NoC2F]->set_state(DBX(DB4, pump_C_replace_F - 1U), AlarmState::None, AlarmState::Notice);
			this->diagnoses[P::NoF2C]->set_state(DBX(DB4, pump_F_replace_C - 1U), AlarmState::None, AlarmState::Notice);
			this->diagnoses[P::NoH2F]->set_state(DBX(DB4, pump_H_replace_F - 1U), AlarmState::None, AlarmState::Notice);
			this->diagnoses[P::NoF2H]->set_state(DBX(DB4, pump_F_replace_H - 1U), AlarmState::None, AlarmState::Notice);
			this->diagnoses[P::NoG2F]->set_state(DBX(DB4, pump_G_replace_F - 1U), AlarmState::None, AlarmState::Notice);
			this->diagnoses[P::NoF2G]->set_state(DBX(DB4, pump_F_replace_G - 1U), AlarmState::None, AlarmState::Notice);
			this->diagnoses[P::NoI2J]->set_state(DBX(DB4, pump_I_replace_J - 1U), AlarmState::None, AlarmState::Notice);
			this->diagnoses[P::NoJ2I]->set_state(DBX(DB4, pump_J_replace_I - 1U), AlarmState::None, AlarmState::Notice);

			{ // check valves
				bool k1_open = DI_manual_valve_open(DB4, manual_valve_SQk1_status);
				bool k2_open = DI_manual_valve_open(DB4, manual_valve_SQk2_status);

				this->diagnoses[P::SQaOpen]->set_state(DI_manual_valve_open(DB4, manual_valve_SQa_status), AlarmState::Notice, AlarmState::None);
				this->diagnoses[P::SQbOpen]->set_state(DI_manual_valve_open(DB4, manual_valve_SQb_status), AlarmState::Notice, AlarmState::None);
				this->diagnoses[P::SQgOpen]->set_state(DI_manual_valve_open(DB4, manual_valve_SQg_status), AlarmState::Notice, AlarmState::None);
				this->diagnoses[P::SQhOpen]->set_state(DI_manual_valve_open(DB4, manual_valve_SQh_status), AlarmState::Notice, AlarmState::None);

				this->diagnoses[P::SQcOpen]->set_state(DI_manual_valve_open(DB4, manual_valve_SQc_status), AlarmState::Notice, AlarmState::None);
				this->diagnoses[P::SQfOpen]->set_state(DI_manual_valve_open(DB4, manual_valve_SQf_status), AlarmState::Notice, AlarmState::None);
				this->diagnoses[P::SQdOpen]->set_state(DI_manual_valve_open(DB4, manual_valve_SQd_status), AlarmState::Notice, AlarmState::None);
				this->diagnoses[P::SQeOpen]->set_state(DI_manual_valve_open(DB4, manual_valve_SQe_status), AlarmState::Notice, AlarmState::None);

				this->diagnoses[P::SQiOpen]->set_state(DI_manual_valve_open(DB4, manual_valve_SQi_status), AlarmState::Notice, AlarmState::None);
				this->diagnoses[P::SQjOpen]->set_state(DI_manual_valve_open(DB4, manual_valve_SQj_status), AlarmState::Notice, AlarmState::None);

				this->diagnoses[P::SQyOpen]->set_state(DI_manual_valve_open(DB4, manual_valve_SQy_status), AlarmState::Notice, AlarmState::None);
				this->diagnoses[P::SQlOpen]->set_state(DI_manual_valve_open(DB4, manual_valve_SQl_status), AlarmState::Notice, AlarmState::None);
				this->diagnoses[P::SQmOpen]->set_state(DI_manual_valve_open(DB4, manual_valve_SQm_status), AlarmState::Notice, AlarmState::None);
				this->diagnoses[P::SQkOpen]->set_state((k1_open || k2_open), AlarmState::Notice, AlarmState::None);

				this->labels[P::SQk1Open]->set_color(k1_open ? subcolor_highlight : subcolor);
				this->labels[P::SQk2Open]->set_color(k2_open ? subcolor_highlight : subcolor);
			}
		}
	}

	void post_read_data(Syslog* logger) override {
		this->master->end_update_sequence();
		this->master->leave_critical_section();
	}

public:
	void fill_extent(float title_height, float vgapsize, float* width, float* height) {
		unsigned int gc_count, pc_count;
		float region_reserved_height = vgapsize * 4.0F + this->region_font->FontSize;

		select_group_conditions(HPDX::PS, &gc_count);
		select_pump_conditions(pump_C_feedback, &pc_count);

		this->diagnosis_height = this->diagnosis_font->FontSize * 2.0F;
		this->misc_region_height = (this->diagnosis_height + vgapsize) * float(gc_count) + region_reserved_height;
		this->pump_region_height = (this->diagnosis_height + vgapsize) * float(pc_count) + region_reserved_height;
		
		SET_BOX(width, 400.0F);
		SET_BOX(height, this->misc_region_height + this->pump_region_height + title_height * 3.0F);
	}

	void load(float width, float height, float title_height, float vgapsize) {
		float region_width = width * 0.90F;
		float diagnosis_width = (region_width - title_height * 1.5F);
		float corner_radius = 8.0F;
		
		this->misc_region = this->master->insert_one(
			new RoundedRectanglet(region_width, this->misc_region_height, corner_radius, region_background));

		this->pump_region = this->master->insert_one(
			new RoundedRectanglet(region_width, this->pump_region_height, corner_radius, region_background));

		this->load_label(this->labels, P::MiscCondition, P::MiscCondition.ToString(), caption_color, this->region_font);
		this->load_label(this->labels, P::PumpCondition, P::PumpCondition.ToString(), caption_color, this->region_font);

		{ // load diagnoses
			float icon_size = this->diagnosis_height * 0.618F;
			
			for (P id = P::NoConsolePSStop; id < P::_; id++) {
				this->slots[id] = this->master->insert_one(new Credit<RoundedRectanglet, P>(
					diagnosis_width, this->diagnosis_height, corner_radius, diagnosis_background), id);

				this->diagnoses[id] = this->master->insert_one(new Credit<Alarmlet, P>(icon_size), id);
				this->load_label(this->labels, id, diagnosis_color, this->diagnosis_font);
			}

			this->load_label(this->labels, P::SQk1Open, subcolor, this->subfont);
			this->load_label(this->labels, P::SQk2Open, subcolor, this->subfont);
		}		
	}

	void reflow(float width, float height, float title_height, float vgapsize) {
		unsigned int gc_count = 0;
		unsigned int pc_count = 0;
		unsigned int feedback = this->master->get_id();
		const P* group = select_group_conditions(this->group, &gc_count);
		const P* pump = select_pump_conditions(feedback, &pc_count);
		
		{ // reflow layout
			float gapsize = (height - title_height - this->misc_region_height - this->pump_region_height) / 3.0F;

			this->master->move_to(this->misc_region, width * 0.5F, title_height + gapsize, GraphletAnchor::CT);
			this->master->move_to(this->pump_region, this->misc_region, GraphletAnchor::CB, GraphletAnchor::CT, 0.0F, gapsize);
			
			this->master->move_to(this->labels[P::MiscCondition], this->misc_region, GraphletAnchor::CT, GraphletAnchor::CT, 0.0F, vgapsize);
			this->master->move_to(this->labels[P::PumpCondition], this->pump_region, GraphletAnchor::CT, GraphletAnchor::CT, 0.0F, vgapsize);
		}

		{ // conceal irrelevant diagnosis
			for (P id = P::NoConsolePSStop; id < P::_; id++) {
				this->master->move_to(this->slots[id], 0.0F, 0.0F);
			}
		}

		this->reflow(this->slots, P::MiscCondition, group, gc_count, GraphletAnchor::CB, GraphletAnchor::CT, vgapsize);
		this->reflow(this->slots, P::PumpCondition, pump, pc_count, GraphletAnchor::CB, GraphletAnchor::CT, vgapsize);
		
		{ // reflow diagnostics
			float inset = vgapsize * 1.618F;
			float step = vgapsize;
			float icon_width;

			this->diagnoses[P::NoConsolePSStop]->fill_extent(0.0F, 0.0F, &icon_width, nullptr);
			step += icon_width;

			for (P id = P::NoConsolePSStop; id < P::_; id++) {
				this->master->move_to(this->diagnoses[id], this->slots[id], GraphletAnchor::LC, GraphletAnchor::LC, step * 0.0F + inset);
				this->master->move_to(this->labels[id], this->slots[id], GraphletAnchor::LC, GraphletAnchor::LC, step * 1.0F + inset + vgapsize);
			}

			if (feedback == pump_K_feedback) {
				this->master->move_to(this->labels[P::SQk1Open], this->labels[P::SQkOpen], GraphletAnchor::RB, GraphletAnchor::LB, vgapsize);
				this->master->move_to(this->labels[P::SQk2Open], this->labels[P::SQk1Open], GraphletAnchor::RB, GraphletAnchor::LB, vgapsize);
			} else {
				this->master->move_to(this->labels[P::SQk1Open], 0.0F, 0.0F);
				this->master->move_to(this->labels[P::SQk2Open], 0.0F, 0.0F);
			}
		}
	}

public:
	bool available() override {
		return (this->master->surface_ready() && this->master->shown());
	}

	void set_pump(Platform::String^ name, HPDX gid, unsigned int details) {
		this->details = details;

		// delay the updating when `switch_id`
		this->pump = name;
		this->group = gid;
	}

	void switch_id(unsigned int id) {
		// `ICreditSatellite` guarantees that the master is loaded and reflowed;
		// furthermore, the `this->master->get_id()` works for `load` and `reflow` even when `switch_id` is delayed.
		this->labels[P::MiscCondition]->set_text(_speak(this->group.ToString() + P::MiscCondition.ToString()), GraphletAnchor::CC);
		this->labels[P::PumpCondition]->set_text(_speak(this->pump + P::PumpCondition.ToString()), GraphletAnchor::CC);
	}

private:
	template<typename E>
	void load_label(std::map<E, Credit<Labellet, E>*>& ls, E id, CanvasSolidColorBrush^ color, CanvasTextFormat^ font = nullptr) {
		ls[id] = this->master->insert_one(new Credit<Labellet, E>(_speak(id), font, color), id);
	}

	template<typename E>
	void load_label(std::map<E, Credit<Labellet, E>*>& ls, E id, Platform::String^ prefix, CanvasSolidColorBrush^ color, CanvasTextFormat^ font = nullptr) {
		ls[id] = this->master->insert_one(new Credit<Labellet, E>(_speak(prefix + id.ToString()), font, color), id);
	}

private:
	template<class G, typename E>
	void reflow(std::map<E, Credit<G, E>*>& gs, E label, const E* ds, unsigned int count, GraphletAnchor ta, GraphletAnchor a, float yoff) {
		IGraphlet* target = this->labels[label];

		for (unsigned int idx = 0; idx < count; idx++) {
			auto g = gs[ds[idx]];

			this->master->move_to(g, target, ta, a, 0.0F, yoff);
			target = g;
		}
	}

private: // never delete these graphlets mannually
	std::map<P, Credit<Labellet, P>*> labels;
	std::map<P, Credit<Alarmlet, P>*> diagnoses;
	std::map<P, Credit<RoundedRectanglet, P>*> slots;
	RoundedRectanglet* misc_region;
	RoundedRectanglet* pump_region;
	
private:
	CanvasTextFormat^ region_font;
	CanvasTextFormat^ diagnosis_font;
	CanvasTextFormat^ subfont;

private:
	float diagnosis_height;
	float misc_region_height;
	float pump_region_height;

private:
	HydraulicPumpDiagnostics* master;
	Platform::String^ pump;
	unsigned int details;
	HPDX group;
};

HydraulicPumpDiagnostics::HydraulicPumpDiagnostics(PLCMaster* plc) : ICreditSatellite(plc->get_logger(), __MODULE__), device(plc) {
	PumpDx* dashboard = new PumpDx(this);
	
	this->dashboard = dashboard;
	
	this->device->push_confirmation_receiver(dashboard);
}

HydraulicPumpDiagnostics::~HydraulicPumpDiagnostics() {
	if (this->dashboard != nullptr) {
		delete this->dashboard;
	}
}

void HydraulicPumpDiagnostics::fill_extent(float* width, float* height) {
	auto dashboard = dynamic_cast<PumpDx*>(this->dashboard);
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

void HydraulicPumpDiagnostics::load(CanvasCreateResourcesReason reason, float width, float height) {
	auto dashboard = dynamic_cast<PumpDx*>(this->dashboard);
	
	if (dashboard != nullptr) {
		auto caption_font = make_bold_text_format("Microsoft YaHei", large_font_size);
		
		dashboard->load(width, height, this->title_height, this->vgapsize);
		
		this->titlebar = this->insert_one(new Rectanglet(width, this->title_height, Colours::make(diagnostics_caption_background)));
		this->title = this->insert_one(new Labellet(this->display_name(), caption_font, diagnostics_caption_foreground));
	}
}

void HydraulicPumpDiagnostics::reflow(float width, float height) {
	auto dashboard = dynamic_cast<PumpDx*>(this->dashboard);

	if (dashboard != nullptr) {
		dashboard->reflow(width, height, this->title_height, this->vgapsize);
		this->move_to(this->title, this->titlebar, GraphletAnchor::CC, GraphletAnchor::CC);
	}
}

void HydraulicPumpDiagnostics::set_pump(Platform::String^ id, HPDX group, unsigned int details) {
	auto dashboard = dynamic_cast<PumpDx*>(this->dashboard);

	if (dashboard != nullptr) {
		dashboard->set_pump(id, group, details);
	}
}

void HydraulicPumpDiagnostics::on_id_changed(unsigned int id) {
	auto dashboard = dynamic_cast<PumpDx*>(this->dashboard);

	if (dashboard != nullptr) {
		dashboard->switch_id(id);
	}
}
