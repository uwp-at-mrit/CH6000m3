#include <map>

#include "page/diagnostics/gland_pump_dx.hpp"
#include "decorator/decorator.hpp"
#include "configuration.hpp"

#include "module.hpp"
#include "brushes.hxx"

#include "graphlet/shapelet.hpp"
#include "graphlet/buttonlet.hpp"
#include "graphlet/dashboard/alarmlet.hpp"

#include "iotables/di_hopper_pumps.hpp"

using namespace WarGrey::SCADA;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI;
using namespace Microsoft::Graphics::Canvas::Text;
using namespace Microsoft::Graphics::Canvas::Brushes;

static CanvasSolidColorBrush^ region_background = Colours::make(diagnostics_region_background);
static CanvasSolidColorBrush^ diagnosis_background = Colours::make(diagnostics_alarm_background);
static CanvasSolidColorBrush^ diagnosis_foreground = Colours::Silver;

static CanvasTextFormat^ diagnosis_font = make_bold_text_format("Microsoft YaHei", normal_font_size);

// WARNING: order matters
private enum class FP : unsigned int {
	// Control Conditions
	RemoteControl, StartReady,
	NotRunning, NoBroken,

	_
};

private class GlandPumpDecorator : public IPlanetDecorator {
public:
	void draw_after_graphlet(IGraphlet* g, CanvasDrawingSession^ ds, float x, float y, float width, float height, bool selected) override {
		if (dynamic_cast<Credit<RoundedRectanglet, FP>*>(g) != nullptr) {
			float name_height = this->name->LayoutBounds.Height;
		
			ds->DrawTextLayout(this->name, x + height, y + (height - name_height) * 0.5F, diagnosis_foreground);
		}
	}

public:
	void set_pump(Platform::String^ name, PumpType type, unsigned int detail_index) {
		this->name = make_text_layout(_speak(name), diagnosis_font);
		this->detail_index = detail_index;
	}

	float get_pump_name_width() {
		return this->name->LayoutBounds.Width;
	}

	PumpType get_pump_type() {
		return this->type;
	}

	unsigned int get_pump_detail_index() {
		return this->detail_index;
	}

private:
	PumpType type;
	unsigned int detail_index;
	CanvasTextLayout^ name;
};

private class GlandPumpDx final : public PLCConfirmation {
public:
	GlandPumpDx(GlandPumpDiagnostics* master, GlandPumpDecorator* decorator) : master(master), decorator(decorator) {
		this->diagnosis_height = diagnosis_font->FontSize * 2.0F;
		this->icon_size = this->diagnosis_height * 0.618F;
	}

public:
	void pre_read_data(Syslog* logger) override {
		this->master->enter_critical_section();
		this->master->begin_update_sequence();
	}

	void on_digital_input(long long timepoint_ms, const uint8* DB4, size_t count4, const uint8* DB205, size_t count205, Syslog* logger) override {
		unsigned int details = this->decorator->get_pump_detail_index();

		if (details == 0U) {
			bool hopper = (this->decorator->get_pump_type() == PumpType::Hopper);

			this->diagnoses[FP::RemoteControl]->set_state(DI_gland_pump_remote_control(DB4, this->index, hopper),
				AlarmState::Notice, AlarmState::None);

			this->diagnoses[FP::NoBroken]->set_state(DI_gland_pump_broken(DB4, this->index, hopper),
				AlarmState::None, AlarmState::Notice);

			this->diagnoses[FP::NotRunning]->set_state(DI_gland_pump_running(DB4, this->index, hopper),
				AlarmState::None, AlarmState::Notice);

			this->diagnoses[FP::StartReady]->set_state(DI_gland_pump_ready(DB4, this->index, hopper),
				AlarmState::Notice, AlarmState::None);
		} else {
			this->diagnoses[FP::RemoteControl]->set_state(DI_gate_flushing_pump_remote_control(DB4, this->index),
				AlarmState::Notice, AlarmState::None);
			
			this->diagnoses[FP::NoBroken]->set_state(DI_gate_flushing_pump_broken(DB4, this->index),
				AlarmState::None, AlarmState::Notice);
			
			this->diagnoses[FP::NotRunning]->set_state(DI_gate_flushing_pump_running(DB4, this->index),
				AlarmState::None, AlarmState::Notice);

			this->diagnoses[FP::StartReady]->set_state(DI_gate_flushing_pump_ready(DB205, details),
				AlarmState::Notice, AlarmState::None);
		}
	}

	void post_read_data(Syslog* logger) override {
		this->master->end_update_sequence();
		this->master->leave_critical_section();
	}

public:
	void fill_extent(float title_height, float* width, float* height) {
		unsigned int count = _N(FP);
		
		this->region_height = this->diagnosis_height * float(count)
			+ (this->diagnosis_height - this->icon_size) * float(count + 1);

		SET_BOX(width, 400.0F);
		SET_BOX(height, this->region_height + title_height + title_height * 0.618F * 2.0F);
	}

