#pragma once

#include "graphlet/filesystem/configuration/gpslet.hpp"

#include "gps.hpp"
#include "syslog.hpp"

namespace WarGrey::DTPM {
	private class ICompassReceiver abstract {
	public:
		virtual bool moveable() { return true; }

	public:
		virtual void pre_move(WarGrey::GYDM::Syslog* logger) = 0;
		virtual void on_location(long long timepoint_ms, double latitude, double longitude, double altitude, double geo_x, double geo_y, WarGrey::GYDM::Syslog* logger) = 0;
		virtual void on_sail(long long timepoint_ms, double kn, double track_deg, WarGrey::GYDM::Syslog* logger) = 0;
		virtual void on_heading(long long timepoint_ms, double deg, WarGrey::GYDM::Syslog* logger) = 0;
		virtual void on_turn(long long timepoint_ms, double degpmin, WarGrey::GYDM::Syslog* logger) = 0;
		virtual void post_move(WarGrey::GYDM::Syslog* logger) = 0;
	};

	private class Compass : public WarGrey::DTPM::GPSReceiver {
	public:
		Compass();

	public:
		bool available(int id) override;
		void on_GGA(int id, long long timepoint_ms, WarGrey::DTPM::GGA* gga, WarGrey::GYDM::Syslog* logger) override;
		void on_VTG(int id, long long timepoint_ms, WarGrey::DTPM::VTG* vtg, WarGrey::GYDM::Syslog* logger) override;
		void on_HDT(int id, long long timepoint_ms, WarGrey::DTPM::HDT* hdt, WarGrey::GYDM::Syslog* logger) override;
		void on_ROT(int id, long long timepoint_ms, WarGrey::DTPM::ROT* rot, WarGrey::GYDM::Syslog* logger) override;
		void on_GLL(int id, long long timepoint_ms, WarGrey::DTPM::GLL* gll, WarGrey::GYDM::Syslog* logger) override;
		void on_GSA(int id, long long timepoint_ms, WarGrey::DTPM::GSA* gsa, WarGrey::GYDM::Syslog* logger) override;
		void on_GSV(int id, long long timepoint_ms, WarGrey::DTPM::GSV* gsv, WarGrey::GYDM::Syslog* logger) override;
		void on_ZDA(int id, long long timepoint_ms, WarGrey::DTPM::ZDA* zda, WarGrey::GYDM::Syslog* logger) override;
		
	public:
		void set_gps_convertion_matrix(WarGrey::DTPM::GPSCS^ gcs);
		void push_receiver(WarGrey::DTPM::ICompassReceiver* receiver);

	private:
		WarGrey::DTPM::GPSCS^ gcs;
		std::deque<WarGrey::DTPM::ICompassReceiver*> receivers;

	private: // never deletes these shared objects
		WarGrey::DTPM::INMEA0183* gps1;
		WarGrey::DTPM::INMEA0183* gps2;
		WarGrey::DTPM::INMEA0183* gyro;
	};

	/************************************************************************************************/
	private class CompassReceiver : public WarGrey::DTPM::ICompassReceiver {
	public:
		void pre_move(WarGrey::GYDM::Syslog* logger) override {}
		void on_location(long long timepoint_ms, double latitude, double longitude, double altitude, double geo_x, double geo_y, WarGrey::GYDM::Syslog* logger) override {}
		void on_sail(long long timepoint_ms, double kn, double track_deg, WarGrey::GYDM::Syslog* logger) override {}
		void on_heading(long long timepoint_ms, double deg, WarGrey::GYDM::Syslog* logger) override {}
		void on_turn(long long timepoint_ms, double degpmin, WarGrey::GYDM::Syslog* logger) override {}
		void post_move(WarGrey::GYDM::Syslog* logger) override {}
	};
}
