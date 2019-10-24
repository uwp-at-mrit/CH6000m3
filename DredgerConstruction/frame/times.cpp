#include <map>

#include "frame/times.hpp"

#include "datum/credit.hpp"
#include "datum/string.hpp"

#include "module.hpp"
#include "preference.hxx"
#include "brushes.hxx"

#include "graphlet/shapelet.hpp"
#include "graphlet/ui/textlet.hpp"

#include "configuration.hpp"
#include "plc.hpp"

#include "iotables/di_hopper_pumps.hpp"

using namespace WarGrey::SCADA;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI;
using namespace Microsoft::Graphics::Canvas::Text;
using namespace Microsoft::Graphics::Canvas::Brushes;

static CanvasSolidColorBrush^ region_background = Colours::make(diagnostics_region_background);
static CanvasSolidColorBrush^ metrics_background = Colours::make(diagnostics_alarm_background);
static CanvasSolidColorBrush^ metrics_foreground = Colours::Green;

static Platform::String^ dredging_open_timepoint_key = "Dredging_Open_UTC_Milliseconds";
static Platform::String^ dredging_close_timepoint_key = "Dredging_Close_UTC_Milliseconds";

// WARNING: order matters
namespace {
	private enum class T : unsigned int { BeginTime, EndTime, DredgingTime, _ };

	private class Times final : public PLCConfirmation {
	public:
		Times(TimesFrame* master, float width) : master(master), width(width) {
			this->label_font = make_bold_text_format("Microsoft YaHei", normal_font_size);
			this->metrics_font = make_bold_text_format("Cambria Math", large_font_size);

			this->slot_height = this->metrics_font->FontSize * 1.2F;
			this->inset = this->metrics_font->FontSize * 0.618F;
			this->hgapsize = this->inset * 0.618F;

			{ // loading dredging timepoint
				this->dredging_start = get_preference(dredging_open_timepoint_key, 0LL);
				this->dredging_end = get_preference(dredging_close_timepoint_key, 0LL);
			}
		}

	public:
		void pre_read_data(Syslog* logger) override {
			this->master->enter_critical_section();
			this->master->begin_update_sequence();
		}

		void on_digital_input(long long timepoint_ms, const uint8* DB4, size_t count4, const uint8* DB205, size_t count205, Syslog* logger) override {
			bool hopper_on = (DI_hopper_pump_running(DB4, ps_hopper_pump_feedback) || DI_hopper_pump_running(DB4, sb_hopper_pump_feedback));
			
			if (hopper_on) {
				if ((this->dredging_start == 0LL) || ((this->dredging_end > 0LL) && (this->dredging_end < timepoint_ms))) {
					this->dredging_start = timepoint_ms;
					put_preference(dredging_open_timepoint_key, this->dredging_start);
				}

				if (this->dredging_end > 0LL) {
					this->dredging_end = 0LL;
					put_preference(dredging_close_timepoint_key, this->dredging_end);
				}
			} else {
				if (this->dredging_end == 0LL) {
					this->dredging_end = timepoint_ms;
					put_preference(dredging_close_timepoint_key, this->dredging_end);
				}
			}

			this->set_metrics(T::BeginTime, this->strftime(this->dredging_start));
			this->set_metrics(T::EndTime, this->strftime(this->dredging_end));
			
			this->set_metrics(T::DredgingTime, this->strftime(this->dredging_start,
				((this->dredging_end > 0LL) ? this->dredging_end : timepoint_ms)));
		}

		void post_read_data(Syslog* logger) override {
			this->master->end_update_sequence();
			this->master->leave_critical_section();
		}

