#include "planet.hpp"

#include "graphlet/planetlet.hpp"
#include "graphlet/filesystem/projectlet.hpp"
#include "graphlet/filesystem/project/profilet.hpp"
#include "graphlet/filesystem/project/dredgetracklet.hpp"
#include "graphlet/filesystem/configuration/aislet.hpp"
#include "graphlet/filesystem/configuration/gpslet.hpp"
#include "graphlet/filesystem/configuration/colorplotlet.hpp"
#include "graphlet/filesystem/configuration/vessel/trailing_suction_dredgerlet.hpp"

#include "syslog.hpp"
#include "metrics.hpp"
#include "compass.hpp"
#include "transponder.hpp"
#include "plc.hpp"

namespace WarGrey::DTPM {
	private class DTPMonitor
		: public virtual WarGrey::SCADA::Planet
		, public virtual WarGrey::DTPM::CompassReceiver
		, public virtual WarGrey::DTPM::AISResponder
		, public virtual WarGrey::SCADA::PLCConfirmation
		, public virtual WarGrey::GYDM::SlangLocalPeer<WarGrey::DTPM::MetricsBlock> {
	public:
		virtual ~DTPMonitor() noexcept;
		DTPMonitor(WarGrey::DTPM::Compass* compass, WarGrey::DTPM::Transponder* ais, WarGrey::SCADA::MRMaster* plc);

	public:
		void load(Microsoft::Graphics::Canvas::UI::CanvasCreateResourcesReason reason, float width, float height) override;
		void reflow(float width, float height) override;
		void update(long long count, long long interval, long long uptime) override;

	public:
		void on_graphlet_ready(WarGrey::SCADA::IGraphlet* g) override;
		void on_tap_selected(WarGrey::SCADA::IGraphlet* g, float local_x, float local_y) override;
		void on_translation_gesture(float deltaX, float deltaY, Windows::Foundation::Numerics::float2& lt, Windows::Foundation::Numerics::float2& rb) override;
		void on_zoom_gesture(float zx, float zy, float deltaScale, Windows::Foundation::Numerics::float2& lt, Windows::Foundation::Numerics::float2& rb) override;
		WarGrey::SCADA::IGraphlet* thumbnail_graphlet() override;

	public:
		bool can_select(WarGrey::SCADA::IGraphlet* g) override;
		bool in_affine_gesture_zone(Windows::Foundation::Numerics::float2& lt, Windows::Foundation::Numerics::float2& rb) override;

	public:
		void pre_move(WarGrey::GYDM::Syslog* logger) override;
		void on_location(long long timepoint_ms, double latitude, double longitude, double altitude, double geo_x, double geo_y, WarGrey::GYDM::Syslog* logger) override;
		void on_sail(long long timepoint_ms, double kn, double track_deg, WarGrey::GYDM::Syslog* logger) override;
		void on_heading(long long timepoint_ms, double deg, WarGrey::GYDM::Syslog* logger) override;
		void post_move(WarGrey::GYDM::Syslog* logger) override;

	public:
		void pre_respond(WarGrey::GYDM::Syslog* logger) override;
		void on_self_position_report(long long timepoint_ms, WarGrey::DTPM::AISPositionReport* position, WarGrey::GYDM::Syslog* logger) override;
		void on_position_report(long long timepoint_ms, uint16 mmsi, WarGrey::DTPM::AISPositionReport* position, WarGrey::GYDM::Syslog* logger) override;
		void on_voyage_report(long long timepoint_ms, uint16 mmsi, WarGrey::DTPM::AISVoyageReport* position, WarGrey::GYDM::Syslog* logger) override;
		void post_respond(WarGrey::GYDM::Syslog* logger) override;

	public:
		void pre_read_data(WarGrey::GYDM::Syslog* logger) override;
		void on_analog_input(long long timepoint_ms, const uint8* DB2, size_t count2, const uint8* DB203, size_t count203, WarGrey::GYDM::Syslog* logger) override;
		void post_read_data(WarGrey::GYDM::Syslog* logger) override;

	public:
		void on_message(long long timepoint_ms, Platform::String^ remote_peer, uint16 port,
			WarGrey::DTPM::MetricsBlock type, const uint8* message,
			WarGrey::GYDM::Syslog* logger) override;
		
	private:
		void on_gps_message(long long timepoint_ms, WarGrey::DTPM::DGPS& dgps);

	private: // never deletes these graphlets manually
		WarGrey::DTPM::TrailingSuctionDredgerlet* vessel;
		WarGrey::DTPM::ITrackDataSource* track_source;
		WarGrey::DTPM::DredgeTracklet* track;
		WarGrey::SCADA::Planetlet* metrics;
		WarGrey::SCADA::Planetlet* times;
		WarGrey::DTPM::Projectlet* project;
		WarGrey::DTPM::Profilet* profile;
		WarGrey::DTPM::GPSlet* gps;
		WarGrey::DTPM::AISlet* traffic;
		WarGrey::DTPM::ColorPlotlet* plot;
		WarGrey::SCADA::Planetlet* drags;
		WarGrey::SCADA::Planetlet* status;

	private: // never deletes these shared objects
		WarGrey::DTPM::Compass* compass;
		WarGrey::DTPM::Transponder* transponder;
		WarGrey::SCADA::MRMaster* plc;

	private: // never deletes these global objects
		WarGrey::DTPM::ResidentMetrics* memory;
	};
}
