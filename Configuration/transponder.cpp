#include "transponder.hpp"
#include "moxa.hpp"

using namespace WarGrey::SCADA;
using namespace WarGrey::DTPM;
using namespace WarGrey::GYDM;

#define ON_MOBILE(responders, on_respond, logger, ...) do { \
for (auto r : responders) { \
if (r->respondable()) { \
r->pre_respond(logger); \
r->on_respond(__VA_ARGS__, logger); \
r->post_respond(logger); \
} \
} \
} while (0)

/*************************************************************************************************/
Transponder::Transponder() {
	this->tranceiver = moxa_tcp_as_ais(MOXA_TCP::AIS, this);
}

void Transponder::set_gps_convertion_matrix(GPSCS^ gcs) {
	this->gcs = gcs;
}

void Transponder::push_receiver(IAISResponder* r) {
	if (r != nullptr) {
		this->responders.push_back(r);
	}
}

/*************************************************************************************************/
void Transponder::on_ASO(int id, long long timepoint_ms, bool self, uint16 mmsi, ASO* prca, uint8 priority, Syslog* logger) {
	if (this->gcs != nullptr) {
		AISPositionReport pr;

		pr.latitude = ais_latitude_filter(prca->latitude);
		pr.longitude = ais_longitude_filter(prca->longitude);
		pr.turn = ais_turn_filter(prca->turn);
		pr.speed = ais_speed_filter(prca->speed);
		pr.course = ais_course_filter(prca->course);
		pr.heading = ais_heading360_filter(prca->heading);

		pr.geo = Degrees_to_XY(pr.latitude, pr.longitude, 0.0, this->gcs->parameter);

		if (self) {
			ON_MOBILE(this->responders, on_self_position_report, logger, timepoint_ms, &pr);
		} else {
			ON_MOBILE(this->responders, on_position_report, logger, timepoint_ms, mmsi, &pr);
		}
	}
}

void Transponder::on_BCS(int id, long long timepoint_ms, bool self, uint16 mmsi, BCS* prcb, uint8 priority, Syslog* logger) {
	if (this->gcs != nullptr) {
		AISPositionReport pr;

		pr.latitude = ais_latitude_filter(prcb->latitude);
		pr.longitude = ais_longitude_filter(prcb->longitude);
		pr.speed = ais_speed_filter(prcb->speed);
		pr.course = ais_course_filter(prcb->course);
		pr.heading = ais_heading360_filter(prcb->heading);

		pr.turn = flnan;
		pr.geo = Degrees_to_XY(pr.latitude, pr.longitude, 0.0, this->gcs->parameter);

		if (self) {
			ON_MOBILE(this->responders, on_self_position_report, logger, timepoint_ms, &pr);
		} else {
			ON_MOBILE(this->responders, on_position_report, logger, timepoint_ms, mmsi, &pr);
		}
	}
}

void Transponder::on_BCSE(int id, long long timepoint_ms, bool self, uint16 mmsi, BCSE* prcb, uint8 priority, Syslog* logger) {
	if (this->gcs != nullptr) {
		AISPositionReport pr;

		pr.latitude = ais_latitude_filter(prcb->latitude);
		pr.longitude = ais_longitude_filter(prcb->longitude);
		pr.speed = ais_speed_filter(prcb->speed);
		pr.course = ais_course_filter(prcb->course);
		pr.heading = ais_heading360_filter(prcb->heading);

		pr.turn = flnan;
		pr.geo = Degrees_to_XY(pr.latitude, pr.longitude, 0.0, this->gcs->parameter);

		if (self) {
			ON_MOBILE(this->responders, on_self_position_report, logger, timepoint_ms, &pr);
		} else {
			ON_MOBILE(this->responders, on_position_report, logger, timepoint_ms, mmsi, &pr);
		}
	}
}

void Transponder::on_SDR(int id, long long timepoint_ms, bool self, uint16 mmsi, SDR* sdr, uint8 priority, Syslog* logger) {
}