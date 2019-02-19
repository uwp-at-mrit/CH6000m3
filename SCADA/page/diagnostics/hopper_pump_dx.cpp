#include <map>

#include "page/diagnostics/hopper_pump_dx.hpp"
#include "configuration.hpp"

#include "module.hpp"
#include "brushes.hxx"

#include "graphlet/shapelet.hpp"
#include "graphlet/buttonlet.hpp"
#include "graphlet/dashboard/alarmlet.hpp"

#include "iotables/di_hopper_pumps.hpp"
#include "iotables/do_hopper_pumps.hpp"

using namespace WarGrey::SCADA;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI;
using namespace Microsoft::Graphics::Canvas::Text;
using namespace Microsoft::Graphics::Canvas::Brushes;

static CanvasSolidColorBrush^ region_background = Colours::make(diagnostics_region_background);
static CanvasSolidColorBrush^ diagnosis_background = Colours::make(diagnostics_alarm_background);

// WARNING: order matters
private enum class HP : unsigned int {
	// Groups
	HopperCondition, UnderWaterCondition,

	// Hopper Conditions
	HPRemoteControl, HPStartReady,
	HPNotRunning, HPNoEmergence, HPNoMaintenance,
	HPPipelineReady, HPLubricatingUnitRunning, HPGearboxPumpsRunning,
	HPGlandPumpsRunning, HPSpeedKnobMoved,

	// Underwater Conditions
	UWPRemoteControl, UWPStartReady,
	UWPNotRunning, UWPNoEmergence, UWPNoMaintenance,
	UWPPipelineReady, UWPGlandPumpsRunning, UWPSpeedKnobMoved,

	_
};

private class HopperPumpDx final : public PLCConfirmation {
public:
	HopperPumpDx(HopperPumpDiagnostics* master, bool ps, unsigned int color) : master(master), ps(ps) {
		this->region_font = make_bold_text_format("Microsoft YaHei", normal_font_size);
		this->diagnosis_font = make_bold_text_format("Microsoft YaHei", small_font_size);

		this->color = Colours::make(color);

		this->hp_start = HP::HPRemoteControl;
		this->hp_end = HP::HPSpeedKnobMoved;
		this->uwp_start = HP::UWPRemoteControl;
		this->uwp_end = HP::UWPSpeedKnobMoved;
	}

public:
	void pre_read_data(Syslog* logger) override {
		this->master->enter_critical_section();
		this->master->begin_update_sequence();
	}

