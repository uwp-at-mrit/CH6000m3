#include <map>

#include "frame/statusbar.hpp"
#include "configuration.hpp"
#include "moxa.hpp"

#include "datum/credit.hpp"
#include "datum/string.hpp"

#include "module.hpp"
#include "text.hpp"
#include "paint.hpp"
#include "brushes.hxx"

#include "graphlet/shapelet.hpp"
#include "graphlet/ui/textlet.hpp"
#include "graphlet/ui/statuslet.hpp"

using namespace WarGrey::SCADA;
using namespace WarGrey::DTPM;

using namespace Windows::System;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI;
using namespace Microsoft::Graphics::Canvas::Text;
using namespace Microsoft::Graphics::Canvas::Brushes;

static CanvasSolidColorBrush^ bar_background = Colours::make(diagnostics_region_background);
static CanvasSolidColorBrush^ bar_foreground = Colours::GhostWhite;
static CanvasSolidColorBrush^ bar_running_color = Colours::Green;
static CanvasSolidColorBrush^ bar_stopped_color = Colours::Red;

// WARNING: order matters
namespace {
	private enum class S : unsigned int {
		// Devices state Indicators
		GPS1, GPS2, GYRO, PLC,
		_
	};

	private class SystemStatuslet : public IGraphlet {
	public:
		SystemStatuslet(CanvasTextFormat^ status_font) : status_font(status_font), ipv4(nullptr), up_to_date(true) {}

	public:
		void on_available_storage_changed(unsigned long long free_bytes, unsigned long long total_bytes) {
			Platform::String^ label = status_speak("storage");
			Platform::String^ percentage = flstring(double(free_bytes) / double(total_bytes) * 100.0, 1);
			Platform::String^ free = sstring(free_bytes, 1);

			this->info->master->enter_critical_section();
			this->storage = make_text_layout(label + free + "(" + percentage + "%)", status_font);
			this->up_to_date = false;
			this->info->master->leave_critical_section();
		}

		void on_ipv4_address_changed(Platform::String^ ipv4) {
			Platform::String^ label = status_speak("ipv4");
			Platform::String^ ip = ((ipv4 == nullptr) ? status_speak("noipv4") : ipv4);

			this->info->master->enter_critical_section();
			this->ipv4 = make_text_layout(label + ip, status_font);
			this->up_to_date = false;
			this->info->master->leave_critical_section();
		}

	public:
		void fill_extent(float x, float y, float* w = nullptr, float* h = nullptr) override {
			SET_BOX(w, this->available_visible_width(x));
			SET_BOX(h, this->available_visible_height(y));
		}

		void update(long long count, long long interval, long long uptime) override {
			if (!this->up_to_date) {
				this->notify_updated();
			}
		}

		void draw(CanvasDrawingSession^ ds, float x, float y, float Width, float Height) override {
			float subwidth = Width / 3.0F;
			float context_y = y + (Height - this->ipv4->LayoutBounds.Height) * 0.5F;

			ds->DrawTextLayout(this->ipv4, x + Width - this->ipv4->LayoutBounds.Width, context_y, Colours::Yellow);
			ds->DrawTextLayout(this->storage, x + subwidth * 1.0F, context_y, Colours::YellowGreen);

			{ // draw App Memory Usage
				AppMemoryUsageLevel level;
				unsigned long long app_usage, private_working_set;
				CanvasSolidColorBrush^ color = Colours::GreenYellow;

				private_working_set = system_physical_memory_usage(&app_usage, &level);

				switch (level) {
				case AppMemoryUsageLevel::OverLimit: color = Colours::Firebrick; break;
				case AppMemoryUsageLevel::High: color = Colours::Orange; break;
				case AppMemoryUsageLevel::Low: color = Colours::RoyalBlue; break;
				}

				ds->DrawText(status_speak("memory") + ": " + sstring(private_working_set, 1), x, context_y, color, this->status_font);
			}
		}

	private:
		CanvasTextFormat^ status_font;
		CanvasTextLayout^ storage;
		CanvasTextLayout^ ipv4;

	private:
		bool up_to_date;
	};

	private class Statusbar final : public ISystemStatusListener {
	public:
		Statusbar(StatusFrame* master, ITCPConnection* plc) : master(master), plc(plc), system_metrics(nullptr) {
			this->status_font = make_bold_text_format("Microsoft YaHei", small_font_size);
			this->devices[S::PLC] = plc;
		}

	public:
		void on_available_storage_changed(unsigned long long free_bytes, unsigned long long total_bytes) override {
			this->system_metrics->on_available_storage_changed(free_bytes, total_bytes);
		}

		void on_ipv4_address_changed(Platform::String^ ipv4) override {
			this->system_metrics->on_ipv4_address_changed(ipv4);
		}

