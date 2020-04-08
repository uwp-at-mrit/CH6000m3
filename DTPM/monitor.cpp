#include "monitor.hpp"
#include "drag_info.hpp"
#include "moxa.hpp"
#include "module.hpp"
#include "configuration.hpp"

#include "frame/drags.hpp"
#include "frame/statusbar.hpp"

#include "metrics/dredge.hpp"
#include "metrics/times.hpp"

#include "graphlet/planetlet.hpp"
#include "graphlet/filesystem/s63let.hpp"
#include "graphlet/filesystem/configuration/gpslet.hpp"
#include "graphlet/filesystem/configuration/vessel/trailing_suction_dredgerlet.hpp"

#include "schema/datalet/track_ds.hpp"

#include "iotables/ai_dredges.hpp"

#include "cs/wgs_xy.hpp"

using namespace WarGrey::SCADA;
using namespace WarGrey::DTPM;
using namespace WarGrey::GYDM;

using namespace Windows::Foundation::Numerics;

using namespace Microsoft::Graphics::Canvas::UI;
using namespace Microsoft::Graphics::Canvas::Brushes;

/*************************************************************************************************/
DTPMonitor::DTPMonitor(Compass* compass, AIS* ais, MRMaster* plc) : Planet(__MODULE__), compass(compass), ais(ais), plc(plc), track_source(nullptr) {
	Syslog* logger = make_system_logger(default_schema_logging_level, "DredgeTrackHistory");

	this->track_source = new TrackDataSource(logger, RotationPeriod::Daily);
	this->track_source->reference();

	if (this->compass != nullptr) {
		this->compass->push_receiver(this);
	}

	if (this->ais != nullptr) {
		this->ais->push_receiver(this);
	}

	if (this->plc != nullptr) {
		this->plc->push_confirmation_receiver(this);
	}

	this->memory = global_resident_metrics();
}

DTPMonitor::~DTPMonitor() {
	if (this->track_source != nullptr) {
		this->track_source->destroy();
	}
}

void DTPMonitor::load(CanvasCreateResourcesReason reason, float width, float height) {
	float side_zone_width = width * 0.1618F;
	float status_height = 28.0F;
	float plot_height = height * 0.8F;
	float plot_width = plot_height / float(ColorPlotSize);
	float map_width = width - side_zone_width - plot_width;
	float profile_width = map_width + plot_width;
	float profile_height = height - plot_height - status_height;

	StatusFrame* status = new StatusFrame(this->plc);
	DragsFrame* drags = new DragsFrame(this->plc);
	ColorPlotlet* plot = new ColorPlotlet("colorplot", plot_width, plot_height);
	S63let* enchart = nullptr;//new S63let("20170817", map_width, plot_height);
	GPSlet* gps = new GPSlet("gps", 64.0F);

	this->vessel = new TrailingSuctionDredgerlet("vessel", 1.0F);
	this->track = new DredgeTracklet(this->track_source, "track", map_width, plot_height);
	
	this->metrics = this->insert_one(new Metricslet(new DredgeMetrics(this->compass, this->plc), "main", side_zone_width, GraphletAnchor::RT, 20U));
	this->times = this->insert_one(new Metricslet(new TimeMetrics(this->plc), "worktime", side_zone_width, GraphletAnchor::RT, 3U));
	this->status = this->insert_one(new Planetlet(status, width, status_height));
	this->drags = this->insert_one(new Planetlet(drags, side_zone_width, 0.0F));
	this->project = this->insert_one(new Projectlet(this->vessel, this->track, plot, L"长江口工程", map_width, plot_height));
	this->profile = this->insert_one(new Profilet(this->vessel, "profile", profile_width, profile_height));
	this->gps = this->insert_one(gps);
	this->plot = this->insert_one(plot);

	this->drags->set_stretch_anchor(GraphletAnchor::RB);
	this->gps->camouflage(false);

	{ // Set initial drags
		DragInfo ps, sb;

		// NOTE: long drags are just the extention of short ones, they therefore are not considered as the initial drags

		fill_ps_drag_info(&ps);
		fill_sb_drag_info(&sb);
		this->vessel->set_ps_drag_info(ps, 2U);
		this->vessel->set_sb_drag_info(sb, 2U);
	}
}

void DTPMonitor::reflow(float width, float height) {
	this->move_to(this->metrics, width, 0.0F, GraphletAnchor::RT);
	this->move_to(this->times, this->metrics, GraphletAnchor::CB, GraphletAnchor::CT);
	this->move_to(this->status, 0.0F, height, GraphletAnchor::LB);
	this->move_to(this->drags, this->status, GraphletAnchor::RT, GraphletAnchor::RB);
	this->move_to(this->plot, 0.0F, 0.0F, GraphletAnchor::LT);
	this->move_to(this->project, this->plot, GraphletAnchor::RT, GraphletAnchor::LT);
	this->move_to(this->gps, this->project, GraphletAnchor::LT, GraphletAnchor::LT);
	this->move_to(this->profile, this->status, GraphletAnchor::LT, GraphletAnchor::LB);

	{ // adjust drags
		float drags_y, times_bottom;

		this->fill_graphlet_location(this->drags, nullptr, &drags_y, GraphletAnchor::CT);
		this->fill_graphlet_location(this->times, nullptr, &times_bottom, GraphletAnchor::CB);

		if (drags_y < times_bottom) {
			this->move_to(this->drags, this->times, GraphletAnchor::CB, GraphletAnchor::CT);
		}
	}
}

