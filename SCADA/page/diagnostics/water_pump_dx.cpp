#include <map>

#include "page/diagnostics/water_pump_dx.hpp"
#include "configuration.hpp"

#include "module.hpp"
#include "brushes.hxx"

#include "graphlet/shapelet.hpp"
#include "graphlet/buttonlet.hpp"
#include "graphlet/dashboard/alarmlet.hpp"

#include "iotables/di_water_pumps.hpp"
#include "iotables/do_water_pumps.hpp"

using namespace WarGrey::SCADA;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI;
using namespace Microsoft::Graphics::Canvas::Text;
using namespace Microsoft::Graphics::Canvas::Brushes;

static CanvasSolidColorBrush^ region_background = Colours::make(diagnostics_region_background);
static CanvasSolidColorBrush^ diagnosis_background = Colours::make(diagnostics_alarm_background);

// WARNING: order matters
private enum class WP : unsigned int {
	// Groups
	StartCondition, RunningCondition,

	// Start Conditions
	RemoteControl,
	NoAlert, NoBroken,
	NotRunning, NoEmergence, NoMaintenance,
	PipelineReady, SpeedKnobMoved,

	// Running Conditions
	StartReady, SpeedKnobMoved0,

	_
};

private class WaterPumpDx final : public PLCConfirmation {
public:
	WaterPumpDx(WaterPumpDiagnostics* master, bool ps, unsigned int color) : master(master), ps(ps) {
		this->region_font = make_bold_text_format("Microsoft YaHei", large_font_size);
		this->diagnosis_font = make_bold_text_format("Microsoft YaHei", normal_font_size);

		this->color = Colours::make(color);

		this->sc_start = WP::RemoteControl;
		this->sc_end = WP::SpeedKnobMoved;
		this->rc_start = WP::StartReady;
		this->rc_end = WP::SpeedKnobMoved0;
	}

public:
	void pre_read_data(Syslog* logger) override {
		this->master->enter_critical_section();
		this->master->begin_update_sequence();
	}

	void on_digital_input(long long timepoint_ms, const uint8* DB4, size_t count4, const uint8* DB205, size_t count205, Syslog* logger) override {
		unsigned int feedback = (this->ps ? ps_water_pump_feedback : sb_water_pump_feedback);
		unsigned int plready = (this->ps ? ps_water_pump_pipeline_ready : sb_water_pump_pipeline_ready) - 1U;
		unsigned int knob = (this->ps ? ps_water_pump_speed_knob_moved : sb_water_pump_speed_knob_moved) - 1U;
		unsigned int knob0 = (this->ps ? ps_water_pump_speed_knob_moved0 : sb_water_pump_speed_knob_moved0) - 1U;

		this->diagnoses[WP::RemoteControl]->set_state(DI_water_pump_remote_control(DB4, feedback), AlarmState::Notice, AlarmState::None);
		this->diagnoses[WP::NoAlert]->set_state(DI_water_pump_alert(DB4, feedback), AlarmState::None, AlarmState::Notice);
		this->diagnoses[WP::NoBroken]->set_state(DI_water_pump_broken(DB4, feedback), AlarmState::None, AlarmState::Notice);
		this->diagnoses[WP::NotRunning]->set_state(DI_water_pump_running(DB4, feedback), AlarmState::None, AlarmState::Notice);
		this->diagnoses[WP::NoEmergence]->set_state(DI_water_pump_emergence(DB4, feedback), AlarmState::None, AlarmState::Notice);
		this->diagnoses[WP::NoMaintenance]->set_state(DI_water_pump_repair(DB4, feedback), AlarmState::None, AlarmState::Notice);
		this->diagnoses[WP::PipelineReady]->set_state(DBX(DB205, plready), AlarmState::Notice, AlarmState::None);
		this->diagnoses[WP::SpeedKnobMoved]->set_state(DBX(DB4, knob), AlarmState::Notice, AlarmState::None);

		this->diagnoses[WP::StartReady]->set_state(DI_water_pump_ready(DB4, feedback), AlarmState::Notice, AlarmState::None);
		this->diagnoses[WP::SpeedKnobMoved0]->set_state(DBX(DB4, knob0), AlarmState::Notice, AlarmState::None);
	}

