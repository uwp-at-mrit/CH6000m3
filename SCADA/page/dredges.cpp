#include <map>

#include "page/dredges.hpp"
#include "page/diagnostics/dredges_dx.hpp"

#include "configuration.hpp"
#include "menu.hpp"

#include "graphlet/shapelet.hpp"
#include "graphlet/buttonlet.hpp"
#include "graphlet/statuslet.hpp"

#include "graphlet/dashboard/cylinderlet.hpp"
#include "graphlet/dashboard/densityflowmeterlet.hpp"
#include "graphlet/symbol/valve/gate_valvelet.hpp"
#include "graphlet/symbol/pump/hopper_pumplet.hpp"
#include "graphlet/device/compensatorlet.hpp"
#include "graphlet/device/gantrylet.hpp"
#include "graphlet/device/overflowlet.hpp"
#include "graphlet/device/winchlet.hpp"
#include "graphlet/device/draglet.hpp"

#include "iotables/ai_metrics.hpp"
#include "iotables/ai_dredges.hpp"
#include "iotables/ai_pumps.hpp"
#include "iotables/ai_valves.hpp"

#include "iotables/di_valves.hpp"
#include "iotables/di_pumps.hpp"
#include "iotables/di_hopper_pumps.hpp"
#include "iotables/di_dredges.hpp"
#include "iotables/di_winches.hpp"

#include "iotables/do_dredges.hpp"
#include "iotables/do_winches.hpp"
#include "iotables/ao_devices.hpp"

#include "module.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Numerics;

using namespace Windows::UI::Xaml::Controls;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI;
using namespace Microsoft::Graphics::Canvas::Text;
using namespace Microsoft::Graphics::Canvas::Brushes;
using namespace Microsoft::Graphics::Canvas::Geometry;

// WARNING: order matters
private enum class DS : unsigned int {
	// pipeline
	D003, D004, D011, D012, D013, D014, D015, D016,
	LMOD, PS, SB, SBL, PSHP, SBHP,
	
	_,

	// unnamed nodes
	ps, sb, d13, d14, d15, d16, d013, d014, d1315, d1416,

	// Pump Dimensions
	C, B, A, F, G, H,

	// labels
	Overlook, Sidelook, Override, pump, hopper, underwater,

	// dimensions
	Overflow, PSWC, SBWC, PSPF1, PSPF2, SBPF1, SBPF2, PSSIP, SBSIP,
	TildeMark, Speed,

	// draghead metrics
	PSDP, SBDP, PSVisor, SBVisor,

	// hopper pump metrics
	PSHPDP, SBHPDP, PSHPVP, SBHPVP,

	// settings
	DesignDepth, TolerantDepth, DivingDepth, DivingCompensation, LandingDepth
};

static ICanvasBrush^ winch_status_color = Colours::Background;
static ICanvasBrush^ winch_status_highlight_color = Colours::Green;

static CanvasSolidColorBrush^ water_color = Colours::Green;
static std::map<DX, DredgesDiagnostics*> satellites; // satellites will be destroyed by `atexit()`;

static void dredges_diagnostics(DredgesPosition position, PLCMaster* plc) {
	DX side = ((position >= DredgesPosition::sbTrunnion) ? DX::SB : DX::PS);

	if (satellites.find(side) == satellites.end()) {
		satellites.insert(std::pair<DX, DredgesDiagnostics*>(side, new DredgesDiagnostics(side, plc)));
	}

	satellites[side]->switch_id(position);
	satellites[side]->show();
}

/*************************************************************************************************/
private class IDredgingSystem : public PLCConfirmation {
public:
	IDredgingSystem(DredgesPage* master) : master(master) {
		this->label_font = make_bold_text_format("Microsoft YaHei", normal_font_size);
		this->station_font = make_bold_text_format("Microsoft YaHei", tiny_font_size);
		this->caption_font = make_bold_text_format("Microsoft YaHei", large_font_size);
		this->caption_color = Colours::Salmon;

		this->plain_style.number_font = make_bold_text_format("Cambria Math", large_metrics_font_size);
		this->plain_style.unit_font = make_bold_text_format("Cambria", normal_font_size);
		this->plain_style.minimize_number_width = 5U;

		this->drag_lines_style = drag_default_lines_style();
		this->drag_styles[0] = drag_default_style(default_ps_color);
		this->drag_styles[1] = drag_default_style(default_sb_color);
		this->drag_styles[2] = drag_default_style(default_sb_color);

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
	virtual void load(float width, float height, float vinset) = 0;
	virtual void reflow(float width, float height, float vinset) = 0;

public:
	virtual bool can_select(IGraphlet* g) { return false; }
	virtual bool can_select_multiple() { return false; }
	virtual bool on_enter_char(PLCMaster* plc) { return false; }
	virtual void on_tap_selected(IGraphlet* g, float x, float y) {}
	virtual void on_gesture(std::list<float2>& anchors, float x, float y) {}

public:
	virtual void draw_cables(CanvasDrawingSession^ ds, float Width, float Height) {}

public:
	void pre_read_data(Syslog* logger) override {
		this->master->enter_critical_section();
		this->master->begin_update_sequence();
	}

	void post_read_data(Syslog* logger) override {
		this->master->end_update_sequence();
		this->master->leave_critical_section();
	}

protected:
	template<typename E>
	void load_dimension(std::map<E, Credit<Dimensionlet, E>*>& ds, E id, Platform::String^ unit
		, bool label = true, unsigned int precision = 1U) {
		Platform::String^ caption = (label ? _speak(id) : nullptr);

		this->plain_style.precision = precision;
		ds[id] = this->master->insert_one(new Credit<Dimensionlet, E>(this->plain_style, unit, caption), id);
	}

	template<typename E>
	void load_dimension(std::map<E, Credit<Dimensionlet, E>*>& ds, std::map<E, Credit<Labellet, E>*>& ls
		, E id, Platform::String^ unit, unsigned int precision) {
		this->load_label(ls, id, Colours::Silver);
		this->load_dimension(ds, id, unit, false, precision);
	}

	template<typename E>
	void load_percentage(std::map<E, Credit<Percentagelet, E>*>& ps, E id) {
		ps[id] = this->master->insert_one(new Credit<Percentagelet, E>(), id);
	}

	template<class C, typename E>
	void load_cylinders(std::map<E, Credit<C, E>*>& cs, E id0, E idn, float height, double vmin, double vmax, Platform::String^ unit) {
		for (E id = id0; id <= idn; id++) {
			auto cylinder = new Credit<C, E>(LiquidSurface::_, vmin, vmax, height * 0.2718F, height, 3.0F, 8U);
			unsigned int precision = (vmax < 10.0) ? 2U : 1U;

			cs[id] = this->master->insert_one(cylinder, id);

			this->load_dimension(this->pressures, this->labels, id, unit, precision);
		}
	}

	template<class DF, typename E>
	void load_densityflowmeters(std::map<E, Credit<DF, E>*>& dfs, E id0, E idn, float height) {
		for (E id = id0; id <= idn; id++) {
			dfs[id] = this->master->insert_one(new Credit<DF, E>(height), id);
		}
	}

	template<class W, typename E>
	void load_winches(std::map<E, Credit<W, E>*>& ws, E id0, E idn, float radius) {
		for (E id = id0; id <= idn; id++) {
			ws[id] = this->master->insert_one(new Credit<W, E>(radius), id);

			this->load_label(this->position_labels, id, this->caption_color);
			this->winch_lengths[id] = this->master->insert_one(new Credit<Dimensionlet, E>("meter"), id);
			this->winch_speeds[id] = this->master->insert_one(new Credit<Dimensionlet, E>("mpm"), id);
			
			this->load_label(this->winch_saddles, _speak(WinchState::SaddleLimited.ToString()), id, winch_status_color);
			this->load_label(this->winch_uppers, _speak(WinchState::UpperLimited.ToString()), id, winch_status_color);
			this->load_label(this->winch_soft_uppers, _speak(WinchState::SoftUpperLimited.ToString()), id, winch_status_color);
			this->load_label(this->winch_soft_lowers, _speak(WinchState::SoftLowerLimited.ToString()), id, winch_status_color);
		}

		this->load_label(this->winch_suctions, _speak(WinchState::SuctionLimited.ToString()), id0, winch_status_color);
		this->load_label(this->winch_slacks, _speak(WinchState::Slack.ToString()), id0, winch_status_color);
	}

	template<class C, typename E>
	void load_compensator(std::map<E, Credit<C, bool>*>& cs, E id, float height, double range) {
		cs[id] = this->master->insert_one(new Credit<C, bool>(range, height / 2.718F, height), (id == DS::PSWC));

		this->load_label(this->labels, id, this->caption_color);
		this->load_dimension(this->lengths, id, "centimeter", false);
		this->load_dimension(this->pressures, id, "bar", false);
	}

	template<class C, typename E>
	void load_compensators(std::map<E, Credit<C, bool>*>& cs, E id0, E idn, float height, double range) {
		for (E id = id0; id <= idn; id++) {
			this->load_compensator(cs, id, height, range);
		}
	}

	template<class D, typename E>
	void load_draghead(std::map<E, Credit<D, bool>*>& ds, E id, E dpid, float radius, DragInfo& info, unsigned int visor_color) {
		ds[id] = this->master->insert_one(new Credit<D, bool>(info, radius, visor_color), (id == DS::PSVisor));

		ds[id]->show_depth_metrics(false);

		this->load_dimension(this->degrees, id, "degrees");
		this->load_dimension(this->pressures, dpid, "bar");
	}

	template<class D, typename E>
	void load_overview_drag(std::map<E, Credit<D, E>*>& ds, E id, float length, unsigned int idx) {
		ds[id] = this->master->insert_one(new Credit<D, E>(this->drag_configs[idx], this->drag_styles[idx], length), id);

		if (id == DS::SBL) {
			this->master->cellophane(ds[id], 0.0F);
		}
	}

	template<class D, typename E>
	void load_sideview_drag(std::map<E, Credit<D, E>*>& ds, E id, float ws_width, double max_depth_degrees, unsigned int idx) {
		auto drag = new Credit<D, E>(this->drag_configs[idx], this->drag_styles[idx], this->drag_lines_style, ws_width, max_depth_degrees);
		
		ds[id] = this->master->insert_one(drag, id);

		if (id == DS::SBL) {
			this->master->cellophane(ds[id], 0.0F);
		}
	}

	template<class D, typename E>
	void load_backview_drag(std::map<E, Credit<D, E>*>& ds, E id, float length, double max_depth_degrees, unsigned int idx) {
		auto drag = new Credit<D, E>(this->drag_configs[idx], this->drag_styles[idx], this->drag_lines_style, length, max_depth_degrees);

		ds[id] = this->master->insert_one(drag, id);

		if (id == DS::SBL) {
			this->master->cellophane(ds[id], 0.0F);
		}
	}

	template<class B, typename E, typename L>
	void load_button(std::map<E, Credit<B, E>*>& bs, E id, L label, float width = 128.0F, float height = 32.0F) {
		bs[id] = this->master->insert_one(new Credit<B, E>(label.ToString(), width, height), id);
	}

	template<class B, typename E, typename CMD>
	void load_buttons(std::map<CMD, GroupCredit<B, E, CMD>*>& bs, E gid, float width = 128.0F, float height = 32.0F) {
		for (CMD cmd = _E(CMD, 0); cmd < CMD::_; cmd++) {
			bs[cmd] = this->master->insert_one(new GroupCredit<B, E, CMD>(cmd.ToString(), width, height), gid, cmd);
		}
	}

	template<class B, typename E, typename CMD>
	void load_buttons(std::map<CMD, GroupCredit<B, E, CMD>*>& bs, E gid, ButtonStyle& style, float width = 128.0F, float height = 32.0F) {
		for (CMD cmd = _E(CMD, 0); cmd < CMD::_; cmd++) {
			bs[cmd] = this->master->insert_one(new GroupCredit<B, E, CMD>(cmd.ToString(), width, height), gid, cmd);
			bs[cmd]->set_style(style);
		}
	}

	template<typename E>
	void load_label(std::map<E, Credit<Labellet, E>*>& ls, E id, ICanvasBrush^ color, CanvasTextFormat^ font = nullptr) {
		this->load_label(ls, _speak(id), id, color, font);
	}

	template<typename E>
	void load_label(std::map<E, Credit<Labellet, E>*>& ls, Platform::String^ label, E id, ICanvasBrush^ color, CanvasTextFormat^ font = nullptr) {
		ls[id] = this->master->insert_one(new Credit<Labellet, E>(label, font, color), id);
	}

protected:
	void reflow_winch_limits(DredgesPosition id) {
		this->master->move_to(this->winch_soft_lowers[id], this->winches[id], GraphletAnchor::CT, GraphletAnchor::CB);
		this->master->move_to(this->winch_saddles[id], this->winch_soft_lowers[id], GraphletAnchor::CT, GraphletAnchor::CB);
		this->master->move_to(this->winch_uppers[id], this->winch_saddles[id], GraphletAnchor::CT, GraphletAnchor::CB);
		this->master->move_to(this->winch_soft_uppers[id], this->winch_uppers[id], GraphletAnchor::CT, GraphletAnchor::CB);

		if (this->winch_suctions.find(id) != this->winch_suctions.end()) {
			this->master->move_to(this->winch_slacks[id], this->winch_soft_uppers[id], GraphletAnchor::CT, GraphletAnchor::CB);
			this->master->move_to(this->winch_suctions[id], this->winch_slacks[id], GraphletAnchor::CT, GraphletAnchor::CB);
		}
	}

	void reflow_draghead_metrics(DS id, DS dpid) {
		float bspace;
		
		this->dragheads[id]->fill_margin(0.0F, 0.0F, nullptr, nullptr, &bspace);
		this->master->move_to(this->pressures[dpid], this->dragheads[id], GraphletAnchor::CT, GraphletAnchor::CB);
		this->master->move_to(this->degrees[id], this->dragheads[id], GraphletAnchor::CB, GraphletAnchor::CT, 0.0F, -bspace);
	}

protected:
	void set_compensator(DS id, const uint8* db203, unsigned int rd_idx, GraphletAnchor a) {
		float progress = RealData(db203, rd_idx + 2U);

		this->compensators[id]->set_value(progress * 0.01F);
		this->lengths[id]->set_value(progress, a);
		this->pressures[id]->set_value(RealData(db203, rd_idx + 0U), a);
	}

	void set_winch_metrics(DredgesPosition id, const uint8* db2, unsigned int speed_idx, unsigned int length_idx, GraphletAnchor a) {
		this->winch_speeds[id]->set_value(DBD(db2, speed_idx), a);
		this->winch_lengths[id]->set_value(DBD(db2, length_idx), a);
	}

	void set_winch_limits(DredgesPosition id0, DredgesPosition idn) {
		for (DredgesPosition id = id0; id <= idn; id++) {
			bool slack = false;
			bool upper = false;
			bool saddle = false;
			bool suction = false;
			bool soft_upper = false;
			bool soft_lower = false;

			switch (this->winches[id]->get_state()) {
			case WinchState::SaddleLimited: saddle = true; break;
			case WinchState::SuctionLimited: suction = true; break;
			case WinchState::UpperLimited: upper = true; break;
			case WinchState::SoftUpperLimited: soft_upper = true; break;
			case WinchState::SoftLowerLimited: soft_lower = true; break;
			case WinchState::SaddleSlack: saddle = true; slack = true; break;
			case WinchState::SuctionSlack: suction = true; slack = true; break;
			case WinchState::Slack: slack = true; break;
			}

			this->winch_saddles[id]->set_color(saddle ? winch_status_highlight_color : winch_status_color);
			this->winch_uppers[id]->set_color(upper ? winch_status_highlight_color : winch_status_color);
			this->winch_soft_uppers[id]->set_color(soft_upper ? winch_status_highlight_color : winch_status_color);
			this->winch_soft_lowers[id]->set_color(soft_lower ? winch_status_highlight_color : winch_status_color);

			if (this->winch_slacks.find(id) != this->winch_slacks.end()) {
				this->winch_slacks[id]->set_color(slack ? winch_status_highlight_color : winch_status_color);
			}

			if (this->winch_suctions.find(id) != this->winch_suctions.end()) {
				this->winch_suctions[id]->set_color(suction ? winch_status_highlight_color : winch_status_color);
			}
		}
	}

	void set_drag_metrics(DS id, DS vid, const uint8* db2, const uint8* db203, DragInfo& info, DredgeAddress* address) {
		unsigned int pidx = address->drag_position;
		float3 ujoints[2];
		float3 draghead = DBD_3(db2, pidx + 36U);
		float3 trunnion = DBD_3(db2, pidx + 0U);
		float tilde = DBD(db2, tilde_mark);
		float suction_depth = trunnion.x;
		
		ujoints[0] = DBD_3(db2, pidx + 12U);
		ujoints[1] = DBD_3(db2, pidx + 24U);

		trunnion.x = 0.0F;
		ujoints[1].y = DBD(db2, pidx + 48U);
		draghead.y = DBD(db2, pidx + 52U);
		draghead.z = DBD(db2, pidx + 56U);

		{ // WARNING: DB2 gives the wrong visor angle, uses DB203 and manually computing it instead.
			double visor_progress = RealData(db203, address->visor_progress) * 0.01F;
			double visor_angle = (info.visor_degrees_max - info.visor_degrees_min) * (1.0 - visor_progress) + info.visor_degrees_min;

			if (this->dragxys.find(id) != this->dragxys.end()) {
				this->dragxys[id]->set_figure(trunnion, ujoints, draghead, visor_angle);
			}

			if (this->dragxzes.find(id) != this->dragxzes.end()) {
				this->dragxzes[id]->set_figure(trunnion, ujoints, draghead, visor_angle);
				this->dragxzes[id]->set_tilde_mark(tilde);
			}

			if (this->dragyzes.find(id) != this->dragyzes.end()) {
				this->dragyzes[id]->set_figure(trunnion, ujoints, draghead, visor_angle);
				this->dragyzes[id]->set_tilde_mark(tilde);
			}

			this->dragheads[vid]->set_depths(suction_depth, draghead.z);
			this->dragheads[vid]->set_angles(visor_angle, this->dragxzes[id]->get_arm_degrees());
			this->degrees[vid]->set_value(visor_angle, GraphletAnchor::LC);
		}
	}

	void set_design_depth(DS id, const uint8* db20, unsigned int target_depth, unsigned int tolerant_depth) {
		float target = DBD(db20, dredging_target_depth);
		float tolerance = DBD(db20, dredging_tolerant_depth);

		if (this->dragxzes.find(id) != this->dragxzes.end()) {
			this->dragxzes[id]->set_design_depth(target, tolerance);
		}

		if (this->dragyzes.find(id) != this->dragyzes.end()) {
			this->dragyzes[id]->set_design_depth(target, tolerance);
		}
	}

	void select_sb_drag(const uint8* db205) {
		float sbs_alpha = (DI_long_sb_drag(db205) ? 0.0F : 1.0F);
		float sbl_alpha = 1.0F - sbs_alpha;
		

		if (this->dragxys.find(DS::SB) != this->dragxys.end()) {
			this->master->cellophane(this->dragxys[DS::SB], sbs_alpha);
			this->master->cellophane(this->dragxys[DS::SBL], sbl_alpha);
		}

		if (this->dragxzes.find(DS::SB) != this->dragxzes.end()) {
			this->master->cellophane(this->dragxzes[DS::SB], sbs_alpha);
			this->master->cellophane(this->dragxzes[DS::SBL], sbl_alpha);
		}

		if (this->dragyzes.find(DS::SB) != this->dragyzes.end()) {
			this->master->cellophane(this->dragyzes[DS::SB], sbs_alpha);
			this->master->cellophane(this->dragyzes[DS::SBL], sbl_alpha);
		}
	}

protected: // never delete these graphlets manually.
	std::map<DS, Credit<Labellet, DS>*> labels;
	std::map<DS, Credit<Compensatorlet, bool>*> compensators;
	std::map<DS, Credit<DragHeadlet, bool>*> dragheads;
	std::map<DS, Credit<DragXYlet, DS>*> dragxys;
	std::map<DS, Credit<DragYZlet, DS>*> dragyzes;
	std::map<DS, Credit<DragXZlet, DS>*> dragxzes;
	std::map<DS, Credit<Dimensionlet, DS>*> forces;
	std::map<DS, Credit<Dimensionlet, DS>*> pressures;
	std::map<DS, Credit<Dimensionlet, DS>*> degrees;
	std::map<DS, Credit<Dimensionlet, DS>*> lengths;
	std::map<DS, Credit<Dimensionlet, DS>*> speeds;

protected: // never delete these graphlets manually.
	std::map<DredgesPosition, Credit<Labellet, DredgesPosition>*> position_labels;
	std::map<DredgesPosition, Credit<Winchlet, DredgesPosition>*> winches;
	std::map<DredgesPosition, Credit<Dimensionlet, DredgesPosition>*> winch_lengths;
	std::map<DredgesPosition, Credit<Dimensionlet, DredgesPosition>*> winch_speeds;
	std::map<DredgesPosition, Credit<Labellet, DredgesPosition>*> winch_suctions;
	std::map<DredgesPosition, Credit<Labellet, DredgesPosition>*> winch_saddles;
	std::map<DredgesPosition, Credit<Labellet, DredgesPosition>*> winch_slacks;
	std::map<DredgesPosition, Credit<Labellet, DredgesPosition>*> winch_uppers;
	std::map<DredgesPosition, Credit<Labellet, DredgesPosition>*> winch_soft_uppers;
	std::map<DredgesPosition, Credit<Labellet, DredgesPosition>*> winch_soft_lowers;
	
protected:
	CanvasTextFormat^ caption_font;
	CanvasTextFormat^ label_font;
	CanvasTextFormat^ station_font;
	ICanvasBrush^ caption_color;
	DimensionStyle percentage_style;
	DimensionStyle plain_style;

protected:
	DragLinesStyle drag_lines_style;
	DragInfo drag_configs[3];
	DragStyle drag_styles[3];

protected:
	DredgesPage* master;
};

/*************************************************************************************************/
private class Dredges final : public IDredgingSystem {
public:
	Dredges(DredgesPage* master) : IDredgingSystem(master) {
		this->ps_address = make_ps_dredging_system_schema();
		this->sb_address = make_sb_dredging_system_schema();
	}

public:
	void pre_read_data(Syslog* logger) override {
		IDredgingSystem::pre_read_data(logger);

		this->station->clear_subtacks();
	}

