#include <map>

#include "page/lubricatings.hpp"
#include "configuration.hpp"
#include "menu.hpp"

#include "module.hpp"
#include "text.hpp"
#include "paint.hpp"
#include "brushes.hxx"
#include "turtle.hpp"

#include "graphlet/shapelet.hpp"
#include "graphlet/statuslet.hpp"
#include "graphlet/dashboard/alarmlet.hpp"
#include "graphlet/symbol/pump/hydraulic_pumplet.hpp"

#include "iotables/ai_pumps.hpp"

#include "iotables/di_pumps.hpp"
#include "iotables/di_hopper_pumps.hpp"

#include "iotables/do_hopper_pumps.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::UI::Xaml::Controls;
using namespace Windows::Foundation::Numerics;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI;
using namespace Microsoft::Graphics::Canvas::Text;
using namespace Microsoft::Graphics::Canvas::Brushes;

private enum class LUOperation { Start, Stop, _ };
private enum class LUGBOperation { Start, Stop, Auto, _ };

static CanvasSolidColorBrush^ region_background = Colours::make(diagnostics_region_background);
static CanvasSolidColorBrush^ alarm_background = Colours::make(diagnostics_alarm_background);

// WARNING: order matters
private enum class LU : unsigned int {
	// Key Labels
	Gearbox, Unit, Master, Spare,

	// alarms
	Pressure, Level, Oil, Water,
	Bearing1, Bearing2, Bearing3,
	GearboxOil, GearboxPressure,
	
	_
};

private class Lubricatings final : public PLCConfirmation {
public:
	Lubricatings(LubricatingsPage* master, bool ps, unsigned int color) : master(master), ps(ps) {
		this->caption_font = make_bold_text_format("Microsoft YaHei", large_font_size);
		this->alarm_font = make_bold_text_format("Microsoft YaHei", normal_font_size);
		this->label_font = make_bold_text_format("Microsoft YaHei", small_font_size);

		this->color = Colours::make(color);
	}

public:
	void pre_read_data(Syslog* logger) override {
		this->master->enter_critical_section();
		this->master->begin_update_sequence();
	}

	void on_digital_input(long long timepoint_ms, const uint8* DB4, size_t count4, const uint8* DB205, size_t count205, Syslog* logger) override {
		unsigned int unit_feedback = (this->ps ? ps_hopper_lubricating_unit_feedback : sb_hopper_lubricating_unit_feedback);
		unsigned int master_feedback = (this->ps ? ps_hopper_gearbox_master_feedback : sb_hopper_gearbox_master_feedback);
		unsigned int spare_feedback = (this->ps ? ps_hopper_gearbox_spare_feedback : sb_hopper_gearbox_spare_feedback);
		unsigned int master_details = (this->ps ? ps_hopper_gearbox_master_details : sb_hopper_gearbox_master_details);
		unsigned int spare_details = (this->ps ? ps_hopper_gearbox_spare_details : sb_hopper_gearbox_spare_details);
		unsigned int unit_alarms = (this->ps ? ps_hopper_lubricating_unit_alarms : sb_hopper_lubricating_unit_alarms);
		unsigned int gearbox_alarms = (this->ps ? ps_hopper_gearbox_alarms : sb_hopper_gearbox_alarms);

		DI_hopper_pump_lubricating_unit(this->units[this->ps], DB4, unit_feedback, DB205, unit_feedback);
		DI_hopper_pump_gearbox(this->pumps[GearboxLubricator::Master], DB4, master_feedback, DB205, master_details);
		DI_hopper_pump_gearbox(this->pumps[GearboxLubricator::Spare], DB4, spare_feedback, DB205, spare_details);

		DI_hopper_lubricating_unit_alarm(this->alarms[LU::Pressure], DB4, unit_alarms + 0U);
		DI_hopper_lubricating_unit_alarm(this->alarms[LU::Level],    DB4, unit_alarms + 1U);
		DI_hopper_lubricating_unit_alarm(this->alarms[LU::Oil],      DB4, unit_alarms + 2U);
		DI_hopper_lubricating_unit_alarm(this->alarms[LU::Water],    DB4, unit_alarms + 3U);
		DI_hopper_lubricating_unit_alarm(this->alarms[LU::Bearing1], DB4, unit_alarms + 4U);
		DI_hopper_lubricating_unit_alarm(this->alarms[LU::Bearing2], DB4, unit_alarms + 5U);
		DI_hopper_lubricating_unit_alarm(this->alarms[LU::Bearing3], DB4, unit_alarms + 6U);

		DI_hopper_lubricating_unit_alarm(this->alarms[LU::GearboxOil],      DB4, gearbox_alarms + 0U);
		DI_hopper_lubricating_unit_alarm(this->alarms[LU::GearboxPressure], DB4, gearbox_alarms + 1U);
	}