	void on_digital_input(long long timepoint_ms, const uint8* DB4, size_t count4, const uint8* DB205, size_t count205, Syslog* logger) override {
		unsigned int feedback = (this->ps ? ps_hopper_pump_feedback : sb_hopper_pump_feedback);
		unsigned int lunit = (this->ps ? ps_hopper_lubricating_unit_feedback : sb_hopper_lubricating_unit_feedback);
		unsigned int hpgmaster = (this->ps ? ps_hopper_master_gland_pump_feedback : sb_hopper_master_gland_pump_feedback);
		unsigned int hpgspare = (this->ps ? ps_hopper_spare_gland_pump_feedback : sb_hopper_spare_gland_pump_feedback);
		unsigned int uwpgmaster = (this->ps ? ps_underwater_master_gland_pump_feedback : sb_underwater_master_gland_pump_feedback);
		unsigned int uwpgspare = (this->ps ? ps_underwater_spare_gland_pump_feedback : sb_underwater_spare_gland_pump_feedback);
		unsigned int gbmaster = (this->ps ? ps_hopper_gearbox_master_feedback : sb_hopper_gearbox_master_feedback);
		unsigned int gbspare = (this->ps ? ps_hopper_gearbox_spare_feedback : sb_hopper_gearbox_spare_feedback);
		unsigned int hplready = (this->ps ? ps_hopper_pipeline_ready : sb_hopper_pipeline_ready) - 1U;
		unsigned int uwplready = (this->ps ? ps_underwater_pipeline_ready : sb_underwater_pipeline_ready) - 1U;
		unsigned int emergence = (this->ps ? ps_hopper_pump_emergence_feedback : sb_hopper_pump_emergence_feedback) - 1U;
		unsigned int knob = (this->ps ? ps_hopper_pump_speed_knob_moved : sb_hopper_pump_speed_knob_moved) - 1U;
		bool hp = DI_hopper_type(DB4, feedback);
		bool uwp = DI_underwater_type(DB4, feedback);

		this->diagnoses[HP::HPStartReady]->set_state(DI_hopper_pump_ready(DB4, feedback, hp), AlarmState::Notice, AlarmState::None);
		this->diagnoses[HP::HPRemoteControl]->set_state(DI_hopper_pump_remote_control(DB4, feedback, hp), AlarmState::Notice, AlarmState::None);
		this->diagnoses[HP::HPNotRunning]->set_state(DI_hopper_pump_running(DB4, feedback, true), AlarmState::None, AlarmState::Notice);
		this->diagnoses[HP::HPNoEmergence]->set_state(DBX(DB205, emergence), AlarmState::None, AlarmState::Notice);
		this->diagnoses[HP::HPNoMaintenance]->set_state(DI_hopper_pump_repair(DB4, feedback, true), AlarmState::None, AlarmState::Notice);
		this->diagnoses[HP::HPPipelineReady]->set_state(DBX(DB205, hplready), AlarmState::Notice, AlarmState::None);
		this->diagnoses[HP::HPSpeedKnobMoved]->set_state(hp && DBX(DB4, knob), AlarmState::Notice, AlarmState::None);
		this->diagnoses[HP::HPGlandPumpsRunning]->set_state(DI_gland_pump_running(DB4, hpgmaster, true), AlarmState::Notice, AlarmState::None);
		this->spare_diagnoses[HP::HPGlandPumpsRunning]->set_state(DI_gland_pump_running(DB4, hpgspare, true), AlarmState::Notice, AlarmState::None);

		this->diagnoses[HP::HPLubricatingUnitRunning]->set_state(DI_hopper_pump_lubricating_unit_running(DB4, lunit), AlarmState::Notice, AlarmState::None);
		this->diagnoses[HP::HPGearboxPumpsRunning]->set_state(DI_hopper_pump_gearbox_running(DB4, gbmaster), AlarmState::Notice, AlarmState::None);
		this->spare_diagnoses[HP::HPGearboxPumpsRunning]->set_state(DI_hopper_pump_gearbox_running(DB4, gbspare), AlarmState::Notice, AlarmState::None);

		this->diagnoses[HP::UWPStartReady]->set_state(DI_hopper_pump_ready(DB4, feedback, uwp), AlarmState::Notice, AlarmState::None);
		this->diagnoses[HP::UWPRemoteControl]->set_state(DI_hopper_pump_remote_control(DB4, feedback, uwp), AlarmState::Notice, AlarmState::None);
		this->diagnoses[HP::UWPNotRunning]->set_state(DI_hopper_pump_running(DB4, feedback, true), AlarmState::None, AlarmState::Notice);
		this->diagnoses[HP::UWPNoEmergence]->set_state(DBX(DB205, emergence), AlarmState::None, AlarmState::Notice);
		this->diagnoses[HP::UWPNoMaintenance]->set_state(DI_hopper_pump_repair(DB4, feedback, true), AlarmState::None, AlarmState::Notice);
		this->diagnoses[HP::UWPPipelineReady]->set_state(DBX(DB205, uwplready), AlarmState::Notice, AlarmState::None);
		this->diagnoses[HP::UWPSpeedKnobMoved]->set_state(uwp && DBX(DB4, knob), AlarmState::Notice, AlarmState::None);
		this->diagnoses[HP::UWPGlandPumpsRunning]->set_state(DI_gland_pump_running(DB4, uwpgmaster, false), AlarmState::Notice, AlarmState::None);
		this->spare_diagnoses[HP::UWPGlandPumpsRunning]->set_state(DI_gland_pump_running(DB4, uwpgspare, false), AlarmState::Notice, AlarmState::None);
	}