	void on_analog_input(long long timepoint_ms, const uint8* DB2, size_t count2, const uint8* DB203, size_t count203, Syslog* logger) override {
		this->overflowpipe->set_value(RealData(DB203, overflow_pipe_progress));
		this->overflowpipe->set_liquid_height(DBD(DB2, average_hopper_height));
		this->lengths[DS::Overflow]->set_value(this->overflowpipe->get_value(), GraphletAnchor::CC);

		this->progresses[DS::D003]->set_value(RealData(DB203, gate_valve_D03_progress), GraphletAnchor::LB);
		this->progresses[DS::D004]->set_value(RealData(DB203, gate_valve_D04_progress), GraphletAnchor::LT);

		this->set_cylinder(DS::PSHPDP, RealData(DB203, this->ps_address->discharge_pressure));
		this->set_cylinder(DS::PSHPVP, RealData(DB203, this->ps_address->vacuum_pressure));
		this->set_cylinder(DS::SBHPDP, RealData(DB203, this->sb_address->discharge_pressure));
		this->set_cylinder(DS::SBHPVP, RealData(DB203, this->sb_address->vacuum_pressure));

		this->set_compensator(DS::PSWC, DB203, this->ps_address->compensator, GraphletAnchor::CC);
		this->set_compensator(DS::SBWC, DB203, this->sb_address->compensator, GraphletAnchor::CC);

		this->pressures[DS::PSDP]->set_value(RealData(DB203, this->ps_address->differential_pressure), GraphletAnchor::CC);
		this->pressures[DS::SBDP]->set_value(RealData(DB203, this->sb_address->differential_pressure), GraphletAnchor::CC);

		this->pressures[DS::PSSIP]->set_value(RealData(DB203, this->ps_address->suction_inflator_pressure), GraphletAnchor::LC);
		this->pressures[DS::SBSIP]->set_value(RealData(DB203, this->sb_address->suction_inflator_pressure), GraphletAnchor::RC);

		this->set_density_speed(DS::PS, DB203, this->ps_address->density_speed);
		this->set_density_speed(DS::SB, DB203, this->sb_address->density_speed);

		this->forces[DS::PSPF1]->set_value(RealData(DB203, this->ps_address->pulling_force + 0U), GraphletAnchor::LC);
		this->forces[DS::PSPF2]->set_value(RealData(DB203, this->ps_address->pulling_force + 1U), GraphletAnchor::LC);
		this->forces[DS::SBPF1]->set_value(RealData(DB203, this->sb_address->pulling_force + 0U), GraphletAnchor::LC);
		this->forces[DS::SBPF2]->set_value(RealData(DB203, this->sb_address->pulling_force + 1U), GraphletAnchor::LC);

		{ // set winches metrics
			unsigned int psws_idx = this->ps_address->winch_speed;
			unsigned int pswl_idx = this->ps_address->winch_length;
			unsigned int sbws_idx = this->sb_address->winch_speed;
			unsigned int sbwl_idx = this->sb_address->winch_length;

			this->set_winch_metrics(DredgesPosition::psTrunnion, DB2, psws_idx + 0U, pswl_idx + 0U, GraphletAnchor::LC);
			this->set_winch_metrics(DredgesPosition::psIntermediate, DB2, psws_idx + 4U, pswl_idx + 4U, GraphletAnchor::LC);
			this->set_winch_metrics(DredgesPosition::psDragHead, DB2, psws_idx + 8U, pswl_idx + 8U, GraphletAnchor::LC);

			this->set_winch_metrics(DredgesPosition::sbTrunnion, DB2, sbws_idx + 0U, sbwl_idx + 0U, GraphletAnchor::RC);
			this->set_winch_metrics(DredgesPosition::sbIntermediate, DB2, sbws_idx + 4U, sbwl_idx + 4U, GraphletAnchor::RC);
			this->set_winch_metrics(DredgesPosition::sbDragHead, DB2, sbws_idx + 8U, sbwl_idx + 8U, GraphletAnchor::RC);
		}

		this->set_drag_metrics(DS::PS, DS::PSVisor, DB2, DB203, this->drag_configs[0], this->ps_address);
		this->set_drag_metrics(DS::SB, DS::SBVisor, DB2, DB203, this->drag_configs[1], this->sb_address);
		this->set_drag_metrics(DS::SBL, DS::SBVisor, DB2, DB203, this->drag_configs[2], this->sb_address);
	}