	void post_read_data(Syslog* logger) override {
		this->master->end_update_sequence();
		this->master->leave_critical_section();
	}

public:
	void load(float x, float width, float height, float vinset) {
		Turtle<GearboxLubricator>* turtle = new Turtle<GearboxLubricator>(vinset, vinset);
		float region_width = width * 0.618F;
		float region_height = (height - vinset * 6.0F /* 3x for the titlebar of the main window */) * 0.5F;
		float corner_radius = 8.0F;
		float pump_radius = vinset * 1.618F;
		
		turtle->move_down()->move_left(3);
		turtle->move_down(3, GearboxLubricator::Master)->move_down(3);
		turtle->move_right(3)->move_down()->jump_up()->move_right(3);
		turtle->move_up(3, GearboxLubricator::Spare)->move_up(3)->move_left(3);

		this->unit = this->master->insert_one(new RoundedRectanglet(region_width, region_height, corner_radius, region_background));
		this->gearbox = this->master->insert_one(new RoundedRectanglet(region_width, region_height, corner_radius, region_background));
		this->station = this->master->insert_one(new Tracklet<GearboxLubricator>(turtle, default_pipe_thickness, default_pipe_color));

		this->load_label(this->captions, LU::Unit, this->color, this->caption_font, true);
		this->load_label(this->captions, LU::Gearbox, this->color, this->caption_font, true);
		this->load_device(this->units, this->ps, pump_radius);
		this->load_devices(this->pumps, this->labels, GearboxLubricator::Master, GearboxLubricator::Spare, pump_radius);

		{ // load alarms
			float alarm_box_width = (region_width - vinset * 3.0F) * 0.5F;
			float alarm_box_height = vinset * 2.0F;
			float alarm_size = alarm_box_height * 0.618F;
		
			for (LU id = LU::Pressure; id <= LU::GearboxPressure; id++) {
				this->boxes[id] = this->master->insert_one(new Credit<RoundedRectanglet, LU>(
					alarm_box_width, alarm_box_height, corner_radius, alarm_background), id);

				this->alarms[id] = this->master->insert_one(new Credit<Alarmlet, LU>(alarm_size), id);
				this->load_label(this->captions, id, Colours::Silver, this->alarm_font);
			}
		}
	}

public:
	void reflow(float x, float width, float height, float vinset) {
		float cx = x + width * 0.5F;
		float gapsize = vinset * 0.5F;

		this->master->move_to(this->unit, cx, vinset, GraphletAnchor::CT);
		this->master->move_to(this->gearbox, cx, height - vinset, GraphletAnchor::CB);
		this->master->move_to(this->units[this->ps], this->unit, 0.5F, 0.24F, GraphletAnchor::CC);
		this->master->move_to(this->station, this->gearbox, 0.5F, 0.46F, GraphletAnchor::CC);

		this->master->move_to(this->captions[LU::Unit], this->units[this->ps], GraphletAnchor::CT, GraphletAnchor::CB, 0.0F, -gapsize);
		this->master->move_to(this->captions[LU::Gearbox], this->station, GraphletAnchor::CT, GraphletAnchor::CB, 0.0F, -gapsize);

		this->station->map_credit_graphlet(this->pumps[GearboxLubricator::Master], GraphletAnchor::CC);
		this->station->map_credit_graphlet(this->pumps[GearboxLubricator::Spare], GraphletAnchor::CC);

		this->master->move_to(this->labels[GearboxLubricator::Master], this->pumps[GearboxLubricator::Master],
			GraphletAnchor::CT, GraphletAnchor::RB, -gapsize);
		
		this->master->move_to(this->labels[GearboxLubricator::Spare], this->pumps[GearboxLubricator::Spare],
			GraphletAnchor::CT, GraphletAnchor::LB, +gapsize);

		{ // reflow alarm boxes
			float region_bottom, object_bottom;
			float alarm_cy = 0.0F;

			this->master->fill_graphlet_location(this->unit, nullptr, &region_bottom, GraphletAnchor::CB);
			this->master->fill_graphlet_location(this->units[this->ps], nullptr, &object_bottom, GraphletAnchor::CB);

			alarm_cy = object_bottom + (region_bottom - object_bottom) * 0.5F;
			this->master->move_to(this->boxes[LU::Level], cx, alarm_cy, GraphletAnchor::RB, -gapsize, -gapsize * 0.5F);
			this->master->move_to(this->boxes[LU::Pressure], this->boxes[LU::Level], GraphletAnchor::LT, GraphletAnchor::LB, 0.0F, -gapsize);
			this->master->move_to(this->boxes[LU::Oil], cx, alarm_cy, GraphletAnchor::RT, -gapsize, +gapsize * 0.5F);
			this->master->move_to(this->boxes[LU::Water], this->boxes[LU::Oil], GraphletAnchor::LB, GraphletAnchor::LT, 0.0F, +gapsize);

			this->master->move_to(this->boxes[LU::Bearing1], this->boxes[LU::Pressure], GraphletAnchor::RC, GraphletAnchor::LC, vinset);
			this->master->move_to(this->boxes[LU::Bearing2], this->boxes[LU::Bearing1], GraphletAnchor::LB, GraphletAnchor::LT, 0.0F, gapsize);
			this->master->move_to(this->boxes[LU::Bearing3], this->boxes[LU::Bearing2], GraphletAnchor::LB, GraphletAnchor::LT, 0.0F, gapsize);

			this->master->fill_graphlet_location(this->gearbox, nullptr, &region_bottom, GraphletAnchor::CB);
			this->master->fill_graphlet_location(this->station, nullptr, &object_bottom, GraphletAnchor::CB);

			alarm_cy = object_bottom + (region_bottom - object_bottom) * 0.5F;
			this->master->move_to(this->boxes[LU::GearboxOil], cx, alarm_cy, GraphletAnchor::RC, -gapsize);
			this->master->move_to(this->boxes[LU::GearboxPressure], cx, alarm_cy, GraphletAnchor::LC, gapsize);
		}

		for (LU id = LU::Pressure; id <= LU::GearboxPressure; id++) {
			this->master->move_to(this->alarms[id], this->boxes[id], GraphletAnchor::LC, GraphletAnchor::LC, vinset);
			this->master->move_to(this->captions[id], this->alarms[id], GraphletAnchor::RC, GraphletAnchor::LC, vinset);
		}
	}

private:
	template<class P, typename E>
	void load_device(std::map<E, P*>& ps, E id, float radius) {
		ps[id] = this->master->insert_one(new P(radius, -90.0), id);
	}
	
