#include <map>

#include "frame/statusbar.hpp"
#include "configuration.hpp"

#include "datum/credit.hpp"

#include "module.hpp"
#include "text.hpp"
#include "paint.hpp"
#include "brushes.hxx"

#include "graphlet/shapelet.hpp"
#include "graphlet/ui/textlet.hpp"

using namespace WarGrey::SCADA;

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

	private class Statusbar final : public GPSReceiver {
	public:
		Statusbar(StatusFrame* master, ITCPStatedConnection* plc, GPS* gps1, GPS* gps2, GPS* gyro)
			: master(master), plc(plc), main_gps(gps1) {
			this->status_font = make_bold_text_format("Microsoft YaHei", small_font_size);
			
			if (gps1 != nullptr) {
				gps1->push_confirmation_receiver(this);
			}

			if (gps2 != nullptr) {
				gps2->push_confirmation_receiver(this);
			}

			this->devices[S::GPS1] = gps1;
			this->devices[S::GPS2] = gps2;
			this->devices[S::GYRO] = gyro;
			this->devices[S::PLC] = plc;
		}

	public:
		void pre_scan_data(int id, Syslog* logger) override {
			this->master->enter_critical_section();
			this->master->begin_update_sequence();
		}

		void post_scan_data(int id, Syslog* logger) override {
			this->master->end_update_sequence();
			this->master->leave_critical_section();
		}

		bool available(int id) override {
			return ((id == this->main_gps->device_identity())
				|| (!this->main_gps->connected()));
		}

	public:
		void load(float width, float height) {
			this->background = this->master->insert_one(new Rectanglet(width, height, bar_background));

			this->load_device_indicator(S::GPS1, this->status_font);
			this->load_device_indicator(S::GPS2, this->status_font);
			this->load_device_indicator(S::GYRO, this->status_font);
			this->load_device_indicator(S::PLC, this->status_font);
		}

	public:
		void reflow(float width, float height) {
			float cy = height * 0.5F;
			float margin = cy - this->status_font->FontSize * 0.5F;

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
		void load_device_indicator(S id, CanvasTextFormat^ font) {
			ITCPStatedConnection* device = this->devices[id];
			ICanvasBrush^ initail_color = ((device != nullptr) && device->connected()) ? bar_running_color : bar_stopped_color;
			
			this->labels[id] = this->master->insert_one(new Credit<Labellet, S>(_speak(id), font, bar_foreground), id);
			this->indicators[id] = this->master->insert_one(new Credit<Rectanglet, S>(font->FontSize, initail_color), id);
		}

	private:
		CanvasTextFormat^ status_font;

	private: // never deletes these graphlets manually
		Rectanglet* background;
		std::map<S, Credit<Labellet, S>*> labels;
		std::map<S, Credit<Rectanglet, S>*> indicators;

	private: // never deletes these objects manually
		StatusFrame* master;
		std::map<S, ITCPStatedConnection*> devices;
		
		ITCPConnection* plc;
		GPS* main_gps;
	};
}

/*************************************************************************************************/
StatusFrame::StatusFrame(ITCPStatedConnection* plc, GPS* gps1, GPS* gps2, GPS* gyro) : Planet(__MODULE__) {
	Statusbar* statusbar = new Statusbar(this, plc, gps1, gps2, gyro);
	
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