	void on_digital_input(long long timepoint_ms, const uint8* DB4, size_t count4, const uint8* DB205, size_t count205, Syslog* logger) override {
		select_sb_drag(DB205);
		
		DI_hopper_pumps(this->hpumps[DS::PSHP], this->hpumps[DS::PSHP], DB4, ps_hopper_pump_feedback, DB205, ps_hopper_pump_details, ps_underwater_pump_details);
		DI_hopper_pumps(this->hpumps[DS::SBHP], this->hpumps[DS::SBHP], DB4, sb_hopper_pump_feedback, DB205, sb_hopper_pump_details, sb_underwater_pump_details);

		DI_gate_valve(this->valves[DS::D003], DB205, gate_valve_D03_feedback, DB205, gate_valve_D03_status);
		DI_gate_valve(this->valves[DS::D004], DB205, gate_valve_D04_feedback, DB205, gate_valve_D04_status);
		DI_gate_valve(this->valves[DS::D011], DB4, gate_valve_D11_feedback, DB205, gate_valve_D11_status);
		DI_gate_valve(this->valves[DS::D012], DB4, gate_valve_D12_feedback, DB205, gate_valve_D12_status);
		DI_gate_valve(this->valves[DS::D013], DB4, gate_valve_D13_feedback, DB205, gate_valve_D13_status);
		DI_gate_valve(this->valves[DS::D014], DB4, gate_valve_D14_feedback, DB205, gate_valve_D14_status);
		DI_gate_valve(this->valves[DS::D015], DB4, gate_valve_D15_feedback, DB205, gate_valve_D15_status);
		DI_gate_valve(this->valves[DS::D016], DB4, gate_valve_D16_feedback, DB205, gate_valve_D16_status);
		
		DI_gantry(this->gantries[DredgesPosition::psTrunnion], DB4, gantry_ps_trunnion_limited, DB205, gantry_ps_trunnion_details);
		DI_gantry(this->gantries[DredgesPosition::psIntermediate], DB4, gantry_ps_intermediate_limited, DB205, gantry_ps_intermediate_details);
		DI_gantry(this->gantries[DredgesPosition::psDragHead], DB4, gantry_ps_draghead_limited, DB205, gantry_ps_draghead_details);

		DI_gantry(this->gantries[DredgesPosition::sbTrunnion], DB4, gantry_sb_trunnion_limited, DB205, gantry_sb_trunnion_details);
		DI_gantry(this->gantries[DredgesPosition::sbIntermediate], DB4, gantry_sb_intermediate_limited, DB205, gantry_sb_intermediate_details);

		if (DI_long_sb_drag(DB205)) {
			DI_gantry(this->gantries[DredgesPosition::sbDragHead], DB4, gantry_sb_long_draghead_limited, DB205, gantry_sb_draghead_details);
		} else {
			DI_gantry(this->gantries[DredgesPosition::sbDragHead], DB4, gantry_sb_short_draghead_limited, DB205, gantry_sb_draghead_details);
		}

		DI_winch(this->winches[DredgesPosition::psTrunnion], DB4, winch_ps_trunnion_feedback, winch_ps_trunnion_limits, DB205, winch_ps_trunnion_details);
		DI_winch(this->winches[DredgesPosition::psIntermediate], DB4, winch_ps_intermediate_feedback, winch_ps_intermediate_limits, DB205, winch_ps_intermediate_details);
		DI_winch(this->winches[DredgesPosition::psDragHead], DB4, winch_ps_draghead_feedback, winch_ps_draghead_limits, DB205, winch_ps_draghead_details);

		DI_winch(this->winches[DredgesPosition::sbTrunnion], DB4, winch_sb_trunnion_feedback, winch_sb_trunnion_limits, DB205, winch_sb_trunnion_details);
		DI_winch(this->winches[DredgesPosition::sbIntermediate], DB4, winch_sb_intermediate_feedback, winch_sb_intermediate_limits, DB205, winch_sb_intermediate_details);
		DI_winch(this->winches[DredgesPosition::sbDragHead], DB4, winch_sb_draghead_feedback, winch_sb_draghead_limits, DB205, winch_sb_draghead_details);

		DI_suction_buttons(this->ps_suctions[SuctionCommand::Inflate], this->ps_suctions[SuctionCommand::Deflate], DB205, suction_ps_buttons);
		DI_suction_buttons(this->sb_suctions[SuctionCommand::Inflate], this->sb_suctions[SuctionCommand::Deflate], DB205, suction_sb_buttons);

		DI_boolean_button(this->lmod_buttons[LMODCommand::PSALMO], DB205, ps_almo_auto);
		DI_boolean_button(this->lmod_buttons[LMODCommand::SBALMO], DB205, sb_almo_auto);

		DI_boolean_button(this->ps_visors[DragVisorCommand::CResistance], DB205, ctension_ps_button);
		DI_boolean_button(this->sb_visors[DragVisorCommand::CResistance], DB205, ctension_sb_button);

		this->set_winch_limits(DredgesPosition::psTrunnion, DredgesPosition::psDragHead);
		this->set_winch_limits(DredgesPosition::sbTrunnion, DredgesPosition::sbDragHead);

		this->set_hopper_type(DS::PS, DB4, ps_hopper_pump_feedback);
		this->set_hopper_type(DS::SB, DB4, sb_hopper_pump_feedback);
	}

	void on_forat(long long timepoint_ms, const uint8* DB20, size_t count, Syslog* logger) override {
		float overflow_target_height = DBD(DB20, overflow_pipe_target_height);
		
		this->overflowpipe->set_target_height(overflow_target_height, (overflow_target_height == 0.0F));
		this->set_design_depth(DS::PS, DB20, dredging_target_depth, dredging_tolerant_depth);
		this->set_design_depth(DS::SB, DB20, dredging_target_depth, dredging_tolerant_depth);
		this->set_design_depth(DS::SBL, DB20, dredging_target_depth, dredging_tolerant_depth);
	}

	void post_read_data(Syslog* logger) override {
		{ // flow water
			this->station->push_subtrack(DS::D003, DS::SB, water_color);
			this->station->push_subtrack(DS::D004, DS::PS, water_color);

			if (this->valves[DS::D003]->get_state() == GateValveState::Open) {
				DS d11[] = { DS::LMOD, DS::sb, DS::SBHP, DS::D003 };
				DS d13[] = { DS::d013, DS::d13, DS::SBHP, DS::D003 };
				DS d15[] = { DS::d15, DS::d1315, DS::SBHP, DS::D003 };

				this->try_flow_water(DS::D011, d11, water_color);
				this->try_flow_water(DS::D013, d13, water_color);
				this->try_flow_water(DS::D015, d15, water_color);
			}

			if (this->valves[DS::D004]->get_state() == GateValveState::Open) {
				DS d12[] = { DS::LMOD, DS::ps, DS::PSHP, DS::D004 };
				DS d14[] = { DS::d014, DS::d14, DS::PSHP, DS::D004 };
				DS d16[] = { DS::d16, DS::d1416, DS::PSHP, DS::D004 };

				this->try_flow_water(DS::D012, d12, water_color);
				this->try_flow_water(DS::D014, d14, water_color);
				this->try_flow_water(DS::D016, d16, water_color);
			}
		}

		IDredgingSystem::post_read_data(logger);
	}

public:
	void load(float width, float height, float vinset) override {
		this->load_station(width, height, vinset);
		this->load_dashboard(width, height, vinset);
	}

	void reflow(float width, float height, float vinset) {
		this->reflow_station(width, height, vinset);
		this->reflow_dashboard(width, height, vinset);
	}

public:
	void load_station(float width, float height, float vinset) {
		float gridsize = vinset * 0.618F;
		float rx = gridsize;
		float ry = rx * 2.0F;
		float rsct = rx * 0.5F;
		float rlmod = gridsize * 1.5F;
		Turtle<DS>* pTurtle = new Turtle<DS>(gridsize, gridsize, DS::LMOD);

		pTurtle->jump_right(1.5F)->move_right(2.5F, DS::D011)->move_right(3, DS::sb)->move_up(4, DS::SBHP);
		pTurtle->move_right(3, DS::D003)->move_right(3, DS::SB)->jump_back();
		pTurtle->move_right(3, DS::d1315)->move_down(4, DS::d13)->move_left(3, DS::d013)->move_down(2, DS::D013)->jump_back(DS::d13);
		pTurtle->move_down(4, DS::d15)->move_left(3)->move_down(2, DS::D015)->jump_back(DS::LMOD);

		pTurtle->jump_left(1.5F)->move_left(2.5F, DS::D012)->move_left(3, DS::ps)->move_up(4, DS::PSHP);
		pTurtle->move_left(3, DS::D004)->move_left(3, DS::PS)->jump_back();
		pTurtle->move_left(3, DS::d1416)->move_down(4, DS::d14)->move_right(3, DS::d014)->move_down(2, DS::D014)->jump_back(DS::d14);
		pTurtle->move_down(4, DS::d16)->move_right(3)->move_down(2, DS::D016);

		this->station = this->master->insert_one(new Tracklet<DS>(pTurtle, default_pipe_thickness, default_pipe_color));

		this->load_percentage(this->progresses, DS::D003);
		this->load_percentage(this->progresses, DS::D004);
		this->load_valves(this->valves, this->labels, DS::D003, DS::D012, vinset, 0.0);
		this->load_valves(this->valves, this->labels, DS::D013, DS::D016, vinset, -90.0);
		this->load_label(this->labels, DS::LMOD.ToString(), DS::LMOD, Colours::Cyan, this->station_font);
		this->load_label(this->hopper_types, _speak(DS::pump), DS::PS, caption_color, this->caption_font);
		this->load_label(this->hopper_types, _speak(DS::pump), DS::SB, caption_color, this->caption_font);

		this->lmod = this->master->insert_one(new Arclet(0.0, 360.0, rlmod, rlmod, default_pipe_thickness, Colours::Green));
		this->hpumps[DS::PSHP] = this->master->insert_one(new Credit<HopperPumplet, DS>(+rx, -ry), DS::PSHP);
		this->hpumps[DS::SBHP] = this->master->insert_one(new Credit<HopperPumplet, DS>(-rx, -ry), DS::SBHP);
		this->suctions[DS::PS] = this->master->insert_one(new Credit<Circlelet, DS>(rsct, default_ps_color, default_pipe_thickness), DS::PS);
		this->suctions[DS::SB] = this->master->insert_one(new Credit<Circlelet, DS>(rsct, default_sb_color, default_pipe_thickness), DS::SB);
	}

	void load_dashboard(float width, float height, float vinset) {
		float shwidth, shheight, shhmargin, shvmargin, xstep, ystep;
		
		this->station->fill_extent(0.0F, 0.0F, &shwidth, &shheight);
		this->station->fill_stepsize(&xstep, &ystep);

		shwidth += (xstep * 2.0F);
		shheight += ystep;
		shvmargin = (width - shwidth) * 0.5F;
		shhmargin = (height - shheight) * 0.5F;

		{ // load dimensions
			float dfmeter_height = shhmargin * 0.72F;
			float cylinder_height = shheight * 0.618F;
			float winch_width = shvmargin * 0.12F;
			float gantry_width = winch_width * 1.4F;
		
			this->overflowpipe = this->master->insert_one(new OverflowPipelet(hopper_height_range, shheight * 0.382F));
			this->load_dimension(this->lengths, DS::Overflow, "meter");
			this->load_dimension(this->forces, DS::PSPF1, "knewton");
			this->load_dimension(this->forces, DS::PSPF2, "knewton");
			this->load_dimension(this->forces, DS::SBPF1, "knewton");
			this->load_dimension(this->forces, DS::SBPF2, "knewton");
			this->load_dimension(this->pressures, DS::PSSIP, "bar", false);
			this->load_dimension(this->pressures, DS::SBSIP, "bar", false);

			this->load_densityflowmeters(this->dfmeters, DS::PS, DS::SB, dfmeter_height);
			this->load_compensators(this->compensators, DS::PSWC, DS::SBWC, cylinder_height, compensator_range);
			this->load_cylinders(this->cylinders, DS::PSHPDP, DS::SBHPDP, cylinder_height, 0.0, 20.0, "bar");
			this->load_cylinders(this->cylinders, DS::PSHPVP, DS::SBHPVP, cylinder_height, -2.0, 2.0, "bar");

			this->load_gantries(this->gantries, DredgesPosition::psTrunnion, DredgesPosition::psDragHead, -gantry_width);
			this->load_gantries(this->gantries, DredgesPosition::sbTrunnion, DredgesPosition::sbDragHead, +gantry_width);

			this->load_winches(this->winches, DredgesPosition::psTrunnion, DredgesPosition::psDragHead, winch_width);
			this->load_winches(this->winches, DredgesPosition::sbTrunnion, DredgesPosition::sbDragHead, winch_width);
		}

		{ // load drags
			float side_drag_width = width * 0.25F;
			float over_drag_height = height * 0.42F;
			float draghead_radius = side_drag_width * 0.2718F;

			this->load_draghead(this->dragheads, DS::PSVisor, DS::PSDP, -draghead_radius, this->drag_configs[0], default_ps_color);
			this->load_draghead(this->dragheads, DS::SBVisor, DS::SBDP, +draghead_radius, this->drag_configs[1], default_sb_color);
		
			this->load_overview_drag(this->dragxys, DS::PS, -over_drag_height, 0);
			this->load_overview_drag(this->dragxys, DS::SB, +over_drag_height, 1);
			this->load_overview_drag(this->dragxys, DS::SBL, +over_drag_height, 2);

			this->load_sideview_drag(this->dragxzes, DS::PS, -side_drag_width, drag_depth_degrees_max, 0);
			this->load_sideview_drag(this->dragxzes, DS::SB, +side_drag_width, drag_depth_degrees_max, 1);
			this->load_sideview_drag(this->dragxzes, DS::SBL, +side_drag_width, drag_depth_degrees_max, 2);
		}

		{ // load buttons
			this->load_buttons(this->ps_suctions, DS::PS);
			this->load_buttons(this->sb_suctions, DS::SB);
			this->load_buttons(this->ps_visors, DS::PS);
			this->load_buttons(this->sb_visors, DS::SB);
			this->load_buttons(this->lmod_buttons, DS::LMOD, xstep * 6.0F);
		}
	}

public:
	void reflow_station(float width, float height, float vinset) {
		GraphletAnchor anchor;
		float dx, dy, xstep, ystep;
		float x0 = 0.0F;
		float y0 = 0.0F;
		
		this->station->fill_stepsize(&xstep, &ystep);

		this->master->move_to(this->station, width * 0.5F, height * 0.5F, GraphletAnchor::CC, 0.0F, -vinset);

		for (auto it = this->valves.begin(); it != this->valves.end(); it++) {
			switch (it->first) {
			case DS::D014: case DS::D016: {
				dx = x0 + xstep; dy = y0; anchor = GraphletAnchor::LC;
			}; break;
			case DS::D013: case DS::D015: {
				dx = x0 - xstep; dy = y0; anchor = GraphletAnchor::RC;
			}; break;
			default: {
				dx = x0; dy = y0 - ystep; anchor = GraphletAnchor::CB;
			}
			}

			this->station->map_credit_graphlet(it->second, GraphletAnchor::CC, x0, y0);
			this->station->map_credit_graphlet(this->labels[it->first], anchor, dx, dy);
		}

		this->station->map_credit_graphlet(this->progresses[DS::D003], GraphletAnchor::CT, 0.0F, ystep);
		this->station->map_credit_graphlet(this->progresses[DS::D004], GraphletAnchor::CT, 0.0F, ystep);

		this->station->map_graphlet_at_anchor(this->lmod, DS::LMOD, GraphletAnchor::CC);
		this->station->map_credit_graphlet(this->labels[DS::LMOD], GraphletAnchor::CC);
		
		this->hpumps[DS::SBHP]->fill_pump_origin(&dx, nullptr);
		this->station->map_credit_graphlet(this->hpumps[DS::SBHP], GraphletAnchor::CC, +std::fabsf(dx));
		this->station->map_credit_graphlet(this->hpumps[DS::PSHP], GraphletAnchor::CC, -std::fabsf(dx));
		this->station->map_credit_graphlet(this->suctions[DS::PS], GraphletAnchor::LC);
		this->station->map_credit_graphlet(this->suctions[DS::SB], GraphletAnchor::RC);

		this->master->move_to(this->cylinders[DS::PSHPDP], this->station, GraphletAnchor::LC, GraphletAnchor::RB, 0.0F, vinset * 2.0F);
		this->master->move_to(this->cylinders[DS::PSHPVP], this->cylinders[DS::PSHPDP], GraphletAnchor::LB, GraphletAnchor::RB, -vinset);
		this->master->move_to(this->cylinders[DS::SBHPDP], this->station, GraphletAnchor::RC, GraphletAnchor::LB, 0.0F, vinset * 2.0F);
		this->master->move_to(this->cylinders[DS::SBHPVP], this->cylinders[DS::SBHPDP], GraphletAnchor::RB, GraphletAnchor::LB, +vinset);
	}