	public:
		void load(float width, float height) {
			this->background = this->master->insert_one(new Rectanglet(width, height, bar_background));
			this->log_receiver = this->master->insert_one(new Statuslinelet(default_gps_logging_level));
			
			this->load_device_indicator(S::GPS1, MOXA_TCP::MRIT_DGPS, this->status_font);
			this->load_device_indicator(S::GPS2, MOXA_TCP::DP_DGPS, this->status_font);
			this->load_device_indicator(S::GYRO, MOXA_TCP::GYRO, this->status_font);
			this->load_device_indicator(S::PLC, this->status_font);

			if (this->system_metrics == nullptr) {
				this->system_metrics = this->master->insert_one(new SystemStatuslet(this->status_font));
				register_system_status_listener(this);
			}
		}

		void reflow(float width, float height) {
			float cy = height * 0.5F;
			float margin = cy - this->status_font->FontSize * 0.5F;
			float sysinfo_x = width - width / 3.0F;

			{ // reflow indicators
				for (S id = S::GPS1; id <= S::PLC; id++) {
					if (id == S::GPS1) {
						this->master->move_to(this->indicators[id], margin, cy, GraphletAnchor::LC);
					} else {
						this->master->move_to(this->indicators[id], this->labels[_E(S, _I(id) - 1)],
							GraphletAnchor::RC, GraphletAnchor::LC, margin * 2.0F);
					}

					this->master->move_to(this->labels[id], this->indicators[id],
						GraphletAnchor::RC, GraphletAnchor::LC, margin * 0.5F);
				}
			}

			this->master->move_to(this->log_receiver, this->labels[S::PLC], GraphletAnchor::RC, GraphletAnchor::LC, margin);
			this->master->move_to(this->system_metrics, sysinfo_x, 0.0F, GraphletAnchor::LT);
		}

	public:
		void update(long long count, long long interval, long long uptime) {
			if (this->plc != nullptr) {
				if (this->plc->connected()) {
					this->plc->send_scheduled_request(count, interval, uptime);
				}
			}

			{ // check devices status
				this->master->begin_update_sequence();

				for (auto device = this->devices.begin(); device != this->devices.end(); device ++) {
					Shapelet* indicator = this->indicators[device->first];
					bool connected = (device->second != nullptr) && device->second->connected();
					ICanvasBrush^ color = (connected ? bar_running_color : bar_stopped_color);
					
					if (indicator->get_color() != color) {
						indicator->set_color(color);
						this->master->notify_graphlet_updated(indicator);
					}
				}

				this->master->end_update_sequence();
			}
		}

	private:
		void load_device_indicator(S id, MOXA_TCP gps, CanvasTextFormat^ font) {
			devices[id] = moxa_tcp_as_gps(gps);
			this->load_device_indicator(id, font);
		}
		
		void load_device_indicator(S id, CanvasTextFormat^ font) {
			ITCPConnection* device = this->devices[id];
			ICanvasBrush^ initail_color = ((device != nullptr) && device->connected()) ? bar_running_color : bar_stopped_color;
			
			this->labels[id] = this->master->insert_one(new Credit<Labellet, S>(_speak(id), font, bar_foreground), id);
			this->indicators[id] = this->master->insert_one(new Credit<Rectanglet, S>(font->FontSize, initail_color), id);

			if ((device != nullptr) && (this->log_receiver != nullptr)) {
				device->get_logger()->push_log_receiver(this->log_receiver);
			}
		}

	private:
		CanvasTextFormat^ status_font;

	private: // never deletes these graphlets manually
		Rectanglet* background;
		SystemStatuslet* system_metrics;
		Statuslinelet* log_receiver;
		std::map<S, Credit<Labellet, S>*> labels;
		std::map<S, Credit<Rectanglet, S>*> indicators;

	private: // never deletes these objects manually
		StatusFrame* master;
		std::map<S, ITCPConnection*> devices;
		ITCPConnection* plc;
	};
}

/*************************************************************************************************/
StatusFrame::StatusFrame(ITCPConnection* plc) : Planet(__MODULE__) {
	Statusbar* statusbar = new Statusbar(this, plc);

	this->statusbar = statusbar;
}

StatusFrame::~StatusFrame() {
	if (this->statusbar != nullptr) {
		delete this->statusbar;
	}
}

void StatusFrame::load(CanvasCreateResourcesReason reason, float width, float height) {
	auto statusbar = dynamic_cast<Statusbar*>(this->statusbar);
	
	if (statusbar != nullptr) {
		statusbar->load(width, height);
	}
}

void StatusFrame::reflow(float width, float height) {
	auto statusbar = dynamic_cast<Statusbar*>(this->statusbar);

	if (statusbar != nullptr) {
		statusbar->reflow(width, height);
	}
}

void StatusFrame::update(long long count, long long interval, long long uptime) {
	auto statusbar = dynamic_cast<Statusbar*>(this->statusbar);

	if (statusbar != nullptr) {
		statusbar->update(count, interval, uptime);
	}
}

bool StatusFrame::can_select(IGraphlet* g) {
	return false;
}
