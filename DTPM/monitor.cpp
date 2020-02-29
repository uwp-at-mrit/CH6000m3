#include "monitor.hpp"
#include "drag_info.hpp"
#include "moxa.hpp"
#include "module.hpp"
#include "configuration.hpp"

#include "frame/times.hpp"
#include "frame/drags.hpp"
#include "frame/statusbar.hpp"

#include "metrics/dredger.hpp"

#include "graphlet/planetlet.hpp"
#include "graphlet/filesystem/s63let.hpp"
#include "graphlet/filesystem/configuration/gpslet.hpp"
#include "graphlet/filesystem/configuration/vessel/trailing_suction_dredgerlet.hpp"

#include "schema/datalet/track_ds.hpp"

#include "iotables/ai_dredges.hpp"

#include "cs/wgs_xy.hpp"

using namespace WarGrey::SCADA;
using namespace WarGrey::DTPM;

using namespace Windows::Foundation::Numerics;

using namespace Microsoft::Graphics::Canvas::UI;
using namespace Microsoft::Graphics::Canvas::Brushes;

/*************************************************************************************************/
DTPMonitor::DTPMonitor(MRMaster* plc) : Planet(__MODULE__), plc(plc), track_source(nullptr) {
	Syslog* logger = make_system_logger(default_schema_logging_level, "DredgeTrackHistory");

	this->track_source = new TrackDataSource(logger, RotationPeriod::Daily);
	this->track_source->reference();

	if (this->plc != nullptr) {
		this->plc->push_confirmation_receiver(this);
	}

	this->gps1 = moxa_tcp_as_gps(MOXA_TCP::MRIT_DGPS, this);
	this->gps2 = moxa_tcp_as_gps(MOXA_TCP::DP_DGPS, this);
	this->gyro = moxa_tcp_as_gps(MOXA_TCP::GYRO, this);
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

	DredgeMetrics* dredge_metrics = new DredgeMetrics(this->plc);
	StatusFrame* status = new StatusFrame(this->plc);
	DragsFrame* drags = new DragsFrame(this->plc);
	ColorPlotlet* plot = new ColorPlotlet("colorplot", plot_width, plot_height);
	S63let* enchart = nullptr;//new S63let("20170817", map_width, plot_height);
	GPSlet* gps = new GPSlet("gps", 64.0F);

	this->vessel = new TrailingSuctionDredgerlet("vessel", 1.0F);
	this->track = new DredgeTracklet(this->track_source, "track", map_width, plot_height);
	
	this->metrics = this->insert_one(new Metricslet(dredge_metrics, "main", side_zone_width, GraphletAnchor::RT));
	this->times = this->insert_one(new Metricslet(dredge_metrics, "worktime", side_zone_width, GraphletAnchor::RT, 3U));
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
		this->gcs = this->gps->clone_gpscs(this->gcs);
	}
}

void DTPMonitor::on_location_changed(double latitude, double longitude, double altitude, double geo_x, double geo_y) {
	if (this->project != nullptr) {
		this->begin_update_sequence();

		if (this->project->move_vessel(geo_x, geo_y)) {
			this->profile->update_outline(this->project->section(geo_x, geo_y), geo_x, geo_y);
		}

		if (this->gps != nullptr) {
			this->gps->set_position(latitude, longitude);
		}

		if (this->track != nullptr) {
			this->track->push_track_dot(DredgeTrackType::GPS, double3(geo_x, geo_y, 0.0));
		}

		this->end_update_sequence();
	}
}

/*************************************************************************************************/
void DTPMonitor::pre_scan_data(int id, Syslog* logger) {
	this->begin_update_sequence();
}

void DTPMonitor::on_GGA(int id, long long timepoint_ms, GGA* gga, Syslog* logger) {
	if (this->gcs != nullptr) {
		double2 location = GPS_to_XY(gga->latitude, gga->longitude, gga->altitude, this->gcs->parameter);

		this->on_location_changed(gga->latitude, gga->longitude, gga->altitude, location.x, location.y);
	}
}

