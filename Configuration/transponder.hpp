#pragma once

#include "graphlet/aislet.hpp"
#include "graphlet/filesystem/configuration/gpslet.hpp"

#include "ais.hpp"
#include "syslog.hpp"

namespace WarGrey::DTPM {
	/************************************************************************************************/
	private class IAISResponder abstract {
	public:
		virtual bool respondable() { return true; }

	public:
		virtual void pre_respond(WarGrey::GYDM::Syslog* logger) = 0;
		virtual void on_position_report(long long timepoint_ms, uint16 mmsi, WarGrey::DTPM::AISPositionReport* position, WarGrey::GYDM::Syslog* logger) = 0;
		virtual void post_respond(WarGrey::GYDM::Syslog* logger) = 0;
	};

	private class Transponder : public WarGrey::DTPM::Transceiver {
	public:
		Transponder();
		
	public:
		void on_ASO(int id, long long timepoint_ms, bool self, uint16 mmsi, WarGrey::DTPM::ASO* aso, uint8 priority, WarGrey::GYDM::Syslog* logger) override;
		void on_SDR(int id, long long timepoint_ms, bool self, uint16 mmsi, WarGrey::DTPM::SDR* sdr, uint8 priority, WarGrey::GYDM::Syslog* logger) override;
		
	public:
		void set_gps_convertion_matrix(WarGrey::DTPM::GPSCS^ gcs);
		void push_receiver(WarGrey::DTPM::IAISResponder* receiver);

	private:
		WarGrey::DTPM::GPSCS^ gcs;
		std::deque<WarGrey::DTPM::IAISResponder*> responders;

	private: // never delete this shared object
		WarGrey::DTPM::INMEA0183* tranceiver;
	};

	/************************************************************************************************/
	private class AISResponder : public WarGrey::DTPM::IAISResponder {
	public:
		void pre_respond(WarGrey::GYDM::Syslog* logger) override {}
		void post_respond(WarGrey::GYDM::Syslog* logger) override {}

	public:
		void on_position_report(long long timepoint_ms, uint16 mmsi, WarGrey::DTPM::AISPositionReport* position, WarGrey::GYDM::Syslog* logger) override {};
	};
}