	void reflow_dashboard(float width, float height, float vinset) {
		float txt_gapsize = vinset * 0.5F;
		float cx = width * 0.5F;
		float cy = height * 0.5F;
		float trunnion_y, intermediate_y, draghead_y;
		
		{ // reflow centeral components
			float wc_offset = vinset * 1.5F;

			this->master->move_to(this->overflowpipe, this->station, GraphletAnchor::CC, GraphletAnchor::CT);
			this->master->move_to(this->lengths[DS::Overflow], this->overflowpipe, GraphletAnchor::CB, GraphletAnchor::CT, 0.0F, txt_gapsize);

			this->master->move_to(this->dfmeters[DS::PS], this->hpumps[DS::PSHP], GraphletAnchor::RT, GraphletAnchor::RB, 0.0F, -vinset);
			this->master->move_to(this->dfmeters[DS::SB], this->hpumps[DS::SBHP], GraphletAnchor::LT, GraphletAnchor::LB, 0.0F, -vinset);
			this->master->move_to(this->compensators[DS::PSWC], this->cylinders[DS::PSHPVP], GraphletAnchor::LB, GraphletAnchor::RB, -wc_offset);

			this->master->move_to(this->dragheads[DS::PSVisor], cx, height * 0.85F, GraphletAnchor::RC, -vinset);
			this->master->move_to(this->dragheads[DS::SBVisor], cx, height * 0.85F, GraphletAnchor::LC, +vinset);
			this->master->move_to(this->compensators[DS::SBWC], this->cylinders[DS::SBHPVP], GraphletAnchor::RB, GraphletAnchor::LB, +wc_offset);
		}

		{ // reflow left dredging system
			float dflx, wclx, dhx;

			this->master->fill_graphlet_location(this->dfmeters[DS::PS], &dflx, nullptr, GraphletAnchor::LC);
			this->master->fill_graphlet_location(this->compensators[DS::PSWC], &wclx, nullptr, GraphletAnchor::LC);
			this->master->fill_graphlet_location(this->dragheads[DS::PSVisor], &dhx, nullptr, GraphletAnchor::LC);

			this->master->move_to(this->dragxys[DS::PS], std::fminf(dflx, wclx), cy, GraphletAnchor::RB, -vinset, vinset * 2.0F);
			this->master->move_to(this->dragxzes[DS::PS], dhx - vinset, height, GraphletAnchor::RB);

			{ // reflow gantries
				float lx = 0.0F;

				this->master->fill_graphlet_location(this->dragxys[DS::PS], nullptr, &trunnion_y, GraphletAnchor::CT);
				this->master->fill_graphlet_location(this->dragxys[DS::PS], nullptr, &intermediate_y, GraphletAnchor::CC);
				this->master->fill_graphlet_location(this->dragxys[DS::PS], nullptr, &draghead_y, GraphletAnchor::CB);

				this->master->move_to(this->gantries[DredgesPosition::psTrunnion], lx, trunnion_y, GraphletAnchor::LT, 0.0F, vinset * 5.0F);
				this->master->move_to(this->gantries[DredgesPosition::psIntermediate], lx, intermediate_y, GraphletAnchor::LC, 0.0F, vinset * 2.0F);
				this->master->move_to(this->gantries[DredgesPosition::psDragHead], lx, draghead_y, GraphletAnchor::LB, 0.0F, -vinset * 1.0F);
			}
		}

		{ // reflow right dredging system
			float dfrx, wcrx, dhrx;

			this->master->fill_graphlet_location(this->dfmeters[DS::SB], &dfrx, nullptr, GraphletAnchor::RC);
			this->master->fill_graphlet_location(this->compensators[DS::SBWC], &wcrx, nullptr, GraphletAnchor::RC);
			this->master->fill_graphlet_location(this->dragheads[DS::SBVisor], &dhrx, nullptr, GraphletAnchor::RC);

			this->master->move_to(this->dragxys[DS::SB], std::fmaxf(dfrx, wcrx), cy, GraphletAnchor::LB, vinset, vinset * 2.0F);
			this->master->move_to(this->dragxzes[DS::SB], dhrx + vinset, height, GraphletAnchor::LB);

			this->master->move_to(this->dragxys[DS::SBL], this->dragxys[DS::SB], GraphletAnchor::CC, GraphletAnchor::CC);
			this->master->move_to(this->dragxzes[DS::SBL], this->dragxzes[DS::SB], GraphletAnchor::CC, GraphletAnchor::CC);

			{ // reflow winches
				float rx = width;

				this->master->fill_graphlet_location(this->dragxys[DS::SB], nullptr, &trunnion_y, GraphletAnchor::CT);
				this->master->fill_graphlet_location(this->dragxys[DS::SB], nullptr, &intermediate_y, GraphletAnchor::CC);
				this->master->fill_graphlet_location(this->dragxys[DS::SB], nullptr, &draghead_y, GraphletAnchor::CB);

				this->master->move_to(this->gantries[DredgesPosition::sbTrunnion], rx, trunnion_y, GraphletAnchor::RT, 0.0F, vinset * 5.0F);
				this->master->move_to(this->gantries[DredgesPosition::sbIntermediate], rx, intermediate_y, GraphletAnchor::RC, 0.0F, vinset * 2.0F);
				this->master->move_to(this->gantries[DredgesPosition::sbDragHead], rx, draghead_y, GraphletAnchor::RB, 0.0F, -vinset * 1.0F);
			}
		}

		{ // reflow dimensions and labels
			for (DS id = DS::PSWC; id <= DS::SBWC; id++) {
				this->master->move_to(this->labels[id], this->compensators[id], GraphletAnchor::CT, GraphletAnchor::CB, 0.0F, -txt_gapsize);
				this->master->move_to(this->lengths[id], this->compensators[id], GraphletAnchor::CB, GraphletAnchor::CT);
				this->master->move_to(this->pressures[id], this->lengths[id], GraphletAnchor::CB, GraphletAnchor::CT);
			}

			for (DS id = DS::PSHPDP; id <= DS::SBHPVP; id++) {
				this->master->move_to(this->labels[id], this->cylinders[id], GraphletAnchor::CT, GraphletAnchor::CB, 0.0F, -txt_gapsize);
				this->master->move_to(this->pressures[id], this->cylinders[id], GraphletAnchor::CB, GraphletAnchor::CT, 0.0F, txt_gapsize);
			}

			for (DredgesPosition id = DredgesPosition::psTrunnion; id <= DredgesPosition::psDragHead; id++) {
				this->master->move_to(this->winches[id], this->gantries[id], GraphletAnchor::CC, GraphletAnchor::CC);
				this->master->move_to(this->winch_lengths[id], this->gantries[id], GraphletAnchor::RC, GraphletAnchor::LC, txt_gapsize);
				this->master->move_to(this->position_labels[id], this->winch_lengths[id], GraphletAnchor::LT, GraphletAnchor::LB);
				this->master->move_to(this->winch_speeds[id], this->winch_lengths[id], GraphletAnchor::LB, GraphletAnchor::LT);

				this->reflow_winch_limits(id);
			}

			for (DredgesPosition id = DredgesPosition::sbTrunnion; id <= DredgesPosition::sbDragHead; id++) {
				this->master->move_to(this->winches[id], this->gantries[id], GraphletAnchor::CC, GraphletAnchor::CC);
				this->master->move_to(this->winch_lengths[id], this->gantries[id], GraphletAnchor::LC, GraphletAnchor::RC, -txt_gapsize);
				this->master->move_to(this->position_labels[id], this->winch_lengths[id], GraphletAnchor::RT, GraphletAnchor::RB);
				this->master->move_to(this->winch_speeds[id], this->winch_lengths[id], GraphletAnchor::RB, GraphletAnchor::RT);

				this->reflow_winch_limits(id);
			}

			{ // reflow pulling forces
				float pf_xoff = vinset * 2.0F;

				this->master->move_to(this->forces[DS::PSPF1], this->dragxzes[DS::PS], GraphletAnchor::LT, GraphletAnchor::LB, +pf_xoff);
				this->master->move_to(this->forces[DS::PSPF2], this->dragxzes[DS::PS], GraphletAnchor::RT, GraphletAnchor::RB, -pf_xoff);
				this->master->move_to(this->forces[DS::SBPF1], this->dragxzes[DS::SB], GraphletAnchor::RT, GraphletAnchor::RB, -pf_xoff);
				this->master->move_to(this->forces[DS::SBPF2], this->dragxzes[DS::SB], GraphletAnchor::LT, GraphletAnchor::LB, +pf_xoff);
			}

			this->master->move_to(this->hopper_types[DS::PS], this->labels[DS::PSHPDP], GraphletAnchor::LT, GraphletAnchor::CB, -vinset, -txt_gapsize);
			this->master->move_to(this->hopper_types[DS::SB], this->labels[DS::SBHPDP], GraphletAnchor::RT, GraphletAnchor::CB, +vinset, -txt_gapsize);

			this->reflow_draghead_metrics(DS::PSVisor, DS::PSDP);
			this->reflow_draghead_metrics(DS::SBVisor, DS::SBDP);
		}

		{ // reflow buttons
			float bx = 0.0F;
			float by = vinset * 0.2F;

			this->master->move_to(this->ps_suctions[SuctionCommand::Inflate], bx, by);
			this->master->move_to(this->ps_suctions[SuctionCommand::Deflate], this->ps_suctions[SuctionCommand::Inflate], GraphletAnchor::RC, GraphletAnchor::LC);
			this->master->move_to(this->pressures[DS::PSSIP], this->ps_suctions[SuctionCommand::Deflate], GraphletAnchor::RC, GraphletAnchor::LC, vinset);

			this->master->move_to(this->sb_suctions[SuctionCommand::Deflate], width - bx, by, GraphletAnchor::RT);
			this->master->move_to(this->sb_suctions[SuctionCommand::Inflate], this->sb_suctions[SuctionCommand::Deflate], GraphletAnchor::LC, GraphletAnchor::RC);
			this->master->move_to(this->pressures[DS::SBSIP], this->sb_suctions[SuctionCommand::Inflate], GraphletAnchor::LC, GraphletAnchor::RC, -vinset);

			this->station->map_graphlet_base_on_anchors(this->lmod_buttons[LMODCommand::PSALMO], DS::LMOD, DS::PSHP, GraphletAnchor::RC);
			this->station->map_graphlet_base_on_anchors(this->lmod_buttons[LMODCommand::SBALMO], DS::LMOD, DS::SBHP, GraphletAnchor::LC);
			this->master->move_to(this->ps_visors[DragVisorCommand::CResistance], this->degrees[DS::PSVisor], GraphletAnchor::CB, GraphletAnchor::CT, 0.0F, vinset);
			this->master->move_to(this->sb_visors[DragVisorCommand::CResistance], this->degrees[DS::SBVisor], GraphletAnchor::CB, GraphletAnchor::CT, 0.0F, vinset);
		}
	}

public:
	bool can_select(IGraphlet* g) override {
		return button_enabled(g);
	}

	void on_tap_selected(IGraphlet* g, float local_x, float local_y) {
		auto suction_btn = dynamic_cast<GroupCredit<Buttonlet, DS, SuctionCommand>*>(g);
		auto visor_btn = dynamic_cast<GroupCredit<Buttonlet, DS, DragVisorCommand>*>(g);
		auto lmod_btn = dynamic_cast<GroupCredit<Buttonlet, DS, LMODCommand>*>(g);
		auto plc = this->master->get_plc_device();

		if (suction_btn != nullptr) {
			plc->send_command(DO_suction_command(suction_btn->id, suction_btn->gid == DS::PS));
		} else if (visor_btn != nullptr) {
			plc->send_command((visor_btn->gid == DS::PS)
				? drag_ps_visor_constant_resistance_command
				: drag_sb_visor_constant_resistance_command);
		} else if (lmod_btn != nullptr) {
			plc->send_command(DO_LMOD_command(lmod_btn->id));
		}
	}

private:
	template<class G, typename E>
	void load_valves(std::map<E, G*>& gs, std::map<E, Credit<Labellet, E>*>& ls, E id0, E idn, float radius, double degrees) {
		for (E id = id0; id <= idn; id++) {
			this->load_label(ls, id.ToString(), id, Colours::Silver, this->station_font);
			gs[id] = this->master->insert_one(new G(radius, degrees), id);
		}
	}

	template<class G, typename E>
	void load_gantries(std::map<E, Credit<G, E>*>& cs, E id0, E idn, float width) {
		for (E id = id0; id <= idn; id++) {
			cs[id] = this->master->insert_one(new Credit<G, E>(width), id);
		}
	}

private:
	void set_cylinder(DS id, float value) {
		this->cylinders[id]->set_value(value);
		this->pressures[id]->set_value(value, GraphletAnchor::CC);
	}

	void set_density_speed(DS id, const uint8* db203, unsigned int idx) {
		this->dfmeters[id]->set_values(RealData(db203, idx + 0U), RealData(db203, idx + 1U));
	}

	void set_hopper_type(DS id, const uint8* db4, unsigned int idx_p1) {
		if (DI_hopper_type(db4, idx_p1)) {
			this->hopper_types[id]->set_text(_speak(DS::hopper), GraphletAnchor::CC);
		} else if (DI_underwater_type(db4, idx_p1)) {
			this->hopper_types[id]->set_text(_speak(DS::underwater), GraphletAnchor::CC);
		} else {
			this->hopper_types[id]->set_text("", GraphletAnchor::CC);
		}
	}

private:
	void try_flow_water(DS vid, DS eid, CanvasSolidColorBrush^ color) {
		switch (this->valves[vid]->get_state()) {
		case GateValveState::Open: {
			this->station->push_subtrack(vid, eid, color);
		}
		}
	}

	void try_flow_water(DS vid, DS* path, unsigned int count, CanvasSolidColorBrush^ color) {
		switch (this->valves[vid]->get_state()) {
		case GateValveState::Open: {
			this->station->push_subtrack(vid, path[0], color);
			this->station->push_subtrack(path, count, color);
		}
		}
	}

