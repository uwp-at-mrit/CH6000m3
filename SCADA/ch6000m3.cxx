#include "application.hxx"
#include "configuration.hpp"
#include "iotables/macro_keys.hpp"
#include "widget.hxx"
#include "plc.hpp"

#include "decorator/headsup.hpp"
#include "navigator/thumbnail.hpp"
#include "planet.hpp"
#include "timer.hxx"

#include "page/hydraulics.hpp"
#include "page/hopper_doors.hpp"
#include "page/glands.hpp"
#include "page/draughts.hpp"
#include "page/lubricatings.hpp"
#include "page/charges.hpp"
#include "page/discharges.hpp"
#include "page/flushs.hpp"
#include "page/dredges.hpp"

#include "splash.hpp"

using namespace WarGrey::SCADA;

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel::Background;

using namespace Windows::System;
using namespace Windows::Foundation;

using namespace Windows::UI::Input;

using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Controls;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI;

private ref class DredgerUniverse : public UniverseDisplay {
public:
	virtual ~DredgerUniverse() {
		if (this->device != nullptr) {
			delete this->device;
		}
	}

internal:
	DredgerUniverse(Platform::String^ name, PLCMaster* device, IUniverseNavigator* navigator, IHeadUpPlanet* heads_up)
		: UniverseDisplay(make_system_logger(default_logging_level, name), name, navigator, heads_up), device(device) {}

protected:
	void construct(CanvasCreateResourcesReason reason) override {
		// this->push_planet(new SplashScreen(620.0F));
		// this->push_planet(new SplashScreen(1240.0F, 0.0F));
		
		this->push_planet(new HydraulicsPage(this->device)); // 0
		this->push_planet(new ChargesPage(this->device)); // 1
		this->push_planet(new DredgesPage(DragView::_, this->device)); // 2
		this->push_planet(new DischargesPage(this->device)); // 3
		this->push_planet(new GlandsPage(this->device)); // 4
		this->push_planet(new FlushsPage(this->device)); // 5
		this->push_planet(new DredgesPage(DragView::PortSide, this->device)); // 6
		this->push_planet(new HopperDoorsPage(this->device)); // 7
		this->push_planet(new LubricatingsPage(this->device)); // 8
		this->push_planet(new DraughtsPage(this->device)); // 9
		this->push_planet(new DredgesPage(DragView::Starboard, this->device)); // 10
		this->push_planet(new DredgesPage(DragView::Suctions, this->device)); // 11
	}

protected private:
	PLCMaster* device;
};

/*************************************************************************************************/
private class PageEventListener : public PLCConfirmation {
public:
	PageEventListener(unsigned int idx) : dbidx(idx), page(2 /* the dredging page */) {}

public:
	void on_analog_input(long long timepoint_ms, const uint8* DB2, size_t count2, const uint8* DB203, size_t count203, Syslog* logger) override {
		this->page = int(DBD(DB2, this->dbidx));
	}

public:
	int get_target_page() {
		return page;
	}

private:
	unsigned int dbidx;
	int page;
};

private ref class DashboardUniverse sealed : public DredgerUniverse {
public:
	virtual ~DashboardUniverse() {
		if (this->page_turner != nullptr) {
			delete this->page_turner;
		}
	}

internal:
	DashboardUniverse(Platform::String^ name, PLCMaster* device, IUniverseNavigator* navigator, IHeadUpPlanet* heads_up, unsigned int dbidx)
		: DredgerUniverse(name, device, navigator, heads_up) {
		this->page_turner = new PageEventListener(dbidx);
		this->device->push_confirmation_receiver(this->page_turner);
	}

public:
	void update(long long count, long long interval, long long uptime) override {
		this->transfer_to(this->page_turner->get_target_page());
	}

private:
	PageEventListener* page_turner;
};

