﻿#include "planet.hpp"

#include "graphlet/planetlet.hpp"
#include "graphlet/filesystem/projectlet.hpp"
#include "graphlet/filesystem/project/profilet.hpp"
#include "graphlet/filesystem/configuration/gpslet.hpp"
#include "graphlet/filesystem/configuration/colorplotlet.hpp"
#include "graphlet/filesystem/configuration/vessel/trailing_suction_dredgerlet.hpp"

#include "syslog.hpp"
#include "plc.hpp"
#include "gps.hpp"

namespace WarGrey::DTPM {
	private class DTPMonitor : public WarGrey::SCADA::Planet, public WarGrey::DTPM::GPSReceiver, public WarGrey::SCADA::PLCConfirmation {
	public:
		DTPMonitor(WarGrey::SCADA::MRMaster* plc, WarGrey::DTPM::GPS* gps1, WarGrey::DTPM::GPS* gps2, WarGrey::DTPM::GPS* gyro);

	public:
		void load(Microsoft::Graphics::Canvas::UI::CanvasCreateResourcesReason reason, float width, float height) override;
		void reflow(float width, float height) override;
		void on_graphlet_ready(WarGrey::SCADA::IGraphlet* g) override;
		void on_tap_selected(WarGrey::SCADA::IGraphlet* g, float local_x, float local_y) override;
		void on_translation_gesture(float deltaX, float deltaY, Windows::Foundation::Numerics::float2& lt, Windows::Foundation::Numerics::float2& rb) override;
		void on_zoom_gesture(float zx, float zy, float deltaScale, Windows::Foundation::Numerics::float2& lt, Windows::Foundation::Numerics::float2& rb) override;
		WarGrey::SCADA::IGraphlet* thumbnail_graphlet() override;

	public:
		bool can_select(WarGrey::SCADA::IGraphlet* g) override;
		bool in_affine_gesture_zone(Windows::Foundation::Numerics::float2& lt, Windows::Foundation::Numerics::float2& rb) override;

	public:
		bool available(int id) override;
		void pre_scan_data(int id, WarGrey::SCADA::Syslog* logger) override;
		void on_GGA(int id, long long timepoint_ms, WarGrey::DTPM::GGA* gga, WarGrey::SCADA::Syslog* logger) override;
		void on_VTG(int id, long long timepoint_ms, WarGrey::DTPM::VTG* vtg, WarGrey::SCADA::Syslog* logger) override;
		void on_GLL(int id, long long timepoint_ms, WarGrey::DTPM::GLL* gll, WarGrey::SCADA::Syslog* logger) override;
		void on_GSA(int id, long long timepoint_ms, WarGrey::DTPM::GSA* gsa, WarGrey::SCADA::Syslog* logger) override;
		void on_GSV(int id, long long timepoint_ms, WarGrey::DTPM::GSV* gsv, WarGrey::SCADA::Syslog* logger) override;
		void on_ZDA(int id, long long timepoint_ms, WarGrey::DTPM::ZDA* zda, WarGrey::SCADA::Syslog* logger) override;
		void on_HDT(int id, long long timepoint_ms, WarGrey::DTPM::HDT* hdt, WarGrey::SCADA::Syslog* logger) override;
		void post_scan_data(int id, WarGrey::SCADA::Syslog* logger) override;

	public:
		void pre_read_data(WarGrey::SCADA::Syslog* logger) override;
		void on_analog_input(long long timepoint_ms, const uint8* DB2, size_t count2, const uint8* DB203, size_t count203, WarGrey::SCADA::Syslog* logger) override;
		void post_read_data(WarGrey::SCADA::Syslog* logger) override;
		
	private:
		void on_location_changed(double latitude, double longitude, double altitude, double geo_x, double geo_y);

	private: // never deletes these graphlets manually
		WarGrey::DTPM::TrailingSuctionDredgerlet* vessel;
		WarGrey::SCADA::Planetlet* metrics;
		WarGrey::SCADA::Planetlet* times;
		WarGrey::DTPM::Projectlet* project;
		WarGrey::DTPM::Profilet* profile;
		WarGrey::DTPM::GPSlet* gps;
		WarGrey::DTPM::ColorPlotlet* plot;
		WarGrey::SCADA::Planetlet* drags;
		WarGrey::SCADA::Planetlet* status;

	private:
		WarGrey::SCADA::MRMaster* plc;
		WarGrey::DTPM::GPSCS^ gcs;
		WarGrey::DTPM::GPS* gps1;
		WarGrey::DTPM::GPS* gps2;
		WarGrey::DTPM::GPS* gyro;
	};
}