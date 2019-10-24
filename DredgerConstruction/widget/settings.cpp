#include "widget/settings.hpp"
#include "ch6000m3/configuration.hpp"

#include "navigator/thumbnail.hpp"

#include "device/vessel/trailing_suction_dredger.hpp"
#include "device/gps_cs.hpp"

#include "preference/colorplot.hpp"

#include "datum/box.hpp"

#include "satellite.hpp"
#include "system.hpp"
#include "module.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::Foundation;
using namespace Windows::System;

using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI;
using namespace Microsoft::Graphics::Canvas::Text;
using namespace Microsoft::Graphics::Canvas::Brushes;

/*************************************************************************************************/
namespace {
	private class BEJ54Convertor : public IGPSConvertor {
	public:
		static BEJ54Convertor* instance() {
			if (BEJ54Convertor::self == nullptr) {
				BEJ54Convertor::self = new BEJ54Convertor();
			}

			return BEJ54Convertor::self;
		}

	public:
		double3 gps_to_xyz(double latitude, double longitude, double altitude, GCSParameter& gcs) override {
			return GPS_to_XYZ(latitude, longitude, altitude, gcs);
		}

		double3 xyz_to_gps(double x, double y, double z, GCSParameter& gcs) override {
			// TODO: unimplemented

			return double3(x, y, z);
		}

	private:
		static BEJ54Convertor* self;

	private:
		BEJ54Convertor() {}
		~BEJ54Convertor() noexcept {}
	};

	::BEJ54Convertor* ::BEJ54Convertor::self;

	/************************************************************************************************/
	private ref class SettingsDisplay sealed : public UniverseDisplay {
	internal:
		SettingsDisplay(Syslog* logger, IHamburger* entity)
			: UniverseDisplay(logger, logger->get_name(), new ThumbnailNavigator(default_logging_level, __MODULE__, 1.0F, 80.0F, 3U, 4.0F))
			, settings(entity), closed(true) {
			this->use_global_mask_setting(false);

			this->_void = ref new SplitView();
			this->_void->Content = this->canvas;
			this->_void->Pane = navigator->user_interface();
			
			this->_void->OpenPaneLength = navigator->min_width();
			this->_void->PanePlacement = SplitViewPanePlacement::Left;
			this->_void->DisplayMode = SplitViewDisplayMode::Inline;
			this->_void->IsPaneOpen = true;
		}

	internal:
		void pickup_planet(IPlanet* planet) {
			UniverseDisplay::push_planet(planet);
		}

	public:
		SplitView^ flyout_content() {
			return this->_void;
		}

	public:
		void on_opening(Platform::Object^ target, Platform::Object^ args) {
			this->darkness = this->global_mask_alpha;
			this->settings->on_showing();
		}

		void on_opened(Platform::Object^ target, Platform::Object^ args) {
			this->closed = false;
			this->global_mask_alpha = 0.8;
			this->settings->on_shown();
		}

		void on_closing(FlyoutBase^ target, FlyoutBaseClosingEventArgs^ args) {
			args->Cancel = !(this->settings->can_hide());
		}

		void on_closed(Platform::Object^ target, Platform::Object^ args) {
			this->closed = true;
			this->settings->on_hiden();
			this->global_mask_alpha = this->darkness;
		}

	public:
		bool shown() override {
			return !(this->closed);
		}

	protected:
		void construct(CanvasCreateResourcesReason reason) override {
			this->push_planet(new TrailingSuctionDredgerPlanet());
			this->push_planet(new GPSCSPlanet(BEJ54Convertor::instance()));
			this->push_planet(new ColorPlotPlanet());
		}

	private: // never delete these objects manually
		IHamburger* settings;
		SplitView^ _void;
		double darkness;
		bool closed;
	};

	private class Settings : public IHamburger {
	public:
		Settings() {
			SettingsDisplay^ _universe = ref new SettingsDisplay(make_system_logger(default_logging_level, __MODULE__), this);

			this->settings = ref new Flyout();
			this->settings->Content = _universe->flyout_content();
			this->settings->Placement = FlyoutPlacementMode::Full;

			this->settings->Opening += ref new EventHandler<Platform::Object^>(_universe, &SettingsDisplay::on_opening);
			this->settings->Opened += ref new EventHandler<Platform::Object^>(_universe, &SettingsDisplay::on_opened);
			this->settings->Closing += ref new TypedEventHandler<FlyoutBase^, FlyoutBaseClosingEventArgs^>(_universe, &SettingsDisplay::on_closing);
			this->settings->Closed += ref new EventHandler<Platform::Object^>(_universe, &SettingsDisplay::on_closed);

			this->universe = _universe;
			FlyoutBase::SetAttachedFlyout(this->universe->canvas, this->settings);
		}

		void fill_extent(float* width, float* height) override {
			Size size = system_screen_size();
			float ratio = 0.75F;

			SET_BOX(width, size.Width * ratio);
			SET_BOX(height, size.Height * ratio);
		}

		bool can_hide() override {
			EditorPlanet* editor = dynamic_cast<EditorPlanet*>(this->universe->current_planet);
			
			return ((editor == nullptr) || editor->up_to_date());
		}

	protected:
		Flyout^ user_interface() override {
			return this->settings;
		}

	private:
		Flyout^ settings;
		UniverseDisplay^ universe;
	};
}

/*************************************************************************************************/
static Settings* the_settings = nullptr;

void WarGrey::SCADA::launch_the_settings() {
	if (the_settings == nullptr) {
		the_settings = new Settings();
	}

	return the_settings->show();
}
