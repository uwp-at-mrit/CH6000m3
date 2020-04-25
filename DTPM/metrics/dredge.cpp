#include "metrics/dredge.hpp"

#include "datum/flonum.hpp"

#include "iotables/ai_metrics.hpp"

#include "metrics.hpp"
#include "module.hpp"
#include "brushes.hxx"

/*************************************************************************************************/
using namespace WarGrey::SCADA;
using namespace WarGrey::DTPM;
using namespace WarGrey::GYDM;

using namespace Microsoft::Graphics::Canvas::Brushes;

/*************************************************************************************************/
#define SET_METRICS(ms, id, v)          ms[_I(id)] = v

/*************************************************************************************************/
namespace {
	private enum class AI : unsigned int {
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
		SET_METRICS(this->metrics, AI::BowDraught, DBD(DB2, fixed_bow_draught));
		SET_METRICS(this->metrics, AI::SternDraught, DBD(DB2, fixed_stern_draught));
		SET_METRICS(this->metrics, AI::AverageDraught, DBD(DB2, average_draught));

		SET_METRICS(this->metrics, AI::PSFlow, DBD(DB2, ps_flow));
		SET_METRICS(this->metrics, AI::SBFlow, DBD(DB2, sb_flow));
		SET_METRICS(this->metrics, AI::PSConcentration, DBD(DB2, ps_concentration));
		SET_METRICS(this->metrics, AI::SBConcentration, DBD(DB2, sb_concentration));
		SET_METRICS(this->metrics, AI::PSProduct, DBD(DB2, ps_accumulated_product));
		SET_METRICS(this->metrics, AI::SBProduct, DBD(DB2, sb_accumulated_product));
		SET_METRICS(this->metrics, AI::MudDensity, DBD(DB2, mud_density));
		SET_METRICS(this->metrics, AI::AverageDensity, DBD(DB2, average_density));

		SET_METRICS(this->metrics, AI::EarthWork, DBD(DB2, earthwork_value));
		SET_METRICS(this->metrics, AI::Capacity, DBD(DB2, vessel_value));
		SET_METRICS(this->metrics, AI::Displacement, DBD(DB2, displacement_value));
		SET_METRICS(this->metrics, AI::Payload, DBD(DB2, payload_value));
	}

public:
	void on_sail(long long timepoint_ms, double s_kn, double track_deg, Syslog* logger) override {
		double angle = this->metrics[_I(AI::BowDirection)] - track_deg;

		this->memory->gps.set(GP::Speed, s_kn);
		this->memory->gps.set(GP::Track, track_deg);

		SET_METRICS(this->metrics, AI::Speed, s_kn);
		SET_METRICS(this->metrics, AI::TrackDirection, track_deg);
		SET_METRICS(this->metrics, AI::FlowPressureAngle, angle);
	}

	void on_heading(long long timepoint_ms, double deg, Syslog* logger) override {
		this->memory->gps.set(GP::BowDirection, deg);

		SET_METRICS(this->metrics, AI::BowDirection, deg);
	}

	void on_turn(long long timepoint_ms, double degpmin, Syslog* logger) override {
		this->memory->gps.set(GP::TurnRate, degpmin);

		SET_METRICS(this->metrics, AI::TurnRate, degpmin);
	}

public:
	double metrics[_N(AI)];

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
	return _N(AI);
}

Platform::String^ DredgeMetrics::label_ref(unsigned int idx) {
	return _speak(_E(AI, idx));
}

MetricValue DredgeMetrics::value_ref(unsigned int idx) {
	MetricValue mv;
	
	mv.type = MetricValueType::Flonum;
	mv.as.flonum = this->provider->metrics[idx];

	switch (_E(AI, idx)) {
	case AI::BowDirection: case AI::FlowPressureAngle: case AI::Speed: case AI::TrackDirection: case AI::TurnRate: mv.precision = 1; break;
	case AI::BowDraught: case AI::SternDraught: case AI::AverageDraught: case AI::PSConcentration: case AI::SBConcentration: mv.precision = 2; break;
	case AI::MudDensity: case AI::AverageDensity: mv.precision = 2; break;
	default: mv.precision = 0; break;
	}

	return mv;
}

CanvasSolidColorBrush^ DredgeMetrics::label_color_ref(unsigned int idx) {
	CanvasSolidColorBrush^ color = Colours::Foreground;

	switch (_E(AI, idx)) {
	case AI::BowDirection: case AI::TrackDirection: case AI::FlowPressureAngle: case AI::LineDirection: color = Colours::Yellow; break;
	case AI::Speed: case AI::TurnRate: color = Colours::Green; break;
	case AI::BowDraught: case AI::SternDraught: case AI::AverageDraught: color = Colours::Orange; break;
	case AI::PSFlow: case AI::SBFlow: case AI::PSConcentration: case AI::SBConcentration: case AI::AverageDensity: color = Colours::DodgerBlue; break;
	case AI::EarthWork: case AI::Capacity: case AI::Displacement: case AI::MudDensity: case AI::Payload: color = Colours::Salmon; break;
	case AI::PSProduct: case AI::SBProduct: color = Colours::Cyan; break;
	}

	return color;
}