	void post_read_data(Syslog* logger) override {
		this->master->end_update_sequence();
		this->master->leave_critical_section();
	}

public:
	void fill_extent(float title_height, float vgapsize, float* width, float* height) {
		unsigned int sc_count = _I(this->hp_end) - _I(this->hp_start) + 1U;
		unsigned int rc_count = _I(this->uwp_end) - _I(this->uwp_start) + 1U;
		float region_reserved_height = vgapsize * 4.0F + this->region_font->FontSize;
		
		this->diagnosis_height = this->diagnosis_font->FontSize * 2.0F;
		this->hp_region_height = (this->diagnosis_height + vgapsize) * float(sc_count) + region_reserved_height;
		this->uwp_region_height = (this->diagnosis_height + vgapsize) * float(rc_count) + region_reserved_height;

		SET_BOX(width, 400.0F);
		SET_BOX(height, this->hp_region_height + this->uwp_region_height + title_height * 5.0F);
	}

	void load(float x, float width, float height, float title_height, float vgapsize) {
		float region_width = width * 0.90F;
		float diagnosis_width = (region_width - title_height * 1.5F);
		float corner_radius = 8.0F;
		
		this->hp_region = this->master->insert_one(
			new RoundedRectanglet(region_width, this->hp_region_height, corner_radius, region_background));

		this->uwp_region = this->master->insert_one(
			new RoundedRectanglet(region_width, this->uwp_region_height, corner_radius, region_background));

		this->load_label(this->labels, HP::HopperCondition, this->color, this->region_font, true);
		this->load_label(this->labels, HP::UnderWaterCondition, this->color, this->region_font, true);

		{ // load buttons
			Platform::String^ label = PSHopperPumpChargeAction::Reset.ToString();

			for (HP id = HP::HopperCondition; id <= HP::UnderWaterCondition; id++) {
				this->resets[id] = this->master->insert_one(new GroupCredit<Buttonlet, bool, HP>(label), this->ps, id);
			}
		}

		{ // load diagnoses
			float icon_size = this->diagnosis_height * 0.618F;
		
			for (HP id = this->hp_start; id <= this->uwp_end; id++) {
				this->slots[id] = this->master->insert_one(new Credit<RoundedRectanglet, HP>(
					diagnosis_width, this->diagnosis_height, corner_radius, diagnosis_background), id);

				switch (id) {
				case HP::HPGearboxPumpsRunning: case HP::HPGlandPumpsRunning: case HP::UWPGlandPumpsRunning: {
					this->spare_diagnoses[id] = this->master->insert_one(new Credit<Alarmlet, HP>(icon_size), id);
				}; break;
				}

				this->diagnoses[id] = this->master->insert_one(new Credit<Alarmlet, HP>(icon_size), id);
				this->load_label(this->labels, id, Colours::Silver, this->diagnosis_font);
			}
		}
	}