	template<unsigned int N>
	void try_flow_water(DS vid, DS (&path)[N], CanvasSolidColorBrush^ color) {
		this->try_flow_water(vid, path, N, color);
	}

private: // never delete these graphlets manually.
	Tracklet<DS>* station;
	std::map<DS, Credit<GateValvelet, DS>*> valves;
	std::map<DS, Credit<HopperPumplet, DS>*> hpumps;
	std::map<DS, Credit<Circlelet, DS>*> suctions;
	std::map<DS, Credit<Cylinderlet, DS>*> cylinders;
	std::map<DS, Credit<DensitySpeedmeterlet, DS>*> dfmeters;
	std::map<DS, Credit<Percentagelet, DS>*> progresses;
	std::map<DS, Credit<Labellet, DS>*> hopper_types;
	std::map<DredgesPosition, Credit<GantrySymbollet, DredgesPosition>*> gantries;
	std::map<SuctionCommand, GroupCredit<Buttonlet, DS, SuctionCommand>*> ps_suctions;
	std::map<SuctionCommand, GroupCredit<Buttonlet, DS, SuctionCommand>*> sb_suctions;
	std::map<DragVisorCommand, GroupCredit<Buttonlet, DS, DragVisorCommand>*> ps_visors;
	std::map<DragVisorCommand, GroupCredit<Buttonlet, DS, DragVisorCommand>*> sb_visors;
	std::map<LMODCommand, GroupCredit<Buttonlet, DS, LMODCommand>*> lmod_buttons;
	OverflowPipelet* overflowpipe;
	Arclet* lmod;

private: // never delete these global objects
	DredgeAddress* ps_address;
	DredgeAddress* sb_address;
};

/*************************************************************************************************/
private class Drags final : public IDredgingSystem {
public:
	Drags(DredgesPage* master, DS side, unsigned int drag_color, unsigned int config_idx)
		: IDredgingSystem(master), DS_side(side), drag_color(drag_color), drag_idx(config_idx) {
		PLCMaster* plc = master->get_plc_device();

		this->pump_style = make_highlight_dimension_style(small_metrics_font_size, 6U, 0, Colours::Background);
		this->highlight_style = make_highlight_dimension_style(small_metrics_font_size, 6U, 0, Colours::Green);
		this->setting_style = make_highlight_dimension_style(normal_metrics_font_size, 6U, 1);

		this->button_style.thickness = 2.0F;
		this->button_style.corner_radius = 1.0F;
		this->button_style.foreground_color = Colours::CornflowerBlue;
		this->button_style.font = make_bold_text_format(tiny_font_size);

		if (plc != nullptr) {
			this->winch_op = make_dredging_winch_menu(dredges_diagnostics, plc);
			this->gantry_op = make_gantry_menu(dredges_diagnostics, plc);
			this->compensator_op = make_wave_compensator_menu(plc);
			this->visor_op = make_drag_visor_menu(plc);
		}

		if (this->DS_side == DS::PS) {
			this->address = make_ps_dredging_system_schema();
			this->sign = -1.0F;

			if (plc != nullptr) {
				this->ggantries_op = make_gantry_group_menu(DredgesGroup::PSGantries, plc);
			}
		} else {
			this->address = make_sb_dredging_system_schema();
			this->sign = +1.0F;

			if (plc != nullptr) {
				this->ggantries_op = make_gantry_group_menu(DredgesGroup::SBGantries, plc);
			}
		}
	}

public:
	void on_analog_input(long long timepoint_ms, const uint8* DB2, size_t count2, const uint8* DB203, size_t count203, Syslog* logger) override {
		this->lengths[DS::TildeMark]->set_value(DBD(DB2, tilde_mark));
		this->speeds[DS::Speed]->set_value(DBD(DB2, gps_speed));

		{ // set winches metrics
			unsigned int psws_idx = this->address->winch_speed;
			unsigned int pswl_idx = this->address->winch_length;
			unsigned int sbws_idx = this->address->winch_speed;
			unsigned int sbwl_idx = this->address->winch_length;

			if (this->DS_side == DS::PS) {
				this->set_winch_metrics(DredgesPosition::psTrunnion, DB2, psws_idx + 0U, pswl_idx + 0U, GraphletAnchor::CC);
				this->set_winch_metrics(DredgesPosition::psIntermediate, DB2, psws_idx + 4U, pswl_idx + 4U, GraphletAnchor::CC);
				this->set_winch_metrics(DredgesPosition::psDragHead, DB2, psws_idx + 8U, pswl_idx + 8U, GraphletAnchor::CC);
			} else {
				this->set_winch_metrics(DredgesPosition::sbTrunnion, DB2, sbws_idx + 0U, sbwl_idx + 0U, GraphletAnchor::CC);
				this->set_winch_metrics(DredgesPosition::sbIntermediate, DB2, sbws_idx + 4U, sbwl_idx + 4U, GraphletAnchor::CC);
				this->set_winch_metrics(DredgesPosition::sbDragHead, DB2, sbws_idx + 8U, sbwl_idx + 8U, GraphletAnchor::CC);
			}
		}

		if (this->DS_side == DS::PS) {
			this->pump_pressures[DredgesPosition::psTrunnion]->set_value(RealData(DB203, pump_C_pressure), GraphletAnchor::CC);
			this->pump_pressures[DredgesPosition::psIntermediate]->set_value(RealData(DB203, pump_B_pressure), GraphletAnchor::CC);
			this->pump_pressures[DredgesPosition::psDragHead]->set_value(RealData(DB203, pump_A_pressure), GraphletAnchor::CC);

			this->rc_speeds[DredgesPosition::psIntermediate]->set_value(RealData(DB203, winch_ps_intermediate_remote_speed), GraphletAnchor::CC);
			this->rc_speeds[DredgesPosition::psDragHead]->set_value(RealData(DB203, winch_ps_draghead_remote_speed), GraphletAnchor::CC);
			
			this->forces[DS::PSPF1]->set_value(RealData(DB203, this->address->pulling_force + 0U), GraphletAnchor::LC);
			this->forces[DS::PSPF2]->set_value(RealData(DB203, this->address->pulling_force + 1U), GraphletAnchor::LC);
			
			this->set_compensator(DS::PSWC, DB203, this->address->compensator, GraphletAnchor::LC);

			this->Alets[DredgesPosition::psTrunnion]->set_value(RealData(DB203, winch_ps_trunnion_A_pressure), GraphletAnchor::LB);
			this->Blets[DredgesPosition::psTrunnion]->set_value(RealData(DB203, winch_ps_trunnion_B_pressure), GraphletAnchor::LT);
			this->Alets[DredgesPosition::psIntermediate]->set_value(RealData(DB203, winch_ps_intermediate_A_pressure), GraphletAnchor::LB);
			this->Blets[DredgesPosition::psIntermediate]->set_value(RealData(DB203, winch_ps_intermediate_B_pressure), GraphletAnchor::LT);
			this->Alets[DredgesPosition::psDragHead]->set_value(RealData(DB203, winch_ps_draghead_A_pressure), GraphletAnchor::LB);
			this->Blets[DredgesPosition::psDragHead]->set_value(RealData(DB203, winch_ps_draghead_B_pressure), GraphletAnchor::LT);
		} else {
			this->pump_pressures[DredgesPosition::sbTrunnion]->set_value(RealData(DB203, pump_F_pressure), GraphletAnchor::CC);
			this->pump_pressures[DredgesPosition::sbIntermediate]->set_value(RealData(DB203, pump_G_pressure), GraphletAnchor::CC);
			this->pump_pressures[DredgesPosition::sbDragHead]->set_value(RealData(DB203, pump_H_pressure), GraphletAnchor::CC);

			this->rc_speeds[DredgesPosition::sbIntermediate]->set_value(RealData(DB203, winch_sb_intermediate_remote_speed), GraphletAnchor::CC);
			this->rc_speeds[DredgesPosition::sbDragHead]->set_value(RealData(DB203, winch_sb_draghead_remote_speed), GraphletAnchor::CC);

			this->forces[DS::SBPF1]->set_value(RealData(DB203, this->address->pulling_force + 0U), GraphletAnchor::LC);
			this->forces[DS::SBPF2]->set_value(RealData(DB203, this->address->pulling_force + 1U), GraphletAnchor::LC);

			this->set_compensator(DS::SBWC, DB203, this->address->compensator, GraphletAnchor::RC);

			this->Alets[DredgesPosition::sbTrunnion]->set_value(RealData(DB203, winch_sb_trunnion_A_pressure), GraphletAnchor::RB);
			this->Blets[DredgesPosition::sbTrunnion]->set_value(RealData(DB203, winch_sb_trunnion_B_pressure), GraphletAnchor::RT);
			this->Alets[DredgesPosition::sbIntermediate]->set_value(RealData(DB203, winch_sb_intermediate_A_pressure), GraphletAnchor::RB);
			this->Blets[DredgesPosition::sbIntermediate]->set_value(RealData(DB203, winch_sb_intermediate_B_pressure), GraphletAnchor::RT);
			this->Alets[DredgesPosition::sbDragHead]->set_value(RealData(DB203, winch_sb_draghead_A_pressure), GraphletAnchor::RB);
			this->Blets[DredgesPosition::sbDragHead]->set_value(RealData(DB203, winch_sb_draghead_B_pressure), GraphletAnchor::RT);
		}

		if (this->DS_side == DS::PS) {
			this->set_drag_metrics(DS::PS, DS::PSVisor, DB2, DB203, this->drag_configs[0], this->address);
		} else {
			this->set_drag_metrics(DS::SB, DS::SBVisor, DB2, DB203, this->drag_configs[1], this->address);
			this->set_drag_metrics(DS::SBL, DS::SBVisor, DB2, DB203, this->drag_configs[2], this->address);
		}
	}

	void on_digital_input(long long timepoint_ms, const uint8* DB4, size_t count4, const uint8* DB205, size_t count205, Syslog* logger) override {
		if (this->DS_side == DS::PS) {
			DI_hydraulic_pump_dimension(this->pump_pressures[DredgesPosition::psTrunnion], DB4, pump_C_feedback);
			DI_hydraulic_pump_dimension(this->pump_pressures[DredgesPosition::psIntermediate], DB4, pump_B_feedback);
			DI_hydraulic_pump_dimension(this->pump_pressures[DredgesPosition::psDragHead], DB4, pump_A_feedback);

			DI_winch(this->winches[DredgesPosition::psTrunnion], DB4, winch_ps_trunnion_feedback, winch_ps_trunnion_limits, DB205, winch_ps_trunnion_details);
			DI_winch(this->winches[DredgesPosition::psIntermediate], DB4, winch_ps_intermediate_feedback, winch_ps_intermediate_limits, DB205, winch_ps_intermediate_details);
			DI_winch(this->winches[DredgesPosition::psDragHead], DB4, winch_ps_draghead_feedback, winch_ps_draghead_limits, DB205, winch_ps_draghead_details);
			this->set_winch_limits(DredgesPosition::psTrunnion, DredgesPosition::psDragHead);

			DI_boolean_button(this->buttons[DredgesPosition::psTrunnion], DB205, winch_ps_trunnion_details.override);
			DI_boolean_button(this->buttons[DredgesPosition::psIntermediate], DB205, winch_ps_intermediate_details.override);
			DI_boolean_button(this->buttons[DredgesPosition::psDragHead], DB205, winch_ps_draghead_details.override);

			DI_gantry(this->gantries[DredgesPosition::psTrunnion], DB4, gantry_ps_trunnion_limited, DB205, gantry_ps_trunnion_details);
			DI_gantry(this->gantries[DredgesPosition::psIntermediate], DB4, gantry_ps_intermediate_limited, DB205, gantry_ps_intermediate_details);
			DI_gantry(this->gantries[DredgesPosition::psDragHead], DB4, gantry_ps_draghead_limited, DB205, gantry_ps_draghead_details);

			DI_boolean_button(this->t_gantry_buttons[GantryCommand::VirtualUp], DB205, gantry_ps_trunnion_virtual_up_limited);
			DI_boolean_button(this->t_gantry_buttons[GantryCommand::VirtualOut], DB205, gantry_ps_trunnion_virtual_out_limited);
			DI_boolean_button(this->i_gantry_buttons[GantryCommand::VirtualUp], DB205, gantry_ps_intermediate_virtual_up_limited);
			DI_boolean_button(this->i_gantry_buttons[GantryCommand::VirtualOut], DB205, gantry_ps_intermediate_virtual_out_limited);
			DI_boolean_button(this->h_gantry_buttons[GantryCommand::VirtualUp], DB205, gantry_ps_draghead_virtual_up_limited);
			DI_boolean_button(this->h_gantry_buttons[GantryCommand::VirtualOut], DB205, gantry_ps_draghead_virtual_out_limited);
		} else {
			select_sb_drag(DB205);

			DI_hydraulic_pump_dimension(this->pump_pressures[DredgesPosition::sbTrunnion], DB4, pump_F_feedback);
			DI_hydraulic_pump_dimension(this->pump_pressures[DredgesPosition::sbIntermediate], DB4, pump_G_feedback);
			DI_hydraulic_pump_dimension(this->pump_pressures[DredgesPosition::sbDragHead], DB4, pump_H_feedback);

			DI_winch(this->winches[DredgesPosition::sbTrunnion], DB4, winch_sb_draghead_feedback, winch_sb_trunnion_limits, DB205, winch_sb_trunnion_details);
			DI_winch(this->winches[DredgesPosition::sbIntermediate], DB4, winch_sb_intermediate_feedback, winch_sb_intermediate_limits, DB205, winch_sb_intermediate_details);
			DI_winch(this->winches[DredgesPosition::sbDragHead], DB4, winch_sb_draghead_feedback, winch_sb_draghead_limits, DB205, winch_sb_draghead_details);
			this->set_winch_limits(DredgesPosition::sbTrunnion, DredgesPosition::sbDragHead);

			DI_boolean_button(this->buttons[DredgesPosition::sbTrunnion], DB205, winch_sb_trunnion_details.override);
			DI_boolean_button(this->buttons[DredgesPosition::sbIntermediate], DB205, winch_sb_intermediate_details.override);
			DI_boolean_button(this->buttons[DredgesPosition::sbDragHead], DB205, winch_sb_draghead_details.override);

			DI_gantry(this->gantries[DredgesPosition::sbTrunnion], DB4, gantry_sb_trunnion_limited, DB205, gantry_sb_trunnion_details);
			DI_gantry(this->gantries[DredgesPosition::sbIntermediate], DB4, gantry_sb_intermediate_limited, DB205, gantry_sb_intermediate_details);

			if (DI_long_sb_drag(DB205)) {
				DI_gantry(this->gantries[DredgesPosition::sbDragHead], DB4, gantry_sb_long_draghead_limited, DB205, gantry_sb_draghead_details);
				this->drag_use_buttons[DragUseCommand::LongDrag]->set_state(ButtonState::Executing);
			} else {
				DI_gantry(this->gantries[DredgesPosition::sbDragHead], DB4, gantry_sb_short_draghead_limited, DB205, gantry_sb_draghead_details);
				this->drag_use_buttons[DragUseCommand::LongDrag]->set_state(ButtonState::Default);
			}

			DI_boolean_button(this->t_gantry_buttons[GantryCommand::VirtualUp], DB205, gantry_sb_trunnion_virtual_up_limited);
			DI_boolean_button(this->t_gantry_buttons[GantryCommand::VirtualOut], DB205, gantry_sb_trunnion_virtual_out_limited);
			DI_boolean_button(this->i_gantry_buttons[GantryCommand::VirtualUp], DB205, gantry_sb_intermediate_virtual_up_limited);
			DI_boolean_button(this->i_gantry_buttons[GantryCommand::VirtualOut], DB205, gantry_sb_intermediate_virtual_out_limited);
			DI_boolean_button(this->h_gantry_buttons[GantryCommand::VirtualUp], DB205, gantry_sb_draghead_virtual_up_limited);
			DI_boolean_button(this->h_gantry_buttons[GantryCommand::VirtualOut], DB205, gantry_sb_draghead_virtual_out_limited);
		}
	}

