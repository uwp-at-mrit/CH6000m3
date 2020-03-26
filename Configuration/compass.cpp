#include "compass.hpp"
#include "moxa.hpp"

using namespace WarGrey::SCADA;
using namespace WarGrey::DTPM;
using namespace WarGrey::GYDM;

#define ON_MOVE(receivers, on_move, logger, ...) do { \
for (auto r : receivers) { \
if (r->moveable()) { \
r->pre_move(logger); \
r->on_move(__VA_ARGS__, logger); \
r->post_move(logger); \
} \
} \
} while (0)

/*************************************************************************************************/
Compass::Compass() : gcs(nullptr) {
	this->gps1 = moxa_tcp_as_gps(MOXA_TCP::MRIT_DGPS, this);
	this->gps2 = moxa_tcp_as_gps(MOXA_TCP::DP_DGPS, this);
	this->gyro = moxa_tcp_as_gps(MOXA_TCP::GYRO, this);
}

void Compass::set_gps_convertion_matrix(GPSCS^ gcs) {
	this->gcs = gcs;
}

void Compass::push_receiver(ICompassReceiver* r) {
	if (r != nullptr) {
		this->receivers.push_back(r);
	}
}

void Compass::on_GGA(int id, long long timepoint_ms, GGA* gga, Syslog* logger) {
	if (this->gcs != nullptr) {
		double2 location = GPS_to_XY(gga->latitude, gga->longitude, gga->altitude, this->gcs->parameter);

		ON_MOVE(this->receivers, on_location, logger, timepoint_ms, gga->latitude, gga->longitude, gga->altitude, location.x, location.y);
	}
}

void Compass::on_VTG(int id, long long timepoint_ms, VTG* vtg, Syslog* logger) {
	ON_MOVE(this->receivers, on_sail, logger, timepoint_ms, vtg->s_kn, vtg->track_deg);
}

void Compass::on_HDT(int id, long long timepoint_ms, HDT* hdt, Syslog* logger) {
	bool valid = true;
	bool gyro_okay = ((this->gyro != nullptr) && this->gyro->connected());

	if (gyro_okay) {
		if (this->gyro->device_identity() != id) {
			valid = false;
		}
	}

	if (valid) {
		double compensated_deg = hdt->heading_deg;

		//if (this->gyro->device_identity() != id) {
		//	if (hdt->heading_deg > 180.0) {
		//		compensated_deg -= 180.0;
		//	} else {
		//		compensated_deg += 180.0;
		//	}
		//}

		ON_MOVE(this->receivers, on_heading, logger, timepoint_ms, compensated_deg);
	}
}

void Compass::on_ROT(int id, long long timepoint_ms, ROT* rot, Syslog* logger) {
	bool valid = false;

	if (this->gyro->connected()) {
		if (id == this->gyro->device_identity()) {
			valid = true;
		}
	} else {
		valid = true;
	}

	if (valid) {
		double rate = (rot->validity ? rot->degpmin : flnan);

		ON_MOVE(this->receivers, on_turn, logger, timepoint_ms, rate);
	}
}

void Compass::on_GLL(int id, long long timepoint_ms, GLL* gll, Syslog* logger) {
	//logger->log_message(Log::Info, L"GLL: [%f]: (%lf, %lf), %s, %s", gll->utc,
		//gll->latitude, gll->longitude,
		//gll->validity.ToString()->Data(),
		//gll->mode.ToString()->Data());
}

void Compass::on_GSA(int id, long long timepoint_ms, GSA* gsa, Syslog* logger) {
	//logger->log_message(Log::Info, L"GSA: %s: (%lf, %lf, %lf), %s, [%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d]",
		//gsa->type.ToString()->Data(), gsa->pdop, gsa->hdop, gsa->vdop, gsa->auto_selection.ToString()->Data(),
		//gsa->PRNs[0], gsa->PRNs[1], gsa->PRNs[2], gsa->PRNs[3], gsa->PRNs[4], gsa->PRNs[5],
		//gsa->PRNs[6], gsa->PRNs[7], gsa->PRNs[8], gsa->PRNs[9], gsa->PRNs[10], gsa->PRNs[11]);
}

void Compass::on_GSV(int id, long long timepoint_ms, GSV* gsv, Syslog* logger) {
	//logger->log_message(Log::Info, L"GSV: %d-%d of %d: (%d, %d, %d, %d), (%d, %d, %d, %d), (%d, %d, %d, %d), (%d, %d, %d, %d)",
		//gsv->sequence0, gsv->sequence0 + 4, gsv->total,
		//gsv->PRNs[0], gsv->elevations[0], gsv->azimuthes[0], gsv->SNRs[0],
		//gsv->PRNs[1], gsv->elevations[1], gsv->azimuthes[1], gsv->SNRs[1],
		//gsv->PRNs[2], gsv->elevations[2], gsv->azimuthes[2], gsv->SNRs[2],
		//gsv->PRNs[3], gsv->elevations[3], gsv->azimuthes[3], gsv->SNRs[3]);
}

void Compass::on_ZDA(int id, long long timepoint_ms, ZDA* zda, Syslog* logger) {
	//logger->log_message(Log::Info, L"ZDA: [%f]: %04d-%02d-%02d, +(%d, %d)", zda->utc,
		//zda->year, zda->month, zda->day,
		//zda->local_hour_offset, zda->local_minute_offset);
}

bool Compass::available(int id) {
	return ((id == this->gyro->device_identity())
		|| (id == this->gps1->device_identity())
		|| (!this->gps1->connected()));
}

