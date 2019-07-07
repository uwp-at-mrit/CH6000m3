#include <map>

#include "page/subpage/underwater_pump_motor.hpp"
#include "ch6000m3/configuration.hpp"

#include "module.hpp"
#include "brushes.hxx"

#include "datum/flonum.hpp"

#include "graphlet/shapelet.hpp"
#include "graphlet/textlet.hpp"

#include "ch6000m3/iotables/ai_hopper_pumps.hpp"

using namespace WarGrey::SCADA;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI;
using namespace Microsoft::Graphics::Canvas::Text;
using namespace Microsoft::Graphics::Canvas::Brushes;

static CanvasSolidColorBrush^ region_background = Colours::make(diagnostics_region_background);
static CanvasSolidColorBrush^ title_background = Colours::make(diagnostics_alarm_background);

// WARNING: order matters
private enum class UWM : unsigned int {
	// Metrics
	U, V, W, MotorDP1, MotorDP2,
	DE, NDEa, NDEr, DEoil, NDEoil,

	_
};

private class Motor final : public PLCConfirmation {
public:
	Motor(UnderwaterPumpMotorInfo* master, bool ps, unsigned int color) : master(master), ps(ps) {
		this->region_font = make_bold_text_format("Microsoft YaHei", large_font_size);
		this->motor_style = make_highlight_dimension_style(normal_font_size, 6U, 4U, 0);

		this->color = Colours::make(color);
	}

public:
	void pre_read_data(Syslog* logger) override {
		this->master->enter_critical_section();
		this->master->begin_update_sequence();
	}

	void on_analog_input(long long timepoint_ms, const uint8* DB2, size_t count2, const uint8* DB203, size_t count203, Syslog* logger) override {
		if (this->ps) {
			this->motors[UWM::U]->set_value(RealData(DB203, ps_motor_U), GraphletAnchor::LT);
			this->motors[UWM::V]->set_value(RealData(DB203, ps_motor_V), GraphletAnchor::LT);
			this->motors[UWM::W]->set_value(RealData(DB203, ps_motor_W), GraphletAnchor::LT);
			this->motors[UWM::DE]->set_value(RealData(DB203, ps_motor_DE), GraphletAnchor::LT);
			this->motors[UWM::NDEa]->set_value(RealData(DB203, ps_motor_NDEa), GraphletAnchor::LT);
			this->motors[UWM::NDEr]->set_value(RealData(DB203, ps_motor_NDEr), GraphletAnchor::LT);
			this->motors[UWM::DEoil]->set_value(RealData(DB203, ps_motor_DEo), GraphletAnchor::LT);
			this->motors[UWM::NDEoil]->set_value(RealData(DB203, ps_motor_NDEo), GraphletAnchor::LT);
			this->motors[UWM::MotorDP1]->set_value(RealData(DB203, ps_motor_dp1), GraphletAnchor::LT);
			this->motors[UWM::MotorDP2]->set_value(RealData(DB203, ps_motor_dp2), GraphletAnchor::LT);
		} else {
			this->motors[UWM::U]->set_value(RealData(DB203, sb_motor_U), GraphletAnchor::LT);
			this->motors[UWM::V]->set_value(RealData(DB203, sb_motor_V), GraphletAnchor::LT);
			this->motors[UWM::W]->set_value(RealData(DB203, sb_motor_W), GraphletAnchor::LT);
			this->motors[UWM::DE]->set_value(RealData(DB203, sb_motor_DE), GraphletAnchor::LT);
			this->motors[UWM::NDEa]->set_value(RealData(DB203, sb_motor_NDEa), GraphletAnchor::LT);
			this->motors[UWM::NDEr]->set_value(RealData(DB203, sb_motor_NDEr), GraphletAnchor::LT);
			this->motors[UWM::DEoil]->set_value(RealData(DB203, sb_motor_DEo), GraphletAnchor::LT);
			this->motors[UWM::NDEoil]->set_value(RealData(DB203, sb_motor_NDEo), GraphletAnchor::LT);
			this->motors[UWM::MotorDP1]->set_value(RealData(DB203, sb_motor_dp1), GraphletAnchor::LT);
			this->motors[UWM::MotorDP2]->set_value(RealData(DB203, sb_motor_dp2), GraphletAnchor::LT);
		}
	}

	void post_read_data(Syslog* logger) override {
		this->master->end_update_sequence();
		this->master->leave_critical_section();
	}

public:
	void fill_extent(float title_height, float vgapsize, float* width, float* height) {
		unsigned int count = (unsigned int)(flceiling(_F(UWM::_) / 2.0F));
		float region_reserved_height = vgapsize + this->region_font->FontSize;
		
		this->metrics_height = this->motor_style.number_font->FontSize * 2.0F;
		this->region_height = this->metrics_height * float(count) + region_reserved_height;

		SET_BOX(width, 350.0F);
		SET_BOX(height, this->region_height + title_height * 2.0F);
	}

	void load(float x, float width, float height, float title_height, float vgapsize) {
		float region_width = width * 0.90F;
		float diagnosis_width = (region_width - title_height * 1.5F);
		float corner_radius = 8.0F;
		
		this->region = this->master->insert_one(
			new RoundedRectanglet(region_width, this->region_height, corner_radius, region_background));

		this->load_label(this->labels, UWM::_, this->color, this->region_font, true);

		{ // load metrics
			Platform::String^ unit = nullptr;
			
			for (UWM id = _E(UWM, 0); id <= UWM::_; id++) {
				switch (id) {
				case UWM::MotorDP1: case UWM::MotorDP2: unit = "bar"; this->motor_style.precision = 2; break;
				default: unit = "celsius"; this->motor_style.precision = 0; break;
				}

				this->motors[id] = this->master->insert_one(new Credit<Dimensionlet, UWM>(this->motor_style, unit, _speak(id)), id);
			}
		}
	}