	public:
		void load(float width, float height) {
			float corner_radius = 8.0F;
			float slot_width = this->width - this->inset * 2.0F;
			float bgheight = (this->slot_height + this->hgapsize) * _F(T::_) + this->inset * 2.0F;

			this->background = this->master->insert_one(new Rectanglet(this->width, bgheight, region_background));

			for (T id = _E0(T); id < T::_; id++) {
				this->slots[id] = this->master->insert_one(new Credit<RoundedRectanglet, T>(
					slot_width, this->slot_height, corner_radius, metrics_background), id);

				this->labels[id] = this->master->insert_one(new Credit<Labellet, T>(_speak(id), this->label_font, metrics_foreground), id);
				this->metrics[id] = this->master->insert_one(new Credit<Labellet, T>("-", this->metrics_font, metrics_foreground));
			}

			{ // set dredging timepoint
				this->set_metrics(T::BeginTime, this->strftime(this->dredging_start));
				this->set_metrics(T::EndTime, this->strftime(this->dredging_end));
				this->set_metrics(T::DredgingTime, this->strftime(this->dredging_start, this->dredging_end));
			}
		}

	public:
		void reflow(float width, float height) {
			for (T id = _E0(T); id < T::_; id++) {
				this->master->move_to(this->slots[id], this->inset, this->inset + (this->slot_height + this->hgapsize) * _F(id));
				
				this->master->move_to(this->labels[id], this->slots[id], GraphletAnchor::LC, GraphletAnchor::LC, this->hgapsize);
				this->master->move_to(this->metrics[id], this->slots[id], GraphletAnchor::RC, GraphletAnchor::RC, -this->hgapsize);
			}
		}

	private:
		void set_metrics(T id, Platform::String^ v) {
			this->metrics[id]->set_text(v, GraphletAnchor::RC);
		}

		Platform::String^ strftime(long long utc_ms) {
			Platform::String^ ts = "-";

			if (utc_ms > 0LL) {
				ts = make_daytimestamp_utc(utc_ms / 1000LL, true);
			}
			
			return ts;
		}

		Platform::String^ strftime(long long start_utc_ms, long long end_utc_ms) {
			long long period = end_utc_ms - start_utc_ms;
			Platform::String^ ts = "00:00";
			
			if ((start_utc_ms > 0LL) && (period > 0LL)) {
				long long hours, minutes, seconds;

				split_time_utc(period / 1000LL, false, &hours, &minutes, &seconds);

				if (hours > 0LL) {
					ts = make_wstring(L"%ld:%02ld:%02ld", hours, minutes, seconds);
				} else {
					ts = make_wstring(L"%02ld:%02ld", minutes, seconds);
				}
			}

			return ts;
		}

	private: // never deletes these graphlets manually
		std::map<T, Credit<RoundedRectanglet, T>*> slots;
		std::map<T, Credit<Labellet, T>*> labels;
		std::map<T, Credit<Labellet, T>*> metrics;
		Rectanglet* background;

	private:
		CanvasTextFormat^ label_font;
		CanvasTextFormat^ metrics_font;

	private:
		float width;
		float slot_height;
		float hgapsize;
		float inset;

	private:
		long long dredging_start;
		long long dredging_end;

	private: // never deletes these objects manually
		TimesFrame* master;
	};
}

/*************************************************************************************************/
TimesFrame::TimesFrame(float width, MRMaster* plc) : Planet(__MODULE__), plc(plc) {
	Times* dashboard = new Times(this, width);
	
	this->dashboard = dashboard;

	if (this->plc != nullptr) {
		this->plc->push_confirmation_receiver(dashboard);
	}
}

TimesFrame::~TimesFrame() {
	if (this->dashboard != nullptr) {
		delete this->dashboard;
	}
}

void TimesFrame::load(CanvasCreateResourcesReason reason, float width, float height) {
	auto dashboard = dynamic_cast<Times*>(this->dashboard);
	
	if (dashboard != nullptr) {
		dashboard->load(width, height);
	}
}

void TimesFrame::reflow(float width, float height) {
	auto dashboard = dynamic_cast<Times*>(this->dashboard);

	if (dashboard != nullptr) {
		dashboard->reflow(width, height);
	}
}

bool TimesFrame::can_select(IGraphlet* g) {
	return false;
}
