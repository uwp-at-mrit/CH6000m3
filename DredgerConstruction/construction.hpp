#include "planet.hpp"

#include "graphlet/planetlet.hpp"
#include "graphlet/filesystem/projectlet.hpp"
#include "graphlet/filesystem/configuration/gpslet.hpp"
#include "graphlet/filesystem/configuration/colorplotlet.hpp"
#include "graphlet/filesystem/configuration/vessel/trailing_suction_dredgerlet.hpp"

#include "syslog.hpp"
#include "plc.hpp"
#include "gps.hpp"

namespace WarGrey::SCADA {
	private class DredgerConstruction : public WarGrey::SCADA::Planet, public WarGrey::SCADA::GPSReceiver, public WarGrey::SCADA::PLCConfirmation {
	public:
		DredgerConstruction(WarGrey::SCADA::MRMaster* plc, WarGrey::SCADA::GPS* gps1, WarGrey::SCADA::GPS* gps2, WarGrey::SCADA::GPS* gyro);

	public:
		void load(Microsoft::Graphics::Canvas::UI::CanvasCreateResourcesReason reason, float width, float height) override;
		void reflow(float width, float height) override;
		void on_graphlet_ready(WarGrey::SCADA::IGraphlet* g) override;
		IGraphlet* thumbnail_graphlet() override;

	public:
		bool can_select(WarGrey::SCADA::IGraphlet* g) override;

	public:
		bool available(int id) override;
		void pre_scan_data(int id, WarGrey::SCADA::Syslog* logger) override;
		void on_GGA(int id, long long timepoint_ms, GGA* gga, WarGrey::SCADA::Syslog* logger) override;
		void on_VTG(int id, long long timepoint_ms, VTG* vtg, WarGrey::SCADA::Syslog* logger) override;
		void on_GLL(int id, long long timepoint_ms, GLL* gll, WarGrey::SCADA::Syslog* logger) override;
		void on_GSA(int id, long long timepoint_ms, GSA* gsa, WarGrey::SCADA::Syslog* logger) override;
		void on_GSV(int id, long long timepoint_ms, GSV* gsv, WarGrey::SCADA::Syslog* logger) override;
		void on_ZDA(int id, long long timepoint_ms, ZDA* zda, WarGrey::SCADA::Syslog* logger) override;
		void on_HDT(int id, long long timepoint_ms, HDT* hdt, WarGrey::SCADA::Syslog* logger) override;
		void post_scan_data(int id, WarGrey::SCADA::Syslog* logger) override;

	public:
		void pre_read_data(WarGrey::SCADA::Syslog* logger) override;
		void on_analog_input(long long timepoint_ms, const uint8* DB2, size_t count2, const uint8* DB203, size_t count203, WarGrey::SCADA::Syslog* logger) override;
		void post_read_data(WarGrey::SCADA::Syslog* logger) override;
		
	private:
		void on_location_changed(double latitude, double longitude, double altitude, double x, double y);

	private: // never deletes these graphlets manually
		WarGrey::SCADA::Planetlet* metrics;
		WarGrey::SCADA::Planetlet* times;
		WarGrey::SCADA::Projectlet* vmap;
		WarGrey::SCADA::GPSlet* gps;
		WarGrey::SCADA::ColorPlotlet* plot;
		WarGrey::SCADA::Planetlet* drags;
		WarGrey::SCADA::Planetlet* status;

	private:
		WarGrey::SCADA::TrailingSuctionDredgerlet* vessel;
		WarGrey::SCADA::GPSCS^ gcs;
		WarGrey::SCADA::MRMaster* plc;
		WarGrey::SCADA::GPS* gps1;
		WarGrey::SCADA::GPS* gps2;
		WarGrey::SCADA::GPS* gyro;
	};
}
