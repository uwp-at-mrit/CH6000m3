#pragma once

#include "gps.hpp"
#include "ais.hpp"

namespace WarGrey::DTPM {
	private enum class MOXA_TCP : unsigned short {
		GYRO = 4001U, DP_DGPS = 4002U, AIS = 4003U, WIND = 4004U,
		TIDE = 4005U, MRIT_DGPS = 4006U, ECHO = 4007U, DP_DT_DREDGER = 4008U, DP_DT_TRACK = 4009U,
		_
	};

	void moxa_tcp_setup();
	void moxa_tcp_teardown();

	WarGrey::SCADA::ITCPConnection* moxa_tcp_ref(WarGrey::DTPM::MOXA_TCP name);
	WarGrey::DTPM::GPS* moxa_tcp_as_gps(WarGrey::DTPM::MOXA_TCP name, WarGrey::DTPM::INMEA0183Receiver* receiver = nullptr);
	WarGrey::DTPM::AIS* moxa_tcp_as_ais(WarGrey::DTPM::MOXA_TCP name, WarGrey::DTPM::INMEA0183Receiver* receiver = nullptr);
}
