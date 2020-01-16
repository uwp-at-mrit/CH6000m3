#include <map>

#include "moxa.hpp"
#include "configuration.hpp"

#include "datum/enum.hpp"

using namespace WarGrey::SCADA;
using namespace WarGrey::DTPM;

using namespace Windows::Foundation::Numerics;

/*************************************************************************************************/
static std::map<MOXA_TCP, ITCPConnection*> moxa_tcp_clients;

static GPS* setup_gps(MOXA_TCP name) {
	Syslog* gps_logger = make_system_logger(default_gps_logging_level, name.ToString());
	GPS* gps = new GPS(gps_logger, moxa_gateway, _S(name));

	gps->set_suicide_timeout(gps_suicide_timeout);
	moxa_tcp_clients.insert(std::pair<MOXA_TCP, ITCPConnection*>(name, gps));

	return gps;
}

/*************************************************************************************************/
void WarGrey::DTPM::moxa_tcp_setup() {
	setup_gps(MOXA_TCP::MRIT_DGPS);
	setup_gps(MOXA_TCP::DP_DGPS);
	setup_gps(MOXA_TCP::GYRO);
}

void WarGrey::DTPM::moxa_tcp_teardown() {
	for (auto it = moxa_tcp_clients.begin(); it != moxa_tcp_clients.end(); it++) {
		delete it->second;
	}

	moxa_tcp_clients.clear();
}

ITCPConnection* WarGrey::DTPM::moxa_tcp_ref(MOXA_TCP name) {
	ITCPConnection* client = nullptr;
	auto it = moxa_tcp_clients.find(name);

	if (it != moxa_tcp_clients.end()) {
		client = it->second;
	}

	return client;
}

IGPS* WarGrey::DTPM::moxa_tcp_as_gps(MOXA_TCP name, IGPSReceiver* receiver) {
	ITCPConnection* client = moxa_tcp_ref(name);
	IGPS* gps = nullptr;

	if ((client != nullptr) && (client->get_type() == TCPType::GPS)) {
		gps = static_cast<IGPS*>(client);
		gps->push_confirmation_receiver(receiver);
	}

	return gps;
}