void DTPMonitor::on_VTG(int id, long long timepoint_ms, VTG* vtg, Syslog* logger) {
	if (this->gps != nullptr) {
		this->gps->set_speed(vtg->s_kn);
	}
}

void DTPMonitor::on_GLL(int id, long long timepoint_ms, GLL* gll, Syslog* logger) {
	logger->log_message(Log::Info, L"GLL: [%f]: (%lf, %lf), %s, %s", gll->utc,
		gll->latitude, gll->longitude,
		gll->validity.ToString()->Data(),
		gll->mode.ToString()->Data());
}

void DTPMonitor::on_GSA(int id, long long timepoint_ms, GSA* gsa, Syslog* logger) {
	//logger->log_message(Log::Info, L"GSA: %s: (%lf, %lf, %lf), %s, [%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d]",
		//gsa->type.ToString()->Data(), gsa->pdop, gsa->hdop, gsa->vdop, gsa->auto_selection.ToString()->Data(),
		//gsa->PRNs[0], gsa->PRNs[1], gsa->PRNs[2], gsa->PRNs[3], gsa->PRNs[4], gsa->PRNs[5],
		//gsa->PRNs[6], gsa->PRNs[7], gsa->PRNs[8], gsa->PRNs[9], gsa->PRNs[10], gsa->PRNs[11]);
}

void DTPMonitor::on_GSV(int id, long long timepoint_ms, GSV* gsv, Syslog* logger) {
	//logger->log_message(Log::Info, L"GSV: %d-%d of %d: (%d, %d, %d, %d), (%d, %d, %d, %d), (%d, %d, %d, %d), (%d, %d, %d, %d)",
		//gsv->sequence0, gsv->sequence0 + 4, gsv->total,
		//gsv->PRNs[0], gsv->elevations[0], gsv->azimuthes[0], gsv->SNRs[0],
		//gsv->PRNs[1], gsv->elevations[1], gsv->azimuthes[1], gsv->SNRs[1],
		//gsv->PRNs[2], gsv->elevations[2], gsv->azimuthes[2], gsv->SNRs[2],
		//gsv->PRNs[3], gsv->elevations[3], gsv->azimuthes[3], gsv->SNRs[3]);
}

void DTPMonitor::on_ZDA(int id, long long timepoint_ms, ZDA* zda, Syslog* logger) {
	//logger->log_message(Log::Info, L"ZDA: [%f]: %04d-%02d-%02d, +(%d, %d)", zda->utc,
		//zda->year, zda->month, zda->day,
		//zda->local_hour_offset, zda->local_minute_offset);
}

void DTPMonitor::on_HDT(int id, long long timepoint_ms, HDT* hdt, Syslog* logger) {
	bool valid = true;
	bool gyro_okay = ((this->gyro != nullptr) && this->gyro->connected());

	if (gyro_okay) {
		if (this->gyro->device_identity() != id) {
			valid = false;
		}
	}

	if (valid) {
		if (this->vessel != nullptr) {
			double compensator_deg = 0.0;

			//if (this->gyro->device_identity() != id) {
			//	if (hdt->heading_deg > 180.0) {
			//		compensator_deg = -180.0;
			//	} else {
			//		compensator_deg = 180.0;
			//	}
			//}

			this->vessel->set_bow_direction(hdt->heading_deg + compensator_deg);
		}
	}
}

void DTPMonitor::post_scan_data(int id, Syslog* logger) {
	this->end_update_sequence();
}

bool DTPMonitor::available(int id) {
	return ((id == this->gyro->device_identity())
		|| (id == this->gps1->device_identity())
		|| (!this->gps1->connected()));
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
		this->track->push_track_dot(DredgeTrackType::PSDrag, draghead);
	}

	read_drag_figures(DB2, &offset, ujoints, &draghead, sb_addr->drag_position);
	
	if (this->track) {
		this->vessel->set_sb_drag_figures(offset, ujoints, draghead);
		this->vessel->fill_sb_track_position(&draghead, vessel_pos);
		this->track->push_track_dot(DredgeTrackType::SBDrag, draghead);
	}
}

void DTPMonitor::post_read_data(Syslog* logger) {
	this->end_update_sequence();
	this->leave_critical_section();
}