	template<class P, typename E>
	void load_devices(std::map<E, P*>& ps, std::map<E, Credit<Labellet, E>*>& ls, E id0, E idn, float radius) {
		for (E id = id0; id <= idn; id++) {
			ps[id] = this->master->insert_one(new P(radius, -90.0), this->ps, id);
			this->load_label(ls, id, Colours::Silver, this->label_font);
		}
	}

	template<typename E>
	void load_label(std::map<E, Credit<Labellet, E>*>& ls, E id, CanvasSolidColorBrush^ color, CanvasTextFormat^ font = nullptr, bool prefix = false) {
		Platform::String^ label = (prefix ? _speak((this->ps ? "PS" : "SB") + id.ToString()) : _speak(id));

		ls[id] = this->master->insert_one(new Credit<Labellet, E>(label, font, color), id);
	}

// never deletes these graphlets mannually
private:
	Tracklet<GearboxLubricator>* station;
	std::map<LU, Credit<Labellet, LU>*> captions;
	std::map<bool, Credit<HydraulicPumplet, bool>*> units;
	std::map<GearboxLubricator, Credit<Labellet, GearboxLubricator>*> labels;
	std::map<GearboxLubricator, GroupCredit<HydraulicPumplet, bool, GearboxLubricator>*> pumps;
	std::map<LU, Credit<Alarmlet, LU>*> alarms;
	std::map<LU, Credit<RoundedRectanglet, LU>*> boxes;
	RoundedRectanglet* unit;
	RoundedRectanglet* gearbox;
	
private:
	CanvasSolidColorBrush^ color;
	CanvasTextFormat^ caption_font;
	CanvasTextFormat^ label_font;
	CanvasTextFormat^ alarm_font;

private:
	LubricatingsPage* master;
	bool ps;
};

