#include <map>
#include <WindowsNumerics.h>

#include "frame/drags.hpp"

#include "configuration.hpp"
#include "plc.hpp"

#include "graphlet/shapelet.hpp"
#include "graphlet/ui/textlet.hpp"
#include "graphlet/device/draglet.hpp"

#include "iotables/ai_metrics.hpp"
#include "iotables/ai_dredges.hpp"
#include "iotables/di_dredges.hpp"
#include "iotables/di_winches.hpp"

#include "module.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::Foundation;
using namespace Windows::Foundation::Numerics;

using namespace Microsoft::Graphics::Canvas::UI;
using namespace Microsoft::Graphics::Canvas::Brushes;

// WARNING: order matters
private enum class DS : unsigned int {
	TideMark, PS, SB, SBL,
	_
};

static CanvasSolidColorBrush^ suction_active_color = Colours::Green;
static CanvasSolidColorBrush^ suction_inactive_color = Colours::DarkGray;
static CanvasSolidColorBrush^ suction_border_color = Colours::SeaGreen;

/*************************************************************************************************/
private class Drags : public PLCConfirmation {
public:
	Drags(DragsFrame* master) : master(master) {
		this->plain_style.number_font = make_bold_text_format("Cambria Math", small_metrics_font_size);
		this->plain_style.unit_font = make_bold_text_format("Cambria", normal_font_size);
		this->plain_style.minimize_number_width = 8U;

		this->ps_address = make_ps_dredging_system_schema();
		this->sb_address = make_sb_dredging_system_schema();
		
		this->setup_drag_style(0, default_ps_color);
		this->setup_drag_style(1, default_sb_color);
		this->setup_drag_style(2, default_sb_color);

		this->drag_lines_style = drag_default_lines_style();
		this->drag_no_lines_style = drag_default_lines_style();
		this->drag_no_lines_style.tide_color = Colours::Transparent;
		this->drag_no_lines_style.target_depth_color = Colours::Transparent;
		this->drag_no_lines_style.tolerance_depth_color = Colours::Transparent;

		this->drag_configs[0].trunnion_gapsize = ps_drag_trunnion_gapsize;
		this->drag_configs[0].trunnion_length = ps_drag_trunnion_length;
		this->drag_configs[0].pipe_lengths[0] = ps_drag_pipe1_length;
		this->drag_configs[0].pipe_lengths[1] = ps_drag_pipe2_length;
		this->drag_configs[0].pipe_radius = ps_drag_radius;
		this->drag_configs[0].head_width = ps_drag_head_width;
		this->drag_configs[0].head_height = ps_drag_head_length;
		this->drag_configs[0].visor_degrees_min = drag_visor_degrees_min;
		this->drag_configs[0].visor_degrees_max = drag_visor_degrees_max;
		this->drag_configs[0].arm_degrees_min = drag_arm_degrees_min;
		this->drag_configs[0].arm_degrees_max = drag_arm_degrees_max;

		this->drag_configs[1].trunnion_gapsize = sb_drag_trunnion_gapsize;
		this->drag_configs[1].trunnion_length = sb_drag_trunnion_length;
		this->drag_configs[1].pipe_lengths[0] = sb_drag_pipe1_length;
		this->drag_configs[1].pipe_lengths[1] = sb_drag_pipe2_length;
		this->drag_configs[1].pipe_radius = sb_drag_radius;
		this->drag_configs[1].head_width = sb_drag_head_width;
		this->drag_configs[1].head_height = sb_drag_head_length;
		this->drag_configs[1].visor_degrees_min = drag_visor_degrees_min;
		this->drag_configs[1].visor_degrees_max = drag_visor_degrees_max;
		this->drag_configs[1].arm_degrees_min = drag_arm_degrees_min;
		this->drag_configs[1].arm_degrees_max = drag_arm_degrees_max;

		this->drag_configs[2] = this->drag_configs[1];
		this->drag_configs[2].pipe_lengths[1] += sb_drag_pipe2_enlength;
	}

public:
	void pre_read_data(Syslog* logger) override {
		this->master->enter_critical_section();
		this->master->begin_update_sequence();
	}