/*************************************************************************************************/
private ref class CH6000m3 sealed : public SplitView {
public:
	CH6000m3() : SplitView() {
		this->Margin = ThicknessHelper::FromUniformLength(0.0);
		this->PanePlacement = SplitViewPanePlacement::Left;
		this->DisplayMode = SplitViewDisplayMode::Overlay;
		this->IsPaneOpen = false;

		this->PointerMoved += ref new PointerEventHandler(this, &CH6000m3::on_pointer_moved);
		this->PointerReleased += ref new PointerEventHandler(this, &CH6000m3::on_pointer_released);

		//this->AddHandler(UIElement::PointerPressedEvent, ref new PointerEventHandler(this, &CH6000m3::on_pointer_pressed), true);
		//this->AddHandler(UIElement::PointerReleasedEvent, ref new PointerEventHandler(this, &CH6000m3::on_pointer_released), true);
	}

public:
	void construct(Platform::String^ name, Size region) {
		Platform::String^ localhost = system_ipv4_address();
		PLCMaster* device = new PLCMaster(make_system_logger(default_plc_master_logging_level, name + ": PLC"), plc_master_suicide_timeout);
		IUniverseNavigator* navigator = new ThumbnailNavigator(default_logging_level, name, region.Width / region.Height, 160.0F);
		HeadsUpPlanet* heads_up = new HeadsUpPlanet(device);

		if (localhost->Equals("192.168.0.11")) {
			this->universe = ref new DashboardUniverse(name, device, navigator, heads_up, left_paging_key);
		} else if (localhost->Equals("192.168.0.12")) {
			this->universe = ref new DashboardUniverse(name, device, navigator, heads_up, right_paging_key);
		} else {
			this->universe = ref new DredgerUniverse(name, device, navigator, heads_up);
		}

		// TODO: Why SplitView::Content cannot do it on its own?
		this->universe->register_virtual_keydown_event_handler(this);

		this->Content = this->universe->canvas;
		this->timeline = ref new CompositeTimerListener();
		this->timer = ref new Timer(this->timeline, frame_per_second);
		this->timeline->push_timer_listener(this->universe);

		{ // construct the functional panel
			StackPanel^ panel = ref new StackPanel();
			
			this->universe->navigator->min_height(region.Height * 0.85F);
			
			this->widget = ref new UniverseWidget(this, this->universe, device);
			this->widget->min_width = this->universe->navigator->min_width();
			this->widget->min_height = region.Height - this->universe->navigator->min_height();

			panel->Orientation = ::Orientation::Vertical;
			panel->HorizontalAlignment = ::HorizontalAlignment::Stretch;
			panel->VerticalAlignment = ::VerticalAlignment::Stretch;

			panel->Children->Append(this->universe->navigator->user_interface());
			panel->Children->Append(this->widget->canvas);

			this->Pane = panel;
			this->OpenPaneLength = this->universe->navigator->min_width();
			this->timeline->push_timer_listener(this->widget);
		}
	}
	
	void on_entered_background(EnteredBackgroundEventArgs^ args) {}
	void on_background_activated(IBackgroundTaskInstance^ task) {}
	void on_leaving_background(LeavingBackgroundEventArgs^ args) {}
	void on_suspending(SuspendingEventArgs^ args) {}
	void on_resuming() {}
	
private:
	void on_pointer_moved(Platform::Object^ sender, PointerRoutedEventArgs^ args) {
		auto pt = args->GetCurrentPoint(this);
		float x = pt->Position.X;

		if (!pt->Properties->IsLeftButtonPressed) {
			if (x <= this->Margin.Left) {
				this->IsPaneOpen = true;
				args->Handled = true;
			} 
		}
	}

	void on_pointer_released(Platform::Object^ sender, PointerRoutedEventArgs^ args) {
		auto pt = args->GetCurrentPoint(this);
		float x = pt->Position.X;

		if (pt->Properties->PointerUpdateKind == PointerUpdateKind::LeftButtonReleased) {
			if (x <= normal_font_size) {
				this->IsPaneOpen = true;
				args->Handled = true;
			}
		}
	}

private:
	CompositeTimerListener^ timeline;
	Timer^ timer;
	PLCMaster* device;

private:
	DredgerUniverse^ universe;
	UniverseWidget^ widget;
};

int main(Platform::Array<Platform::String^>^ args) {
	return launch_universal_windows_application<CH6000m3>(default_logging_level, remote_test_server);
}