	void post_read_data(Syslog* logger) override {
		this->master->end_update_sequence();
		this->master->leave_critical_section();
	}

public:
	void fill_extent(float title_height, float vgapsize, float* width, float* height) {
		unsigned int sc_count = _I(this->sc_end) - _I(this->sc_start) + 1U;
		unsigned int rc_count = _I(this->rc_end) - _I(this->rc_start) + 1U;
		float region_reserved_height = vgapsize * 4.0F + this->region_font->FontSize;
		
		this->diagnosis_height = this->diagnosis_font->FontSize * 2.0F;
		this->sc_region_height = (this->diagnosis_height + vgapsize) * float(sc_count) + region_reserved_height;
		this->rc_region_height = (this->diagnosis_height + vgapsize) * float(rc_count) + region_reserved_height;

		SET_BOX(width, 400.0F);
		SET_BOX(height, this->sc_region_height + this->rc_region_height + title_height * 4.0F);
	}

	void load(float x, float width, float height, float title_height, float vgapsize) {
		float region_width = width * 0.90F;
		float diagnosis_width = (region_width - title_height * 1.5F);
		float corner_radius = 8.0F;
		
		this->sc_region = this->master->insert_one(
			new RoundedRectanglet(region_width, this->sc_region_height, corner_radius, region_background));

		this->rc_region = this->master->insert_one(
			new RoundedRectanglet(region_width, this->rc_region_height, corner_radius, region_background));

		this->load_label(this->labels, WP::StartCondition, this->color, this->region_font, true);
		this->load_label(this->labels, WP::RunningCondition, this->color, this->region_font, true);

		this->reset = this->master->insert_one(new Credit<Buttonlet, bool>(PSWaterPumpAction::Reset.ToString()), this->ps);

		{ // load diagnoses
			float icon_size = this->diagnosis_height * 0.618F;
		
			for (WP id = this->sc_start; id <= this->rc_end; id++) {
				this->slots[id] = this->master->insert_one(new Credit<RoundedRectanglet, WP>(
					diagnosis_width, this->diagnosis_height, corner_radius, diagnosis_background), id);

				this->diagnoses[id] = this->master->insert_one(new Credit<Alarmlet, WP>(icon_size), id);
				this->load_label(this->labels, id, Colours::Silver, this->diagnosis_font);
			}
		}
	}