	void on_forat(long long timepoint_ms, const uint8* DB20, size_t count, Syslog* logger) override {
		float target_depth = DBD(DB20, dredging_target_depth);
		float tolerant_depth = DBD(DB20, dredging_tolerant_depth);

		this->settings[DS::DesignDepth]->set_value(target_depth);
		this->settings[DS::TolerantDepth]->set_value(tolerant_depth);

		this->dragxzes[this->DS_side]->set_design_depth(target_depth, tolerant_depth);

		if (this->DS_side == DS::SB) {
			this->dragxzes[DS::SBL]->set_design_depth(target_depth, tolerant_depth);
		}

		this->settings[DS::DivingDepth]->set_value(DBD(DB20, draghead_diving_depth));
		this->settings[DS::DivingCompensation]->set_value(DBD(DB20, compensator_diving_progress));
		this->settings[DS::LandingDepth]->set_value(DBD(DB20, draghead_landing_depth));
	}

public:
	void load(float width, float height, float vinset) override {
		DragInfo config = this->drag_configs[this->drag_idx];
		float over_drag_height = height * 0.56F;
		float side_drag_width = width * 0.32F;
		float draghead_radius = side_drag_width * 0.20F;
		float gantry_radius = over_drag_height * 0.24F;
		float winch_width = gantry_radius * 0.618F;
		float gvabtn_size = 36.0F;
		
		this->load_sideview_drag(this->dragxzes, this->DS_side, side_drag_width * this->sign, drag_depth_degrees_max, this->drag_idx);
		this->load_overview_drag(this->dragxys, this->DS_side, over_drag_height * this->sign, this->drag_idx);

		if (this->DS_side == DS::SB) {
			this->load_sideview_drag(this->dragxzes, DS::SBL, side_drag_width * this->sign, drag_depth_degrees_max, this->drag_idx + 1);
			this->load_overview_drag(this->dragxys, DS::SBL, over_drag_height * this->sign, this->drag_idx + 1);
		}

		this->load_label(this->labels, this->DS_side, this->caption_color, this->caption_font);
		this->load_label(this->labels, DS::Overlook, this->caption_color, this->label_font);
		this->load_label(this->labels, DS::Sidelook, this->caption_color, this->label_font);

		this->load_dimension(this->lengths, DS::TildeMark, "meter");
		this->load_dimension(this->speeds, DS::Speed, "knot");

		this->load_setting(this->settings, this->labels, DS::DesignDepth, "meter", 50.0);
		this->load_setting(this->settings, DS::TolerantDepth, "meter", 50.0);
		this->load_setting(this->settings, this->labels, DS::DivingDepth, "meter", 50.0);
		this->load_setting(this->settings, this->labels, DS::DivingCompensation, "centimeter", compensator_range * 100.0);
		this->load_setting(this->settings, this->labels, DS::LandingDepth, "meter", 50.0);
		
		if (this->DS_side == DS::PS) {
			this->load_draghead(this->dragheads, DS::PSVisor, DS::PSDP, -draghead_radius, config, default_ps_color);
			this->load_gantries(this->gantries, DredgesPosition::psTrunnion, DredgesPosition::psDragHead, -gantry_radius);
			this->load_compensator(this->compensators, DS::PSWC, gantry_radius, compensator_range);
			this->load_dimensions(this->pump_pressures, DredgesPosition::psTrunnion, DredgesPosition::psDragHead, DS::C, "bar");
			this->load_detailed_winches(this->winches, DredgesPosition::psTrunnion, DredgesPosition::psDragHead, winch_width);
			this->load_dimension(this->forces, DS::PSPF1, "knewton");
			this->load_dimension(this->forces, DS::PSPF2, "knewton");

			this->load_buttons(this->t_gantry_buttons, DredgesPosition::psTrunnion, this->button_style, gvabtn_size, gvabtn_size);
			this->load_buttons(this->i_gantry_buttons, DredgesPosition::psIntermediate, this->button_style, gvabtn_size, gvabtn_size);
			this->load_buttons(this->h_gantry_buttons, DredgesPosition::psDragHead, this->button_style, gvabtn_size, gvabtn_size);
		} else {
			this->load_draghead(this->dragheads, DS::SBVisor, DS::SBDP, +draghead_radius, config, default_sb_color);
			this->load_gantries(this->gantries, DredgesPosition::sbTrunnion, DredgesPosition::sbDragHead, +gantry_radius);
			this->load_compensator(this->compensators, DS::SBWC, gantry_radius, compensator_range);
			this->load_dimensions(this->pump_pressures, DredgesPosition::sbTrunnion, DredgesPosition::sbDragHead, DS::F, "bar");
			this->load_detailed_winches(this->winches, DredgesPosition::sbTrunnion, DredgesPosition::sbDragHead, winch_width);
			this->load_dimension(this->forces, DS::SBPF1, "knewton");
			this->load_dimension(this->forces, DS::SBPF2, "knewton");

			this->load_buttons(this->t_gantry_buttons, DredgesPosition::sbTrunnion, this->button_style, gvabtn_size, gvabtn_size);
			this->load_buttons(this->i_gantry_buttons, DredgesPosition::sbIntermediate, this->button_style, gvabtn_size, gvabtn_size);
			this->load_buttons(this->h_gantry_buttons, DredgesPosition::sbDragHead, this->button_style, gvabtn_size, gvabtn_size);
			
			this->load_button(this->drag_use_buttons, DragUseCommand::LongDrag, DragUseCommand::LongDrag);
		}
	}

	void reflow(float width, float height, float vinset) override {
		float txt_gapsize = vinset * 0.5F;
		float cx = width * 0.5F;
		float cy = height * 0.5F;
		float vgapsize;

		if (this->DS_side == DS::PS) {
			this->master->move_to(this->dragxys[DS::PS], cx, cy, GraphletAnchor::CC);
			this->master->move_to(this->dragxzes[DS::PS], this->dragxys[DS::PS], GraphletAnchor::LT, GraphletAnchor::RT, -vinset);
			this->master->move_to(this->dragheads[DS::PSVisor], this->dragxys[DS::PS], GraphletAnchor::LB, GraphletAnchor::RC, -vinset, vinset * 2.0F);

			{ // reflow gantries and winches
				auto gantry = this->gantries[DredgesPosition::psIntermediate];
				
				this->pump_pressures[DredgesPosition::psTrunnion]->fill_extent(0.0F, 0.0F, nullptr, &vgapsize);

				vgapsize = (vgapsize + vinset) * 2.0F;
				this->master->move_to(gantry, this->dragxys[DS::PS], GraphletAnchor::RC, GraphletAnchor::LC, 0.0F, -vinset);
				this->master->move_to(this->gantries[DredgesPosition::psTrunnion], gantry, GraphletAnchor::LT, GraphletAnchor::LB, 0.0F, -vgapsize);
				this->master->move_to(this->gantries[DredgesPosition::psDragHead], gantry, GraphletAnchor::LB, GraphletAnchor::LT, 0.0F, +vgapsize);
				this->reflow_winches(DS::PSWC, DredgesPosition::psTrunnion, DredgesPosition::psDragHead, GraphletAnchor::RC, GraphletAnchor::LC, vinset * 2.0F);
			}

			for (DredgesPosition id = DredgesPosition::psTrunnion; id <= DredgesPosition::psDragHead; id++) {
				this->master->move_to(this->pump_pressures[id], this->gantries[id], GraphletAnchor::RB, GraphletAnchor::CT, -vinset, txt_gapsize);
				this->master->move_to(this->position_labels[id], this->winches[id], GraphletAnchor::LC, GraphletAnchor::RB);
				this->master->move_to(this->Alets[id], this->winches[id], GraphletAnchor::RC, GraphletAnchor::LB, txt_gapsize, -1.0F);
				this->master->move_to(this->Blets[id], this->winches[id], GraphletAnchor::RC, GraphletAnchor::LT, txt_gapsize, 1.0F);
				this->master->move_to(this->buttons[id], this->Blets[id], GraphletAnchor::CB, GraphletAnchor::CT, 0.0F, 1.0F);

				this->reflow_winch_limits(id);

				if (this->rc_speeds.find(id) != this->rc_speeds.end()) {
					this->master->move_to(this->rc_speeds[id], this->winches[id], GraphletAnchor::CB, GraphletAnchor::CT);
					this->master->move_to(this->winch_lengths[id], this->rc_speeds[id], GraphletAnchor::CB, GraphletAnchor::CT);
				} else {
					this->master->move_to(this->winch_lengths[id], this->winches[id], GraphletAnchor::CB, GraphletAnchor::CT);
				}

				this->master->move_to(this->winch_speeds[id], this->winch_lengths[id], GraphletAnchor::CB, GraphletAnchor::CT);
			}
		} else {
			this->master->move_to(this->dragxys[DS::SB], cx, cy, GraphletAnchor::CC);
			this->master->move_to(this->dragxzes[DS::SB], this->dragxys[DS::SB], GraphletAnchor::RT, GraphletAnchor::LT, vinset);
			this->master->move_to(this->dragheads[DS::SBVisor], this->dragxys[DS::SB], GraphletAnchor::RB, GraphletAnchor::LC, vinset, vinset * 2.0F);

			this->master->move_to(this->dragxys[DS::SBL], this->dragxys[DS::SB], GraphletAnchor::CC, GraphletAnchor::CC);
			this->master->move_to(this->dragxzes[DS::SBL], this->dragxzes[DS::SB], GraphletAnchor::CC, GraphletAnchor::CC);
			this->master->move_to(this->drag_use_buttons[DragUseCommand::LongDrag], this->dragxys[DS::SB], GraphletAnchor::CB, GraphletAnchor::CT, 0.0F, vinset);

			{ // reflow gantries and winches
				auto gantry = this->gantries[DredgesPosition::sbIntermediate];
				
				this->pump_pressures[DredgesPosition::sbTrunnion]->fill_extent(0.0F, 0.0F, nullptr, &vgapsize);

				vgapsize = (vgapsize + vinset) * 2.0F;
				this->master->move_to(gantry, this->dragxys[DS::SB], GraphletAnchor::LC, GraphletAnchor::RC, 0.0F, -vinset);
				this->master->move_to(this->gantries[DredgesPosition::sbTrunnion], gantry, GraphletAnchor::RT, GraphletAnchor::RB, 0.0F, -vgapsize);
				this->master->move_to(this->gantries[DredgesPosition::sbDragHead], gantry, GraphletAnchor::RB, GraphletAnchor::RT, 0.0F, +vgapsize);
				this->reflow_winches(DS::SBWC, DredgesPosition::sbTrunnion, DredgesPosition::sbDragHead, GraphletAnchor::LC, GraphletAnchor::RC, vinset * -2.0F);
			}

			for (DredgesPosition id = DredgesPosition::sbTrunnion; id <= DredgesPosition::sbDragHead; id++) {
				this->master->move_to(this->pump_pressures[id], this->gantries[id], GraphletAnchor::LB, GraphletAnchor::CT, vinset, txt_gapsize);
				this->master->move_to(this->position_labels[id], this->winches[id], GraphletAnchor::RC, GraphletAnchor::LB);
				this->master->move_to(this->Alets[id], this->winches[id], GraphletAnchor::LC, GraphletAnchor::RB, -txt_gapsize, -1.0F);
				this->master->move_to(this->Blets[id], this->winches[id], GraphletAnchor::LC, GraphletAnchor::RT, -txt_gapsize, 1.0F);
				this->master->move_to(this->buttons[id], this->Blets[id], GraphletAnchor::CB, GraphletAnchor::CT, 0.0F, 1.0F);

				this->reflow_winch_limits(id);

				if (this->rc_speeds.find(id) != this->rc_speeds.end()) {
					this->master->move_to(this->rc_speeds[id], this->winches[id], GraphletAnchor::CB, GraphletAnchor::CT);
					this->master->move_to(this->winch_lengths[id], this->rc_speeds[id], GraphletAnchor::CB, GraphletAnchor::CT);
				} else {
					this->master->move_to(this->winch_lengths[id], this->winches[id], GraphletAnchor::CB, GraphletAnchor::CT);
				}

				this->master->move_to(this->winch_speeds[id], this->winch_lengths[id], GraphletAnchor::CB, GraphletAnchor::CT);
			}
		}

		{ // reflow gantry virtual action buttons
			this->reflow_gantry_button(this->t_gantry_buttons);
			this->reflow_gantry_button(this->i_gantry_buttons);
			this->reflow_gantry_button(this->h_gantry_buttons);
		}

		{ // reflow labels, dimensions and settings
			DS asetting = DS::DivingCompensation;
			float pf_xoff = vinset * 2.0F;

			this->master->move_to(this->labels[this->DS_side], cx, vinset, GraphletAnchor::CC);
			this->master->move_to(this->labels[DS::Overlook], this->dragxys[this->DS_side], GraphletAnchor::CT, GraphletAnchor::CB, 0.0, -vinset * 2.0F);
			this->master->move_to(this->labels[DS::Sidelook], this->dragxzes[this->DS_side], 0.5F, this->labels[DS::Overlook], 0.5F, GraphletAnchor::CC);

			this->master->move_to(this->lengths[DS::TildeMark], this->labels[DS::Sidelook], GraphletAnchor::CT, GraphletAnchor::RB, -vinset, -vinset);
			this->master->move_to(this->speeds[DS::Speed], this->labels[DS::Sidelook], GraphletAnchor::CT, GraphletAnchor::LB, +vinset, -vinset);

			if (this->DS_side == DS::PS) {	
				this->reflow_draghead_metrics(DS::PSVisor, DS::PSDP);
				this->master->move_to(this->labels[DS::PSWC], this->compensators[DS::PSWC], GraphletAnchor::CT, GraphletAnchor::CB);
				this->master->move_to(this->forces[DS::PSPF1], this->dragxzes[DS::PS], GraphletAnchor::LT, GraphletAnchor::LB, +pf_xoff);
				this->master->move_to(this->forces[DS::PSPF2], this->dragxzes[DS::PS], GraphletAnchor::RT, GraphletAnchor::RB, -pf_xoff);

				this->master->move_to(this->labels[DS::DesignDepth], this->dragxzes[DS::PS], GraphletAnchor::LB, GraphletAnchor::LT, vinset, vinset);
				this->master->move_to(this->settings[DS::DesignDepth], this->labels[DS::DesignDepth], GraphletAnchor::RC, GraphletAnchor::LC, vinset);
				this->master->move_to(this->settings[DS::TolerantDepth], this->settings[DS::DesignDepth], GraphletAnchor::RC, GraphletAnchor::LC, vinset);

				this->master->move_to(this->labels[asetting], this->dragxzes[DS::PS], 0.0F, this->dragheads[DS::PSVisor], 0.42F, GraphletAnchor::LC);
				this->master->move_to(this->settings[asetting], this->labels[asetting], GraphletAnchor::RC, GraphletAnchor::LC, +vinset);
			} else {
				this->reflow_draghead_metrics(DS::SBVisor, DS::SBDP); 
				this->master->move_to(this->labels[DS::SBWC], this->compensators[DS::SBWC], GraphletAnchor::CT, GraphletAnchor::CB);
				this->master->move_to(this->forces[DS::SBPF1], this->dragxzes[DS::SB], GraphletAnchor::RT, GraphletAnchor::RB, -pf_xoff);
				this->master->move_to(this->forces[DS::SBPF2], this->dragxzes[DS::SB], GraphletAnchor::LT, GraphletAnchor::LB, +pf_xoff);

				this->master->move_to(this->settings[DS::TolerantDepth], this->dragxzes[DS::SB], GraphletAnchor::RB, GraphletAnchor::RT, -vinset, vinset);
				this->master->move_to(this->settings[DS::DesignDepth], this->settings[DS::TolerantDepth], GraphletAnchor::LC, GraphletAnchor::RC, -vinset);
				this->master->move_to(this->labels[DS::DesignDepth], this->settings[DS::DesignDepth], GraphletAnchor::LC, GraphletAnchor::RC, -vinset);

				this->master->move_to(this->settings[asetting], this->dragxzes[DS::SB], 1.0F, this->dragheads[DS::SBVisor], 0.36F, GraphletAnchor::RC);
				this->master->move_to(this->labels[asetting], this->settings[asetting], GraphletAnchor::LC, GraphletAnchor::RC, -vinset);
			}

			this->master->move_to(this->settings[DS::DivingDepth], this->settings[asetting], GraphletAnchor::LT, GraphletAnchor::LB, 0.0F, -vinset);
			this->master->move_to(this->settings[DS::LandingDepth], this->settings[asetting], GraphletAnchor::LB, GraphletAnchor::LT, 0.0F, vinset);
			this->master->move_to(this->labels[DS::DivingDepth], this->labels[asetting], 0.0F, this->settings[DS::DivingDepth], 0.5F, GraphletAnchor::LC);
			this->master->move_to(this->labels[DS::LandingDepth], this->labels[asetting], 0.0F, this->settings[DS::LandingDepth], 0.5F, GraphletAnchor::LC);
		}
	}

public:
	void draw_cables(CanvasDrawingSession^ ds, float Width, float Height) {
		ICanvasBrush^ color = Colours::DarkGray;
		float thickness = 2.0F;

		if (this->DS_side == DS::PS) {
			this->draw_cable(ds, DredgesPosition::psTrunnion, GraphletAnchor::RT, GraphletAnchor::LC, color, thickness);
			this->draw_cable(ds, DredgesPosition::psIntermediate, GraphletAnchor::RT, GraphletAnchor::LC, color, thickness);
			this->draw_cable(ds, DS::PSWC, DredgesPosition::psDragHead, GraphletAnchor::RT, GraphletAnchor::LC, color, thickness);
		} else {
			this->draw_cable(ds, DredgesPosition::sbTrunnion, GraphletAnchor::LT, GraphletAnchor::RC, color, thickness);
			this->draw_cable(ds, DredgesPosition::sbIntermediate, GraphletAnchor::LT, GraphletAnchor::RC, color, thickness);
			this->draw_cable(ds, DS::SBWC, DredgesPosition::sbDragHead, GraphletAnchor::LT, GraphletAnchor::RC, color, thickness);
		}
	}

public:
	bool can_select(IGraphlet* g) override {
		auto settings = dynamic_cast<Credit<Rectanglet, DS>*>(g);
		
		return ((dynamic_cast<Winchlet*>(g) != nullptr) 
			|| (dynamic_cast<Gantrylet*>(g) != nullptr)
			|| (dynamic_cast<Compensatorlet*>(g) != nullptr)
			|| (dynamic_cast<DragHeadlet*>(g) != nullptr)
			|| button_enabled(g));
	}