	void on_digital_input(long long timepoint_ms, const uint8* DB4, size_t count4, const uint8* DB205, size_t count205, Syslog* logger) override {
		this->select_sb_drag(DB205);

		this->suctions[DS::PS]->set_color(DI_winch_suction_limited(DB4, &winch_ps_trunnion_limits) ? suction_active_color : suction_inactive_color);
		this->suctions[DS::SB]->set_color(DI_winch_suction_limited(DB4, &winch_sb_trunnion_limits) ? suction_active_color : suction_inactive_color);
	}

	void on_analog_input(long long timepoint_ms, const uint8* DB2, size_t count2, const uint8* DB203, size_t count203, Syslog* logger) override {
		this->lengths[DS::TideMark]->set_value(DBD(DB2, tide_mark));
	
		this->set_drag_metrics(DS::PS, DB2, DB203, this->drag_configs[0], this->ps_address);
		this->set_drag_metrics(DS::SB, DB2, DB203, this->drag_configs[1], this->sb_address);
		this->set_drag_metrics(DS::SBL, DB2, DB203, this->drag_configs[2], this->sb_address);
	}

	void on_forat(long long timepoint_ms, const uint8* DB20, size_t count, Syslog* logger) override {
		float target = DBD(DB20, dredging_target_depth);
		float tolerance = DBD(DB20, dredging_tolerant_depth);

		this->drags[DS::PS]->set_design_depth(target, tolerance);
	}

	void post_read_data(Syslog* logger) override {
		this->master->end_update_sequence();
		this->master->leave_critical_section();
	}

public:
	void load(float width, float height) {
		float suction_width = width * 0.1F;
		float drag_width = width * 0.95F;
		double length0 = drag_length(this->drag_configs[0]);
		double length1 = drag_length(this->drag_configs[1]);
		double length2 = drag_length(this->drag_configs[2]);
		double longest_length = flmax(length0, flmax(length1, length2));

		this->load_drag(this->drags, DS::PS, -drag_width * length0 / longest_length, drag_depth_degrees_max, this->drag_lines_style, 0);
		this->load_drag(this->drags, DS::SB, drag_width * length1 / longest_length, drag_depth_degrees_max, this->drag_no_lines_style, 1);
		this->load_drag(this->drags, DS::SBL, drag_width * length2 / longest_length, drag_depth_degrees_max, this->drag_no_lines_style, 2);

		this->load_dimension(this->lengths, DS::TideMark, "meter", 2);
		this->load_dimension(this->lengths, DS::PS, "meter", 1);
		this->load_dimension(this->lengths, DS::SB, "meter", 1);

		this->load_suction(this->suctions, DS::PS, suction_width);
		this->load_suction(this->suctions, DS::SB, suction_width);
	}

	void reflow(float width, float height) {
		/* NOTE: The SBL drag extends from left to right, it's graphical origin works for the suction's graghical offset. */
		float2 sb_origin = this->drags[DS::SBL]->space_to_local(double3(0.0, 0.0, 0.0));
		float cx = width * 0.5F;

		this->master->move_to(this->drags[DS::PS], cx, 0.0F, GraphletAnchor::CT);
		this->master->move_to(this->drags[DS::SB], cx, 0.0F, GraphletAnchor::CT);
		this->master->move_to(this->drags[DS::SBL], this->drags[DS::SB], GraphletAnchor::LT, GraphletAnchor::LT);

		this->master->move_to(this->lengths[DS::TideMark], cx, 0.0F, GraphletAnchor::CT);
		this->master->move_to(this->lengths[DS::SB], this->drags[DS::PS], GraphletAnchor::CB, GraphletAnchor::CB);
		this->master->move_to(this->lengths[DS::PS], this->lengths[DS::SB], GraphletAnchor::CT, GraphletAnchor::CB);

		/* NOTE: The PS drag is the shortest drag, it's graphical anchor works for the suction's graphical anchor. */
		this->master->move_to(this->suctions[DS::PS], this->drags[DS::PS], GraphletAnchor::LB, GraphletAnchor::CC, sb_origin.x);
		this->master->move_to(this->suctions[DS::SB], this->drags[DS::PS], GraphletAnchor::RB, GraphletAnchor::CC, -sb_origin.x);
	}

protected:
	template<typename E>
	void load_dimension(std::map<E, Credit<Dimensionlet, E>*>& ds, E id, Platform::String^ unit, unsigned int precision = 1U) {
		this->plain_style.precision = precision;
		ds[id] = this->master->insert_one(new Credit<Dimensionlet, E>(this->plain_style, unit, _speak(id)), id);
	}