	void reflow(float x, float width, float height, float title_height, float vgapsize) {
		float btn_height;

		this->reset->fill_extent(0.0F, 0.0F, nullptr, &btn_height);

		{ // reflow layout
			float gapsize = (height - title_height - this->sc_region_height - this->rc_region_height - btn_height) * 0.25F;

			this->master->move_to(this->sc_region, x + width * 0.5F, title_height + gapsize, GraphletAnchor::CT);
			this->master->move_to(this->rc_region, this->sc_region, GraphletAnchor::CB, GraphletAnchor::CT, 0.0F, gapsize);

			this->master->move_to(this->labels[WP::StartCondition], this->sc_region, GraphletAnchor::CT, GraphletAnchor::CT, 0.0F, vgapsize);
			this->master->move_to(this->labels[WP::RunningCondition], this->rc_region, GraphletAnchor::CT, GraphletAnchor::CT, 0.0F, vgapsize);
			this->master->move_to(this->reset, this->rc_region, GraphletAnchor::CB, GraphletAnchor::CT, 0.0F, gapsize);
		}

		{ // reflow start condition boxes
			IGraphlet* target = this->labels[WP::StartCondition];

			for (WP id = this->sc_start; id <= this->sc_end; id++) {
				this->master->move_to(this->slots[id], target, GraphletAnchor::CB, GraphletAnchor::CT, 0.0F, vgapsize);
				target = this->slots[id];
			}
		}

		{ // reflow running condition boxes
			IGraphlet* target = this->labels[WP::RunningCondition];

			for (WP id = this->rc_start; id <= this->rc_end; id++) {
				this->master->move_to(this->slots[id], target, GraphletAnchor::CB, GraphletAnchor::CT, 0.0F, vgapsize);
				target = this->slots[id];
			}
		}

		for (WP id = this->sc_start; id <= this->rc_end; id++) {
			this->master->move_to(this->diagnoses[id], this->slots[id], GraphletAnchor::LC, GraphletAnchor::LC, vgapsize);
			this->master->move_to(this->labels[id], this->diagnoses[id], GraphletAnchor::RC, GraphletAnchor::LC, vgapsize);
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
	std::map<WP, Credit<Labellet, WP>*> labels;
	std::map<WP, Credit<Alarmlet, WP>*> diagnoses;
	std::map<WP, Credit<RoundedRectanglet, WP>*> slots;
	Credit<Buttonlet, bool>* reset;
	RoundedRectanglet* sc_region;
	RoundedRectanglet* rc_region;
	
private:
	CanvasSolidColorBrush^ color;
	CanvasTextFormat^ region_font;
	CanvasTextFormat^ diagnosis_font;

private:
	float diagnosis_height;
	float sc_region_height;
	float rc_region_height;

private:
	WP sc_start;
	WP sc_end;
	WP rc_start;
	WP rc_end;

private:
	WaterPumpDiagnostics* master;
	bool ps;
};

WaterPumpDiagnostics::WaterPumpDiagnostics(PLCMaster* plc) : ISatellite(plc->get_logger(), __MODULE__), device(plc) {
	WaterPumpDx* ps_dashboard = new WaterPumpDx(this, true, default_ps_color);
	WaterPumpDx* sb_dashboard = new WaterPumpDx(this, false, default_sb_color);

	this->ps_dashboard = ps_dashboard;
	this->sb_dashboard = sb_dashboard;
	
	this->device->push_confirmation_receiver(ps_dashboard);
	this->device->push_confirmation_receiver(sb_dashboard);
}

WaterPumpDiagnostics::~WaterPumpDiagnostics() {
	if (this->ps_dashboard != nullptr) {
		delete this->ps_dashboard;
		delete this->sb_dashboard;
	}
}

void WaterPumpDiagnostics::fill_extent(float* width, float* height) {
	auto ps_dashboard = dynamic_cast<WaterPumpDx*>(this->ps_dashboard);
	auto sb_dashboard = dynamic_cast<WaterPumpDx*>(this->sb_dashboard);
	float ps_width = 400.0F;
	float ps_height = 600.0F;
	float sb_width = 400.0F;
	float sb_height = 600.0F;

	this->title_height = large_font_size * 2.0F;
	this->vgapsize = this->title_height * 0.25F;

	if (ps_dashboard != nullptr) {
		ps_dashboard->fill_extent(this->title_height, this->vgapsize, &ps_width, &ps_height);
	}

	if (sb_dashboard != nullptr) {
		sb_dashboard->fill_extent(this->title_height, this->vgapsize, &sb_width, &sb_height);
	}

	SET_BOX(width, ps_width + sb_width);
	SET_BOX(height, std::fmaxf(ps_height, sb_height));
}

void WaterPumpDiagnostics::load(CanvasCreateResourcesReason reason, float width, float height) {
	auto ps_dashboard = dynamic_cast<WaterPumpDx*>(this->ps_dashboard);
	auto sb_dashboard = dynamic_cast<WaterPumpDx*>(this->sb_dashboard);
	
	if ((ps_dashboard != nullptr) && (sb_dashboard)) {
		auto caption_font = make_bold_text_format("Microsoft YaHei", large_font_size);
		float half_width = width * 0.5F;

		ps_dashboard->load(0.0F, half_width, height, this->title_height, this->vgapsize);
		sb_dashboard->load(half_width, half_width, height, this->title_height, this->vgapsize);

		this->titlebar = this->insert_one(new Rectanglet(width, this->title_height, Colours::make(diagnostics_caption_background)));
		this->title = this->insert_one(new Labellet(this->display_name(), caption_font, diagnostics_caption_foreground));
	}
}

void WaterPumpDiagnostics::reflow(float width, float height) {
	auto ps_dashboard = dynamic_cast<WaterPumpDx*>(this->ps_dashboard);
	auto sb_dashboard = dynamic_cast<WaterPumpDx*>(this->sb_dashboard);

	if ((ps_dashboard != nullptr) && (sb_dashboard != nullptr)) {
		float half_width = width * 0.5F;
		
		ps_dashboard->reflow(0.0F,       half_width, height, this->title_height, this->vgapsize);
		sb_dashboard->reflow(half_width, half_width, height, this->title_height, this->vgapsize);

		this->move_to(this->title, this->titlebar, GraphletAnchor::CC, GraphletAnchor::CC);
	}
}

bool WaterPumpDiagnostics::can_select(IGraphlet* g) {
	return (dynamic_cast<Buttonlet*>(g) != nullptr);
}

void WaterPumpDiagnostics::on_tap_selected(IGraphlet* g, float local_x, float local_y) {
	auto reset = dynamic_cast<Credit<Buttonlet, bool>*>(g);

	if (reset != nullptr) {
		this->device->send_command(DO_water_pump_reset_command(reset->id));
	}
}
