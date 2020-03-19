#include <map>

#include "metrics/times.hpp"

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
using namespace WarGrey::DTPM;
using namespace WarGrey::GYDM;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::UI;
using namespace Microsoft::Graphics::Canvas::Text;
using namespace Microsoft::Graphics::Canvas::Brushes;

static CanvasSolidColorBrush^ region_background = Colours::make(diagnostics_region_background);
static CanvasSolidColorBrush^ metrics_background = Colours::make(diagnostics_alarm_background);
static CanvasSolidColorBrush^ metrics_foreground = Colours::Green;

static Platform::String^ dredging_open_timepoint_key = "Dredging_Open_UTC_Milliseconds";
static Platform::String^ dredging_close_timepoint_key = "Dredging_Close_UTC_Milliseconds";

namespace {
	private enum class T : unsigned int { BeginTime, EndTime, DredgingTime, _ };

	private class TProvider final : public PLCConfirmation {
	public:
		TProvider() {
			this->dredging_start = get_preference(dredging_open_timepoint_key, 0LL);
			this->dredging_end = get_preference(dredging_close_timepoint_key, 0LL);
		}

	public:
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

			this->current_timepoint = timepoint_ms;
		}

	public:
		long long dredging_start;
		long long dredging_end;
		long long current_timepoint;

	private: // never deletes these objects manually
		TimeMetrics* master;
	};
}

/*************************************************************************************************/
static TProvider* provider = nullptr;

TimeMetrics::TimeMetrics(MRMaster* plc) {
	if (provider == nullptr) {
		provider = new TProvider();

		if (plc != nullptr) {
			plc->push_confirmation_receiver(provider);
		}
	}
}

unsigned int TimeMetrics::capacity() {
	return _N(T);
}

Platform::String^ TimeMetrics::label_ref(unsigned int idx) {
	return _speak(_E(T, idx));
}

MetricValue TimeMetrics::value_ref(unsigned int idx) {
	MetricValue mv;

	mv.type = MetricValueType::Null;
	mv.as.fixnum = 0LL;

	switch (_E(T, idx)) {
	case T::DredgingTime: {
		mv.type = MetricValueType::Period;

		if (provider->dredging_start > 0LL) {
			long long end = ((provider->dredging_end > 0) ? provider->dredging_end : provider->current_timepoint);
			
			mv.as.fixnum = end - provider->dredging_start;
		}
	}; break;
	case T::BeginTime: {
		if (provider->dredging_start > 0) {
			mv.type = MetricValueType::Time;
			mv.as.fixnum = provider->dredging_start;
		}
	}; break;
	case T::EndTime: {
		if (provider->dredging_end > 0) {
			mv.type = MetricValueType::Time;
			mv.as.fixnum = provider->dredging_end;
		}
	}; break;
	}

	mv.as.fixnum /= 1000LL;

	return mv;
}

CanvasSolidColorBrush^ TimeMetrics::label_color_ref(unsigned int idx) {
	CanvasSolidColorBrush^ color = Colours::Foreground;

	return color;
}

/*************************************************************************************************/
Timepoint::Timepoint() : IASNSequence(2) {}

Timepoint::Timepoint(const uint8* basn, size_t* offset) : Timepoint() {
	this->from_octets(basn, offset);
}

size_t Timepoint::field_payload_span(size_t idx) {
	size_t span = 0;

	switch (idx) {
	case 0: span = asn_real_span(this->dredging_start); break;
	case 1: span = asn_real_span(this->dredging_end); break;
	}

	return span;
}

size_t Timepoint::fill_field(size_t idx, uint8* octets, size_t offset) {
	switch (idx) {
	case 0: offset = asn_real_into_octets(this->dredging_start, octets, offset); break;
	case 1: offset = asn_real_into_octets(this->dredging_end, octets, offset); break;
	}

	return offset;
}

void Timepoint::extract_field(size_t idx, const uint8* basn, size_t* offset) {
	switch (idx) {
	case 0: this->dredging_start = asn_octets_to_real(basn, offset); break;
	case 1: this->dredging_end = asn_octets_to_real(basn, offset); break;
	}
}
