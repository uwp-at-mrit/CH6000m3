#include "metrics/dredge.hpp"

#include "datum/flonum.hpp"

#include "iotables/ai_metrics.hpp"

#include "metrics.hpp"
#include "module.hpp"
#include "brushes.hxx"

/*************************************************************************************************/
using namespace WarGrey::SCADA;
using namespace WarGrey::DTPM;

using namespace Microsoft::Graphics::Canvas::Brushes;

/*************************************************************************************************/
#define SET_METRICS(ms, id, v)          ms[_I(id)] = v

/*************************************************************************************************/
namespace {
	private enum class DM : unsigned int {
		BowDirection, TrackDirection, FlowPressureAngle, LineDirection, Speed, TurnRate,
		BowDraught, SternDraught, AverageDraught,
		PSFlow, SBFlow, PSConcentration, SBConcentration, AverageDensity,
		Capacity, Payload, Displacement, EarthWork,
		PSProduct, SBProduct,
		MudDensity,
		_
	};
}

class WarGrey::DTPM::DredgeMetrics::Provider final : virtual public PLCConfirmation, virtual public CompassReceiver {
public:
	Provider() {
		this->memory = global_resident_metrics();
	}

public:
	void on_analog_input(long long timepoint_ms, const uint8* DB2, size_t count2, const uint8* DB203, size_t count203, Syslog* logger) override {
		SET_METRICS(this->metrics, DM::BowDraught, DBD(DB2, fixed_bow_draught));
		SET_METRICS(this->metrics, DM::SternDraught, DBD(DB2, fixed_stern_draught));
		SET_METRICS(this->metrics, DM::AverageDraught, DBD(DB2, average_draught));

		SET_METRICS(this->metrics, DM::PSFlow, DBD(DB2, ps_flow));
		SET_METRICS(this->metrics, DM::SBFlow, DBD(DB2, sb_flow));
		SET_METRICS(this->metrics, DM::PSConcentration, DBD(DB2, ps_concentration));
		SET_METRICS(this->metrics, DM::SBConcentration, DBD(DB2, sb_concentration));
		SET_METRICS(this->metrics, DM::PSProduct, DBD(DB2, ps_accumulated_product));
		SET_METRICS(this->metrics, DM::SBProduct, DBD(DB2, sb_accumulated_product));
		SET_METRICS(this->metrics, DM::MudDensity, DBD(DB2, mud_density));
		SET_METRICS(this->metrics, DM::AverageDensity, DBD(DB2, average_density));

		SET_METRICS(this->metrics, DM::EarthWork, DBD(DB2, earthwork_value));
		SET_METRICS(this->metrics, DM::Capacity, DBD(DB2, vessel_value));
		SET_METRICS(this->metrics, DM::Displacement, DBD(DB2, displacement_value));
		SET_METRICS(this->metrics, DM::Payload, DBD(DB2, payload_value));
	}

public:
	void on_sail(long long timepoint_ms, double s_kn, double track_deg, Syslog* logger) override {
		double angle = this->metrics[_I(DM::BowDirection)] - track_deg;

		this->memory->gps.set(GP::Speed, s_kn);
		this->memory->gps.set(GP::Track, track_deg);

		SET_METRICS(this->metrics, DM::Speed, s_kn);
		SET_METRICS(this->metrics, DM::TrackDirection, track_deg);
		SET_METRICS(this->metrics, DM::FlowPressureAngle, angle);
	}

	void on_heading(long long timepoint_ms, double deg, Syslog* logger) override {
		this->memory->gps.set(GP::BowDirection, deg);

		SET_METRICS(this->metrics, DM::BowDirection, deg);
	}

	void on_turn(long long timepoint_ms, double degpmin, Syslog* logger) override {
		this->memory->gps.set(GP::TurnRate, degpmin);

		SET_METRICS(this->metrics, DM::TurnRate, degpmin);
	}

public:
	double metrics[_N(DM)];

private: // never deletes these global objects
	ResidentMetrics* memory;
};

/*************************************************************************************************/
DredgeMetrics::DredgeMetrics(Compass* compass, MRMaster* plc) {
	this->provider = new DredgeMetrics::Provider();

	if (compass != nullptr) {
		compass->push_receiver(this->provider);
	}

	if (plc != nullptr) {
		plc->push_confirmation_receiver(this->provider);
	}
}

unsigned int DredgeMetrics::capacity() {
	return _N(DM);
}

Platform::String^ DredgeMetrics::label_ref(unsigned int idx) {
	return _speak(_E(DM, idx));
}

MetricValue DredgeMetrics::value_ref(unsigned int idx) {
	MetricValue mv;
	
	mv.type = MetricValueType::Flonum;
	mv.as.flonum = this->provider->metrics[idx];

	switch (_E(DM, idx)) {
	case DM::BowDirection: case DM::FlowPressureAngle: case DM::Speed: case DM::TrackDirection: case DM::TurnRate: mv.precision = 1; break;
	case DM::BowDraught: case DM::SternDraught: case DM::AverageDraught: case DM::PSConcentration: case DM::SBConcentration: mv.precision = 2; break;
	case DM::MudDensity: case DM::AverageDensity: mv.precision = 2; break;
	default: mv.precision = 0; break;
	}

	return mv;
}

CanvasSolidColorBrush^ DredgeMetrics::label_color_ref(unsigned int idx) {
	CanvasSolidColorBrush^ color = Colours::Foreground;

	switch (_E(DM, idx)) {
	case DM::BowDirection: case DM::TrackDirection: case DM::FlowPressureAngle: case DM::LineDirection: color = Colours::Yellow; break;
	case DM::Speed: case DM::TurnRate: color = Colours::Green; break;
	case DM::BowDraught: case DM::SternDraught: case DM::AverageDraught: color = Colours::Orange; break;
	case DM::PSFlow: case DM::SBFlow: case DM::PSConcentration: case DM::SBConcentration: case DM::AverageDensity: color = Colours::DodgerBlue; break;
	case DM::EarthWork: case DM::Capacity: case DM::Displacement: case DM::MudDensity: case DM::Payload: color = Colours::Salmon; break;
	case DM::PSProduct: case DM::SBProduct: color = Colours::Cyan; break;
	}

	return color;
}