	bool can_select_multiple() override {
		return true;
	}

	void on_tap_selected(IGraphlet* g, float local_x, float local_y) override {
		auto winch = dynamic_cast<Winchlet*>(g);
		auto gantry = dynamic_cast<Gantrylet*>(g);
		auto compensator = dynamic_cast<Compensatorlet*>(g);
		auto visor = dynamic_cast<DragHeadlet*>(g);
		auto override_btn = dynamic_cast<Credit<Buttonlet, DredgesPosition>*>(g);
		auto virtual_btn = dynamic_cast<GroupCredit<Buttonlet, DredgesPosition, GantryCommand>*>(g);
		auto long_btn = dynamic_cast<Credit<Buttonlet, DragUseCommand>*>(g);

		if (winch != nullptr) {
			menu_popup(this->winch_op, g, local_x, local_y);
		} else if (gantry != nullptr) {
			menu_popup(this->gantry_op, g, local_x, local_y);
		} else if (compensator != nullptr) {
			menu_popup(this->compensator_op, g, local_x, local_y);
		} else if (visor != nullptr) {
			menu_popup(this->visor_op, g, local_x, local_y);
		} else if (override_btn != nullptr) {
			this->master->get_plc_device()->send_command(DO_winch_override_command(override_btn->id));
		} else if (virtual_btn != nullptr) {
			this->master->get_plc_device()->send_command(DO_gantry_virtual_action_command(virtual_btn->gid, virtual_btn->id));
		} else if (long_btn != nullptr) {
			this->master->get_plc_device()->send_command(DO_use_long_sb_drag_command(long_btn->get_state() == ButtonState::Default));
		}
	}

	void on_gesture(std::list<float2>& anchors, float x, float y) {
		if (this->DS_side == DS::PS) {
			if (this->gantries_selected(DredgesPosition::psTrunnion, DredgesPosition::psDragHead, 2)) {
				group_menu_popup(this->ggantries_op, this->master, x, y);
			}
		} else {
			if (this->gantries_selected(DredgesPosition::sbTrunnion, DredgesPosition::sbDragHead, 2)) {
				group_menu_popup(this->ggantries_op, this->master, x, y);
			}
		}
	}

	bool on_enter_char(PLCMaster* plc) {
		auto editor = dynamic_cast<Credit<Dimensionlet, DS>*>(this->master->get_focus_graphlet());
		bool handled = false;

		if (editor != nullptr) {
			float length = float(editor->get_input_number());

			switch (editor->id) {
			case DS::DesignDepth: plc->send_setting(dredging_target_depth, length); break;
			case DS::TolerantDepth: plc->send_setting(dredging_tolerant_depth, length); break;

			case DS::DivingDepth: plc->send_setting(draghead_diving_depth, length); break;
			case DS::DivingCompensation: plc->send_setting(compensator_diving_progress, length); break;
			case DS::LandingDepth: plc->send_setting(draghead_landing_depth, length); break;
			}

			this->master->hide_virtual_keyboard();
			this->master->set_caret_owner(nullptr);

			handled = true;
		}

		return handled;
	}

private:
	template<class G, typename E>
	void load_gantries(std::map<E, Credit<G, E>*>& cs, E id0, E idn, float radius) {
		for (E id = id0; id <= idn; id++) {
			cs[id] = this->master->insert_one(new Credit<G, E>(radius), id);
		}
	}

	template<class W, typename E>
	void load_detailed_winches(std::map<E, Credit<W, E>*>& ws, E id0, E idn, float radius) {
		float button_width;

		this->load_winches(ws, id0, idn, radius);

		for (E id = id0; id <= idn; id++) {
			if (id != id0) {
				this->load_percentage(this->rc_speeds, id);
			}

			this->Alets[id]->fill_extent(0.0F, 0.0F, &button_width, nullptr);
			this->load_button(this->buttons, id, DS::Override, button_width);
		}
	}

	template<typename E, typename D>
	void load_dimensions(std::map<E, Credit<Dimensionlet, E>*>& ds, E id0, E idn, D lbl0, Platform::String^ unit) {
		Platform::String^ A = _speak("Alet");
		Platform::String^ B = _speak("Blet");
		D label = lbl0;

		for (E id = id0; id <= idn; id++, label++) {
			this->Alets[id] = this->master->insert_one(new Credit<Dimensionlet, E>(this->pump_style, unit, A), id);
			this->Blets[id] = this->master->insert_one(new Credit<Dimensionlet, E>(this->pump_style, unit, B), id);

			ds[id] = this->master->insert_one(new Credit<Dimensionlet, E>(unit, label.ToString()), id);

			ds[id]->set_style(DimensionState::Default, this->pump_style);
			ds[id]->set_style(DimensionState::Highlight, this->highlight_style);
		}
	}

	template<typename E>
	void load_setting(std::map<E, Credit<Dimensionlet, E>*>& ds, E id, Platform::String^ unit, double maximum) {
		ds[id] = this->master->insert_one(new Credit<Dimensionlet, E>(DimensionState::Input, this->setting_style, unit), id);
		ds[id]->set_maximum(maximum);
	}

	template<typename E>
	void load_setting(std::map<E, Credit<Dimensionlet, E>*>& ds, std::map<E, Credit<Labellet, E>*>& ls
		, E id, Platform::String^ unit, double maximum) {
		this->load_setting(ds, id, unit, maximum);
		this->load_label(ls, id, Colours::Silver, this->label_font);
	}

private:
	template<typename D, typename E>
	void reflow_winches(D wc, E trunnion, E draghead, GraphletAnchor ga, GraphletAnchor wa, float gapsize) {
		float compensator_width, compensator_height;
		
		this->compensators[wc]->fill_extent(0.0F, 0.0F, &compensator_width, &compensator_height);
		
		{ // align gantries and winches
			float gc_gap = gapsize * 2.0F;
			float gw_gap = gc_gap * 2.0F + compensator_width * ((gapsize > 0.0F) ? 1.0F : -1.0F);
			float gantry_joint_dy = 0.0F;

			for (E id = trunnion; id <= draghead; id++) {
				float gantry_height = 0.0F;
				
				this->gantries[id]->fill_extent(0.0F, 0.0F, nullptr, &gantry_height);
				gantry_joint_dy = this->gantries[id]->get_winch_joint_y() - gantry_height * 0.5F;
				this->master->move_to(this->winches[id], this->gantries[id], ga, wa, gw_gap, gantry_joint_dy);
			}

			{ // align the draghead gantry and the wave compensator
				float wc_joint_dy = this->compensators[wc]->get_cable_joint_y() - compensator_height * 0.5F;
				
				this->master->move_to(this->compensators[wc], this->gantries[draghead], ga, wa, gc_gap, gantry_joint_dy - wc_joint_dy);
				this->master->move_to(this->lengths[wc], this->compensators[wc], GraphletAnchor::CB, GraphletAnchor::CT);
				this->master->move_to(this->pressures[wc], this->lengths[wc], GraphletAnchor::CB, GraphletAnchor::CT);
			}
		}
	}

	template<class B, typename E, typename CMD>
	void reflow_gantry_button(std::map<CMD, GroupCredit<B, E, CMD>*>& btns) {
		auto up_btn = btns[GantryCommand::VirtualUp];
		auto out_btn = btns[GantryCommand::VirtualOut];
		auto gantry = this->gantries[up_btn->gid];
		float offset = 4.0F;
		float fy = 0.0F;

		gantry->fill_extent(0.0F, 0.0F, nullptr, &fy);
		fy = gantry->get_winch_joint_y() / fy;
		
		if (this->DS_side == DS::PS) {
			this->master->move_to(up_btn, gantry, 1.0F, fy, GraphletAnchor::LB, offset, -offset);
			this->master->move_to(out_btn, gantry, 1.0F, fy, GraphletAnchor::LT, offset, offset);
		} else {
			this->master->move_to(up_btn, gantry, 0.0F, fy, GraphletAnchor::RB, -offset, -offset);
			this->master->move_to(out_btn, gantry, 0.0F, fy, GraphletAnchor::RT, -offset, offset);
		}
	}

private:
	template<typename E>
	void draw_cable(CanvasDrawingSession^ ds, E id, GraphletAnchor ga, GraphletAnchor wa, ICanvasBrush^ color, float thickness) {
		float gantry_x, gantry_y, winch_x, winch_y;
		float gantry_joint = this->gantries[id]->get_winch_joint_y();

		this->master->fill_graphlet_location(this->gantries[id], &gantry_x, &gantry_y, ga);
		this->master->fill_graphlet_location(this->winches[id], &winch_x, &winch_y, wa);

		ds->DrawLine(gantry_x, gantry_y + gantry_joint, winch_x, winch_y, color, thickness);
	}