	template<typename E>
	void load_suction(std::map<E, Credit<Circlelet, E>*>& ss, E id, float diameter, float border_width = 3.0F) {
		ss[id] = new Credit<Circlelet, E>(diameter * 0.5F, suction_inactive_color, suction_border_color, border_width);
		this->master->insert_one(ss[id], id);
	}

	template<class D, typename E>
	void load_drag(std::map<E, Credit<D, E>*>& ds, E id, double ws_width, double max_depth_degrees, DragLinesStyle& lstyle, unsigned int idx) {
		auto drag = new Credit<D, E>(this->drag_configs[idx], this->drag_styles[idx], lstyle, float(ws_width), max_depth_degrees);
		
		ds[id] = this->master->insert_one(drag, id);

		if (id == DS::SBL) {
			this->master->cellophane(ds[id], 0.0F);
		}
	}

private:
	void setup_drag_style(unsigned int idx, unsigned int color) {
		this->drag_styles[idx] = drag_default_style(color, 2U, small_font_size, 1.0F);

		this->drag_styles[idx].hatchmark_color = Colours::Transparent;
		this->drag_styles[idx].arm_angle_color = Colours::Transparent;
		this->drag_styles[idx].joint_angle_color = Colours::Transparent;
		this->drag_styles[idx].joint_meter_color = Colours::Transparent;
	}

	void set_drag_metrics(DS id, const uint8* db2, const uint8* db203, DragInfo& info, DredgeAddress* address) {
		double3 draghead, trunnion, ujoints[2];
		double visor_angle;
		float tide = DBD(db2, tide_mark);
		
		read_drag_figures(db2, db203, &trunnion, ujoints, &draghead, &visor_angle,
			address->drag_position, address->visor_progress, info.visor_degrees_min, info.visor_degrees_max);

		this->drags[id]->set_figure(trunnion, ujoints, draghead, visor_angle);
		this->drags[id]->set_tide_mark(tide);
	}

	void select_sb_drag(const uint8* db205) {
		float sbs_alpha = (DI_long_sb_drag(db205) ? 0.0F : 1.0F);
		float sbl_alpha = 1.0F - sbs_alpha;
		
		this->master->cellophane(this->drags[DS::SB], sbs_alpha);
		this->master->cellophane(this->drags[DS::SBL], sbl_alpha);
	}

private: // never delete these graphlets manually.
	std::map<DS, Credit<DragXZlet, DS>*> drags;
	std::map<DS, Credit<Dimensionlet, DS>*> lengths;
	std::map<DS, Credit<Circlelet, DS>*> suctions;
	
private:
	DimensionStyle plain_style;

private:
	DragLinesStyle drag_lines_style;
	DragLinesStyle drag_no_lines_style;
	DragInfo drag_configs[3];
	DragStyle drag_styles[3];

private: // never delete these global objects
	DredgeAddress* sb_address;
	DredgeAddress* ps_address;

private:
	DragsFrame* master;
};

/*************************************************************************************************/
DragsFrame::DragsFrame(MRMaster* plc) : Planet(__MODULE__) {
	Drags* dashboard = new Drags(this);

	this->dashboard = dashboard;

	if (plc != nullptr) {
		plc->push_confirmation_receiver(dashboard);
	}
}

DragsFrame::~DragsFrame() {
	if (this->dashboard != nullptr) {
		delete this->dashboard;
	}
}

void DragsFrame::load(CanvasCreateResourcesReason reason, float width, float height) {
	auto db = dynamic_cast<Drags*>(this->dashboard);
	
	if (db != nullptr) {
		db->load(width, height);
	}
}

void DragsFrame::reflow(float width, float height) {
	auto db = dynamic_cast<Drags*>(this->dashboard);
	
	if (db != nullptr) {
		db->reflow(width, height);
	}
}

void DragsFrame::on_timestream(long long timepoint_ms, size_t addr0, size_t addrn, uint8* data, size_t size, Syslog* logger) {
	auto db = dynamic_cast<Drags*>(this->dashboard);

	if (db != nullptr) {
		db->on_all_signals(timepoint_ms, addr0, addrn, data, size, logger);
	}
}

bool DragsFrame::can_select(IGraphlet* g) {
	return false;
}