	void reflow(float x, float width, float height, float title_height, float vgapsize) {
		float btn_height;

		this->resets[HP::HopperCondition]->fill_extent(0.0F, 0.0F, nullptr, &btn_height);

		{ // reflow layout
			float gapsize = (height - title_height - this->hp_region_height - this->uwp_region_height - btn_height * 2.0F) / 6.0F;

			this->master->move_to(this->hp_region, x + width * 0.5F, title_height + gapsize, GraphletAnchor::CT);
			this->master->move_to(this->resets[HP::HopperCondition], this->hp_region, GraphletAnchor::CB, GraphletAnchor::CT, 0.0F, gapsize);
			this->master->move_to(this->uwp_region, this->resets[HP::HopperCondition], GraphletAnchor::CB, GraphletAnchor::CT, 0.0F, gapsize);
			this->master->move_to(this->resets[HP::UnderWaterCondition], this->uwp_region, GraphletAnchor::CB, GraphletAnchor::CT, 0.0F, gapsize);

			this->master->move_to(this->labels[HP::HopperCondition], this->hp_region, GraphletAnchor::CT, GraphletAnchor::CT, 0.0F, vgapsize);
			this->master->move_to(this->labels[HP::UnderWaterCondition], this->uwp_region, GraphletAnchor::CT, GraphletAnchor::CT, 0.0F, vgapsize);
		}

		{ // reflow hopper pump condition boxes
			IGraphlet* target = this->labels[HP::HopperCondition];

			for (HP id = this->hp_start; id <= this->hp_end; id++) {
				this->master->move_to(this->slots[id], target, GraphletAnchor::CB, GraphletAnchor::CT, 0.0F, vgapsize);
				target = this->slots[id];
			}
		}

		{ // reflow underwater pump condition boxes
			IGraphlet* target = this->labels[HP::UnderWaterCondition];

			for (HP id = this->uwp_start; id <= this->uwp_end; id++) {
				this->master->move_to(this->slots[id], target, GraphletAnchor::CB, GraphletAnchor::CT, 0.0F, vgapsize);
				target = this->slots[id];
			}
		}

		{ // reflow diagnostics
			float inset = vgapsize * 1.618F;
			float icon_width;

			this->diagnoses[this->hp_start]->fill_extent(0.0F, 0.0F, &icon_width, nullptr);

			for (HP id = this->hp_start; id <= this->uwp_end; id++) {
				if (this->spare_diagnoses.find(id) == this->spare_diagnoses.end()) {
					this->master->move_to(this->diagnoses[id], this->slots[id], GraphletAnchor::LC,
						GraphletAnchor::LC, (icon_width + vgapsize) * 1.0F + inset);
				} else {
					this->master->move_to(this->diagnoses[id], this->slots[id], GraphletAnchor::LC, GraphletAnchor::LC, inset);
					this->master->move_to(this->spare_diagnoses[id], this->slots[id], GraphletAnchor::LC,
						GraphletAnchor::LC, (icon_width + vgapsize) * 2.0F + inset);
				}

				this->master->move_to(this->labels[id], this->slots[id], GraphletAnchor::LC,
					GraphletAnchor::LC, (icon_width + vgapsize) * 3.0F + inset + vgapsize);
			}
		}
	}

public:
	bool available() override {
		return (this->master->surface_ready() && this->master->shown());
	}

private:
	template<typename E>
	void load_label(std::map<E, Credit<Labellet, E>*>& ls, E id, CanvasSolidColorBrush^ color
		, CanvasTextFormat^ font = nullptr, bool prefix = false) {
		Platform::String^ label = (prefix ? _speak((this->ps ? "PS" : "SB") + id.ToString()) : _speak(id));

		ls[id] = this->master->insert_one(new Credit<Labellet, E>(label, font, color), id);
	}

private: // never delete these graphlets mannually
	std::map<HP, Credit<Labellet, HP>*> labels;
	std::map<HP, Credit<Alarmlet, HP>*> diagnoses;
	std::map<HP, Credit<Alarmlet, HP>*> spare_diagnoses;
	std::map<HP, Credit<RoundedRectanglet, HP>*> slots;
	std::map<HP, GroupCredit<Buttonlet, bool, HP>*> resets;
	RoundedRectanglet* hp_region;
	RoundedRectanglet* uwp_region;
	
private:
	CanvasSolidColorBrush^ color;
	CanvasTextFormat^ region_font;
	CanvasTextFormat^ diagnosis_font;

private:
	float diagnosis_height;
	float hp_region_height;
	float uwp_region_height;

private:
	HP hp_start;
	HP hp_end;
	HP uwp_start;
	HP uwp_end;

private:
	HopperPumpDiagnostics* master;
	bool ps;
};