	void reflow(float x, float width, float height, float title_height, float vgapsize) {
		unsigned int count = (unsigned int)(flceiling(_F(UWM::_) / 2.0F));
		float region_width, metrics_width, metrics_height, label_height;

		this->motors[UWM::U]->fill_extent(0.0F, 0.0F, &metrics_width, &metrics_height);
		this->labels[UWM::_]->fill_extent(0.0F, 0.0F, nullptr, &label_height);
		this->region->fill_extent(0.0F, 0.0F, &region_width, nullptr);

		{ // reflow layout
			float vinset = (height - title_height - this->region_height) / 2.0F;

			this->master->move_to(this->region, x + width * 0.5F, title_height + vinset, GraphletAnchor::CT);
			this->master->move_to(this->labels[UWM::_], this->region, GraphletAnchor::CT, GraphletAnchor::CT, 0.0F, vgapsize);
		}

		{ // reflow metrics
			float vinset = (this->region_height - (metrics_height + vgapsize) * count - label_height - vgapsize) / 2.0F;
			float hinset = flmin((region_width - metrics_width * 2.0F) / 3.0F, vinset);

			this->master->move_to(this->motors[UWM::U], this->region, 0.0F, this->labels[UWM::_], 1.0F, GraphletAnchor::LT, hinset, vinset);
			this->master->move_to(this->motors[UWM::DE], this->region, 1.0F, this->labels[UWM::_], 1.0F, GraphletAnchor::RT, -hinset, vinset);

			for (UWM id = UWM::U; id < UWM::NDEoil; id++) {
				if (id != UWM::MotorDP2) {
					this->master->move_to(this->motors[_E(UWM, _I(id) + 1U)],
						this->motors[id], GraphletAnchor::LB, GraphletAnchor::LT,
						0.0F, vgapsize);
				}
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
		Platform::String^ label = (prefix ? _speak((this->ps ? "PS" : "SB") + ((id == E::_) ? "" : id.ToString())) : _speak(id));

		ls[id] = this->master->insert_one(new Credit<Labellet, E>(label, font, color), id);
	}

private: // never delete these graphlets mannually
	std::map<UWM, Credit<Labellet, UWM>*> labels;
	std::map<UWM, Credit<Dimensionlet, UWM>*> motors;
	std::map<UWM, Credit<RoundedRectanglet, UWM>*> slots;
	RoundedRectanglet* region;
	
private:
	CanvasSolidColorBrush^ color;
	CanvasTextFormat^ region_font;
	DimensionStyle motor_style;

private:
	float metrics_height;
	float region_height;

private:
	UnderwaterPumpMotorInfo* master;
	bool ps;
};

UnderwaterPumpMotorInfo::UnderwaterPumpMotorInfo(PLCMaster* plc) : ISatellite(plc->get_logger(), __MODULE__), device(plc) {
	Motor* ps_dashboard = new Motor(this, true, default_ps_color);
	Motor* sb_dashboard = new Motor(this, false, default_sb_color);

	this->ps_dashboard = ps_dashboard;
	this->sb_dashboard = sb_dashboard;
	
	this->device->push_confirmation_receiver(ps_dashboard);
	this->device->push_confirmation_receiver(sb_dashboard);
}

UnderwaterPumpMotorInfo::~UnderwaterPumpMotorInfo() {
	if (this->ps_dashboard != nullptr) {
		delete this->ps_dashboard;
		delete this->sb_dashboard;
	}
}

void UnderwaterPumpMotorInfo::fill_extent(float* width, float* height) {
	auto ps_dashboard = dynamic_cast<Motor*>(this->ps_dashboard);
	auto sb_dashboard = dynamic_cast<Motor*>(this->sb_dashboard);
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
	SET_BOX(height, flmax(ps_height, sb_height));
}

void UnderwaterPumpMotorInfo::load(CanvasCreateResourcesReason reason, float width, float height) {
	auto ps_dashboard = dynamic_cast<Motor*>(this->ps_dashboard);
	auto sb_dashboard = dynamic_cast<Motor*>(this->sb_dashboard);
	
	if ((ps_dashboard != nullptr) && (sb_dashboard)) {
		auto caption_font = make_bold_text_format("Microsoft YaHei", large_font_size);
		float half_width = width * 0.5F;

		ps_dashboard->load(0.0F, half_width, height, this->title_height, this->vgapsize);
		sb_dashboard->load(half_width, half_width, height, this->title_height, this->vgapsize);

		this->titlebar = this->insert_one(new Rectanglet(width, this->title_height, Colours::make(diagnostics_caption_background)));
		this->title = this->insert_one(new Labellet(this->display_name(), caption_font, diagnostics_caption_foreground));
	}
}

void UnderwaterPumpMotorInfo::reflow(float width, float height) {
	auto ps_dashboard = dynamic_cast<Motor*>(this->ps_dashboard);
	auto sb_dashboard = dynamic_cast<Motor*>(this->sb_dashboard);

	if ((ps_dashboard != nullptr) && (sb_dashboard != nullptr)) {
		float half_width = width * 0.5F;
		
		ps_dashboard->reflow(0.0F,       half_width, height, this->title_height, this->vgapsize);
		sb_dashboard->reflow(half_width, half_width, height, this->title_height, this->vgapsize);

		this->move_to(this->title, this->titlebar, GraphletAnchor::CC, GraphletAnchor::CC);
	}
}

bool UnderwaterPumpMotorInfo::can_select(IGraphlet* g) {
	return false;
}