	template<typename D, typename E>
	void draw_cable(CanvasDrawingSession^ ds, D wc, E id, GraphletAnchor ga, GraphletAnchor wa, ICanvasBrush^ color, float thickness) {
		float gantry_x, gantry_y, winch_x, winch_y, wc_gx, wc_wx, wc_y;
		float gantry_joint = this->gantries[id]->get_winch_joint_y();
		float wc_joint = this->compensators[wc]->get_cable_joint_y();
		float wc_adjust = this->compensators[wc]->get_cable_joint_size();
		float sign = ((wc == DS::PSWC) ? 1.0F : -1.0F);

		this->master->fill_graphlet_location(this->gantries[id], &gantry_x, &gantry_y, ga);
		this->master->fill_graphlet_location(this->winches[id], &winch_x, &winch_y, wa);
		this->master->fill_graphlet_location(this->compensators[wc], &wc_wx, &wc_y, ga);
		this->master->fill_graphlet_location(this->compensators[wc], &wc_gx, nullptr, wa);

		ds->DrawLine(gantry_x, gantry_y + gantry_joint, wc_gx + wc_adjust * sign, wc_y + wc_joint, color, thickness);
		ds->DrawLine(wc_wx - wc_adjust * sign, wc_y + wc_joint, winch_x, winch_y, color, thickness);
	}

private:
	bool gantries_selected(DredgesPosition id0, DredgesPosition idn, int tolerance) {
		bool okay = false;
		int ok = 0;

		for (DredgesPosition id = id0; id <= idn; id++) {
			if (this->master->is_selected(this->gantries[id])) {
				ok += 1;

				if (ok >= tolerance) {
					okay = true;
					break;
				}
			}
		}

		return okay;
	}

private: // never delete these graphlets manually.
	std::map<DredgesPosition, Credit<Gantrylet, DredgesPosition>*> gantries;
	std::map<DredgesPosition, Credit<Dimensionlet, DredgesPosition>*> pump_pressures;
	std::map<DredgesPosition, Credit<Dimensionlet, DredgesPosition>*> Alets;
	std::map<DredgesPosition, Credit<Dimensionlet, DredgesPosition>*> Blets;
	std::map<DredgesPosition, Credit<Percentagelet, DredgesPosition>*> rc_speeds;
	std::map<DredgesPosition, Credit<Buttonlet, DredgesPosition>*> buttons;
	std::map<DragUseCommand, Credit<Buttonlet, DragUseCommand>*> drag_use_buttons;
	std::map<GantryCommand, GroupCredit<Buttonlet, DredgesPosition, GantryCommand>*> t_gantry_buttons;
	std::map<GantryCommand, GroupCredit<Buttonlet, DredgesPosition, GantryCommand>*> i_gantry_buttons;
	std::map<GantryCommand, GroupCredit<Buttonlet, DredgesPosition, GantryCommand>*> h_gantry_buttons;
	std::map<DS, Credit<Dimensionlet, DS>*> settings;

private:
	DimensionStyle pump_style;
	DimensionStyle highlight_style;
	DimensionStyle setting_style;
	ButtonStyle button_style;

private:
	MenuFlyout^ ggantries_op;
	MenuFlyout^ gantry_op;
	MenuFlyout^ winch_op;
	MenuFlyout^ compensator_op;
	MenuFlyout^ visor_op;

private:
	DS DS_side;
	DredgeAddress* address; // global object
	unsigned int drag_color;
	unsigned int drag_idx;
	float sign;
};

private class DragCableDecorator : public IPlanetDecorator {
public:
	DragCableDecorator(IDredgingSystem* master) : master(master) {}

public:
	void draw_before(CanvasDrawingSession^ ds, float Width, float Height) override {
		this->master->draw_cables(ds, Width, Height);
	}

private:
	IDredgingSystem* master;
};

/*************************************************************************************************/
private class Suctions final : public IDredgingSystem {
public:
	Suctions(DredgesPage* master) : IDredgingSystem(master) {
		this->ps_address = make_ps_dredging_system_schema();
		this->sb_address = make_sb_dredging_system_schema();
	}

public:
	void on_analog_input(long long timepoint_ms, const uint8* DB2, size_t count2, const uint8* DB203, size_t count203, Syslog* logger) override {
		this->set_drag_metrics(DS::PS, DS::PSVisor, DB2, DB203, this->drag_configs[0], this->ps_address);
		this->set_drag_metrics(DS::SB, DS::SBVisor, DB2, DB203, this->drag_configs[1], this->sb_address);
		this->set_drag_metrics(DS::SBL, DS::SBVisor, DB2, DB203, this->drag_configs[2], this->sb_address);
	}

	void on_digital_input(long long timepoint_ms, const uint8* DB4, size_t count4, const uint8* DB205, size_t count205, Syslog* logger) override {
		select_sb_drag(DB205);

		DI_winch(this->winches[DredgesPosition::psTrunnion], DB4, winch_ps_trunnion_feedback, winch_ps_trunnion_limits, DB205, winch_ps_trunnion_details);
		DI_winch(this->winches[DredgesPosition::psIntermediate], DB4, winch_ps_intermediate_feedback, winch_ps_intermediate_limits, DB205, winch_ps_intermediate_details);
		DI_winch(this->winches[DredgesPosition::psDragHead], DB4, winch_ps_draghead_feedback, winch_ps_draghead_limits, DB205, winch_ps_draghead_details);

		DI_winch(this->winches[DredgesPosition::sbTrunnion], DB4, winch_sb_trunnion_feedback, winch_sb_trunnion_limits, DB205, winch_sb_trunnion_details);
		DI_winch(this->winches[DredgesPosition::sbIntermediate], DB4, winch_sb_intermediate_feedback, winch_sb_intermediate_limits, DB205, winch_sb_intermediate_details);
		DI_winch(this->winches[DredgesPosition::sbDragHead], DB4, winch_sb_draghead_feedback, winch_sb_draghead_limits, DB205, winch_sb_draghead_details);
	}

	void on_forat(long long timepoint_ms, const uint8* DB20, size_t count, Syslog* logger) override {
		this->set_design_depth(DS::PS, DB20, dredging_target_depth, dredging_tolerant_depth);
		this->set_design_depth(DS::SB, DB20, dredging_target_depth, dredging_tolerant_depth);
		this->set_design_depth(DS::SBL, DB20, dredging_target_depth, dredging_tolerant_depth);
	}

public:
	void load(float width, float height, float vinset) override {
		{ // load dimensions
			float winch_width = width * 0.314F * 0.12F;
			
			this->load_winches(this->winches, DredgesPosition::psTrunnion, DredgesPosition::psDragHead, winch_width);
			this->load_winches(this->winches, DredgesPosition::sbTrunnion, DredgesPosition::sbDragHead, winch_width);
		}

		{ // load drags
			float back_drag_height = height * 0.64F;
			float side_drag_width = width * 0.25F;
			float draghead_radius = side_drag_width * 0.2718F;

			this->load_draghead(this->dragheads, DS::PSVisor, DS::PSDP, -draghead_radius, this->drag_configs[0], default_ps_color);
			this->load_draghead(this->dragheads, DS::SBVisor, DS::SBDP, +draghead_radius, this->drag_configs[1], default_sb_color);

			this->load_backview_drag(this->dragyzes, DS::PS, -back_drag_height, drag_depth_degrees_max, 0);
			this->load_backview_drag(this->dragyzes, DS::SB, +back_drag_height, drag_depth_degrees_max, 1);
			this->load_backview_drag(this->dragyzes, DS::SBL, +back_drag_height, drag_depth_degrees_max, 2);

			this->load_sideview_drag(this->dragxzes, DS::PS, -side_drag_width, drag_depth_degrees_max, 0);
			this->load_sideview_drag(this->dragxzes, DS::SB, +side_drag_width, drag_depth_degrees_max, 1);
			this->load_sideview_drag(this->dragxzes, DS::SBL, +side_drag_width, drag_depth_degrees_max, 2);
		}
	}

	void reflow(float width, float height, float vinset) override {
		float cx = width * 0.5F;
		float inset = vinset;

		{ // reflow centeral components
			this->master->move_to(this->dragheads[DS::PSVisor], cx, height * 0.85F, GraphletAnchor::RC, -vinset);
			this->master->move_to(this->dragheads[DS::SBVisor], cx, height * 0.85F, GraphletAnchor::LC, +vinset);
		}

		{ // reflow left dredging system
			float dhx;

			this->master->fill_graphlet_location(this->dragheads[DS::PSVisor], &dhx, nullptr, GraphletAnchor::LC);

			this->master->move_to(this->dragyzes[DS::PS], 0.0F, height * 0.5F, GraphletAnchor::LC);
			this->master->move_to(this->dragxzes[DS::PS], this->dragyzes[DS::PS], GraphletAnchor::RT, GraphletAnchor::LT, +vinset);

			this->master->move_to(this->winches[DredgesPosition::psTrunnion], this->dragxzes[DS::PS], GraphletAnchor::RT, GraphletAnchor::RB, -inset);
			this->master->move_to(this->winches[DredgesPosition::psIntermediate], this->dragxzes[DS::PS], GraphletAnchor::CT, GraphletAnchor::CB);
			this->master->move_to(this->winches[DredgesPosition::psDragHead], this->dragxzes[DS::PS], GraphletAnchor::LT, GraphletAnchor::LB, +inset);
		}

		{ // reflow right dredging system
			float dhrx;

			this->master->fill_graphlet_location(this->dragheads[DS::SBVisor], &dhrx, nullptr, GraphletAnchor::RC);

			this->master->move_to(this->dragyzes[DS::SB], width, height * 0.5F, GraphletAnchor::RC);
			this->master->move_to(this->dragxzes[DS::SB], this->dragyzes[DS::SB], GraphletAnchor::LT, GraphletAnchor::RT, -vinset);

			this->master->move_to(this->dragyzes[DS::SBL], this->dragyzes[DS::SB], GraphletAnchor::CC, GraphletAnchor::CC);
			this->master->move_to(this->dragxzes[DS::SBL], this->dragxzes[DS::SB], GraphletAnchor::CC, GraphletAnchor::CC);

			this->master->move_to(this->winches[DredgesPosition::sbTrunnion], this->dragxzes[DS::SB], GraphletAnchor::LT, GraphletAnchor::LB, +inset);
			this->master->move_to(this->winches[DredgesPosition::sbIntermediate], this->dragxzes[DS::SB], GraphletAnchor::CT, GraphletAnchor::CB);
			this->master->move_to(this->winches[DredgesPosition::sbDragHead], this->dragxzes[DS::SB], GraphletAnchor::RT, GraphletAnchor::RB, -inset);
		}

		{ // reflow dimensions and labels
			this->reflow_draghead_metrics(DS::PSVisor, DS::PSDP);
			this->reflow_draghead_metrics(DS::SBVisor, DS::SBDP);
		}
	}

private: // never delete these graphlets manually.
	
private: // never delete these global objects
	DredgeAddress* ps_address;
	DredgeAddress* sb_address;
};


/*************************************************************************************************/
DredgesPage::DredgesPage(DragView type, PLCMaster* plc)
	: Planet(__MODULE__ + ((type == DragView::_) ? "" : "_" + type.ToString())), device(plc) {
	IDredgingSystem* dashboard = nullptr;
	
	switch (type) {
	case DragView::PortSide: dashboard = new Drags(this, DS::PS, default_ps_color, 0); break;
	case DragView::Starboard: dashboard = new Drags(this, DS::SB, default_sb_color, 1); break;
	case DragView::Suctions: dashboard = new Suctions(this); break;
	default: dashboard = new Dredges(this); break;
	}

	this->dashboard = dashboard;

	if (this->device != nullptr) {
		this->device->push_confirmation_receiver(dashboard);
	}

	this->push_decorator(new DragCableDecorator(dashboard));
}

DredgesPage::~DredgesPage() {
	if (this->dashboard != nullptr) {
		delete this->dashboard;
	}
}

void DredgesPage::load(CanvasCreateResourcesReason reason, float width, float height) {
	auto db = dynamic_cast<IDredgingSystem*>(this->dashboard);
	
	if (db != nullptr) {
		db->load(width, height, statusbar_height());
	}
}

void DredgesPage::reflow(float width, float height) {
	auto db = dynamic_cast<IDredgingSystem*>(this->dashboard);
	
	if (db != nullptr) {
		db->reflow(width, height, statusbar_height());
	}
}

void DredgesPage::on_timestream(long long timepoint_ms, size_t addr0, size_t addrn, uint8* data, size_t size, Syslog* logger) {
	auto db = dynamic_cast<IDredgingSystem*>(this->dashboard);

	if (db != nullptr) {
		db->on_all_signals(timepoint_ms, addr0, addrn, data, size, logger);
	}
}

PLCMaster* DredgesPage::get_plc_device() {
	return this->device;
}

bool DredgesPage::can_select(IGraphlet* g) {
	auto db = dynamic_cast<IDredgingSystem*>(this->dashboard);

	return ((this->device != nullptr) && (db != nullptr) && (db->can_select(g)));
}

bool DredgesPage::can_select_multiple() {
	auto db = dynamic_cast<IDredgingSystem*>(this->dashboard);

	return ((this->device != nullptr) && (db != nullptr) && (db->can_select_multiple()));
}

bool DredgesPage::on_key(VirtualKey key, bool wargrey_keyboard) {
	bool handled = Planet::on_key(key, wargrey_keyboard);

	if ((this->device != nullptr) && (!handled)) {
		auto db = dynamic_cast<IDredgingSystem*>(this->dashboard);

		if (db != nullptr) {
			if (key == VirtualKey::Enter) {
				handled = db->on_enter_char(this->device);
			}
		}
	}

	return handled;
}

void DredgesPage::on_focus(IGraphlet* g, bool yes) {
	if (yes) {
		auto editor = dynamic_cast<IEditorlet*>(g);

		if (editor != nullptr) {
			if (this->device != nullptr) {
				this->show_virtual_keyboard(ScreenKeyboard::Numpad);
			} else {
				this->set_caret_owner(nullptr);
			}
		}
	}
}

void DredgesPage::on_tap_selected(IGraphlet* g, float local_x, float local_y) {
	auto db = dynamic_cast<IDredgingSystem*>(this->dashboard);

	if (db != nullptr) {
		db->on_tap_selected(g, local_x, local_y);
	}
}

void DredgesPage::on_gesture(std::list<float2>& anchors, float x, float y) {
	auto db = dynamic_cast<IDredgingSystem*>(this->dashboard);

	if (db != nullptr) {
		db->on_gesture(anchors, x, y);
	}
}