HopperPumpDiagnostics::HopperPumpDiagnostics(PLCMaster* plc) : ISatellite(plc->get_logger(), __MODULE__), device(plc) {
	HopperPumpDx* ps_dashboard = new HopperPumpDx(this, true, default_ps_color);
	HopperPumpDx* sb_dashboard = new HopperPumpDx(this, false, default_sb_color);

	this->ps_dashboard = ps_dashboard;
	this->sb_dashboard = sb_dashboard;
	
	this->device->push_confirmation_receiver(ps_dashboard);
	this->device->push_confirmation_receiver(sb_dashboard);
}

HopperPumpDiagnostics::~HopperPumpDiagnostics() {
	if (this->ps_dashboard != nullptr) {
		delete this->ps_dashboard;
		delete this->sb_dashboard;
	}
}

void HopperPumpDiagnostics::fill_extent(float* width, float* height) {
	auto ps_dashboard = dynamic_cast<HopperPumpDx*>(this->ps_dashboard);
	auto sb_dashboard = dynamic_cast<HopperPumpDx*>(this->sb_dashboard);
	float ps_width = 400.0F;
	float ps_height = 600.0F;
	float sb_width = 400.0F;
	float sb_height = 600.0F;

	this->title_height = large_font_size * 2.0F;
	this->vgapsize = this->title_height * 0.16F;

	if (ps_dashboard != nullptr) {
		ps_dashboard->fill_extent(this->title_height, this->vgapsize, &ps_width, &ps_height);
	}

	if (sb_dashboard != nullptr) {
		sb_dashboard->fill_extent(this->title_height, this->vgapsize, &sb_width, &sb_height);
	}

	SET_BOX(width, ps_width + sb_width);
	SET_BOX(height, std::fmaxf(ps_height, sb_height));
}

void HopperPumpDiagnostics::load(CanvasCreateResourcesReason reason, float width, float height) {
	auto ps_dashboard = dynamic_cast<HopperPumpDx*>(this->ps_dashboard);
	auto sb_dashboard = dynamic_cast<HopperPumpDx*>(this->sb_dashboard);
	
	if ((ps_dashboard != nullptr) && (sb_dashboard)) {
		auto caption_font = make_bold_text_format("Microsoft YaHei", large_font_size);
		float half_width = width * 0.5F;

		ps_dashboard->load(0.0F, half_width, height, this->title_height, this->vgapsize);
		sb_dashboard->load(half_width, half_width, height, this->title_height, this->vgapsize);

		this->titlebar = this->insert_one(new Rectanglet(width, this->title_height, Colours::make(diagnostics_caption_background)));
		this->title = this->insert_one(new Labellet(this->display_name(), caption_font, diagnostics_caption_foreground));
	}
}

void HopperPumpDiagnostics::reflow(float width, float height) {
	auto ps_dashboard = dynamic_cast<HopperPumpDx*>(this->ps_dashboard);
	auto sb_dashboard = dynamic_cast<HopperPumpDx*>(this->sb_dashboard);

	if ((ps_dashboard != nullptr) && (sb_dashboard != nullptr)) {
		float half_width = width * 0.5F;
		
		ps_dashboard->reflow(0.0F,       half_width, height, this->title_height, this->vgapsize);
		sb_dashboard->reflow(half_width, half_width, height, this->title_height, this->vgapsize);

		this->move_to(this->title, this->titlebar, GraphletAnchor::CC, GraphletAnchor::CC);
	}
}

bool HopperPumpDiagnostics::can_select(IGraphlet* g) {
	return (dynamic_cast<Buttonlet*>(g) != nullptr);
}

void HopperPumpDiagnostics::on_tap_selected(IGraphlet* g, float local_x, float local_y) {
	auto reset = dynamic_cast<GroupCredit<Buttonlet, bool, HP>*>(g);

	if (reset != nullptr) {
		this->device->send_command(DO_hopper_pump_reset_command(reset->gid, (reset->id == HP::HopperCondition)));
	}
}