	void load(float width, float height, float title_height) {
		float region_width = width * 0.90F;
		float diagnosis_width = (region_width - title_height * 1.5F);
		float corner_radius = 8.0F;
		
		this->region = this->master->insert_one(
			new RoundedRectanglet(region_width, this->region_height, corner_radius, region_background));

		{ // load diagnoses
			for (FP id = _E0(FP); id < FP::_; id++) {
				this->slots[id] = this->master->insert_one(new Credit<RoundedRectanglet, FP>(
					diagnosis_width, this->diagnosis_height, corner_radius, diagnosis_background), id);

				this->diagnoses[id] = this->master->insert_one(new Credit<Alarmlet, FP>(this->icon_size), id);
				this->load_label(this->labels, id, diagnosis_foreground, diagnosis_font);
			}
		}
	}

	void reflow(float width, float height, float title_height) {
		unsigned int count = _N(FP);
		float vgapsize = (this->region_height - this->diagnosis_height * float(count)) / float(count + 1);
		float hgapsize = vgapsize * 0.5F;
		float cx = width * 0.5F;
		float cy = (height + title_height) * 0.5F;

		this->master->move_to(this->region, cx, cy, GraphletAnchor::CC);

		for (FP id = _E0(FP); id < FP::_; id++) {
			this->master->move_to(this->slots[id], this->region, GraphletAnchor::CT, GraphletAnchor::CT,
				0.0F, (vgapsize + this->diagnosis_height) * _F(id) + vgapsize);
		}

		for (FP id = _E0(FP); id < FP::_; id++) {
			this->master->move_to(this->diagnoses[id], this->slots[id], GraphletAnchor::LC, GraphletAnchor::LC, hgapsize);
			this->master->move_to(this->labels[id], this->diagnoses[id], GraphletAnchor::RC, GraphletAnchor::LC,
				this->decorator->get_pump_name_width() + hgapsize * 2.0F);
		}
	}

public:
	bool available() override {
		return (this->master->surface_ready() && this->master->shown());
	}

	void switch_id(unsigned int id) {
		this->index = id;
	}

private:
	template<typename E>
	void load_label(std::map<E, Credit<Labellet, E>*>& ls, E id, CanvasSolidColorBrush^ color, CanvasTextFormat^ font = nullptr) {
		ls[id] = this->master->insert_one(new Credit<Labellet, E>(_speak(id.ToString()), font, color), id);
	}

private: // never delete these graphlets mannually
	std::map<FP, Credit<Labellet, FP>*> labels;
	std::map<FP, Credit<Alarmlet, FP>*> diagnoses;
	std::map<FP, Credit<RoundedRectanglet, FP>*> slots;
	RoundedRectanglet* region;

private:
	float region_height;
	float diagnosis_height;
	float icon_size;

private:
	GlandPumpDiagnostics* master;
	GlandPumpDecorator* decorator;
	unsigned int index;
};

GlandPumpDiagnostics::GlandPumpDiagnostics(PLCMaster* plc) : ICreditSatellite(plc->get_logger(), __MODULE__), device(plc) {
	GlandPumpDecorator* decorator = new GlandPumpDecorator();
	GlandPumpDx* dashboard = new GlandPumpDx(this, decorator);

	this->dashboard = dashboard;
	this->decorator = decorator;

	this->push_decorator(decorator);
	this->device->push_confirmation_receiver(dashboard);
}

GlandPumpDiagnostics::~GlandPumpDiagnostics() {
	if (this->dashboard != nullptr) {
		delete this->dashboard;
	}
}

void GlandPumpDiagnostics::fill_extent(float* width, float* height) {
	auto db = dynamic_cast<GlandPumpDx*>(this->dashboard);
	float db_width = 400.0F;
	float db_height = 600.0F;

	this->title_height = large_font_size * 2.0F;

	if (db != nullptr) {
		db->fill_extent(this->title_height, &db_width, &db_height);
	}

	SET_BOX(width, db_width);
	SET_BOX(height, db_height);
}

void GlandPumpDiagnostics::load(CanvasCreateResourcesReason reason, float width, float height) {
	auto db = dynamic_cast<GlandPumpDx*>(this->dashboard);
	
	if (db != nullptr) {
		auto caption_font = make_bold_text_format("Microsoft YaHei", large_font_size);
		
		db->load(width, height, this->title_height);
		
		this->titlebar = this->insert_one(new Rectanglet(width, this->title_height, Colours::make(diagnostics_caption_background)));
		this->title = this->insert_one(new Labellet(this->display_name(), caption_font, diagnostics_caption_foreground));
	}
}

void GlandPumpDiagnostics::reflow(float width, float height) {
	auto db = dynamic_cast<GlandPumpDx*>(this->dashboard);

	if (db != nullptr) {
		db->reflow(width, height, this->title_height);
		
		this->move_to(this->title, this->titlebar, GraphletAnchor::CC, GraphletAnchor::CC);
	}
}

bool GlandPumpDiagnostics::can_select(IGraphlet* g) {
	return false;
}

void GlandPumpDiagnostics::on_id_changed(unsigned int id) {
	auto db = dynamic_cast<GlandPumpDx*>(this->dashboard);

	if (db != nullptr) {
		db->switch_id(id);
	}
}

void GlandPumpDiagnostics::set_pump(Platform::String^ name, PumpType type, unsigned int detail_index) {
	auto decorator = dynamic_cast<GlandPumpDecorator*>(this->decorator);

	if (decorator != nullptr) {
		decorator->set_pump(name, type, detail_index);
	}
}