void DTPMonitor::update(long long count, long long interval, long long uptime) {
}

void DTPMonitor::on_message(long long timepoint_ms, Platform::String^ remote_peer, uint16 port, MetricsBlock type, const uint8* message, Syslog* logger) {
}

void DTPMonitor::on_gps_message(long long timepoint_ms, DGPS& dgps) {

}

IGraphlet* DTPMonitor::thumbnail_graphlet() {
	return this->project;
}

bool DTPMonitor::can_select(IGraphlet* g) {
	return (g == this->gps);
}

void DTPMonitor::on_tap_selected(IGraphlet* g, float local_x, float local_y) {
	if (this->gps == g) {
		if (this->project != nullptr) {
			this->project->center_vessel();
		}
	}
}

bool DTPMonitor::in_affine_gesture_zone(float2& lt, float2& rb) {
	return ((this->project != nullptr)
		&& (this->project->ready())
		&& this->project->contain_region(lt, rb));
}

void DTPMonitor::on_translation_gesture(float deltaX, float deltaY, float2& lt, float2& rb) {
	this->project->translate(deltaX, deltaY);
}

void DTPMonitor::on_zoom_gesture(float zx, float zy, float deltaScale, float2& lt, float2& rb) {
	float px, py;

	this->fill_graphlet_location(this->project, &px, &py);
	this->project->zoom(zx - px, zy - py, deltaScale);
}

void DTPMonitor::on_graphlet_ready(IGraphlet* g) {
	if (this->gps == g) {
		this->compass->set_gps_convertion_matrix(this->gps->clone_gpscs());
	}
}

/*************************************************************************************************/
void DTPMonitor::pre_move(Syslog* logger) {
	this->begin_update_sequence();
}

void DTPMonitor::on_location(long long timepoint_ms, double latitude, double longitude, double altitude, double geo_x, double geo_y, Syslog* logger) {
	if (this->project != nullptr) {
		if (this->project->move_vessel(geo_x, geo_y)) {
			this->profile->update_outline(this->project->section(geo_x, geo_y), geo_x, geo_y);
		}

		if (this->gps != nullptr) {
			this->gps->set_position(latitude, longitude);
		}

		if (this->track != nullptr) {
			// Note: The visibility of GPS track does not controlled by depth0, so just choose an impossible deep depth here.
			this->track->filter_dredging_dot(DredgeTrackType::GPS, double3(geo_x, geo_y, 0.0));
		}
	}
}

void DTPMonitor::on_sail(long long timepoint_ms, double s_kn, double track_deg, Syslog* logger) {
	if (this->gps != nullptr) {
		this->memory->gps.set(GP::Speed, s_kn);
		this->gps->set_speed(s_kn);
	}
}

void DTPMonitor::on_heading(long long timepoint_ms, double deg, Syslog* logger) {
	if (this->vessel != nullptr) {
		this->memory->gps.set(GP::Heading, deg);
		this->vessel->set_bow_direction(deg);
	}
}

void DTPMonitor::post_move(Syslog* logger) {
	this->end_update_sequence();
}

/*************************************************************************************************/
void DTPMonitor::pre_interpret_payload(int id, Syslog* logger) {
	this->begin_update_sequence();
}

void DTPMonitor::on_PRCA(int id, long long timepoint_ms, bool self, PRCA* prca, Syslog* logger) {
	logger->log_message(Log::Info, L"PRCA: (%f, %f)", prca->longitude.unbox(), prca->latitude.unbox());
}

void DTPMonitor::post_interpret_payload(int id, Syslog* logger) {
	this->end_update_sequence();
}

/*************************************************************************************************/
void DTPMonitor::pre_read_data(Syslog* logger) {
	this->enter_critical_section();
	this->begin_update_sequence();
}

void DTPMonitor::on_analog_input(long long timepoint_ms, const uint8* DB2, size_t count2, const uint8* DB203, size_t count203, Syslog* logger) {
	double3 offset, draghead, ujoints[DRAG_SEGMENT_MAX_COUNT];
	DredgeAddress* ps_addr = make_ps_dredging_system_schema();
	DredgeAddress* sb_addr = make_sb_dredging_system_schema();
	size_t count = sizeof(ujoints) / sizeof(double3);
	double2 vessel_pos = this->project->vessel_position();

	read_drag_figures(DB2, &offset, ujoints, &draghead, ps_addr->drag_position);

	if (this->track != nullptr) {
		this->vessel->set_ps_drag_figures(offset, ujoints, draghead);
		this->vessel->fill_ps_track_position(&draghead, vessel_pos);
		this->track->filter_dredging_dot(DredgeTrackType::PSDrag, draghead);
	}

	read_drag_figures(DB2, &offset, ujoints, &draghead, sb_addr->drag_position);
	
	if (this->track) {
		this->vessel->set_sb_drag_figures(offset, ujoints, draghead);
		this->vessel->fill_sb_track_position(&draghead, vessel_pos);
		this->track->filter_dredging_dot(DredgeTrackType::SBDrag, draghead);
	}
}

void DTPMonitor::post_read_data(Syslog* logger) {
	this->end_update_sequence();
	this->leave_critical_section();
}