LubricatingsPage::LubricatingsPage(PLCMaster* plc) : Planet(__MODULE__), device(plc) {
	Lubricatings* ps_dashboard = new Lubricatings(this, true, default_ps_color);
	Lubricatings* sb_dashboard = new Lubricatings(this, false, default_sb_color);

	this->ps_dashboard = ps_dashboard;
	this->sb_dashboard = sb_dashboard;

	if (this->device != nullptr) {
		this->unit_op = make_lubrication_unit_menu(plc);
		this->gearbox_op = make_gearbox_lubricator_menu(plc);

		this->device->push_confirmation_receiver(ps_dashboard);
		this->device->push_confirmation_receiver(sb_dashboard);
	}
}

LubricatingsPage::~LubricatingsPage() {
	if (this->ps_dashboard != nullptr) {
		delete this->ps_dashboard;
		delete this->sb_dashboard;
	}
}

void LubricatingsPage::load(CanvasCreateResourcesReason reason, float width, float height) {
	auto ps_dashboard = dynamic_cast<Lubricatings*>(this->ps_dashboard);
	auto sb_dashboard = dynamic_cast<Lubricatings*>(this->sb_dashboard);
	
	if ((ps_dashboard != nullptr) && (sb_dashboard)) {
		float vinset = statusbar_height();
		float half_width = width * 0.5F;

		ps_dashboard->load(0.0F, half_width, height, vinset);
		sb_dashboard->load(half_width, half_width, height, vinset);
	}
}

void LubricatingsPage::reflow(float width, float height) {
	auto ps_dashboard = dynamic_cast<Lubricatings*>(this->ps_dashboard);
	auto sb_dashboard = dynamic_cast<Lubricatings*>(this->sb_dashboard);

	if ((ps_dashboard != nullptr) && (sb_dashboard != nullptr)) {
		float vinset = statusbar_height();
		float half_width = width * 0.5F;
		
		ps_dashboard->reflow(0.0F,       half_width, height, vinset);
		sb_dashboard->reflow(half_width, half_width, height, vinset);
	}
}

void LubricatingsPage::on_timestream(long long timepoint_ms, size_t addr0, size_t addrn, uint8* data, size_t size, Syslog* logger) {
	auto ps_dashboard = dynamic_cast<Lubricatings*>(this->ps_dashboard);
	auto sb_dashboard = dynamic_cast<Lubricatings*>(this->sb_dashboard);

	if ((ps_dashboard != nullptr) && (sb_dashboard != nullptr)) {
		ps_dashboard->on_all_signals(timepoint_ms, addr0, addrn, data, size, logger);
		sb_dashboard->on_all_signals(timepoint_ms, addr0, addrn, data, size, logger);
	}
}

bool LubricatingsPage::can_select(IGraphlet* g) {
	return (((this->device != nullptr) && (this->device->authorized())
		&& (dynamic_cast<HydraulicPumplet*>(g) != nullptr)));
}

void LubricatingsPage::on_tap_selected(IGraphlet* g, float local_x, float local_y) {
	auto unit = dynamic_cast<Credit<HydraulicPumplet, bool>*>(g);
	auto pump = dynamic_cast<GroupCredit<HydraulicPumplet, bool, GearboxLubricator>*>(g);
	
	if (unit != nullptr) {
		menu_popup(this->unit_op, g, local_x, local_y);
	} else if (pump != nullptr) {
		menu_popup(this->gearbox_op, g, local_x, local_y);
	}
}
